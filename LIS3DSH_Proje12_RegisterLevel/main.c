#include "stm32f4xx.h"                  												// Device header

#define			WHO_AM_I			(0X0F)																// Registerlar isimleriyle birlikte tanimlaniyor
#define			CTRL_REG3			(0X23)
#define			CTRL_REG4			(0X20)
#define			CTRL_REG5			(0X24)
#define			CTRL_REG6			(0X25)
#define			FIFO_CTRL			(0X2E)
#define			OUT_X					(0X29)
#define			OUT_Y					(0X2B)
#define			OUT_Z					(0X2D)
#define			FIFO_SRC 			(0X2F)

int8_t		x,y,z;																								// Eksen datalari için isaretli 1 Bayt degiskenler
uint8_t		status,	status_1;																			// FIFO durumlarini izleyecegimiz  degiskenler

void 		SetClock( void);																				// CLOCK AYAR FONKSIYONLARI									
void		SPI_LIS3DSH_Tx( uint8_t	adr,	uint8_t data);						// SPI ILE MODULE DATA GONDERME FONK 
uint8_t	SPI_LIS3DSH_Rx( uint8_t adr);														// SPI ILE MODULDEN DATA OKUMA FONK
void		spi1_init( void);																				// SPI AYAR ve BASLATMA FONK
void		LIS3DSH_Init( void);																		// MODULU BASLATMA VE AYAR FONK

void		EXTI0_IRQHandler( void)																	// EXTI0 INT FONK
{
	EXTI->PR		|=		EXTI_PR_PR0;																// EXTIbiriminde EXTI0 bayragi degisiyor
	
	status			 =		SPI_LIS3DSH_Rx( FIFO_SRC);									// Data okumadan once FIFO Kontrol
	
	x 					 = 		SPI_LIS3DSH_Rx( OUT_X);											// Eksen datalarini al
	y 					 = 		SPI_LIS3DSH_Rx( OUT_Y);
	z 					 = 		SPI_LIS3DSH_Rx( OUT_Z);
	
	status_1		 =		SPI_LIS3DSH_Rx( FIFO_SRC);									// Eksen datalari bir defa okunduktan sonra FIFO derinligi 1 azalacak
	
	if ( x < -10) 		GPIOD->BSRR		=		GPIO_BSRR_BS12;						// Eksenlere gore LED ler SETleniyor
	else							GPIOD->BSRR		=		GPIO_BSRR_BR12;
	
	if ( x > 10) 		  GPIOD->BSRR		=		GPIO_BSRR_BS14;
	else							GPIOD->BSRR		=		GPIO_BSRR_BR14;
	
	if ( y < -10) 	  GPIOD->BSRR		=		GPIO_BSRR_BS15;
	else							GPIOD->BSRR		=		GPIO_BSRR_BR15;
	
	if ( y > 10) 		  GPIOD->BSRR		=		GPIO_BSRR_BS13;
	else							GPIOD->BSRR		=		GPIO_BSRR_BR13;

}

int main()
{
	SetClock();																										// Clock ayarlari
	
	SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);									// PortD Clock acildi
	
	GPIOD->MODER			|=	( 1 << GPIO_MODER_MODE12_Pos) |					// Pinler Output olarak atandi
												( 1 << GPIO_MODER_MODE13_Pos) |
												( 1 << GPIO_MODER_MODE14_Pos) |
												( 1 << GPIO_MODER_MODE15_Pos) ;
	
	spi1_init();																									// SPI Baslangic Ayarlari
	
	LIS3DSH_Init();																								// Modul baslangic ayarlari
	
	while (1)
	{
		// Sonsuz Dongu...
	}
	
}

void	LIS3DSH_Init()																						// MODULU BASLATMA VE AYAR FONK
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);									// INT1-PE0 baglidir, EXTI birimi  SYSCFG kullaniyor. Clock aktif yap
	
	SYSCFG->EXTICR[0]		|=		(SYSCFG_EXTICR1_EXTI0_PE <<					// EXTI. INT port ayari yapiliyor (3. Pine kadar Exti0 da lar)
														 SYSCFG_EXTICR1_EXTI0_Pos );	
	
	EXTI->IMR						|=		EXTI_IMR_IM0;												// External Int. 0  HATTI aktif ediliyor
	
	EXTI->RTSR					|=		EXTI_RTSR_TR0;											// Yukselen kenarda kesme olacak
	
	NVIC_EnableIRQ(EXTI0_IRQn);																		// EXTI0 Genel kesme izinleme
	
	SPI_LIS3DSH_Tx( CTRL_REG6, 0x80);															// Modulde programin her baslangicinda YENIDEN BASLATMA yapiliyor
	
	SPI_LIS3DSH_Tx( CTRL_REG3,	1	 );															// Yazilimsal Reset atiliyor
	
	for( int i = 0; i < 0xFF; i++);																// Reseti bekle biraz
	
	if( SPI_LIS3DSH_Rx( WHO_AM_I) == 0x3F)												// Modul ile haberlesmeyi test ediyor. Dogru mu?
	{
		SPI_LIS3DSH_Tx( CTRL_REG3, 0x48);														// HIGH olunca int olacak ve INT1 pini yuksek yapiliyor
		SPI_LIS3DSH_Tx( CTRL_REG4, 0x47);														// Cikis Data olcegi ayarlaniyor, X Y Z aktif ediliyor
		SPI_LIS3DSH_Tx( CTRL_REG5, 0x48);														// Tam olcek secenegi ve filtre secimi yapiliyor (Perf arttirimi, olmadan da bu ayar yapilir
		SPI_LIS3DSH_Tx( CTRL_REG6, 0x64);														// FIFO ve derinlgi ayarlanabilidigi WATERMAK  aktiflestiriliyor, FIFO watermak int aktif ediliyor
		SPI_LIS3DSH_Tx( FIFO_CTRL, 0x41);														// FIFO akis modunda aktif, Watermak =1( xyz okudnugudna int olusacak), watermak 
																																// WATERMAK degeri degiserek INT olusma suresi ve FIFO genislifgi kadar data okunabilir
	}
}
void	spi1_init()																								// SPI AYAR ve BASLATMA FONK
{
	SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);																				// SCK MOSI MISO pinleri burada old icin A yi Aktif et
	SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN);																				// CS Pini E portuna bagli Aktif ediliyor
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_SPI1EN	);																				// SPI clock hatti aciliyor
		
	GPIOA->MODER		|=  	( 2 << GPIO_MODER_MODE5_Pos);																	// A pinleri Alternate Mod alindi
	GPIOA->MODER		|=	  ( 2 << GPIO_MODER_MODE6_Pos);
	GPIOA->MODER		|=		( 2 << GPIO_MODER_MODE7_Pos);
	
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL5_Pos);																	// Pinler SPI ile iliskilendiriliyor
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL6_Pos);
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL7_Pos);
	
	GPIOE->BSRR			|=		GPIO_BSRR_BS3;																								// CS, PE3 e bagli. LOW oldugunda haberlesme yapacak o sebeple SET yap haberlesmeyi bekle
	GPIOE->MODER		|= 		( 1 << GPIO_MODER_MODE3_Pos);																	// CS pini output yapiliyor
	
	SPI1->CR1				 =		(SPI_CR1_SSM) | (SPI_CR1_SSI) |																// NSS pin yonetimi yazilimsal sagla, SSM-SSI pinleri SET ediliyor
												(7 << SPI_CR1_BR_Pos) | (SPI_CR1_MSTR) |											// BaudRATE icin prescaler = 256 yap, SPI MASTER olarak calisacak
												(SPI_CR1_CPOL) | (SPI_CR1_CPHA) ;															// CPOL Set = Haberlesme olmadiginda HIGH olacak
																																											// CPHA Set = Data yakalama Clock sinyalinin 2. kenarinda olacak
	SPI1->CR1				|=		(SPI_CR1_SPE) ;																								// SPI1 enable ediliyor

}
uint8_t	SPI_LIS3DSH_Rx( uint8_t adr)														// SPI ILE MODULDEN DATA OKUMA FONK
{
	adr							|=		0x80;																		// Modul okumasinda 7. Bit 0 olmalidir
	
	uint8_t		rx		=			0;																			// Modulden alinacak data için degisken
	
	GPIOE->BSRR			|=		GPIO_BSRR_BR3;													// Haberlesme baslasin CS = 0
	
	SPI1->DR				 =		adr;																		// Okuma yapilacak register adresi DR' ye yukle
	
	while( ! ( SPI1->SR	& SPI_SR_RXNE));													// Data gonder hattin okunmasini bekle, RXNE bos olana kadar bekle
	
	rx							=			SPI1->DR;																// Gecersiz data degiskene al, RXNE bayragi temizleniyor
	
	SPI1->DR				= 		0;																			//	DR 0 yaz module gonder, 8 clock a ihtiyaci var, istedigine de gonderebilirsin
	
	while( ! ( SPI1->SR	& SPI_SR_RXNE));													//  Data gonderdikten sonra hattin okunmasini bekle, RXNE = 1 data okundu demek
	
	rx 							= 		SPI1->DR;																//	Okunan datamiz degiskene aliniyor
	
	GPIOE->BSRR			=			GPIO_BSRR_BS3;													// 	CS = 1 yap haberlesme bitsin
	
	return rx;																										//	okunani dondur
}


void	SPI_LIS3DSH_Tx( uint8_t	adr,	uint8_t data)								// SPI ILE MODULE DATA GONDERME FONK 
{
	uint8_t			void_Rx = 	0;																		// Modul gondermez, RXNE bayragi icin hattan alinacak data icin degisken
	
	GPIOE->BSRR					|=	GPIO_BSRR_BR3;												// Modulu CS pini  = 0, haberlesmeyi baslatmak icin

	SPI1->DR						= 	adr;																	// Modulde Yazma islemi yapacak adres, MODULE gidiyor 7.BIT = 0
	
	while( ! ( SPI1->SR & SPI_SR_RXNE));													// Data gonder, hattin okunmasini bekle	RXNE set olana kadar bekle
	
	void_Rx 						=		SPI1->DR;															// Gecersiz data degiskene al, RXNE bayrak temzile
	
	SPI1->DR						=		data;																	// 
	
	while( ! ( SPI1->SR & SPI_SR_RXNE));													// RXNE Set olana kadar bekle, gecersiz data oku, RXNE bayrak temizle
	
	void_Rx							=		SPI1->DR;															// Register adresine yazilacak olan data  degiskeni
	
	GPIOE->BSRR					|=	GPIO_BSRR_BS3;												// Yazma gerceklestigi icin CS = 1 oluyor
	
}

void SetClock(void)																											    /* System Clock Ayar Fonk.*/
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

