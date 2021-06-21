/*
    \file   main.c
    \brief  RTC PIT Wake from Sleep
    (c) 2018 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/cpufunc.h>

FUSES = {
	.WDTCFG = 0x00, // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00, // BODCFG {SLEEP=DIS, ACTIVE=DIS, SAMPFREQ=1KHz, LVL=BODLEVEL0}
	//.OSCCFG = 0x7E, // OSCCFG {FREQSEL=20MHZ, OSCLOCK=CLEAR}
	.TCD0CFG = 0x00, // TCD0CFG {CMPA=CLEAR, CMPB=CLEAR, CMPC=CLEAR, CMPD=CLEAR, CMPAEN=CLEAR, CMPBEN=CLEAR, CMPCEN=CLEAR, CMPDEN=CLEAR}
	.SYSCFG0 = 0xF6, // SYSCFG0 {EESAVE=CLEAR, RSTPINCFG=UPDI, CRCSRC=NOCRC}
	.SYSCFG1 = 0xF8, // SYSCFG1 {SUT=0MS}
	.APPEND = 0x00, // APPEND {APPEND=User range:  0x0 - 0xFF}
	.BOOTEND = 0x00, // BOOTEND {BOOTEND=User range:  0x0 - 0xFF}
};

LOCKBITS = 0xC5; // {LB=NOLOCK}


void RTC_init(void);
void LED0_init(void);
inline void LED0_toggle(void);
void SLPCTRL_init(void);
void MAIN_PRESCALER(char division);

void RTC_init(void)
{
     CPU_CCP = CCP_IOREG_gc;                        //UNLOCK FOR CLOCK ALTERING
     //CLKCTRL.OSC32KCTRLA |=    0b00000010;            //SET 32khz clock source.
     CLKCTRL.OSC32KCTRLA |=    CLKCTRL_CLKSEL_OSCULP32K_gc;

     
    while (RTC.PITSTATUS > 0){}    
    RTC.DBGCTRL = RTC_DBGRUN_bm;        //SET TO RUN IN DEBUG.
       RTC.CLKSEL |= RTC_CLKSEL_INT1K_gc;            //SETS CLOCK FROM OSCULP32K @ 1.024kHz
       RTC.PITINTCTRL = RTC_PI_bm;            //PIT INTERRUPT ENABLE (NOT RUN ENABLE!)
       RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;            //SETS UP FOR XXX cycles, PIT DISABLED.
       
     //RTC.PITCTRLA |=         0b00000001;            //**FOR TESTITNG - ENABLES PIT - DELETE LATER
}

ISR(RTC_PIT_vect)
{
    /* Clear flag by writing '1': */
    RTC.PITINTFLAGS = RTC_PI_bm;
    
    LED0_toggle();
}

void LED0_init(void)
{
    /* Make High (OFF) */
    PORTA.OUT |= 0x80;
    /* Make output */
    PORTA.DIR |= 0x80;
}

inline void LED0_toggle(void)
{
    PORTA.OUT ^= 0x80;
}

ISR(RTC_PIT_vect)
{
    /* Clear flag by writing '1': */
    RTC.PITINTFLAGS = RTC_PI_bm;
    
    LED0_toggle();
}

void SLPCTRL_init(void)
{
    SLPCTRL.CTRLA |= SLPCTRL_SMODE_PDOWN_gc;
    SLPCTRL.CTRLA |= SLPCTRL_SEN_bm;
}

void MAIN_PRESCALER(char division)
{
    CPU_CCP = CCP_IOREG_gc; //unlock ioreg
    CLKCTRL_MCLKCTRLA  =  CLKCTRL_CLKSEL_OSC20M_gc; //0x80;
    
    CLKCTRL_MCLKCTRLB  &= 0x1f;
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL_MCLKCTRLB  = division<<1;
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL_MCLKCTRLB |= 0x01;
}

int main(void)
{
    PORTA.DIR |= 0x80; //initialisaton led
	
	char DIV2 = 0x0;
    char DIV4 = 0x1;
    char DIV8 = 0x2;
    char DIV16 = 0x3;
    char DIV32 = 0x4;
    char DIV64 = 0x5;
    char DIV6 = 0x8;
    char DIV10 = 0x09;
    char DIV12 = 0xA;
    char DIV24 = 0xB;
    char DIV48 = 0xC;
	
    RTC_init();
    SLPCTRL_init();
    
    /* Enable Global Interrupts */
    sei();
    
    while (1) 
    {
		PORTA.OUT = 0x0;
        PORTA.OUT = 0x80;
		
        /* Put the CPU in sleep */
        sleep_cpu();
        
        /* The PIT interrupt will wake the CPU */
    }
}