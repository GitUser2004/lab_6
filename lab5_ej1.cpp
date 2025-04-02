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
void _error_(char *pcFilename, uint32_t ui32Line) {}
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

        SysCtlDelay(g_ui32SysClock / (1000 * 3));
    }
}

void
PWM0Gen1IntHandler(void)
{

    MAP_PWMGenIntClear(PWM1_BASE, PWM_GEN_1, PWM_INT_CNT_LOAD);
    MAP_PWMGenIntClear(PWM1_BASE, PWM_GEN_1, PWM_INT_CNT_LOAD);

}

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count) {
    while (ui32Count--) {
        MAP_UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
    }
}

int main(void) {
    // Configurar reloj del sistema
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN |
                                           SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_240), 120000000);

    // Habilitar periféricos
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configurar PWM para dos motores (PF1 y PF2)
    // Motor 1: PF1 (M0PWM1)
    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    // Motor 2: PF2 (M0PWM2)
    GPIOPinConfigure(GPIO_PF2_M0PWM2);
    
    // Configurar pines como salidas PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2);

    // Configurar reloj PWM (división por 8)
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);
    uint32_t pwmClock = g_ui32SysClock / 8;

    // Configurar generador PWM (compartido para ambos canales)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    uint32_t pwmPeriod = pwmClock / 5000; // Frecuencia de 250 Hz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, pwmPeriod);

    // Habilitar salidas PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT | PWM_OUT_2_BIT, true);
    
    // Iniciar generador PWM
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    // Configurar UART
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    MAP_UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 115200,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Configurar interrupciones UART
    MAP_IntEnable(INT_UART0);
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntMasterEnable();

    // Configurar pines GPIO para control adicional
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0); // LED
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1); // Motores

    UARTSend((uint8_t *)"\033[2JControl de Motores PWM listo\n", 30);

    while (1) {
        if (aux == 1) {
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pwmPeriod / 10); // Motor 2 (PF2, M0PWM2)
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pwmPeriod / 10); // Motor 1 (PF1, M0PWM1)

            
            
            // Encender motores (activar enable si es necesario)
            MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); // LED encendido
        } 
        else if (aux == 0) {
            // Modo 0: Duty cycle 100% para ambos motores
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pwmPeriod); // Motor 1 (PF1)
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pwmPeriod); // Motor 2 (PF2)
            
            // Apagar motores
            MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0); // LED apagado
        }
    }
}