#define MAX_VDP_SPRITE          80

#define SPRITE_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

uint8_t spr_num;
VDPSprite sprites[MAX_VDP_SPRITE];

// Append sprite to the end of the list, order front -> back
// Only adds if the sprite's coords are on screen
#define sprite_add(s) {                                                                        \
	if(spr_num < MAX_VDP_SPRITE && (unsigned)(s.x-96) < 352 && (unsigned)(s.y-96) < 256) {     \
		sprites[spr_num] = s;                                                                  \
		sprites[spr_num].link = spr_num+1;                                                     \
		spr_num++;                                                                             \
	}                                                                                          \
}

// Perform sprite_add against an array of VDPSprites
#define sprite_addq(arr, num) {                                                                \
	for(uint8_t i = num; i--; ) sprite_add(arr[i]);                                            \
}

#define sprite_pos(s, px, py) {                                                                \
	(s).x = (px) + 128;                                                                        \
	(s).y = (py) + 128;                                                                        \
}

#define sprite_pri(s, pri)   { (s).attribut &= ~(1<<15); (s).attribut |= ((pri)&1) << 15; }
#define sprite_pal(s, pal)   { (s).attribut &= ~(3<<13); (s).attribut |= ((pal)&3) << 13; }
#define sprite_vflip(s, flp) { (s).attribut &= ~(1<<12); (s).attribut |= ((flp)&1) << 12; }
#define sprite_hflip(s, flp) { (s).attribut &= ~(1<<11); (s).attribut |= ((flp)&1) << 11; }
#define sprite_index(s, ind) { (s).attribut &= ~0x7FF;   (s).attribut |= (ind)&0x7FF; }

// Send sprite list to VDP
#define sprites_send() {                                                                       \
	if(spr_num) {                                                                              \
		sprites[spr_num-1].link = 0;                                                           \
		DMA_doDma(DMA_VRAM, (uint32_t)sprites, VDP_SPRITE_TABLE, spr_num<<2, 2);               \
		spr_num = 0;                                                                           \
	}                                                                                          \
}

#define sprites_clear() {                                                                      \
	spr_num = 0;                                                                               \
	VDPSprite s = (VDPSprite) { .x=128, .y=128, .size=0 };                                     \
	sprite_add(s);                                                                             \
	sprites_send();                                                                            \
}
