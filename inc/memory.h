#define STACK_SIZE      0x400
#define MEMORY_HIGH     (0x01000000 - STACK_SIZE)

void MEM_init();
void MEM_free(void *ptr);
void* MEM_alloc(uint16_t size);

void memset(void *to, uint8_t value, uint16_t len);
void memcpy(void *to, const void *from, uint16_t len);
