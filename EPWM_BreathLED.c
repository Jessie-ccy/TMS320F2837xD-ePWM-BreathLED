/*
 1101_AC & DC DRIVERS
 proj.4 ePWM
 input_NONE
 output_
 - ePWM1A is on GPIO0 (PIN 40)
 - ePWM1B is on GPIO1 (PIN 39)
 - ePWM2A is on GPIO2 (PIN 38)
 - ePWM2B is on GPIO3 (PIN 37)
 - ePWM3A is on GPIO4 (PIN 36)
 - ePWM3B is on GPIO5 (PIN 35)

 */

//
// Included Files
//
#include "F28x_Project.h"

// Function declare
void InitEPwm1Example(void); // Initialize EPWM1
void InitEPwm2Example(void); // Initialize EPWM2
void InitEPwm3Example(void); // Initialize EPWM3
__interrupt void epwm1_isr(void); // EPWM interrupt, base on switching frequency

// Constant value define
/*
//state 2s
const float SwitchingFreq = 10000; // Switching frequency
const float EPWM_DB = 500; // Deadband pulse number, EPWM_DB=BaseClock(Hz)*Deadtime(sec)
const float DutyStep = 0.0001;
const float CountStepRange = 10000;
*/
/*
//state 4s
const float SwitchingFreq = 10000; // Switching frequency
const float EPWM_DB = 500; // Deadband pulse number, EPWM_DB=BaseClock(Hz)*Deadtime(sec)
const float DutyStep = 0.00005;
const float CountStepRange = 20000;
*/

//state 1s
const float SwitchingFreq = 10000; // Switching frequency
const float EPWM_DB = 500; // Deadband pulse number, EPWM_DB=BaseClock(Hz)*Deadtime(sec)
const float DutyStep = 0.0002;
const float CountStepRange = 5000;//0.5s shift light
const float DutyStep2 = 0.00005;
const float CountStepRange2 = 20000;






// Variables be used in program
int TimeBase; // EPWM period time

Uint16 count, count2, count3, count_up_down, count_up_down2, count_up_down3;

void main(void)
{
    // Initialize system control
    InitSysCtrl();

    // Initialize GPIO pins for ePWM1, ePWM2, ePWM3
    InitEPwm1Gpio();
    InitEPwm2Gpio();
    InitEPwm3Gpio();

    // enable PWM1, PWM2 and PWM3
    //
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM3 = 1;

    // Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags are cleared.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    InitPieVectTable();

    // Setting interrupt linked
    EALLOW;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;

    // Pre-calculate variables which need divide operation
    TimeBase = 50000000 / 2 / SwitchingFreq;    //  50M/2/10k=2.5k  Page115 for "/2"
    count = 1;
    count2 = 1;
    count3 = CountStepRange2;
    count_up_down = 0;
    count_up_down2 = 0;
    count_up_down3 = 0;

    // Initialize all the Device Peripherals
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; // Closed synchronize between base clock and all enabled ePWM modules
    EDIS;

    InitEPwm1Example();
    InitEPwm2Example();
    InitEPwm3Example();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; // Enable synchronize between base clock and all enabled ePWM modules
    EDIS;

    IER |= M_INT3; //for epwm interrupt
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1; // Base on PIE MUXed vector table, enable EPWM1 interrupt

    // Enable Global interrupt INTM
    EINT;
    //Enable Global realtime interrupt DBGM
    ERTM;

    // Infinite loop
    for (;;)
    {
        asm ("          NOP");
    }
}


// epwm1_isr - EPWM1 ISR
__interrupt void epwm1_isr(void)
{

    if (count_up_down == 0)
    {
        count++;
    }
    else if (count_up_down == 1)
    {
        count--;
    }

    if (count >= CountStepRange)
    {
        count_up_down = 1;
    }
    else if (count == 0)
    {
        count_up_down = 0;
    }

    EPwm1Regs.CMPA.bit.CMPA = TimeBase * (1.0 - count * DutyStep);
    if (count_up_down2 == 0)
    {
        count2++;
    }
    else if (count_up_down2 == 1)
    {
        count2--;
    }

    if (count2 >= CountStepRange2)
    {
        count_up_down2 = 1;
    }
    else if (count2 == 0)
    {
        count_up_down2 = 0;
    }
    EPwm2Regs.CMPA.bit.CMPA = TimeBase * (1.0 - count2 * DutyStep2);
    if (count_up_down3 == 0)
    {
        count3++;
    }
    else if (count_up_down3 == 1)
    {
        count3--;
    }

    if (count3 >= CountStepRange2)
    {
        count_up_down3 = 1;
    }
    else if (count3 == 0)
    {
        count_up_down3 = 0;
    }
    EPwm3Regs.CMPA.bit.CMPA = TimeBase * (1.0 - count3 * DutyStep2);

    // Clear interrupt
    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}



//
// InitEPwm1Example - Initialize EPWM1 configuration
//
void InitEPwm1Example()
{
    EPwm1Regs.TBPRD = TimeBase;                       // Set timer period
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;           // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                     // Clear counter

    // Setup TBCLK
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count UPDOWN
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;  // TBCLK=SYSCLKOUT/(HSPCLKDIV¡ÑCLKDIV)

    // Setup compare
    EPwm1Regs.CMPA.bit.CMPA = TimeBase;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;           // If counter>CMPA, output high
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;          // If counter<CMPA, output low

    // Active Low PWMs - Setup Deadband
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;// Set deadband mode: Both rising and falling edge
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;// Active high mode. Neither EPWMxA nor EPWMxB is inverted
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;// EPWMxA In (from the action-qualifier) is the source for both falling-edge and rising-edge delay.
    EPwm1Regs.DBRED.bit.DBRED = EPWM_DB;// Setting Dead-Band Generator Rising Edge Delay Count Register
    EPwm1Regs.DBFED.bit.DBFED = EPWM_DB;// Setting Dead-Band Generator Falling Edge Delay Count Register

    // Setup interrupt
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;    // Select INT on Zero event
    EPwm1Regs.ETSEL.bit.INTEN = 1;               // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST; // Generate an interrupt on the first event

}

//
// InitEPwm2Example - Initialize EPWM2 configuration
//
void InitEPwm2Example()
{
    EPwm2Regs.TBPRD = TimeBase;                       // Set timer period
    EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;           // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                     // Clear counter

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count UPDOWN
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm2Regs.CMPA.bit.CMPA = TimeBase;
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM2A on Zero
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm2Regs.DBRED.bit.DBRED = EPWM_DB;
    EPwm2Regs.DBFED.bit.DBFED = EPWM_DB;

    EPwm2Regs.ETSEL.bit.INTEN = 0;                // Disable interrupt

}

//
// InitEPwm3Example - Initialize EPWM3 configuration
//
void InitEPwm3Example()
{
    EPwm3Regs.TBPRD = TimeBase;
    EPwm3Regs.TBPHS.bit.TBPHS = 0x0000;
    EPwm3Regs.TBCTR = 0x0000;

    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm3Regs.CMPA.bit.CMPA = TimeBase;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm3Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm3Regs.DBRED.bit.DBRED = EPWM_DB;
    EPwm3Regs.DBFED.bit.DBFED = EPWM_DB;

    EPwm3Regs.ETSEL.bit.INTEN = 0;                 // Disable interrupt
}
