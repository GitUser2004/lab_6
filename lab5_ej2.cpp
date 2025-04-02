#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "utils/uartstdio.h"

uint32_t g_ui32SysClock;
uint8_t receivedChar;
float receivedValue;
int aux = 0;

#ifdef DEBUG
void error(char *pcFilename, uint32_t ui32Line) {}
#endif

void UARTIntHandler(void) {
    uint32_t ui32Status;
    ui32Status = MAP_UARTIntStatus(UART0_BASE, true);
    MAP_UARTIntClear(UART0_BASE, ui32Status);

    while (MAP_UARTCharsAvail(UART0_BASE)) {
        receivedChar = MAP_UARTCharGetNonBlocking(UART0_BASE);
        receivedValue = (float)(receivedChar) - '0';

        if (receivedValue == 0) {
            aux = 0;
        } 
        else if (receivedValue == 1) {
            aux = 1;
        }
        else if (receivedValue == 2) {
            aux = 2;
        }
        else if (receivedValue == 3) {
            aux = 3;
        }

        SysCtlDelay(g_ui32SysClock / (1000 * 3));
    }
}

void
PWM0Gen1IntHandler(void)
{

    MAP_PWMGenIntClear(PWM0_BASE, PWM_GEN_1, PWM_INT_CNT_LOAD);

}

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count) {
    while (ui32Count--) {
        MAP_UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
    }
}

int main(void) {
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN |
                                           SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_240), 120000000);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    //PF1 (M0PWM1)
    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    // Motor 2: PF2 (M0PWM2)
    GPIOPinConfigure(GPIO_PF2_M0PWM2);
    
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2);

    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);
    uint32_t pwmClock = g_ui32SysClock / 8;

    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    uint32_t pwmPeriod = pwmClock / 5000; // Frecuencia de 250 Hz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, pwmPeriod);

    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT | PWM_OUT_2_BIT, true);
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    MAP_UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 115200,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    MAP_IntEnable(INT_UART0);
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntMasterEnable();

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);//LED
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);//LED

    UARTSend((uint8_t *)"\033[2JControl de Motores PWM listo\n", 30);

    while (1) {
        /*
        if (aux == 1) {
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pwmPeriod / 10); //PF1
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pwmPeriod / 10); //PF2)
            
            MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); 
        } 
        else if (aux == 0) {
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pwmPeriod); //PF1)
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pwmPeriod); //PF2)
            
            MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0); 
        }*/
        if (aux == 1) {
            
            MAP_GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_PIN_5);
        }
        else{
            
            MAP_GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0);
        }
        

    }
}