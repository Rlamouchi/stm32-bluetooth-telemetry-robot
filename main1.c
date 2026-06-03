#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>

// DS1621 Constants
#define DS1621_ADDR 0x90

// Global Variables
char buffer[10];
volatile int i = 0;
uint16_t adc[3];
volatile uint16_t k = 0;
char status_buff[80];
int8_t temp_c = 0;

// I2C
void I2C_Init(void) {
    RCC->AHB1ENR |= (1 << 1);          // GPIOB clock
    RCC->APB1ENR |= (1 << 21);         // I2C1 clock    

    GPIOB->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB->MODER |=  ((2 << (6 * 2)) | (2 << (7 * 2)));
    GPIOB->OTYPER |= (1 << 6) | (1 << 7);              
    GPIOB->PUPDR |= (1 << (6 * 2)) | (1 << (7 * 2));    
   
    GPIOB->AFR[0] &= ~((0xF << 24) | (0xF << 28));
    GPIOB->AFR[0] |=  ((4 << 24) | (4 << 28));          
 
    I2C1->CR1 |= (1 << 15);            
    I2C1->CR1 &= ~(1 << 15);
    I2C1->CR2 = 16;                    
    I2C1->CCR = 80;                    
    I2C1->TRISE = 17;
    I2C1->CR1 |= (1 << 0);            
}

int I2C_Start(void) {
    uint32_t timeout = 10000;
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0))) {
        if (--timeout == 0) return 1;
    }
    return 0;
}

int I2C_WriteAddr(uint8_t addr) {
    uint32_t timeout = 10000;
    I2C1->DR = addr;
    while (!(I2C1->SR1 & (1 << 1))) {
        if (--timeout == 0) return 1;
    }
    (void)I2C1->SR1;                  
    (void)I2C1->SR2;
    return 0;
}

void I2C_SendData(uint8_t data) {
    uint32_t timeout = 10000;
    while (!(I2C1->SR1 & (1 << 7))) { if (--timeout == 0) break; }
    I2C1->DR = data;
    timeout = 10000;
    while (!(I2C1->SR1 & (1 << 2))) { if (--timeout == 0) break; }
}

void DS1621_Init_Sensor(void) {
    if (I2C_Start() != 0) return;
    I2C_WriteAddr(DS1621_ADDR);
    I2C_SendData(0xEE);                
    I2C1->CR1 |= (1 << 9);            
}

int8_t DS1621_Read(void) {
    int8_t val = 0;
    if (I2C_Start() != 0) return -127;
    if (I2C_WriteAddr(DS1621_ADDR) != 0) { I2C1->CR1 |= (1 << 9); return -127; }
    I2C_SendData(0xAA);                
    I2C_Start();                      
    I2C_WriteAddr(DS1621_ADDR | 0x01);
    I2C1->CR1 &= ~(1 << 10);          
    I2C1->CR1 |= (1 << 9);            
    uint32_t timeout = 10000;
    while (!(I2C1->SR1 & (1 << 6))) { if (--timeout == 0) break; }
    val = I2C1->DR;
    return val;
}

// ADC, USART, TIMERS

void GPIO_Config(void){
    RCC->AHB1ENR |= (1<<0) | (1<<1);
    // USART2 Pins (PA2, PA3)
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |=  ((2 << 4) | (2 << 6));
    GPIOA->AFR[0] |= (7 << 8) | (7 << 12);
    // ADC Pins (PA0, PA1, PA4)
    GPIOA->MODER |= (3<<0) | (3<<2) | (3<<8);
}

void usart_2(void){
    RCC->APB1ENR |= (1<<17);
    USART2->BRR = 0x0683;  
    USART2->CR1 |= (1<<2) | (1<<3) | (1<<5) | (1<<13); // TE, RE, RXNEIE, UE
    NVIC_EnableIRQ(USART2_IRQn);
}

void ADC_1(void){
    RCC->APB2ENR |= (1<<8);             //cloak
    ADC->CCR |= (1<<16);              // ADC PRESCALER /4
    ADC1->SQR1 |= (2<<20); //         L=2          
    ADC1->SQR3 |= (0<<0) | (1<<5) | (4<<10);
    ADC1->CR1 |= (1<<5) | (1<<8) | (1<<11); //EOCIE SCAN DISC
    ADC1->CR2 |= (1<<10) | (1<<28) | (6<<24) | (1<<0); //EOCS TIMER TRIGGER ENABLE EXTERNE TRIGGER ADC ON
    NVIC_EnableIRQ(ADC_IRQn);
}

void time_2(void){
    RCC->APB1ENR |= (1<<0);
    TIM2->PSC = 15999;
    TIM2->ARR = 1999;
    TIM2->CR2 |= (2 << 4);            
    TIM2->CR1 |= (1<<0);
}

void config_TIM3(void) {
    RCC->APB1ENR |= (1 << 1);  
    // PA6, PA7
    GPIOA->MODER |= (2 << (6 * 2)) | (2 << (7 * 2));
    GPIOA->AFR[0] |= (2 << 24) | (2 << 28);
    // PB0, PB1
    GPIOB->MODER |= (2 << (0 * 2)) | (2 << (1 * 2));
    GPIOB->AFR[0] |= (2 << 0) | (2 << 4);

    TIM3->PSC = 15; TIM3->ARR = 1999;      
    TIM3->CCMR1 = 0x6868; TIM3->CCMR2 = 0x6868;  
    TIM3->CCER = 0x1111; TIM3->CR1 |= (1 << 0);
}



void usart2_SendString(char *s) {
    while (*s) {
        while (!(USART2->SR & (1 << 7)));
        USART2->DR = *s++;
    }
}

//  MOVEMENTS
void avance(void)  { TIM3->CCR1 = 1000; TIM3->CCR2 = 0;    TIM3->CCR3 = 1000; TIM3->CCR4 = 0;    }
void arriere(void) { TIM3->CCR1 = 0;    TIM3->CCR2 = 700; TIM3->CCR3 = 0;    TIM3->CCR4 = 700; }
void gauche(void)  { TIM3->CCR1 = 900; TIM3->CCR2 = 0;    TIM3->CCR3 = 400;  TIM3->CCR4 = 0;    }
void droite(void)  { TIM3->CCR1 = 400;  TIM3->CCR2 = 0;    TIM3->CCR3 =900; TIM3->CCR4 = 0;    }
void stop(void)    { TIM3->CCR1 = 0;    TIM3->CCR2 = 0;    TIM3->CCR3 = 0;    TIM3->CCR4 = 0;    }



int main(void){
    GPIO_Config();
    usart_2();
    time_2();
    ADC_1();
    config_TIM3();
    I2C_Init();        
    DS1621_Init_Sensor();
   
    while(1){
        temp_c = DS1621_Read();
       
       
        sprintf(status_buff, "TEMP:%d C0:%d C1:%d C4:%d\r\n",
                temp_c, adc[0], adc[1], adc[2]);
        usart2_SendString(status_buff);
       
        for(int d=0; d<1500000; d++); // delay
    }
}



void ADC_IRQHandler(void){
    if(ADC1->SR & (1<<1)){
        adc[k++] = ADC1->DR;
        if (k > 2) k = 0;
    }
}

void USART2_IRQHandler(void) {
    if (USART2->SR & (1 << 5)) {
        char p = USART2->DR;
        if (p == '\r' || p == '\n') return;

        buffer[i++] = p;

        if (i >= 2) {
            buffer[2] = '\0';
            if (strcmp(buffer, "aa") == 0) { usart2_SendString("ad1\r\n"); avance(); }
            else if (strcmp(buffer, "rr") == 0) { usart2_SendString("ad2\r\n"); arriere(); }
            else if (strcmp(buffer, "gg") == 0) { usart2_SendString("cd1\r\n"); gauche(); }
            else if (strcmp(buffer, "dd") == 0) { usart2_SendString("cd2\r\n"); droite(); }
            else if (strcmp(buffer, "ss") == 0) { usart2_SendString("TR0\r\n"); stop(); }
           
            i = 0;
            memset(buffer, 0, sizeof(buffer));
        }
    }
	}