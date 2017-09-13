#include "common.h"
#include "vdp.h"
#include "led.h"

const uint8_t marquee_ym2017[8*4] = {
	0b10101011,
	0b10101001,
	0b11011010,
	0b11011010,
	0b11011011,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b10110001,
	0b00111101,
	0b10110001,
	0b10110111,
	0b10110001,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b10110100,
	0b01010111,
	0b01010110,
	0b01010110,
	0b10110110,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b01111111,
	0b01111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

const uint8_t marquee_thank[8*5] = {
	0b10001010,
	0b11011010,
	0b11011000,
	0b11011010,
	0b11011010,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b11011011,
	0b10101001,
	0b10001010,
	0b10101011,
	0b10101011,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b01010111,
	0b01010111,
	0b01001111,
	0b01010111,
	0b01010111,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b10101101,
	0b10101010,
	0b11011010,
	0b11011010,
	0b11011101,
	0b11111111,
	0b11111111,
	0b11111111,
	
	0b10101111,
	0b10101111,
	0b10101111,
	0b10101111,
	0b11011111,
	0b11111111,
	0b11111111,
	0b11111111,
};

// LED status
uint8_t ledmode = LM_MARQUEE_TITLE;
uint8_t marquee_tile = 0;
uint8_t marquee_pixel = 0;
uint16_t ledtimer = 0;
uint8_t led[8] = {
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

void update_leds() {
	static volatile uint8_t* const sram_ctrl = (uint8_t*) 0xA130F1;
	*sram_ctrl = 1; // Enable SRAM R/W
	
	for(uint16_t i = 0; i < 8; i++) {
		volatile uint8_t *sram_data = (uint8_t*) (0x200000 + (1 << (i+1)));
		for(uint16_t j = 0; j < (IS_PALSYSTEM ? 6 : 5); j++) {
			*sram_data = led[i];
			*sram_data = led[i];
			*sram_data = led[i];
			*sram_data = led[i];
			*sram_data = led[i];
		}
	}
	
	*sram_ctrl = 0; // Disable SRAM R/W
	
	switch(ledmode) {
		case LM_SCROLL: {
			if(++ledtimer > 30) {
				ledtimer = 0;
				for(uint16_t i = 0; i < 8; i++) {
					// m68k can't ROR a single byte of memory, so...
					led[i] = (led[i] << 7) | (led[i] >> 1);
				}
			}
		}
		break;
		case LM_COLUMNS: {
			if(++ledtimer > (IS_PALSYSTEM ? 7 : 8)) {
				ledtimer = 0;
				for(uint16_t row = 7; row > 0; row--) {
					led[row] = led[row - 1];
				}
				led[0] = 0xFF;
			}
		}
		break;
		case LM_MARQUEE_TITLE: {
			if(++ledtimer > 30) {
				ledtimer = 0;
				
				for(uint16_t i = 0; i < 8; i++) {
					led[i] >>= 1;
					uint8_t newval = marquee_ym2017[marquee_tile*8 + i];
					newval &= 0x80 >> marquee_pixel;
					led[i] |= newval ? 0x80 : 0;
				}
				if(++marquee_pixel > 7) {
					marquee_pixel = 0;
					if(++marquee_tile > 3) marquee_tile = 0;
				}
			}
		}
		break;
		case LM_MARQUEE_CREDITS: {
			if(++ledtimer > 30) {
				ledtimer = 0;
				
				for(uint16_t i = 0; i < 8; i++) {
					led[i] >>= 1;
					uint8_t newval = marquee_thank[marquee_tile*8 + i];
					newval &= 0x80 >> marquee_pixel;
					led[i] |= newval ? 0x80 : 0;
				}
				if(++marquee_pixel > 7) {
					marquee_pixel = 0;
					if(++marquee_tile > 4) marquee_tile = 0;
				}
			}
		}	
		break;
		default: break;
	}
}

