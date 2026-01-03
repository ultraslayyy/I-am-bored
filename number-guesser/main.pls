DECLARE
    v_num   NUMBER;
    v_guess NUMBER := &user_input;
BEGIN
    v_num := ROUND(DBMS_RANDOM.VALUE(0, 100));

    IF v_guess > v_num THEN
        DBMS_OUTPUT.PUT_LINE('Lower');
    ELSIF v_guess < v_num THEN
        DBMS_OUTPUT.PUT_LINE('Higher');
    ELSE
        DBMS_OUTPUT.PUT_LINE('You got it! The number was ' || v_num);
    END IF;
END