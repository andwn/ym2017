#include "common.h"
#include "resources.h"
#include "vistab.h"
#include "songtab.h"

#define TIME(m,s,f) ((60*60*(m))+(60*(s))+(f))

const song_info_def song_info[SONG_COUNT] = {
	{ 
		.xgm_data 	= BGM_PalmORama,
		.xgm_length = TIME(3, 3,20),
		.vis_data	= PalmORama,
		.vis_length = PalmORama_Size,
		.loopstart  = TIME(0, 4, 0),
		.loopend    = TIME(1,38, 0),
		.title 		= "Palm-O-Rama",
		.artist 	= "Luke McQueen",
	},{
		.xgm_data 	= BGM_Deliverance,
		.xgm_length = TIME(2,13, 0),
		.vis_data	= Deliverance,
		.vis_length = Deliverance_Size,
		.title 		= "Deliverance",
		.artist 	= "nooly",
	},{
		.xgm_data 	= BGM_AstroBlast,
		.xgm_length = TIME(2,11, 0),
		.vis_data	= AstroBlast,
		.vis_length = AstroBlast_Size,
		.title 		= "Astro Blaster",
		.artist 	= "Jredd",
	},{ 
		.xgm_data 	= BGM_LatinFunk,
		.xgm_length = TIME(3,25, 0),
		.vis_data	= LatinFunk,
		.vis_length = LatinFunk_Size,
		.title 		= "Funky Latin Party",
		.artist 	= "Cosmo Buggi",
	},{ 
		.xgm_data 	= BGM_SolarMod,
		.xgm_length = TIME(4,59, 0),
		.vis_data	= SolarMod,
		.vis_length = SolarMod_Size,
		.title 		= "Solar Modulation",
		.artist 	= "Savaged Regime",
	},{ 
		.xgm_data 	= BGM_SunsetBeach,
		.xgm_length = TIME(2,54,20),
		.vis_data	= SunsetBeach,
		.vis_length = SunsetBeach_Size,
		.loopstart  = TIME(0, 7, 0),
		.loopend    = TIME(2, 9, 0),
		.title 		= "Sunset Beach Galaxy",
		.artist 	= "Tobikomi",
	},{ 
		.xgm_data 	= BGM_LetGo,
		.xgm_length = TIME(3, 8, 0),
		.vis_data	= LetGo,
		.vis_length = LetGo_Size,
		.title 		= "A Night to Let Go",
		.artist 	= "Uctumi",
	},{ 
		.xgm_data 	= BGM_Hostage,
		.xgm_length = TIME(2,49, 0),
		.vis_data	= Hostage,
		.vis_length = Hostage_Size,
		.title 		= "Hostage Extraction",
		.artist 	= "Dya",
	},{
		.xgm_data 	= BGM_FestZone,
		.xgm_length = TIME(6, 1, 0),
		.vis_data	= FestZone,
		.vis_length = FestZone_Size,
		.title 		= "Festicity Zone",
		.artist 	= "Laz",
	},
};
