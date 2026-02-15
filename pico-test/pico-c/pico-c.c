#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

#define LED_PIN "LED"
#define BOOSTEL 23

#define INT_MAX 0x7fffffff
#define INT_MIN (-INT_MAX - 1)

#define DOT_MS        200
#define DASH_MS       (DOT_MS * 3)
#define GAP_MS        DOT_MS
#define LETTER_GAP_MS (DOT_MS * 3)
#define WORD_GAP_MS   (DOT_MS * 7)

typedef enum {
    MODE_BLINK,
    MODE_MANUAL,
    MODE_MORSE,
    MODE_TOGGLE,
    MODE_SOS
} mode_t;

static mode_t current_mode = MODE_BLINK;

static uint32_t interval_ms = 500;
static uint32_t last_toggle = 0;
static int led_state = 0;

int toggled = 0;

typedef struct {
    char ch;
    const char *code;
} morse_entry_t;

typedef struct {
    int led_on;
    uint32_t duration;
} morse_event_t;

static morse_event_t morse_events[256];
static uint16_t morse_len = 0;
static uint16_t morse_index = 0;
static uint32_t morse_start = 0;

static const morse_entry_t MORSE_TABLE[41] = {
    {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
    {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
    {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
    {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
    {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."},

    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."},

    {'.', ".-.-.-"}, {',', "--..--"}, {'?', "..--.."}, {'!', "-.-.--"},
    {'%', "------..-.-----"}
};

static inline uint32_t now_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

static inline char to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

static inline int atoi(const char *s) {
    int sign = 1;
    int res = 0;
    int idx = 0;

    while (s[idx] == ' ') {
        idx++;
    }

    if (s[idx] == '-' || s[idx] == '+') {
        if (s[idx] == '-') {
            sign = -1;
        }

        idx++;
    }

    while (s[idx] >= '0' && s[idx] <= '9') {
        int digit = s[idx] - '0';

        if (res > INT_MAX / 10 || (res == INT_MAX / 10 && digit > 7)) {
            return sign == 1 ? INT_MAX : INT_MIN;
        }

        res = 10 * res + digit;
        idx++;
    }
    
    return res * sign;
}

static const char *morse_for(char c) {
    for (uint8_t i = 0; i < ARR_LEN(MORSE_TABLE); ++i) {
        if (MORSE_TABLE[i].ch == c) {
            return MORSE_TABLE[i].code;
        }
    }
    return NULL;
}

static void build_morse(const char *text) {
    morse_len = 0;
    morse_index = 0;
    morse_start = now_ms();

    while (*text && morse_len < 256) {
        char c = to_upper((unsigned char)*text++);

        if (c == ' ') {
            morse_events[morse_len++] = (morse_event_t){0, WORD_GAP_MS};
            continue;
        }

        const char *code = morse_for(c);
        if (!code) continue;

        while (*code && morse_len < 256) {
            morse_events[morse_len++] = (morse_event_t){
                .led_on = 1,
                .duration = (*code == '.') ? DOT_MS : DASH_MS
            };

            morse_events[morse_len++] = (morse_event_t){
                .led_on = false,
                .duration = GAP_MS
            };

            code++;
        }

        morse_events[morse_len++] = (morse_event_t){
            .led_on = false,
            .duration = LETTER_GAP_MS
        };
    }
}

static void update_morse(uint32_t now) {
    if (morse_index >= morse_len) {
        gpio_put(LED_PIN, 0);
        return;
    }

    morse_event_t *ev = &morse_events[morse_index];

    if (now - morse_start >= ev->duration) {
        morse_start = now;
        morse_index++;
    } else {
        gpio_put(LED_PIN, ev->led_on);
    }
}

static void handle_command(char *cmd) {
    char *tok = strtok(cmd, " \r\n");
    if (!tok) return;

    if (!strcmp(tok, "MODE")) {
        tok = strtok(NULL, " \r\n");
        if (!tok) return;

        gpio_put(LED_PIN, 0);

        if (strcmp(tok, "BLINK") == 0) {
            current_mode = MODE_BLINK;
        } else if (strcmp(tok, "MANUAL") == 0) {
            current_mode = MODE_MANUAL;
        } else if (strcmp(tok, "MORSE") == 0) {
            current_mode = MODE_MORSE;
        } else if (strcmp(tok, "TOGGLE") == 0) {
            current_mode = MODE_TOGGLE;
        } else if (strcmp(tok, "SOS") == 0) {
            current_mode = MODE_SOS;
            build_morse("SOS");
        }

        printf("OK MODE\n");
    } else if (strcmp(tok, "SPEED") == 0) {
        tok = strtok(NULL, " \r\n");
        if (!tok) return;
        interval_ms = atoi(tok);
        printf("OK SPEED %lu\n", interval_ms);
    } else if (strcmp(tok, "TEXT") == 0) {
        tok = strtok(NULL, "\r\n");
        if (!tok) return;

        build_morse(tok);
        printf("OK TEXT\n");
    } else if (strcmp(tok, "STATUS") == 0) {
        printf("MODE %d\n", current_mode);
    } else {
        printf("ERR\n");
    }
}

static inline void update_blink(uint32_t now) {
    if (now - last_toggle >= interval_ms) {
        last_toggle = now;
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);
    }
}

static inline void update_manual(void) {
    gpio_put(LED_PIN, gpio_get(BOOSTEL));
}

static inline void update_toggle(void) {
    gpio_put(LED_PIN, !gpio_get(LED_PIN));
}

static inline void update_sos(uint32_t now) {
    update_morse(now);
}

int main(void) {
    stdio_uart_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("READY\n");

    char cmd_buf[128];

    while (1) {
        uint32_t now = now_ms();

        if (fgets(cmd_buf, sizeof(cmd_buf), stdin)) {            
            handle_command(cmd_buf);
        }

        switch (current_mode) {
            case MODE_BLINK:
                update_blink(now);
                break;
            case MODE_MANUAL:
                update_manual();
                break;
            case MODE_MORSE:
                update_morse(now);
                break;
            case MODE_TOGGLE:
                update_toggle();
                break;
            case MODE_SOS:
                update_sos(now);
                break;
        }

        tight_loop_contents();
    }
}
