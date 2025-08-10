/********************************** (C) COPYRIGHT *******************************
 * Sistema para medição de potência ativa e reativa de baixo custo
 *
 * Autor: Aluizio d'Affonsêca Netto
 *
 * Pinos:
 *       PA2/A0  (3) - Corrente
 *       PC4/A2  (7) - Tensão
 *       PC2/SCL (6) - DISPLAY OLED
 *       PC1/SDA (5) - DISPLAY OLED
 *
 *Rev.:   1 - 01-08-2025 Criação de projetoe testes inciais
 *Rev.:   2 - 10-08-2025 Versão publicada com correções e inicio de documentação
 *
 */

#include "debug.h"
#include "math.h"
#include "oled_049.h"

const float escala1 = (0.00789005); //escala de corrente
const float escala2 = (0.2393219);  //escala de tensão

//buffer para amostras do ADC
#define N_ADC_BUF (300)
#define N_ADC_BUF_2 (150)
#define N_ADC_BUF_3 (50)
volatile uint16_t adc_buf[N_ADC_BUF];

//variaveis de calculo de RMS
#define N_ADC_SUM (6000)
volatile int32_t adc_sum1 = 0, adc_sum1_r;
volatile int32_t adc_sum2 = 0, adc_sum2_r;
volatile int32_t adc_prod1_2 = 0, adc_prod1_2_r = 0;
volatile int32_t adc_sum_sq1 = 0, adc_sum_sq1_r;
volatile int32_t adc_sum_sq2 = 0, adc_sum_sq2_r;
volatile uint32_t adc_count = 0;
volatile uint8_t adc_f_calc = 0;
volatile int32_t adc_v1, adc_v2;

float v_rms1, v_adc1;
float v_rms2, v_adc2;
float p_adc1_2;
float fp_p;

//configuração de filtro DC para redirada de componente DC
#define N_M_ADC (128)
volatile int32_t m_adc_1 = 2048, m_adc_2 = 2048, ms_adc_1 = 0, ms_adc_2 = 0;
volatile uint32_t count_m_adc = 0;

#define N_S_DIV_SQ (6) //numero de bits do fator de escala para acumuladores de produtos e quadrados
#define S_DIV_SQ ((int32_t)1<<N_S_DIV_SQ)

void ADC_Function_Init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    //channel A0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //cahnnel A2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 3;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_CyclesMode7);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_CyclesMode7);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 3, ADC_SampleTime_CyclesMode7);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);  //habilita DMA
    ADC_Cmd(ADC1, ENABLE);
    ADC_BufferCmd(ADC1, ENABLE); //enable buffer
}


void TIM1_PWM_In(u16 arr, u16 psc)
{
    //GPIO_InitTypeDef        GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC | RCC_PB2Periph_TIM1, ENABLE);

    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    //GPIO_Init(GPIOC, &GPIO_InitStructure);

    //GPIO_WriteBit(GPIOC, GPIO_Pin_1, SET);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);


    NVIC_InitStructure.NVIC_IRQChannel =TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;
    NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
    TIM_Cmd(TIM1, ENABLE);
}


int32_t limite_valor(int32_t min, int32_t max, int32_t value) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

void calc_parcial_IRQ(uint32_t b) {

    static uint16_t kc;

    for (kc = 0; kc < N_ADC_BUF_3; kc++) {

        if (b){
            adc_v1 = (adc_buf[3*kc + N_ADC_BUF_2]+adc_buf[3*kc + 2 + N_ADC_BUF_2])>>1;
            adc_v2 = adc_buf[3*kc + 1 + N_ADC_BUF_2];
        }else {
            adc_v1 = ((adc_buf[3*kc]+adc_buf[3*kc+2])>>1);
            adc_v2 = adc_buf[3*kc+1];
        }


        adc_sum1 += adc_v1;
        adc_sum2 += adc_v2;

        //remove componente DC
        adc_v1 -= m_adc_1;
        adc_v2 -= m_adc_2;
        ms_adc_1 += adc_v1;
        ms_adc_2 += adc_v2;
        if(++count_m_adc >= N_M_ADC) {
            count_m_adc = 0;
            if (ms_adc_1 > 0) m_adc_1++;
            else m_adc_1--;

            if (ms_adc_2 > 0) m_adc_2++;
            else m_adc_2--;

            m_adc_1 = limite_valor(0,4095, m_adc_1);
            m_adc_2 = limite_valor(0,4095, m_adc_2);

            ms_adc_1 = 0;
            ms_adc_2 = 0;
        }

        adc_sum_sq1 += ((adc_v1*adc_v1) >> N_S_DIV_SQ);
        adc_sum_sq2 += ((adc_v2*adc_v2) >> N_S_DIV_SQ);
        adc_prod1_2 += ((adc_v1*adc_v2) >> N_S_DIV_SQ);
        adc_count++;
        if (adc_count>= N_ADC_SUM) {
            adc_sum1_r = adc_sum1;
            adc_sum2_r = adc_sum2;
            adc_sum_sq1_r = adc_sum_sq1;
            adc_sum_sq2_r = adc_sum_sq2;
            adc_prod1_2_r = adc_prod1_2;
            adc_prod1_2 = 0;
            adc_sum1 = 0;
            adc_sum2 = 0;
            adc_sum_sq1 = 0;
            adc_sum_sq2 = 0;
            adc_count = 0;
            adc_f_calc = 1;
        }

    }

}

//interrupção para DMA canal 1, Metade do buffer completo
void DMA1_Channel1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel1_IRQHandler(void) {



    //interrupção para metade do buffer
    if(DMA_GetITStatus(DMA1_IT_HT1)) {

        calc_parcial_IRQ(0);
        DMA_ClearITPendingBit(DMA1_IT_HT1);
    }

    //interrupção de buffer completo
    if(DMA_GetITStatus(DMA1_IT_TC1)) {

        calc_parcial_IRQ(1);

        //printf("ADC - %04d;%04d;%04d\r\n", TxBuf[0], TxBuf[1], TxBuf[2]);
        DMA_ClearITPendingBit(DMA1_IT_TC1);

    }

}

void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA_CHx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);

}

void printChar(char c) {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    USART_SendData(USART1, c);
}
void printString(char *buf) {
    while (*buf != '\0') {
        printChar(*buf++);
    }
}
void printInt(int32_t num) {
    int32_t i = 0;
    char temp[12];

    if(num == 0) {
        printChar('0');
        return;
    }

    if(num < 0) {
        printChar('-');
        num = -num;
    }

    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    //envia carcateres na ordem invetida
    while (--i >= 0) {
        printChar(temp[i]);
    }

}

void printFloat(float v, int decimalDigits)
{
    int32_t kd = 1;
    int32_t intPart, fractPart;
    for (;decimalDigits!=0; kd*=10, decimalDigits--);
    intPart = (int32_t)v;
    fractPart = (int32_t)((v-(float)(int32_t)v)*kd);
    if(fractPart < 0) fractPart *= -1;
    //printf("%d.%d", intPart, fractPart);
    printInt(intPart);
    printChar('.');
    if (fractPart > 0) {
        kd/=10;
        while(fractPart < kd){
            printChar('0');
            kd/=10;
        }
        printInt(fractPart);
    }
    else {
        printChar('0');
    }

}



/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    //printf("SystemClk:%d\r\n", SystemCoreClock);
    //printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    printString("ADC TIM \r\n");
    printString("SystemClk:");
    printInt(SystemCoreClock);
    printString("\r\n");

    ssd1306Init();
    ssd1306Fill(0x00);
    ssd1306Print(0,0,"POWER"); // 10 caracteres máximo

    ADC_Function_Init();
    DMA_Tx_Init(DMA1_Channel1, (u32)&ADC1->RDATAR, (u32)adc_buf, N_ADC_BUF);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);
    DMA_ITConfig(DMA1_Channel1,DMA_IT_HT,ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);


    //TIM1_PWM_In(1000-1,48000-1);
    TIM1_PWM_In(1000-1,8-1);//frequencia 48e6/(1000*6) = 8000Hz

    while(1)
    {
        if(adc_f_calc) {

            v_adc1 = (float)adc_sum1_r/((float)N_ADC_SUM);
            v_adc2 = (float)adc_sum2_r/((float)N_ADC_SUM);
            //v_rms1 = sqrtf(adc_sum_sq1_r/((float)N_ADC_SUM) - v_adc1*v_adc1);
            //v_rms2 = sqrtf(adc_sum_sq2_r/((float)N_ADC_SUM) - v_adc2*v_adc2);
            v_rms1 = sqrtf((float)S_DIV_SQ*adc_sum_sq1_r/((float)N_ADC_SUM));
            v_rms2 = sqrtf((float)S_DIV_SQ*adc_sum_sq2_r/((float)N_ADC_SUM));
            p_adc1_2 =(float)S_DIV_SQ*(float)adc_prod1_2_r /((float)N_ADC_SUM);

            //ajuste deescalas
            v_adc1 = v_adc1 * escala1;
            v_adc2 = v_adc2 * escala2;
            v_rms1 = v_rms1 * escala1;
            v_rms2 = v_rms2 * escala2;
            p_adc1_2 = p_adc1_2*escala1*escala2;


            //printf( "sq1 %d, adc1 %d\r\n", adc_sum_sq1_r, m_adc_1);
            printString("sq1 "); printInt(adc_sum_sq1_r); printString(" adc1 "); printInt(m_adc_1); printString("\r\n");
            printString("dc1 "); printFloat(v_adc1, 3); printString(" rms1 "); printFloat(v_rms1, 3); printString("\r\n");
            //printf( "sq2 %d, adc2 %u\r\n", adc_sum_sq2_r, m_adc_2);
            printString("sq2 "); printInt(adc_sum_sq2_r); printString(" adc2 "); printInt(m_adc_2); printString("\r\n");
            printString("dc1 "); printFloat(v_adc2, 3); printString(" rms2 "); printFloat(v_rms2, 3); printString("\r\n");

            if (v_rms1 > 0.01 && v_rms2 > 0.01 && fabsf(p_adc1_2) > 0.1) {
                fp_p = p_adc1_2 /(v_rms1*v_rms2);
            }else {
                fp_p = 0;
            }
            printString( "p "); printFloat(p_adc1_2,3); printString( " fp "); printFloat(fp_p,3);
            printString( "\r\n\r\n");


            //valores no display
            //ssd1306Fill(0x00);
            ssd1306Print(0,0,"I        "); ssd1306PrintFloat(0,12, v_rms1, 3);
            ssd1306Print(1,0,"V        "); ssd1306PrintFloat(1,12, v_rms2, 1);
            ssd1306Print(2,0,"P        "); ssd1306PrintFloat(2,12, p_adc1_2, 2);
            ssd1306Print(3,0,"FP       "); ssd1306PrintFloat(3,18, fp_p, 2);



            adc_f_calc = 0;
        }
    }
}
