#ifndef			__DELAY_H
#define			__DELAY_H

#include "stm32f4xx.h"                  // Device header
#include "stdint.h"											

extern uint32_t SysTickCounter;					// SysTick Handler icindeki sayac

void delay_ms(uint32_t);
void delay_s(uint8_t);

#endif
