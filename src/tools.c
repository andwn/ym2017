#include "common.h"

#include "memory.h"
#include "string.h"
#include "timer.h"
#include "vdp.h"

#include "tools.h"

uint16_t randbase;

void setRandomSeed(uint16_t seed) {
    // xor it with a random value to avoid 0 value
    randbase = seed ^ 0xD94B;
}

uint16_t random() {
    randbase ^= (randbase >> 1) ^ GET_HVCOUNTER;
    randbase ^= (randbase << 1);

    return randbase;
}
