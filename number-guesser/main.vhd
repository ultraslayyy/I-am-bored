library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity guess_game is
    Port (
        clk     : in  std_logic;
        reset   : in  std_logic;
        guess   : in  unsigned(6 downto 0);

        higher  : out std_logic;
        lower   : out std_logic;
        correct : out std_logic
    );
end guess_game;

architecture Behavioral of guess_game is

    signal lfsr   : unsigned(6 downto 0) := "1010101"; -- seed
    signal target : unsigned(6 downto 0);

begin

    lfsr_proc : process(clk)
        variable feedback : std_logic;
    begin
        if rising_edge(clk) then
            feedback := lfsr(6) xor lfsr(5);

            lfsr <= lfsr(5 downto 0) & feedback;
        end if;
    end process;

    signal new_target : std_logic;

    target_proc : process(clk)
    begin
        if rising_edge(clk) then
            if new_target = '1' then
                if lfsr <= to_unsigned(100, 7) then
                    target <= lfsr;
                end if;
            end if;
        end if;
    end process;

    compare_proc : process(guess, target)
    begin
        if guess > target then
            higher  <= '0';
            lower   <= '1';
            correct <= '0';

        elsif guess < target then
            higher  <= '1';
            lower   <= '0';
            correct <= '0';

        else
            higher  <= '0';
            lower   <= '0';
            correct <= '1';
        end if;
    end process;

end Behavioral;