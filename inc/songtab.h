#define SONG_COUNT 9

typedef struct {
	const uint8_t *xgm_data;
	uint32_t xgm_length;
	const vistab *vis_data;
	uint16_t vis_length;
	uint32_t loopstart;
	uint32_t loopend;
	char title[38];
	char artist[38];
	//char bio[256];
} song_info_def;

extern const song_info_def song_info[SONG_COUNT];
