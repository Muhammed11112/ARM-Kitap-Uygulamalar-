#include "main.h"
#include "sensor.h"

int8_t		x,y,z;																								// Eksen datalari için isaretli 1 Bayt degiskenler
uint8_t		status,	status_1;																			

SPI_HandleTypeDef hspi1;

void GPIO_Init(void);																						// PIN AYAR ve BASLATMA FONK


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
	
	spi1_init();																									// SPI ayarlari																	
	LIS3DSH_Init();																								// Sensor Ayarlari
	
	//Eksen bilgileri kontrol edilecek, ilgili ekseni simgeleyen LED ON konumuna getiriclecek
  while (1)
  {
	
	if ( x < -10) 		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, 1);		// Eksenlere gore LED ler SETleniyor
	else							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, 0);
	
	if ( x > 10) 		  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 1);		// Eksenlere gore LED ler SETleniyor
	else							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
	
	if ( y < -10) 	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, 1);		// Eksenlere gore LED ler SETleniyor
	else							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, 0);
	
	if ( y > 10) 		  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, 1);		// Eksenlere gore LED ler SETleniyor
	else							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, 0);
		
  }
}

void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  __HAL_RCC_GPIOD_CLK_ENABLE();	// clock aktif
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin 	= GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;	// pin 12-13-14-15 D portu
  GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull 	= GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}







