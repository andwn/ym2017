enum { LM_NOTHING, LM_SCROLL, LM_COLUMNS, LM_MARQUEE_TITLE, LM_MARQUEE_CREDITS };
extern uint8_t ledmode;
extern uint16_t ledtimer;
extern uint8_t led[8];

void update_leds();
