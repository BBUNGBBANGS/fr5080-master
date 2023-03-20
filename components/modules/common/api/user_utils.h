#ifndef _USER_UTILS_H
#define _USER_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
//#include "co_printf.h"
#include "cmsis_compiler.h" 

struct list_entry_t 
{
    struct list_entry_t *Flink;
    struct list_entry_t *Blink;
    
};

void init_list_head(struct list_entry_t *head);
#define init_list_head(ListHead) (\
        (ListHead)->Flink = (ListHead)->Blink = (ListHead) )
    
#define init_list_entry(Entry) (\
        (Entry)->Flink = (Entry)->Blink = 0 )
    
#define is_entry_available(Entry) (\
        ((Entry)->Flink == 0))
    
bool is_list_empty(struct list_entry_t *head);
#define is_list_empty(ListHead) (\
        ((ListHead)->Flink == (ListHead)))
    
#define get_head_list(ListHead) (ListHead)->Flink
    
#define get_tail_list(ListHead) (ListHead)->Blink
    
#define get_next_node(Node)     (Node)->Flink
    
#define get_prior_node(Node)    (Node)->Blink
    
#define is_node_connected(n) (((n)->Blink->Flink == (n)) && ((n)->Flink->Blink == (n)))

void insert_tail_list(struct list_entry_t* head, struct list_entry_t* entry);
void insert_head_list(struct list_entry_t* head, struct list_entry_t* entry);

struct list_entry_t* remove_head_list(struct list_entry_t* head);
void remove_entry_list(struct list_entry_t* entry);

bool is_node_on_list(struct list_entry_t* head, struct list_entry_t* node);
void move_list(struct list_entry_t* dest, struct list_entry_t* src);


#define BIT(x) (1<<(x))

#define ROUND(x,y)  ( (x%y)?(x/y+1):(x/y) )

#define SET_FEILD(reg,field,pos,value) \
    (reg & ~(field<<pos) | (value<<pos))

#define MIN(x,y) ( (x<y)?(x):(y) )

#define MAX(x,y) ( (x>y)?(x):(y) )

#define TO_BIG_EDN_16(x) ( (x&0xff)<<8 | (x&0xff00)>>8 )

#define TO_LITTLE_EDN_16(x) ( (x&0xff)<<8 | (x&0xff00)>>8 )

__STATIC_INLINE void show_reg(uint8_t *data,uint32_t len,uint8_t dbg_on)
{
    uint32_t i=0;
    if(len == 0 || (dbg_on==0)) return;
    for(; i<len; i++)
    {
        printf("0x%02X ",data[i]);
    }
    printf("\r\n");
}

__STATIC_INLINE void show_utf8_hex(uint8_t *data,uint32_t len,uint8_t dbg_on)
{
    uint32_t i=0;
    if(len == 0 || (dbg_on==0)) return;
    for(; i<len; i++)
    {
        printf("%02X",data[i]);
    }
    printf("\r\n");
}

void co_delay_100us(uint32_t num);

/*
 * num should not be greater than 8_000_000
 */
void co_delay_10us(uint32_t num);
void cpu_delay_100us(uint32_t num);
uint8_t hex4bit_to_char(const uint8_t hex_value);
uint8_t hex4bit_to_caps_char(const uint8_t hex_value);

void hex_arr_to_str(const uint8_t hex_arr[],uint8_t arr_len,uint8_t *str);
void str_to_hex_arr(const uint8_t *str, uint8_t hex_arr[],uint8_t arr_len);
void val_to_str(const uint32_t int_value,uint8_t *str);

char char_to_val(const char c);
int str_to_val( const char str[], char base, char n);
int ascii_strn2val( const char str[], char base, char n);

__attribute__((section("ram_code"))) void retry_handshake(void);

void set_user_pincode(uint8_t *pin, uint8_t len);

#endif  //_USER_UTILS_H



