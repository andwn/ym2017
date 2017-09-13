#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t channel_map[10];
uint32_t lastwrite[8];
uint16_t line_count;

int cmdlen(uint8_t cmd) {
	if(cmd >= 0x30 && cmd <= 0x3F) return 1;
	if(cmd >= 0x40 && cmd <= 0x4E) return 2;
	if(cmd == 0x4F || cmd == 0x50) return 1;
	if(cmd >= 0x51 && cmd <= 0x61) return 2;
	if(cmd == 0x62 || cmd == 0x63) return 0;
	if(cmd == 0x64) return 3;
	if(cmd == 0x67) return 6;
	if(cmd == 0x68) return 11;
	if(cmd >= 0x70 && cmd <= 0x8F) return 0;
	if(cmd == 0x90) return 4;
	if(cmd == 0x91) return 4;
	if(cmd == 0x92) return 5;
	if(cmd == 0x93) return 10;
	if(cmd == 0x94) return 1;
	if(cmd == 0x95) return 4;
	if(cmd >= 0xa0 && cmd <= 0xBF) return 2;
	if(cmd >= 0xc0 && cmd <= 0xDF) return 3;
	if(cmd >= 0xE0) return 4;
	printf("WARN: Bad command: 0x%02hhx\n", cmd);
	return 0;
}

char *getvarname(const char *filename) {
	char *rtn;
	// Find last slash and dot for extension, if any
	int pos = 0, ext = 0;
	for(int i = 0; i < strlen(filename); i++) {
		if(filename[i] == '/' || filename[i] == '\\') pos = i+1;
		if(filename[i] == '.') ext = i;
	}
	// Copy just the name without path or extension into new string
	int size = ext - pos;
	if(size <= 0) size = strlen(filename) - pos;
	rtn = calloc(size + 1, 1);
	strncpy(rtn, &filename[pos], size);
	// Replace bad chars
	for(int i = 0; i < strlen(rtn); i++) {
		if(rtn[i] == '-') rtn[i] = '_';
		if(rtn[i] == ' ') rtn[i] = '_';
	}
	return rtn;
}

#define WAITTIME (735 * 8)

void writecmd(uint32_t time, uint8_t channel, FILE *out) {
	if(time - lastwrite[channel] < WAITTIME) return;
	lastwrite[channel] = time;
	uint8_t t1 = (time >> 16) & 0xFF, t2 = (time >> 8) & 0xFF, t3 = time & 0xFF;
	fprintf(out, "\t{ { 0x%02hhx, 0x%02hhx, 0x%02hhx }, 0x%02hhx },\n", t1, t2, t3, channel);
	//fwrite(&t1, 1, 1, out);
	//fwrite(&t2, 1, 1, out);
	//fwrite(&t3, 1, 1, out);
	//fwrite(&channel, 1, 1, out);
	line_count++;
}

int main(int argc, char *argv[]) {
	if(argc < 3 && !(argc & 1)) {
		printf("Usage: tabgen <vgm file> <channel map> [<vgm2> <map2> ...]\n");
		return EXIT_SUCCESS;
	}
	FILE *out = fopen("vistab.c", "wb");
	if(!out) {
		printf("FATAL: Failed to create '%s'\n", argv[argc-1]);
		return EXIT_FAILURE;
	}
	fprintf(out, "#include \"common.h\"\n#include \"vistab.h\"\n\n");
	
	for(int i = 1; i < argc; i += 2) {
		line_count = 0;
		
		// Set channel map
		if(strlen(argv[i+1]) != 10) {
			printf("ERROR: Bad channel map for '%s': '%s'\n", argv[i], argv[i+1]);
			continue;
		}
		for(int c = 0; c < 10; c++) {
			channel_map[c] = argv[i+1][c] - 0x30;
		}
		
		// Open VGM file
		FILE *in = fopen(argv[i], "rb");
		if(!in) {
			printf("ERROR: Failed to open '%s'\n", argv[i]);
			continue;
		}
		// Verify "Vgm " indicator
		char head[4];
		fread(head, 1, 4, in);
		if(strncmp(head, "Vgm ", 4) != 0) {
			printf("ERROR: '%s' is not a VGM file.\n", argv[i]);
			fclose(in);
			continue;
		}
		// Get VGM version
		uint32_t version;
		fseek(in, 0x08, SEEK_SET);
		fread(&version, 4, 1, in);
		printf("INFO: Version: 0x%04x\n", version);
		// Get data start point in file (relative)
		uint32_t datapos;
		fseek(in, 0x34, SEEK_SET);
		fread(&datapos, 4, 1, in);
		//printf("TRACE: Data starts at: 0x34 + 0x%06x\n", datapos);
		fseek(in, 0x34 + datapos, SEEK_SET);
		// Write start of variable definition
		char *varname = getvarname(argv[i]);
		fprintf(out, "const vistab %s[] = {\n", varname);
		//free(varname);
		// Open output bin
		//FILE *out = fopen(varname, "wb");
		//if(!out) {
		//	printf("ERROR: Failed to open '%s'\n", varname);
		//	fclose(in);
		//	continue;
		//}
		
		// GET TO WORK BITCH!!!
		uint8_t cmd = 0x00;
		uint32_t time = 0;
		
		uint8_t dactransfer = 0;
		uint8_t psgchannel = 0;
		uint8_t psglatch = 0;
		uint8_t psgvol = 0xF;
		memset(lastwrite, 0, 4*8);
		
		while(!feof(in)) {
			if(!fread(&cmd, 1, 1, in)) break;
			//printf("TRACE: Command: 0x%02hhx\n", cmd);
			if(cmd >= 0x70 && cmd <= 0x7F) {
				time += (cmd & 0xF) + 1;
				//printf("TRACE: Wait for 0x%02hhx\n", (cmd & 0xF) + 1);
				continue;
			}
			if(cmd >= 0x80 && cmd <= 0x8F) {
				time += cmd & 0xF;
				//printf("TRACE: Data write + wait for 0x%02hhx\n", cmd & 0xF);
				continue;
			}
			
			//if(cmd != 0x52 && cmd != 0x53) dactransfer = 0;
			
			switch(cmd) {
				case 0x50: { // Write value to SN76489 register
					uint8_t reg;
					fread(&reg, 1, 1, in);
					if(reg & 0b10000000) { // LATCH/DATA
						psgchannel = (reg & 0b01100000) >> 5;
						psglatch = (reg & 0b00010000) >> 4;
					}
					if(psglatch == 0) { // Volume change
						uint8_t oldvol = psgvol;
						psgvol = reg & 0b00001111;
						if(psgvol < 0b1000 && psgvol < oldvol) {
							// Volume increase (0 is full, F is silent)
							//uint8_t psgdisp = 6 + psgchannel;
							//printf("Channel %hhu: PSG\n", psgdisp);
							writecmd(time, channel_map[psgchannel + 6], out);
						}
					}
				}
				break;
				case 0x52:
				case 0x53: { // Write value to YM2612 register
					uint8_t reg, val;
					fread(&reg, 1, 1, in);
					fread(&val, 1, 1, in);
					if(reg == 0x2A) { // DAC
						if(!dactransfer) {
							writecmd(time, channel_map[5], out);
							//printf("Channel 5: DAC\n");
						}
						dactransfer = 1;
					} else {
						dactransfer = 0;
					}
					if(reg == 0x28) { // Channel write
						uint8_t channel = val & 7;
						// Channel index skips from 0b010 to 0b100, so there is no 4
						if(channel > 3) channel--;
						if(val >> 4) {
							writecmd(time, channel_map[channel], out);
							//printf("Channel %hhu: FM\n", channel);
						}
					}
				}
				break;
				case 0x61: {
					uint16_t wait;
					fread(&wait, 2, 1, in);
					time += wait;
					//printf("TRACE: Wait for 0x%04hx\n", wait);
				}
				break;
				case 0x62: {
					time += 735; // /3=245
					//printf("TRACE: Wait for 1 NTSC frame\n");
				}
				break;
				case 0x63: {
					time += 882; // /3=294
					//printf("TRACE: Wait for 1 PAL frame\n");
				}
				break;
				case 0x66: {
					//printf("INFO: End of stream reached\n");
					fseek(in, 0, SEEK_END);
				}
				break;
				case 0x67: { // Data
					uint32_t size;
					fseek(in, 2, SEEK_CUR); // Skip 0x66 and type
					fread(&size, 4, 1, in);
					//printf("TRACE: Data block size: 0x%06x\n", size);
					fseek(in, size, SEEK_CUR);
				}
				break;
				case 0x68: {
					uint32_t size = 0;
					fseek(in, 8, SEEK_CUR); // Skip 0x66 and other args
					fread(&size, 3, 1, in);
					//printf("TRACE: PCM Write size: 0x%06x\n", size);
					fseek(in, size, SEEK_CUR);
				}
				break;
				case 0x93: // Start DAC stream
				{
					fseek(in, 10, SEEK_CUR);
					writecmd(time, channel_map[5], out);
					//printf("Channel 5: DAC\n");
				}
				break;
				case 0x95: // Start DAC stream (fast call)
				{
					fseek(in, 4, SEEK_CUR);
					writecmd(time, channel_map[5], out);
					//printf("Channel 5: DAC\n");
				}
				break;
				default: fseek(in, cmdlen(cmd), SEEK_CUR); break; // Skip
			}
		}
		
		fprintf(out, "};\n\n");
		fprintf(out, "#define %s_Size %d\n\n", varname, line_count);
		
		free(varname);
		fclose(in);
		//fclose(out);
	}
	
	fclose(out);
	
	return EXIT_SUCCESS;
}
