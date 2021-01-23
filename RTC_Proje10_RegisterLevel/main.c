#include "stm32f4xx.h"                  // Device header

/* RTC AY TANIMLARI(BCD) */ 
#define		OCAK		((uint8_t)0x01)
#define		SUBAT		((uint8_t)0x02)
#define		MART		((uint8_t)0x03)
#define 	NISAN		((uint8_t)0x04)
#define		MAYIS		((uint8_t)0x05)
#define 	HAZIRAN	((uint8_t)0x06)
#define		TEMMUZ	((uint8_t)0x07)
#define		AGUSTOS	((uint8_t)0x08)
#define		EYLUL		((uint8_t)0x09)
#define		EKIM		((uint8_t)0x10)
#define		KASIM		((uint8_t)0x11)
#define		ARALIK	((uint8_t)0x12)

/* RTC GUN TANIMLARI (BCD) */
#define		PAZARTESI		((uint8_t)0x01)
#define		SALI				((uint8_t)0x02)
#define 	CARSAMBA		((uint8_t)0x03)
#define		PERSEMBE		((uint8_t)0x04)
#define		CUMA				((uint8_t)0x05)
#define		CUMARTESI		((uint8_t)0x06)
#define		PAZAR				((uint8_t)0x07)

void SetClock(void);																																			// System Clock Ayar Fonk.
void RTC_Alarm_IRQHandler(void);																													// Alarm int. fonksiyonu
void RTC_Init(void);																																			// RTC Baslangic Ayarlari
void Set_Time_BCD( uint8_t saat, uint8_t dakika, uint8_t saniye);													// Saat ayar fonksiyonu
void Set_Date_BCD( uint8_t yil, uint8_t ay, uint8_t gun, uint8_t hafta_gun);							// Takvin ayar fonksiyonu
void	Set_Alarm_A_BCD( uint8_t gun, uint8_t saat, uint8_t dakika, uint8_t saniye);				// RTC Alarm A Fonksiyonu
																																

int main()
{
	SetClock();
	
	SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);										 // PORT D Clock acildi
	
	GPIOD->MODER		|=		( 1 << GPIO_MODER_MODE12_Pos)	|						 // D12-15 output olarak ayarlandi
												( 1 << GPIO_MODER_MODE13_Pos) |
												( 1 << GPIO_MODER_MODE14_Pos) |
												( 1 << GPIO_MODER_MODE15_Pos) ;
	
	RTC_Init();																											 // RTC On ayarlar 
	
	Set_Time_BCD( 0x09,	0x04,	0x45);																 // Saat: 09:04:45 ayarlandi
	
	Set_Date_BCD( 0x18, KASIM, 0x10, CUMARTESI );										 // Takvim ayarlandi
	
	Set_Alarm_A_BCD( 0x10,	0x09, 0x05, 0 );												 // Alarm ayarlandi
	
	while(1)
	{
		// Sonsuz bekleyis..
	}
	
}	// end of main

void SetClock(void)																								 // System Clock Ayar Fonk.
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

void RTC_Alarm_IRQHandler()																				 // Alarm int. fonksiyonu
{
	if( (RTC->ISR & RTC_ISR_ALRAF ) == RTC_ISR_ALRAF)								  // Alarm A gerceklesti mi?
	{
			RTC->ISR		&=	~RTC_ISR_ALRAF;																// Alarm A Flag temizle
			
			GPIOD->ODR	 =	GPIO_ODR_OD12 | GPIO_ODR_OD13 |								// Alarm oldugunda LEDLERI YAK
											GPIO_ODR_OD14 | GPIO_ODR_OD15;								// yani cikis vermek demek
	}
}

void RTC_Init(void)																								 // RTC Baslangic Ayarlari
{
	PWR->CR			|=	PWR_CR_DBP;																			// BackUP domain reg. yazma korumasi kaldir
	RCC->CSR		|=	RCC_CSR_LSION;																	// RTC Clock aktif yap 32kHz
	
	while ((RCC->CSR & RCC_CSR_LSIRDY) != RCC_CSR_LSIRDY );					// LSI hazir olana kadar bekle
	
	RCC->BDCR		|=  RCC_BDCR_BDRST;																	// Back up domain reset at
	RCC->BDCR		&=	~RCC_BDCR_BDRST;																// Reseti kaldir
			
	RCC->BDCR		|=	RCC_BDCR_RTCEN | (0x02 << RCC_BDCR_RTCSEL_Pos); // RTC nin clocku LSIdan gelsin
	
	RTC->WPR		=  	0xCA;																						// RTC nin sifre korumasi kaldiriliyor
	RTC->WPR		=		0x53;																						// RTC nin sifre korumasi kaldiriliyor
	
	RTC->ISR		|=	RTC_ISR_INIT;																		// RCC de ayar yapilacak, INIT Moduna gec
	
	while( (RTC->ISR	& RTC_ISR_INITF) != RTC_ISR_INITF );					// Moda gecene kadar bayrak okunuyor
	
	RTC->PRER		=	  ( (128 - 1) << RTC_PRER_PREDIV_A_Pos) |					// PreScaler degerler girildi
									( (250 - 1) << RTC_PRER_PREDIV_S_Pos);
	
	RTC->ISR		&=	~RTC_ISR_INIT;																	// Init moddan cikiliyor
	
	RTC->WPR		= 	0x23;																						// Yanlis key gir ve rtc birimini yazmalara kapa
}

void Set_Time_BCD( uint8_t saat, uint8_t dakika, uint8_t saniye)	 // Saat ayar fonksiyonu
{
	RTC->WPR		=  	0xCA;																						// RTC nin sifre korumasi kaldiriliyor
	RTC->WPR		=		0x53;																						// RTC nin sifre korumasi kaldiriliyor
	
	RTC->ISR   |=		RTC_ISR_INIT;																		// Init moduna gec, islem yapabilmek icin
	
	while( (RTC->ISR	& RTC_ISR_INITF) != RTC_ISR_INITF );					// Moda gecene kadar bayrak okunuyor
	
	RTC->TR			=		(saat      << RTC_TR_HU_Pos )	|
									(dakika 	 << RTC_TR_MNU_Pos) |									// Tek seferde, BCD olarak parametrelerin girilmesi gerekiyor
									(saniye		 << RTC_TR_SU_Pos ) ;				
	
	RTC->ISR		&=	~RTC_ISR_INIT;																	// Init moddan cikiliyor
	
	RTC->WPR		= 	0x23;																						// Yanlis key gir ve rtc birimini yazmalara kapa
	
}

void Set_Date_BCD( uint8_t yil, uint8_t ay, uint8_t gun, uint8_t hafta_gun)
{																																	 // Takvin ayar fonksiyonu
	RTC->WPR		=  	0xCA;																						// RTC nin sifre korumasi kaldiriliyor
	RTC->WPR		=		0x53;																						// RTC nin sifre korumasi kaldiriliyor
	
	RTC->ISR   |=		RTC_ISR_INIT;																		// Init moduna gec, islem yapabilmek icin
	
	while( (RTC->ISR	& RTC_ISR_INITF) != RTC_ISR_INITF );					// Moda gecene kadar bayrak okunuyor
	
	RTC->DR			=		(yil 				<< RTC_DR_YU_Pos	) |								
									(ay					<< RTC_DR_MU_Pos	) |								// Tek seferde, BCD olarak parametrelerin girilmesi gerekiyor
									(gun				<< RTC_DR_DU_Pos	) |
									(hafta_gun	<< RTC_DR_WDU_Pos )	;
	
	RTC->ISR		&=	~RTC_ISR_INIT;																	// Init moddan cikiliyor
	
	RTC->WPR		= 	0x23;																						// Yanlis key gir ve rtc birimini yazmalara kapat
	
}

void	Set_Alarm_A_BCD( uint8_t gun, uint8_t saat, uint8_t dakika, uint8_t saniye)
{																																	 // RTC Alarm A Fonksiyonu
	RTC->WPR		=  	0xCA;																						// RTC nin sifre korumasi kaldiriliyor
	RTC->WPR		=		0x53;																						// RTC nin sifre korumasi kaldiriliyor
	
	RTC->ISR   |=		RTC_ISR_INIT;																		// Init moduna gec, islem yapabilmek icin
	
	while( (RTC->ISR	& RTC_ISR_INITF) != RTC_ISR_INITF );					// Moda gecene kadar bayrak okunuyor
	
	RTC->CR		 &=		~RTC_CR_ALRAE;																	// Aktif olabilir o sebeple tebdir icin kapa ve sonrasi alarm kurmak icinde gerek
	
	RTC->ALRMAR =		( gun			<< RTC_ALRMAR_DU_Pos		) |
									( saat		<< RTC_ALRMAR_HU_Pos		) |						// Alarm zamani, tek seferde BCD olarak gir
									( dakika	<< RTC_ALRMAR_MNU_Pos		)	|
									( saniye	<< RTC_ALRMAR_SU_Pos		)	;
	
	RTC->CR		|=		RTC_CR_ALRAE;																		// RTC control reg. Alarm etkinlestiriliyor
	
	if ( (RTC->ISR & RTC_ISR_ALRAWF ) == RTC_ISR_ALRAWF )						// Alarm A' da  degisiklij var mi? Donanim kaldirir bayragi
	{
			RTC->CR			|=		RTC_CR_ALRAIE;														// Alarm A int aktiflestir
		
			NVIC_EnableIRQ( RTC_Alarm_IRQn );														// NVIC deki interrupt aktiflestir
		
			EXTI->IMR		|=		EXTI_IMR_IM17;														// Alarm A EXTI deki 17 .hat bagli, maskeleyerek etkinlestir
		
			EXTI->RTSR	|=		EXTI_RTSR_TR17;														// 17. hat yukselen kenarda aktif olsun
		
	}
	
	RTC->ISR		&=	~RTC_ISR_INIT;																	// Init moddan cikiliyor
	
	RTC->WPR		= 	0x23;																						// Yanlis key gir ve rtc birimini yazmalara kapat
}

