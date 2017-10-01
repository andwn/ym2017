#include "common.h"

#include "dma.h"
#include "joy.h"
#include "led.h"
#include "resources.h"
#include "sprite.h"
#include "system.h"
#include "timer.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "xgm.h"
#include "z80_ctrl.h"

#include "vistab.h"
#include "songtab.h"

// Input handling
uint16_t joystate, oldstate;
#define joy_pressed(btn) ((joystate&(btn)) && !(oldstate&(btn)))

// Track information
uint8_t track = 0;

// Play mode
enum { PM_ONESONG, PM_ORDER, PM_SHUFFLE };
uint8_t playMode = PM_ONESONG;
const char playModeStr[3][10] = { "One Song", "In Order", "Shuffle " };

const char creditsStr[8] = "Credits";
const char playModeStr2[10] = "Play Mode";


uint16_t bar_pixel;
uint16_t bar_lastpixel;
uint32_t bar_tiles[32][8];
uint16_t bar_map[32];

const uint32_t vis_tiles[16][8];
uint16_t vis_map[8][8][2];


void move_cursor(uint8_t old, uint8_t new) {
	VDP_setTextPalette(PAL1);
	if(old == SONG_COUNT + 1) {
		VDP_drawText(playModeStr2, 4, 4 + old);
		VDP_drawText(playModeStr[playMode], 16, 4 + old);
	} else if(old == SONG_COUNT + 3) {
		VDP_drawText(creditsStr, 4, 4 + old);
	} else {
		VDP_drawText(song_info[old].title, 4, 4 + old);
	}
	VDP_setTextPalette(PAL0);
	if(new == SONG_COUNT + 1) {
		VDP_drawText(playModeStr2, 4, 4 + new);
		VDP_drawText(playModeStr[playMode], 16, 4 + new);
	} else if(new == SONG_COUNT + 3) {
		VDP_drawText(creditsStr, 4, 4 + new);
	} else {
		VDP_drawText(song_info[new].title, 4, 4 + new);
	}
}

/***********************************************************************************************
 * Vertical blank
 **********************************************************************************************/

void vsync() {
	vblank = 0;
	while(!vblank);
}

void aftervsync() {
	// Update BGM
	XGM_doVBlankProcess();
	// Flush DMA queue
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	update_leds();
	DMA_flushQueue();
	XGM_set68KBUSProtection(FALSE);
	// Handle async fade
	if(fading_cnt > 0) VDP_doStepFading(FALSE);
	// Refresh sprite list and read controller input
	sprites_send();
	JOY_update();
}

/***********************************************************************************************
 * Title Screen / Splash Display
 **********************************************************************************************/

void gm_title() {
	VDP_setEnable(FALSE);
	
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_drawText("Press Start", 14, 22);
	// Logo
	VDP_loadTileSet(&TS_Logo, TILE_USERINDEX, TRUE);
	VDP_fillTileMapRectInc(PLAN_B, TILE_USERINDEX, 0, 0, 40, 28);
	//VDP_loadTileData((uint32_t*) vis_tiles, TILE_FONTINDEX-16, 16, TRUE);
	VDP_setEnable(TRUE);
	
	joystate = oldstate = ~0;
	
	while(TRUE) {
		oldstate = joystate;
		joystate = JOY_readJoypad(JOY_1);
		/*
		// This was to test the marquee by printing the bars on the title
		// cause I didn't have a cart to test at the time
		if(ledtimer == 0) {
			for(uint16_t i = 0; i < 8; i++) {
				for(uint16_t j = 0; j < 8; j++) {
					if(led[i] & (1<<j)) {
						vis_map[i][j][0] = 0;
						vis_map[i][j][1] = 0;
					} else {
						vis_map[i][j][0] = TILE_FONTINDEX-16+(i<<1);
						vis_map[i][j][1] = TILE_FONTINDEX-15+(i<<1);
					}
				}
				DMA_queueDma(DMA_VRAM, (uint32_t) vis_map[i],
						VDP_PLAN_A + (64*(14+i)*2) + (20*2), 16, 2);
			}
		*/
		
		if(joy_pressed(BUTTON_START)) return;
		
		vsync(); aftervsync();
	}
}

/***********************************************************************************************
 * Track List
 **********************************************************************************************/

void gm_list() {
	ledmode = LM_COLUMNS;
	VDP_setEnable(FALSE);
	
	sprites_clear();
	VDP_clearPlan(PLAN_A, TRUE);
	// Background
	VDP_loadTileSet(&TS_List, TILE_USERINDEX, TRUE);
	VDP_fillTileMapRectInc(PLAN_B, TILE_ATTR_FULL(PAL1,0,0,0,TILE_USERINDEX), 0, 0, 40, 28);
	VDP_setHorizontalScroll(PLAN_B, 0);
	VDP_setVerticalScroll(PLAN_B, 0);
	
	// Draw track list
	for(uint8_t i = 0; i < SONG_COUNT; i++) {
		VDP_setTextPalette((i == track) ? PAL0 : PAL1);
		VDP_drawText(song_info[i].title, 4, 4 + i);
	}
	VDP_drawText(playModeStr2, 4, 4 + SONG_COUNT + 1);
	VDP_drawText(playModeStr[playMode], 16, 4 + SONG_COUNT + 1);
	VDP_drawText(creditsStr, 4, 4 + SONG_COUNT + 3);
	VDP_setTextPalette(PAL0);
	
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_List.data);
	VDP_setPaletteColor(16+15, 0xAAA); // Gray out unselected tracks
	
	VDP_setEnable(TRUE);
	
	joystate = oldstate = ~0;
	
	while(TRUE) {
		oldstate = joystate;
		joystate = JOY_readJoypad(JOY_1);
		// Switch between tracks
		if(joy_pressed(BUTTON_UP)) {
			uint8_t old = track;
			if(track == 0) {
				track = SONG_COUNT + 3;
			} else {
				if(track < SONG_COUNT) track--;
				else track -= 2;
			}
			move_cursor(old, track);
		} else if(joy_pressed(BUTTON_DOWN)) {
			uint8_t old = track;
			if(track == SONG_COUNT + 3) {
				track = 0;
			} else {
				if(track < SONG_COUNT - 1) track++;
				else track += 2;
			}
			move_cursor(old, track);
		}
		
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_C)) {
			if(track == SONG_COUNT+1) {
				// Play Mode
				if(++playMode > PM_SHUFFLE) playMode = PM_ONESONG;
				VDP_drawText(playModeStr[playMode], 16, 4 + SONG_COUNT + 1);
			} else if(track == SONG_COUNT+3) {
				// Credits
				return;
			} else {
				return;
			}
		}
		
		vsync(); aftervsync();
	}
}

/***********************************************************************************************
 * Credits
 **********************************************************************************************/

void gm_credits() {
	uint16_t scroll = 0;
	
	VDP_setEnable(FALSE);
	
	sprites_clear();
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_loadTileSet(&TS_Credits, TILE_USERINDEX, TRUE);
	VDP_loadTileSet(&TS_CSLogo, TILE_USERINDEX + TS_Credits.numTile, TRUE);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Credits.data);
	VDP_setPaletteColor(47, 0x000); // Color 15 of PAL2, for black text
	VDP_setPalette(PAL3, PAL_CSLogo.data);
	ledmode = LM_MARQUEE_CREDITS;
	
	// Draw background
	for(uint16_t x = 0; x < 4; x++) {
		for(uint16_t y = 0; y < 2; y++) {
			for(uint16_t row = 0; row < 16; row++) {
				uint16_t ind = TILE_USERINDEX + (row*16);
				VDP_fillTileMapRectInc(
						PLAN_B, TILE_ATTR_FULL(PAL1,0,0,0,ind),
						x*16, row + y*16, 16, 1);
			}
		}
	}
	
	// Draw Catskull logo
	VDP_fillTileMapRectInc(
			PLAN_A, TILE_ATTR_FULL(PAL3,0,0,0,TILE_USERINDEX + TS_Credits.numTile),
			4, 18, 32, 6);
	
	VDP_setTextPalette(PAL2);
	uint16_t x = 6, y = 2;
	VDP_drawText("YM2017", 						x+11,	y); y+=2;
	VDP_drawText("Hardware              Jazz",	x,		y); y+=2;
	VDP_drawText("Software             Grind",	x,		y); y+=2;
	VDP_drawText("Logo                  Keff",	x,		y); y+=2;
	VDP_drawText("Backgrounds             Ui",	x,		y); y+=2;
	VDP_drawText("Producer          Catskull",	x,		y); y+=3;
	VDP_drawText("Thanks for listening!", 		x+2,	y); y+=10;
	VDP_drawText("catskullelectronics.com",     x+2,    y);
	VDP_setTextPalette(PAL0);
	
	VDP_setEnable(TRUE);
	
	XGM_startPlay(BGM_Fireworks);
	
	joystate = oldstate = ~0;
	while(TRUE) {
		oldstate = joystate;
		joystate = JOY_readJoypad(JOY_1);
		
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_B)) {
			// XGM sometimes doesn't stop properly, and fast forwards through the
			// rest of the song. It sounds awful and this is the only workaround I know
			Z80_init();
			return;
		}
		
		scroll++;
		VDP_setHorizontalScroll(PLAN_B, -scroll);
		VDP_setVerticalScroll(PLAN_B, scroll);
		vsync(); aftervsync();
	}
}

/***********************************************************************************************
 * Now Playing
 **********************************************************************************************/

uint16_t bar_pixel;
uint16_t bar_lastpixel;
uint32_t bar_tiles[32][8];
uint16_t bar_map[32];

const uint32_t vis_tiles[16][8];
uint16_t vis_map[8][8][2];

void draw_track_info(uint8_t track_num, uint8_t clear) {
	if(clear) {
		VDP_setEnable(FALSE);
		VDP_clearPlan(PLAN_A, TRUE);
		VDP_setEnable(TRUE);
	}
	// Track info
	VDP_drawText("Now Playing", 4, 4);
	VDP_drawText("by", 6, 8);
	VDP_drawText(song_info[track_num].title,  5, 6);
	VDP_drawText(song_info[track_num].artist, 9, 8);
	
	VDP_drawText("Start / B - Back to Menu", 8, 25);
	
	// Draw total time
	uint16_t seconds = (song_info[track_num].xgm_length / 60) % 60;
	uint16_t minutes = (song_info[track_num].xgm_length / 60) / 60;
	
	char str[8] = "/ ";
	str[2] = 0x30 + minutes / 10;
	str[3] = 0x30 + minutes % 10;
	str[4] = ':';
	str[5] = 0x30 + seconds / 10;
	str[6] = 0x30 + seconds % 10;
	str[7] = '\0';
	VDP_drawText(str, 10, 21);
	
	// Setup the bar
	bar_pixel = song_info[track_num].xgm_length >> 8;
	bar_lastpixel = 0;
	
	for(uint16_t i = 0; i < 32; i++) {
		bar_tiles[i][0] = 0xFFFFFFFF;
		bar_tiles[i][1] = 0x77777777;
		bar_tiles[i][2] = 0x77777777;
		bar_tiles[i][3] = 0x77777777;
		bar_tiles[i][4] = 0x77777777;
		bar_tiles[i][5] = 0x77777777;
		bar_tiles[i][6] = 0x77777777;
		bar_tiles[i][7] = 0xFFFFFFFF;
		bar_map[i] = TILE_EXTRA2INDEX + i;
	}
	DMA_queueDma(DMA_VRAM, (uint32_t) bar_map, VDP_PLAN_A + (64*23*2) + (4*2), 32, 2);
	DMA_queueDma(DMA_VRAM, (uint32_t) bar_tiles, TILE_EXTRA2INDEX*32, 32*16, 2);
}

void draw_next_track(uint8_t track_num) {
	if(playMode != PM_ONESONG) {
		VDP_drawText("Up Next", 4, 10);
		VDP_drawText(song_info[track_num].title, 5, 12);
	}
}

uint32_t get_current_frame(uint8_t t, uint16_t visindex) {
	const uint8_t *time = song_info[t].vis_data[visindex].time;
	return ((time[0] << 16) | (time[1] << 8) | time[2]);
}

void gm_playing() {
	if(track > SONG_COUNT) {
		gm_credits();
		return;
	}
	
	uint16_t visindex = 0;
	uint16_t framerate = IS_PALSYSTEM ? 50 : 60;
	uint16_t frametime = 735; //= IS_PALSYSTEM ? 882 : 735;
	uint8_t timer = 0;
	uint8_t shuffleOrder[SONG_COUNT] = {};
	
	if(playMode == PM_SHUFFLE) {
		uint8_t setTrack = FALSE;
		// Fill shuffle array
		for(uint16_t i = 0; i < SONG_COUNT; i++) {
			uint8_t tryAgain = TRUE;
			while(tryAgain) {
				tryAgain = FALSE;
				shuffleOrder[i] = random() % SONG_COUNT;
				for(uint16_t j = 0; j < i; j++) {
					if(shuffleOrder[j] == shuffleOrder[i]) {
						tryAgain = TRUE;
						break;
					}
				}
			}
			// Set the track number to the index on the shuffle array corresponding to the
			// actual track number, otherwise we will play the wrong song
			if(!setTrack && shuffleOrder[i] == track) {
				track = i;
				setTrack = TRUE;
			}
		}
	}
	
	VDP_setEnable(FALSE);
	
	sprites_clear();
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_loadTileSet(&TS_Playing, TILE_USERINDEX, TRUE);
	VDP_loadTileData((uint32_t*) vis_tiles, TILE_FONTINDEX-16, 16, TRUE);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Playing.data);
	
	// Let's change the bar colors
	VDP_setPaletteColor(8, 0xE62);
	VDP_setPaletteColor(7, 0xC44);
	VDP_setPaletteColor(6, 0xA24);
	VDP_setPaletteColor(5, 0x826);
	VDP_setPaletteColor(4, 0x628);
	VDP_setPaletteColor(3, 0x42A);
	VDP_setPaletteColor(2, 0x22C);
	VDP_setPaletteColor(1, 0x02E);
	
	VDP_setEnable(TRUE);
	
	joystate = oldstate = ~0;
	if(playMode == PM_SHUFFLE) {
		draw_track_info(shuffleOrder[track], FALSE);
		draw_next_track(shuffleOrder[(track+1) % SONG_COUNT]);
		XGM_startPlay(song_info[shuffleOrder[track]].xgm_data);
	} else {
		draw_track_info(track, FALSE);
		draw_next_track((track+1) % SONG_COUNT);
		XGM_startPlay(song_info[track].xgm_data);
	}
	
	uint8_t loop_count = 0;
	
    while(TRUE) {
		oldstate = joystate;
		joystate = JOY_readJoypad(JOY_1);
		
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_B)) {
			// XGM sometimes doesn't stop properly, and fast forwards through the
			// rest of the song. It sounds awful and this is the only workaround I know
			Z80_init();
			return;
		}
		
		uint32_t elapsed = XGM_getElapsed();
		uint8_t t = playMode == PM_SHUFFLE ? shuffleOrder[track] : track;
		
		// Update LED visualization
		uint32_t elapsed_fixed = elapsed;
		uint8_t loop_count_new = 0;
		while(song_info[t].loopend && elapsed_fixed >= song_info[t].loopend) {
			elapsed_fixed -= song_info[t].loopend - song_info[t].loopstart;
			loop_count_new++;
		}
		uint32_t current_frame = get_current_frame(t, visindex);
		if(loop_count_new > loop_count) {
			loop_count = loop_count_new;
			visindex = 0;
			current_frame = get_current_frame(t, visindex);
		}
		while(visindex < song_info[t].vis_length && current_frame <= elapsed_fixed * frametime) {
			uint16_t channel = song_info[t].vis_data[visindex].channel;
			for(uint16_t i = 0; i < 8; i++) {
				led[i] &= ~(1 << channel);
			}
			visindex++;
			// Get the frame of the next action
			current_frame = get_current_frame(t, visindex);
		}
		
		// Draw timer
		if((timer & 15) == 0) {
			uint16_t seconds = elapsed / 60;
			uint16_t minutes = seconds / 60;
			seconds %= 60;
			
			char str[6];
			str[0] = 0x30 + minutes / 10;
			str[1] = 0x30 + minutes % 10;
			str[2] = ':';
			str[3] = 0x30 + seconds / 10;
			str[4] = 0x30 + seconds % 10;
			str[5] = '\0';
			VDP_drawText(str, 4, 21);
		}
		// Draw bar
		if((timer & 15) == 1) {
			uint16_t from = bar_lastpixel;
			uint16_t to = ((elapsed * 60 / 60) << 8) / song_info[t].xgm_length;
			if(to > from) {
				bar_lastpixel = to;
				for(uint16_t i = from; i < to; i++) {
					uint16_t col = 28 - ((i & 7) << 2);
					for(uint16_t row = 1; row < 7; row++) {
						bar_tiles[i/8][row] |= 0xF << col;
					}
				}
				uint16_t tile = from / 8;
				int16_t len = (to - from) / 8 + 1;
				if(len < 1) len = 1;
				if(len > 32) len = 32;
				DMA_queueDma(DMA_VRAM, (uint32_t) bar_tiles[tile], 
						(TILE_EXTRA2INDEX + tile)*32, len*16, 2);
			}
		}
		if(++timer > framerate) timer = 0;
		// Draw visualization
		if(ledtimer == 0) {
			for(uint16_t i = 0; i < 8; i++) {
				for(uint16_t j = 0; j < 8; j++) {
					if(led[i] & (1<<j)) {
						vis_map[i][j][0] = 0;
						vis_map[i][j][1] = 0;
					} else {
						vis_map[i][j][0] = TILE_FONTINDEX-16+(i<<1);
						vis_map[i][j][1] = TILE_FONTINDEX-15+(i<<1);
					}
				}
				DMA_queueDma(DMA_VRAM, (uint32_t) vis_map[i],
						VDP_PLAN_A + (64*(14+i)*2) + (20*2), 16, 2);
			}
		}
		
		// Change song / exit if we reached the end
		if(elapsed > song_info[t].xgm_length) {
			Z80_init();
			// Wait a moment
			uint16_t moment = 0;
			while(++moment < 30) {
				oldstate = joystate;
				joystate = JOY_readJoypad(JOY_1);
				if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_B)) return;
				vsync(); aftervsync();
			}
			switch(playMode) {
				case PM_ONESONG:
				{
					//XGM_stopPlay();
					return;
				}
				case PM_ORDER:
				{
					if(++track >= SONG_COUNT) track = 0;
					t = track;
					draw_track_info(t, TRUE);
					draw_next_track((track+1) % SONG_COUNT);
					//Z80_init();
					XGM_startPlay(song_info[t].xgm_data);
					visindex = 0;
					break;
				}
				case PM_SHUFFLE:
				{
					if(++track >= SONG_COUNT) track = 0;
					t = shuffleOrder[track];
					draw_track_info(t, TRUE);
					draw_next_track(shuffleOrder[(track+1) % SONG_COUNT]);
					//Z80_init();
					XGM_startPlay(song_info[t].xgm_data);
					visindex = 0;
					break;
				}
			}
		}
		
		vsync(); aftervsync();
    }
}

// Throwing this at the end cause it's long and in the way

const uint32_t vis_tiles[16][8] = {{
	0x00000000,
	0x01111111,
	0x01111111,
	0x01111111,
	0x01111111,
	0x01111111,
	0x01111111,
	0x00000000,
}, {
	0x00000000,
	0x11111110,
	0x11111110,
	0x11111110,
	0x11111110,
	0x11111110,
	0x11111110,
	0x00000000,
}, {
	0x00000000,
	0x02222222,
	0x02222222,
	0x02222222,
	0x02222222,
	0x02222222,
	0x02222222,
	0x00000000,
}, {
	0x00000000,
	0x22222220,
	0x22222220,
	0x22222220,
	0x22222220,
	0x22222220,
	0x22222220,
	0x00000000,
}, {
	0x00000000,
	0x03333333,
	0x03333333,
	0x03333333,
	0x03333333,
	0x03333333,
	0x03333333,
	0x00000000,
}, {
	0x00000000,
	0x33333330,
	0x33333330,
	0x33333330,
	0x33333330,
	0x33333330,
	0x33333330,
	0x00000000,
}, {
	0x00000000,
	0x04444444,
	0x04444444,
	0x04444444,
	0x04444444,
	0x04444444,
	0x04444444,
	0x00000000,
}, {
	0x00000000,
	0x44444440,
	0x44444440,
	0x44444440,
	0x44444440,
	0x44444440,
	0x44444440,
	0x00000000,
}, {
	0x00000000,
	0x05555555,
	0x05555555,
	0x05555555,
	0x05555555,
	0x05555555,
	0x05555555,
	0x00000000,
}, {
	0x00000000,
	0x55555550,
	0x55555550,
	0x55555550,
	0x55555550,
	0x55555550,
	0x55555550,
	0x00000000,
}, {
	0x00000000,
	0x06666666,
	0x06666666,
	0x06666666,
	0x06666666,
	0x06666666,
	0x06666666,
	0x00000000,
}, {
	0x00000000,
	0x66666660,
	0x66666660,
	0x66666660,
	0x66666660,
	0x66666660,
	0x66666660,
	0x00000000,
}, {
	0x00000000,
	0x07777777,
	0x07777777,
	0x07777777,
	0x07777777,
	0x07777777,
	0x07777777,
	0x00000000,
}, {
	0x00000000,
	0x77777770,
	0x77777770,
	0x77777770,
	0x77777770,
	0x77777770,
	0x77777770,
	0x00000000,
}, {
	0x00000000,
	0x08888888,
	0x08888888,
	0x08888888,
	0x08888888,
	0x08888888,
	0x08888888,
	0x00000000,
}, {
	0x00000000,
	0x88888880,
	0x88888880,
	0x88888880,
	0x88888880,
	0x88888880,
	0x88888880,
	0x00000000,
}};
