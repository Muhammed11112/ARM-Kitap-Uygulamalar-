#include "sensor.h"
#include "stm32f4xx.h"

extern SPI_HandleTypeDef hspi1;
extern int8_t		x,y,z;																								// Eksen datalari için isaretli 1 Bayt degiskenler
extern uint8_t		status,	status_1;	

void	LIS3DSH_Init()																						// MODULU BASLATMA VE AYAR FONK
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();								// INT1-PE0 baglidir, EXTI birimi  SYSCFG kullaniyor. Clock aktif yap
	
	GPIO_InitTypeDef MyInterruptPin = {0};
	/*
	SYSCFG->EXTICR[0]		|=		(SYSCFG_EXTICR1_EXTI0_PE <<					// EXTI. INT port ayari yapiliyor (3. Pine kadar Exti0 da lar)
														 SYSCFG_EXTICR1_EXTI0_Pos );	
	
	EXTI->IMR						|=		EXTI_IMR_IM0;												// External Int. 0  HATTI aktif ediliyor
	
	EXTI->RTSR					|=		EXTI_RTSR_TR0;											// Yukselen kenarda kesme olacak
	*/
	  /*Configure GPIO pin : PE0 */
  MyInterruptPin.Pin = GPIO_PIN_0;
  MyInterruptPin.Mode = GPIO_MODE_IT_RISING;
  MyInterruptPin.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &MyInterruptPin);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	
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

uint8_t	SPI_LIS3DSH_Rx( uint8_t adr)														// SPI ILE MODULDEN DATA OKUMA FONK
{

	uint8_t			rx  =			0;																			// Modulden alinacak data
	adr						 |= 		0x80;																		// Yazma icin 7. Bit bir yapiliyor
	
	HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);				// CS pini LOW yapiliyor, haberlesme basliyor
	
	HAL_SPI_Transmit( &hspi1, &adr,  1, 10);											// Okuma yapilacak register gonderiliyor
	HAL_SPI_Receive ( &hspi1, &rx,  1,  10);											// Data RX e aliniyor, 8+8 16 clock gonderilmis oluyor
	
	HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_SET);					// CS pini tekrar high yapiliyor
	

	return rx;																										//	okunani dondur
}


void	SPI_LIS3DSH_Tx( uint8_t	adr,	uint8_t data)								// SPI ILE MODULE DATA GONDERME FONK 
{
	HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);				// CS pini LOW yapiliyor, haberlesme basliyor
	
	HAL_SPI_Transmit( &hspi1, &adr,  1, 10);											// Register adresi
	HAL_SPI_Transmit( &hspi1, &data, 1, 10);											// Register'a yazilacak data
	
	HAL_GPIO_WritePin( GPIOE, GPIO_PIN_3, GPIO_PIN_SET);					// CS pini tekrar high yapiliyor
	
}


void	spi1_init()																								// SPI AYAR ve BASLATMA FONK
{
	__HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_SPI1_CLK_ENABLE();																			// SPI clock hatti aciliyor
	
	GPIO_InitTypeDef MySPIpin = {0};
	
	MySPIpin.Pin		= GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;				// spin ayarla
	MySPIpin.Mode		= GPIO_MODE_AF_PP;
	MySPIpin.Speed	= GPIO_SPEED_FREQ_HIGH;
	MySPIpin.Pull		= GPIO_NOPULL;
	MySPIpin.Alternate		= GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOA, &MySPIpin);
	
	/*
	GPIOA->MODER		|=  	( 2 << GPIO_MODER_MODE5_Pos);																	// A pinleri Alternate Mod alindi
	GPIOA->MODER		|=	  ( 2 << GPIO_MODER_MODE6_Pos);
	GPIOA->MODER		|=		( 2 << GPIO_MODER_MODE7_Pos);
	
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL5_Pos);																	// Pinler SPI ile iliskilendiriliyor
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL6_Pos);
	GPIOA->AFR[0]		|=		( 5 << GPIO_AFRL_AFSEL7_Pos);
	*/
	
	
  //__HAL_RCC_GPIOE_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	MySPIpin.Pin 	= GPIO_PIN_3;
  MySPIpin.Mode 	= GPIO_MODE_OUTPUT_PP;
  MySPIpin.Pull 	= GPIO_NOPULL;
  MySPIpin.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &MySPIpin);
	
	/*
	GPIOE->BSRR			|=		GPIO_BSRR_BS3;																								// CS, PE3 e bagli. LOW oldugunda haberlesme yapacak o sebeple SET yap haberlesmeyi bekle
	GPIOE->MODER		|= 		( 1 << GPIO_MODER_MODE3_Pos);																	// CS pini output yapiliyor
	*/
	hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1) ;
	
	/*
	SPI1->CR1				 =		(SPI_CR1_SSM) | (SPI_CR1_SSI) |																// NSS pin yonetimi yazilimsal sagla, SSM-SSI pinleri SET ediliyor
												(7 << SPI_CR1_BR_Pos) | (SPI_CR1_MSTR) |											// BaudRATE icin prescaler = 256 yap, SPI MASTER olarak calisacak
												(SPI_CR1_CPOL) | (SPI_CR1_CPHA) ;															// CPOL Set = Haberlesme olmadiginda HIGH olacak
																																											// CPHA Set = Data yakalama Clock sinyalinin 2. kenarinda olacak
	SPI1->CR1				|=		(SPI_CR1_SPE) ;			
	*/
	__HAL_SPI_ENABLE(&hspi1);			// SPI1 enable ediliyor
}

void		EXTI0_IRQHandler( void)																	// EXTI0 INT FONK
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_0);																// EXTIbiriminde EXTI0 bayragi degisiyor
	
	status			 =		SPI_LIS3DSH_Rx( FIFO_SRC);									// Data okumadan once FIFO Kontrol
	
	x 					 = 		SPI_LIS3DSH_Rx( OUT_X);											// Eksen datalarini al
	y 					 = 		SPI_LIS3DSH_Rx( OUT_Y);
	z 					 = 		SPI_LIS3DSH_Rx( OUT_Z);
	
	status_1		 =		SPI_LIS3DSH_Rx( FIFO_SRC);									// Eksen datalari bir defa okunduktan sonra FIFO derinligi 1 azalacak

}
