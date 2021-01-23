#include "stm32f4xx.h"                  // Device header
#include "delay.h"

void  SetClock(void);

int main(void)
{
		SetClock();
		RCC->AHB1ENR |= 0x08;													// gpiod aktif
		GPIOD->MODER  |= 0x1000000;					          // 12. pin cikis
		
		SysTick_Config(SystemCoreClock / 1000);  			// SysTick 1ms ile ayarlandi
																									// systicconf 168m olarak, hseyi ise 8m olarak duzelttim
																									// Butun interrupt, deger vs atamlari bu fonksiyon icinde yapiliyor.
		
		while (1)
		{
				GPIOD->ODR ^= 1 << 12;										// pin tersle 
				delay_s(5);																// 1 s delay
		}
	
}


void  SetClock()																											  /* System Clock Ayar Fonk.*/
{
		uint16_t PLL_M = 8, PLL_P = 2, PLL_Q = 4, PLL_N = 336;
	
		__IO uint32_t StartUpCounter = 0, HSEStatus = 0;
	
		RCC->CR |= ((uint32_t) RCC_CR_HSEON);					 										 /* HSE AKTIF */
	
		do
		{
				HSEStatus = RCC->CR & RCC_CR_HSERDY;													 
				StartUpCounter++;
		} while((HSEStatus == 0) && (StartUpCounter != 0x5000));					 /* HSE hazir olana kadar bekleme kodu, TimeOut onlemi alinmisitir*/
		
		if ((RCC->CR  & RCC_CR_HSERDY) != RESET)
		{
				HSEStatus = (uint32_t)0x01;																		 /* Bayrak hazirsa 1 olsun*/
		}
		else
		{
				HSEStatus = (uint32_t)0x00;																		 /* Bayrak hazirsa 1 olsun*/
		}
		
		if (HSEStatus == (uint32_t)0x01)
		{
				RCC->APB1ENR |= RCC_APB1ENR_PWREN;														 /* Power Interface Clock Enable */
				PWR->CR  |= PWR_CR_VOS;																				 /* Genel anlamda guc dengesi icin */
			
				RCC->CFGR |= RCC_CFGR_HPRE_DIV1;															 /* HCLK = SYSCLOCK / 1 (AHB PRESCALER)*/
				RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;															 /* APB2 (HIGH_SPEED) = HCLK / 2 */
				RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;															 /* APB1 (LOW_SPEED)  = HCLK / 4 */
				
				RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) 
										 | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);			 /* Ana PLL Ayarlari */ 
			
				RCC->CR |= RCC_CR_PLLON;																			 /* Ana PLL Enable Yap */
			
				while (( RCC->CR & RCC_CR_PLLRDY) == 0)												 /* PLL Hazir olana kadar bekle */
				{
				}
				
				FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | 							 /* Flash Arayuz (Oonbellek ve bekleme ayarlari */
													FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;
				
				RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));						 /* MainPLL sistem clock kaynagi olarak secildi */
				RCC->CFGR |= RCC_CFGR_SW_PLL;																	 /* PLL selected as system clock */
				
				while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL)
				{	
				}																															 /* PLL System clock u olana kadar bekle */
		}
		
		else 
		{																																	 /* HSE de baslarken hata olursa bunla basetmek icin kod ekleyebilirsin */
		}
		
		SystemCoreClockUpdate();
}

