#ifndef _BUTTON_H
#define _BUTTON_H

#include <stdint.h>
#include "type_lib.h"

#define BUTTON_FN0  BIT0
#define BUTTON_FN1  BIT1
#define BUTTON_FN2  BIT2
#define BUTTON_FN3  BIT3
#define BUTTON_FN4  BIT4
#define BUTTON_FN5  BIT5
#define BUTTON_FN6  BIT6
#define BUTTON_FN7  BIT7
#define BUTTON_FN8  BIT8
#define BUTTON_FN9  BIT9
#define BUTTON_FN10  BIT10
#define BUTTON_FN11  BIT11
#define BUTTON_FN12  BIT12
#define BUTTON_FN13  BIT13
#define BUTTON_FN14  BIT14
#define BUTTON_FN15  BIT15
#define BUTTON_FN16  BIT16
#define BUTTON_FN17  BIT17
#define BUTTON_FN18  BIT18
#define BUTTON_FN19  BIT19
#define BUTTON_FN20  BIT20
#define BUTTON_FN21  BIT21
#define BUTTON_FN22  BIT22
#define BUTTON_FN23  BIT23
#define BUTTON_FN24  BIT24
#define BUTTON_FN25  BIT25
#define BUTTON_FN26  BIT26
#define BUTTON_FN27  BIT27
#define BUTTON_FN28  BIT28
#define BUTTON_FN29  BIT29
#define BUTTON_FN30  BIT30
#define BUTTON_FN31  BIT31




enum button_type_t {
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_SHORT_PRESSED,
    BUTTON_MULTI_PRESSED,
    BUTTON_LONG_PRESSED,
    BUTTON_LONG_PRESSING,
    BUTTON_LONG_RELEASED,
    BUTTON_LONG_LONG_PRESSED,
    BUTTON_LONG_LONG_RELEASED,
    BUTTON_COMB_PRESSED,
    BUTTON_COMB_RELEASED,
    BUTTON_COMB_SHORT_PRESSED,
    BUTTON_COMB_LONG_PRESSED,
    BUTTON_COMB_LONG_PRESSING,
    BUTTON_COMB_LONG_RELEASED,
    BUTTON_COMB_LONG_LONG_PRESSED,
    BUTTON_COMB_LONG_LONG_RELEASED,
};

struct button_msg_t {
    uint32_t button_index;
    enum button_type_t button_type;
    uint8_t button_cnt; //only for multi click
};

typedef void (*button_func_handler_t)(struct button_msg_t*);

void bt_button_set_cb_func(button_func_handler_t func);

void button_toggle_detected(uint32_t curr_button);

void button_init(uint32_t enable_io);

#endif  //_BUTTON_H

