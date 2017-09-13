#include <stdint.h>

#define min(X, Y)   (((X) < (Y))?(X):(Y))
#define max(X, Y)   (((X) > (Y))?(X):(Y))
#define abs(X)      (((X) < 0)?-(X):(X))

// SGDK / Rescomp Types
typedef struct {
    uint16_t value;
} VDPPlan;

typedef struct {
    uint16_t compression;
    uint16_t numTile;
    uint32_t *tiles;
} TileSet;

typedef struct {
    uint16_t index;
    uint16_t length;
    uint16_t *data;
} Palette;

typedef struct {
    int16_t y;
    union {
        struct {
            uint8_t size;
            uint8_t link;
        };
        uint16_t size_link;
    };
    uint16_t attribut;
    int16_t x;
} VDPSprite;

typedef struct {
    int16_t y;          // respect VDP sprite field order
    uint16_t size;
    int16_t x;
    uint16_t numTile;
} VDPSpriteInf;

typedef struct {
    uint16_t numSprite;
    VDPSpriteInf **vdpSpritesInf;
    uint16_t UNUSED_numCollision;
    uint32_t UNUSED_collisions;
    TileSet *tileset;
    int16_t w;
    int16_t h;
    uint16_t timer;
} AnimationFrame;

typedef struct {
    uint16_t numFrame;
    AnimationFrame **frames;
    uint16_t length;
    uint8_t *sequence;
    int16_t loop;
} Animation;

typedef struct {
    Palette *palette;
    uint16_t numAnimation;
    Animation **animations;
    uint16_t maxNumTile;
    uint16_t maxNumSprite;
} SpriteDefinition;

extern volatile uint8_t vblank;

void vsync();
void aftervsync();
