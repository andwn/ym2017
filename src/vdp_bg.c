#include "common.h"

#include "dma.h"
#include "memory.h"
#include "tools.h"
#include "string.h"
#include "vdp.h"
#include "vdp_pal.h"
#include "vdp_tile.h"

#include "vdp_bg.h"

static VDPPlan text_plan;
static uint16_t text_basetile;

void VDP_setHorizontalScroll(VDPPlan plan, int16_t value)
{
    volatile uint16_t *pw;
    volatile uint32_t *pl;
    uint16_t addr;

    /* Point to vdp port */
    pw = (uint16_t *) GFX_DATA_PORT;
    pl = (uint32_t *) GFX_CTRL_PORT;

    addr = VDP_HSCROLL_TABLE;
    if (plan.value == CONST_PLAN_B) addr += 2;

    *pl = GFX_WRITE_VRAM_ADDR(addr);
    *pw = value;
}

void VDP_setHorizontalScrollTile(VDPPlan plan, uint16_t tile, int16_t* values, uint16_t len, uint16_t use_dma)
{
    uint16_t addr;

    addr = VDP_HSCROLL_TABLE + ((tile & 0x1F) * (4 * 8));
    if (plan.value == CONST_PLAN_B) addr += 2;

    VDP_setAutoInc(4 * 8);

    if (use_dma) DMA_doDma(DMA_VRAM, (uint32_t) values, addr, len, -1);
    else
    {
        volatile uint16_t *pw;
        volatile uint32_t *pl;
        uint16_t *src;
        uint16_t i;

        /* Point to vdp port */
        pw = (uint16_t *) GFX_DATA_PORT;
        pl = (uint32_t *) GFX_CTRL_PORT;

        *pl = GFX_WRITE_VRAM_ADDR(addr);

        src = (uint16_t*) values;

        i = len;
        while(i--) *pw = *src++;
    }
}

void VDP_setHorizontalScrollLine(VDPPlan plan, uint16_t line, int16_t* values, uint16_t len, uint16_t use_dma)
{
    uint16_t addr;

    addr = VDP_HSCROLL_TABLE + ((line & 0xFF) * 4);
    if (plan.value == CONST_PLAN_B) addr += 2;

    VDP_setAutoInc(4);

    if (use_dma) DMA_doDma(DMA_VRAM, (uint32_t) values, addr, len, -1);
    else
    {
        volatile uint16_t *pw;
        volatile uint32_t *pl;
        uint16_t *src;
        uint16_t i;

        /* Point to vdp port */
        pw = (uint16_t *) GFX_DATA_PORT;
        pl = (uint32_t *) GFX_CTRL_PORT;

        *pl = GFX_WRITE_VRAM_ADDR(addr);

        src = (uint16_t*) values;

        i = len;
        while(i--) *pw = *src++;
    }
}

void VDP_setVerticalScroll(VDPPlan plan, int16_t value)
{
    volatile uint16_t *pw;
    volatile uint32_t *pl;
    uint16_t addr;

    /* Point to vdp port */
    pw = (uint16_t *) GFX_DATA_PORT;
    pl = (uint32_t *) GFX_CTRL_PORT;

    addr = 0;
    if (plan.value == CONST_PLAN_B) addr += 2;

    *pl = GFX_WRITE_VSRAM_ADDR(addr);
    *pw = value;
}

void VDP_setVerticalScrollTile(VDPPlan plan, uint16_t tile, int16_t* values, uint16_t len, uint16_t use_dma)
{
    uint16_t addr;

    addr = (tile & 0x1F) * 4;
    if (plan.value == CONST_PLAN_B) addr += 2;

    VDP_setAutoInc(4);

    if (use_dma) DMA_doDma(DMA_VSRAM, (uint32_t) values, addr, len, -1);
    else
    {
        volatile uint16_t *pw;
        volatile uint32_t *pl;
        uint16_t *src;
        uint16_t i;

        /* Point to vdp port */
        pw = (uint16_t *) GFX_DATA_PORT;
        pl = (uint32_t *) GFX_CTRL_PORT;

        *pl = GFX_WRITE_VSRAM_ADDR(addr);

        src = (uint16_t*) values;

        i = len;
        while(i--) *pw = *src++;
    }
}


void VDP_clearPlan(VDPPlan plan, uint16_t wait)
{
    switch(plan.value)
    {
        case CONST_PLAN_A:
            VDP_clearTileMap(VDP_PLAN_A, 0, 1 << (planWidthSft + planHeightSft), wait);
            break;

        case CONST_PLAN_B:
            VDP_clearTileMap(VDP_PLAN_B, 0, 1 << (planWidthSft + planHeightSft), wait);
            break;

        case CONST_PLAN_WINDOW:
            VDP_clearTileMap(VDP_PLAN_WINDOW, 0, 1 << (windowWidthSft + 5), wait);
            break;
    }
}

VDPPlan VDP_getTextPlan()
{
    return text_plan;
}

uint16_t VDP_getTextPalette()
{
    return (text_basetile >> 13) & 3;
}

uint16_t VDP_getTextPriority()
{
    return (text_basetile >> 15) & 1;
}

void VDP_setTextPlan(VDPPlan plan)
{
    text_plan = plan;
}

void VDP_setTextPalette(uint16_t pal)
{
    text_basetile &= ~(3 << 13);
    text_basetile |= (pal & 3) << 13;
}

void VDP_setTextPriority(uint16_t prio)
{
    text_basetile &= ~(1 << 15);
    text_basetile |= (prio & 1) << 15;
}

void VDP_drawTextBG(VDPPlan plan, const char *str, uint16_t x, uint16_t y)
{
    uint32_t len;
    uint16_t data[128];
    char *s;
    uint16_t *d;
    uint16_t i;

    // get the horizontal plan size (in cell)
    i = (plan.value == CONST_PLAN_WINDOW)?windowWidth:planWidth;
    len = strlen(str);

    // if string don't fit in plan, we cut it
    if (len > (uint16_t) (i - x))
        len = i - x;

    s = (char*) str;
    d = data;
    while(i--)
        *d++ = TILE_FONTINDEX + (*s++ - 32);

    VDP_setTileMapDataRectEx(plan, data, text_basetile, x, y, len, 1, len);
}

void VDP_clearTextBG(VDPPlan plan, uint16_t x, uint16_t y, uint16_t w)
{
    VDP_fillTileMapRect(plan, 0, x, y, w, 1);
}

void VDP_clearTextAreaBG(VDPPlan plan, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    VDP_fillTileMapRect(plan, 0, x, y, w, h);
}

void VDP_clearTextLineBG(VDPPlan plan, uint16_t y)
{
    VDP_fillTileMapRect(plan, 0, 0, y, (plan.value == CONST_PLAN_WINDOW)?windowWidth:planWidth, 1);
}

void VDP_drawText(const char *str, uint16_t x, uint16_t y)
{
    VDP_drawTextBG(text_plan, str, x, y);
}

void VDP_clearText(uint16_t x, uint16_t y, uint16_t w)
{
    VDP_clearTextBG(text_plan, x, y, w);
}

void VDP_clearTextArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    VDP_clearTextAreaBG(text_plan, x, y, w, h);
}

void VDP_clearTextLine(uint16_t y)
{
    VDP_clearTextLineBG(text_plan, y);
}
