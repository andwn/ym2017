# YM2017

The software which runs on the YM2017 music cart for MegaDrive.

More info: https://catskull.net/ym2017

### Building

Currently only GNU/Linux systems are supported for building. 
Feel free to contribute a Windows build/Makefile, but I don't have the means right now.

1. Set up [marsdev](https://github.com/andwn/marsdev)
2. Just clone & make!
    
    git clone https://github.com/andwn/ym2017
    cd ym2017
    make


If you have a YM2017 cart, it is reflashable with Krikzz flasher. So if you want
to play with the LEDs by making your own ROMs you are free to do so.

Even if you don't, it'll work on a flash cart or emulator. Anyone can see how it works
or reuse parts for their own purposes.


### How to add a new track

First you need a VGM. Toss it into `res/bgm`. Inside of that folder is `resources.res`. 
Add a line like this:

    XGM BGM_MyNewTrack	"bgm/MyNewTrack.vgm" 0

That'll get the song into the ROM, but it also needs to be added to the playlist.
Open up `src/songtab.c` and add a new entry to the table:

    {
		.xgm_data 	= BGM_MyNewTrack,
		.xgm_length = TIME(6, 0, 0),
		.title 		= "My New Track",
		.artist 	= "Me",
	},

The SONG_COUNT in `inc/songtab.h` also needs to be increased.

Now your track is selectable and you can actually listen to it, but the visualization is
missing. This is the hard part, and it is rather hacky. Look in `tools/tabgen` and you will
see tabgen.c. This was written to generate a table based on a VGM file. First built it,
then feed it your VGM file.

    gcc tabgen.c -o tabgen
    ./tabgen ../../res/bgm/MyNewTrack.vgm 012356667

You're probably wondering what the heck those numbers are for. Basically, some tracks don't
use all the sound channels. Also, the MegaDrive has 10 sound channels, while the LED array
only has 8 columns. To workaround both of these a string of 10 digits must be given. Each
digit represents one of the channels on both chips, first 6 are the YM2612 and last 4 are
the SN76489. The values of the digits, between 0 and 7, represent which column of the array
the channel is assigned to. So for this example, only the 2 rightmost columns are for the
SN76489 PSG channels, combining the 3 square channels and leaving the noise channel on its
own.

A new file names vistab.c should be created. At the bottom is a #define line. Cut & paste 
this into `inc/vistab.h`, as well as declare the array.

    #define MyNewTrack_Size 4321
    extern const vistab MyNewTrack[];

Next, copy the whole table (ctrl+a selects everything in the file) to the end of `src/vistab.c`.

Now that the data is there, point to it in `src/songtab.c`.

    {
		.xgm_data 	= BGM_MyNewTrack,
		.xgm_length = TIME(6, 0, 0),
		.vis_data	= MyNewTrack,
		.vis_length = MyNewTrack_Size,
		.title 		= "My New Track",
		.artist 	= "Me",
	},

Don't forget to update the time with the real track length! Also, if you have a VGM that loops,
the visualization will suddenly stop after hitting the loop point. There is a workaround for
this though. Notice how a couple tracks have 2 extra values set.

    .loopstart  = TIME(0, 4, 0),
	.loopend    = TIME(1,38, 0),

Just figure out at what time the loop points are in the track and add the values, and the
visualization should loop along when the track does.


### How to generate the marquees

You don't. I drew text into arrays of 1's and 0's manually.


### Credits / Thanks

The YM2017 team!

    Hardware          Jazz
    Software          Grind
    Logo              Keff
    Backgrounds       Ui
    Producer          Catskull

The artists!

    Luke McQueen
    nooly
    Jredd
    Cosmo Buggi
    Savaged Regime
    Tobikomi
    Uctumi
    Dya
    Laz

Parts of Stef's SGDK are used including the XGM sound driver, LZ4W compression, and the font.
