#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "driver_plf.h"

static uint16_t co_read16p(void const *ptr16)
{
    uint16_t value = ((uint8_t *)ptr16)[0] | ((uint8_t *)ptr16)[1] << 8;
    return value;
}

static uint32_t co_read32p(void const *ptr32)
{
    uint16_t addr_l, addr_h;
    addr_l = co_read16p(ptr32);
    addr_h = co_read16p((uint8_t *)ptr32 + 2);
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}

void aes_decrypt(const uint8_t *key, const uint8_t *enc_data, uint8_t *plain_data, void (*callback)(uint8_t *))
{
    volatile uint8_t *aes_ongoing = (volatile uint8_t *)0x20001474;
    uint8_t save_result[16];
    uint8_t result_saved = 0;

    GLOBAL_INT_DISABLE();
    if(*aes_ongoing) {
        if((*(volatile uint32_t *)0x200014a4 & (1<<3)) == 0) {
            *(volatile uint32_t *)0x4000000c &= (~(1<<7));
            while((*(volatile uint32_t *)0x40000014 & (1<<7)) == 0);
            *(volatile uint32_t *)0x40000018 = ((1<<7));
        }
        memcpy(save_result, (void *)((16*16+80+6+16) + 0x40010000), 16);
        result_saved = true;
    }

    memcpy((void *)((16*16+80+6) + 0x40010000), enc_data, 16);
    *(volatile uint32_t *)0x400000b4 = co_read32p(&(key[0]));
    *(volatile uint32_t *)0x400000b8 = co_read32p(&(key[4]));
    *(volatile uint32_t *)0x400000bc = co_read32p(&(key[8]));
    *(volatile uint32_t *)0x400000c0 = co_read32p(&(key[12]));
    *(volatile uint32_t *)0x400000c4 = (16*16+80+6);
    *(volatile uint32_t *)0x400000b0 = ((1<<1));
    *(volatile uint32_t *)0x400000b0 |= ((1<<0));
    while((*(volatile uint32_t *)0x40000014 & (1<<7)) == 0);
    *(volatile uint32_t *)0x40000018 = ((1<<7));
    *(volatile uint32_t *)0x400000b0 = 0;

    memcpy(plain_data, (void *)((16*16+80+6+16) + 0x40010000), 16);
    GLOBAL_INT_RESTORE();

    if(result_saved) {
        memcpy((void *)((16*16+80+6+16) + 0x40010000), save_result, 16);
        // Prevent going to deep sleep during encryption
        *(volatile uint16_t *)0x20001596 &= (~(0x0010));
        // mark that AES is done
        GLOBAL_INT_DISABLE();
        *(volatile uint32_t *)0x200014a4 |= (1<<3);
        GLOBAL_INT_RESTORE();
    }

    if(callback) {
        callback(plain_data);
    }
}

void aes_encrypt_(const uint8_t *key, const uint8_t *plain_data, uint8_t *enc_data, void (*callback)(uint8_t *))
{
    volatile uint8_t *aes_ongoing = (volatile uint8_t *)0x20001474;
    uint8_t save_result[16];
    uint8_t result_saved = 0;

    GLOBAL_INT_DISABLE();
    if(*aes_ongoing) {
        if((*(volatile uint32_t *)0x200014a4 & (1<<3)) == 0) {
            *(volatile uint32_t *)0x4000000c &= (~(1<<7));
            while((*(volatile uint32_t *)0x40000014 & (1<<7)) == 0);
            *(volatile uint32_t *)0x40000018 = ((1<<7));
        }
        memcpy(save_result, (void *)((16*16+80+6+16) + 0x40010000), 16);
        result_saved = true;
    }

    memcpy((void *)((16*16+80+6) + 0x40010000), plain_data, 16);
    *(volatile uint32_t *)0x400000b4 = co_read32p(&(key[0]));
    *(volatile uint32_t *)0x400000b8 = co_read32p(&(key[4]));
    *(volatile uint32_t *)0x400000bc = co_read32p(&(key[8]));
    *(volatile uint32_t *)0x400000c0 = co_read32p(&(key[12]));
    *(volatile uint32_t *)0x400000c4 = (16*16+80+6);
    *(volatile uint32_t *)0x400000b0 = ((1<<1));
    *(volatile uint32_t *)0x400000b0 |= ((1<<0));
    while((*(volatile uint32_t *)0x40000014 & (1<<7)) == 0);
    *(volatile uint32_t *)0x40000018 = ((1<<7));
    *(volatile uint32_t *)0x400000b0 = 0;

    memcpy(enc_data, (void *)((16*16+80+6+16) + 0x40010000), 16);
    GLOBAL_INT_RESTORE();

    if(result_saved) {
        memcpy((void *)((16*16+80+6+16) + 0x40010000), save_result, 16);
        // Prevent going to deep sleep during encryption
        *(volatile uint16_t *)0x20001596 &= (~(0x0010));
        // mark that AES is done
        GLOBAL_INT_DISABLE();
        *(volatile uint32_t *)0x200014a4 |= (1<<3);
        GLOBAL_INT_RESTORE();
    }

    if(callback) {
        callback(enc_data);
    }
}

