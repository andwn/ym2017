#include "common.h"

#include "dma.h"
#include "joy.h"
#include "memory.h"
#include "resources.h"
#include "sprite.h"
#include "string.h"
#include "timer.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "xgm.h"
#include "z80_ctrl.h"

#include "system.h"

extern void gm_title();
extern void gm_list();
extern void gm_playing();

// exception state consumes 78 bytes of memory
uint32_t registerState[8+8];
uint32_t pcState;
uint32_t addrState;
uint16_t ext1State;
uint16_t ext2State;
uint16_t srState;

static void addValueU8(char *dst, char *str, uint8_t value) {
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 2);
    strcat(dst, v);
}

static void addValueU16(char *dst, char *str, uint16_t value)
{
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 4);
    strcat(dst, v);
}

static void addValueU32(char *dst, char *str, uint32_t value) {
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 8);
    strcat(dst, v);
}

static uint16_t showValueU32U16(char *str1, uint32_t value1, char *str2, uint16_t value2, uint16_t pos) {
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU16(s, str2, value2);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU16U32U16(char *str1, uint16_t value1, char *str2, uint32_t value2, char *str3, uint16_t value3, uint16_t pos) {
    char s[64];

    strclr(s);
    addValueU16(s, str1, value1);
    addValueU32(s, str2, value2);
    addValueU16(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U16U16(char *str1, uint32_t value1, char *str2, uint16_t value2, char *str3, uint16_t value3, uint16_t pos) {
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU16(s, str2, value2);
    addValueU16(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U32(char *str1, uint32_t value1, char *str2, uint32_t value2, uint16_t pos) {
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU32(s, str2, value2);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U32U32(char *str1, uint32_t value1, char *str2, uint32_t value2, char *str3, uint32_t value3, uint16_t pos) {
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU32(s, str2, value2);
    addValueU32(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showRegisterState(uint16_t pos) {
    uint16_t y = pos;

    y = showValueU32U32U32("D0=", registerState[0], " D1=", registerState[1], " D2=", registerState[2], y);
    y = showValueU32U32U32("D3=", registerState[3], " D4=", registerState[4], " D5=", registerState[5], y);
    y = showValueU32U32("D6=", registerState[6], " D7=", registerState[7], y);
    y = showValueU32U32U32("A0=", registerState[8], " A1=", registerState[9], " A2=", registerState[10], y);
    y = showValueU32U32U32("A3=", registerState[11], " A4=", registerState[12], " A5=", registerState[13], y);
    y = showValueU32U32("A6=", registerState[14], " A7=", registerState[15], y);

    return y;
}

static uint16_t showStackState(uint16_t pos) {
    char s[64];
    uint16_t y = pos;
    uint32_t *sp = (uint32_t*) registerState[15];

    uint16_t i = 0;
    while(i < 24) {
        strclr(s);
        addValueU8(s, "SP+", i * 4);
        strcat(s, " ");
        y = showValueU32U32(s, *(sp + (i + 0)), " ", *(sp + (i + 1)), y);
        i += 2;
    }

    return y;
}

static uint16_t showExceptionDump(uint16_t pos) {
    uint16_t y = pos;

    y = showValueU32U16("PC=", pcState, " SR=", srState, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}

static uint16_t showException4WDump(uint16_t pos) {
    uint16_t y = pos;

    y = showValueU32U16U16("PC=", pcState, " SR=", srState, " VO=", ext1State, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}

static uint16_t showBusAddressErrorDump(uint16_t pos) {
    uint16_t y = pos;

    y = showValueU16U32U16("FUNC=", ext1State, " ADDR=", addrState, " INST=", ext2State, y);
    y = showValueU32U16("PC=", pcState, " SR=", srState, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}


// address error default callback
void _address_error_cb() {
    VDP_init();
    VDP_drawText("ADDRESS ERROR !", 10, 3);

    showBusAddressErrorDump(5);

    while(1);
}

// illegal instruction exception default callback
void _illegal_instruction_cb() {
    VDP_init();
    VDP_drawText("ILLEGAL INSTRUCTION !", 7, 3);

    showException4WDump(5);

    while(1);
}

// division by zero exception default callback
void _zero_divide_cb() {
    VDP_init();
    VDP_drawText("DIVIDE BY ZERO !", 10, 3);

    showExceptionDump(5);

    while(1);
}

void _start_entry() {
    setRandomSeed(0xC427); // Reset RNG
    __asm__("move #0x2300,%sr"); // Enable interrupts
    // Initialize the system
    MEM_init();
    VDP_init();
    DMA_init(0, 0);
    JOY_init();
    Z80_init();
    // Do the stuff
    gm_title();
    while(TRUE) {
		gm_list();
		gm_playing();
	}
}
