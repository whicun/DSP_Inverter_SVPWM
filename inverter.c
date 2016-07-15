#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "math.h"

#define pi 3.1415926
#define digit 100000
#define period 7500  // 10KHz��Ӧʱ������TBCLK = SYSCLKOUT
//#define period 60000  // 10KHz��Ӧʱ������TBCLK = SYSCLKOUT(for test)
#define M 0.95  // ���ƶ�
int period_count = 0;  // �ز�������
int Tinv[3] = {0, 0, 0};  // �����Ӧ�Ƚ�ֵ
int last[3];  // ������Tinvֵ(for test)
double Dm = 0, Dn = 0, D0 = 0;  // ռ�ձ�

void ePWMSetup(void);
double roundn(double);  // �ض�С�����λ��
interrupt void epwm4_timer_isr(void);

int main()
{
   InitSysCtrl();

   DINT;

   InitPieCtrl();

   IER = 0x0000;
   IFR = 0x0000;

   InitPieVectTable();

   EALLOW;
   PieVectTable.EPWM4_INT = &epwm4_timer_isr;  // ePWM1�жϺ������
   EDIS;

   ePWMSetup();
	
   IER |= M_INT3;  // enable ePWM1 CPU_interrupt
   PieCtrlRegs.PIEIER3.bit.INTx4 = 1;  // enable ePWM1 pie_interrupt

   EINT;   // ���ж� INTM ʹ��
   ERTM;   // Enable Global realtime interrupt DBGM

   int i;
   for(; ;)
   {
	   asm("          NOP");
	   for(i=1;i<=10;i++)
	   {}
   }

   return 0;
}

void ePWMSetup(void)
{
   EALLOW;

	GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // GPIO ��ʼ��Ϊepwm���
	GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;
	GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;

   EDIS;

   // ----------------EPwm1---------------------
   EPwm4Regs.TBPRD = period;
   EPwm4Regs.TBPHS.half.TBPHS = 0;  // ʱ�����ڼĴ���
   EPwm4Regs.TBCTR = 0;  // ʱ�������Ĵ�������
   EPwm4Regs.TBCTL.bit.PHSDIR = TB_UP;
   EPwm4Regs.TBCTL.bit.CLKDIV = 0;
   EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;
   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

   EPwm4Regs.CMPA.half.CMPA = period / 2; // duty_cycle = 0.5
   EPwm4Regs.CMPB = 0;
   EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
   EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;
   EPwm4Regs.AQCTLA.bit.CAD = AQ_CLEAR;

   EPwm4Regs.DBCTL.bit.IN_MODE = DBA_ALL;
   EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;  // A����ת��B��ת
   EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   EPwm4Regs.DBRED = 80; // Deadzone
   EPwm4Regs.DBFED = 80;

   EPwm4Regs.ETSEL.bit.INTEN = 1;  // ʹ��ePWM�ж�
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;
   EPwm4Regs.ETPS.all = 0x01; // interrupt on first event
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;

   // ----------------EPwm2---------------------
   EPwm5Regs.TBPRD = period;
   EPwm5Regs.TBPHS.half.TBPHS = 0;  // ʱ�����ڼĴ���
   EPwm5Regs.TBCTR = 0;  // ʱ�������Ĵ�������
   EPwm5Regs.TBCTL.bit.PHSDIR = TB_UP;
   EPwm5Regs.TBCTL.bit.CLKDIV = 0;
   EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;
   EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
   EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

   EPwm5Regs.CMPA.half.CMPA = period / 2; // duty_cycle = 0.5
   EPwm5Regs.CMPB = 0;
   EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;

   EPwm5Regs.AQCTLA.bit.CAU = AQ_SET;
   EPwm5Regs.AQCTLA.bit.CAD = AQ_CLEAR;

   EPwm5Regs.DBCTL.bit.IN_MODE = DBA_ALL;
   EPwm5Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;  // A����ת��B��ת
   EPwm5Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   EPwm5Regs.DBRED = 80; // Deadzone
   EPwm5Regs.DBFED = 80;

   // ----------------EPwm6---------------------
   EPwm6Regs.TBPRD = period;
   EPwm6Regs.TBPHS.half.TBPHS = 0;  // ʱ�����ڼĴ���
   EPwm6Regs.TBCTR = 0;  // ʱ�������Ĵ�������
   EPwm6Regs.TBCTL.bit.PHSDIR = TB_UP;
   EPwm6Regs.TBCTL.bit.CLKDIV = 0;
   EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0;
   EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
   EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;

   EPwm6Regs.CMPA.half.CMPA = period / 2; // duty_cycle = 0.5
   EPwm6Regs.CMPB = 0;
   EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;

   EPwm6Regs.AQCTLA.bit.CAU = AQ_SET;
   EPwm6Regs.AQCTLA.bit.CAD = AQ_CLEAR;

   EPwm6Regs.DBCTL.bit.IN_MODE = DBA_ALL;
   EPwm6Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;  // A����ת��B��ת
   EPwm6Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   EPwm6Regs.DBRED = 80; // Deadzone
   EPwm6Regs.DBFED = 80;
}

interrupt void epwm4_timer_isr(void)
{
	double Angle = 0;
	double theta = 0;
	int sector = 0;

    Angle = fmod((100 * pi * (period_count / 10000.0)),(2 * pi));
   	theta = fmod(Angle,1/3.0 * pi);
   	sector = floor( Angle / (1/3.0 * pi)) + 1;
   	Dm = M * sin(1/3.0 * pi - theta);
   	Dn = M * sin(theta);
   	D0 = 0.5 * (1 - Dm - Dn);
   	Dm = roundn(Dm);
   	Dn = roundn(Dn);
   	D0 = roundn(D0);
   	if (D0 < 0) D0 = 0;

   	switch (sector)
   	{
       case 1:
           Tinv[0] = floor(period * (D0));
           Tinv[1] = floor(period * (D0 + Dm));
           Tinv[2] = floor(period * (D0 + Dm + Dn));
           break;
        case 2:
            Tinv[0] = floor(period * (D0 + Dn));
            Tinv[1] = floor(period * (D0));
            Tinv[2] = floor(period * (D0 + Dm + Dn));
            break;
        case 3:
            Tinv[0] = floor(period * (D0 + Dm + Dn));
            Tinv[1] = floor(period * (D0));
            Tinv[2] = floor(period * (D0 + Dm));
            break;
        case 4:
            Tinv[0] = floor(period * (D0 + Dm + Dn));
            Tinv[1] = floor(period * (D0 + Dn));
            Tinv[2] = floor(period * (D0));
            break;
        case 5:
            Tinv[0] = floor(period * (D0 + Dm));
            Tinv[1] = floor(period * (D0 + Dm + Dn));
            Tinv[2] = floor(period * (D0));
            break;
        case 6:
            Tinv[0] = floor(period * (D0));
            Tinv[1] = floor(period * (D0 + Dm + Dn));
            Tinv[2] = floor(period * (D0 + Dn));
   	}
   	if (Tinv[0] == 0)
   		Tinv[0]++;
   	if(Tinv[0] == period)
   		Tinv[0]--;
   	if (Tinv[1] == 0)
   		Tinv[1]++;
   	if(Tinv[1] == period)
   		Tinv[1]--;
   	if (Tinv[2] == 0)
   		Tinv[2]++;
   	if(Tinv[2] == period)
   		Tinv[2]--;
   	EPwm4Regs.CMPA.half.CMPA = Tinv[0];
   	EPwm5Regs.CMPA.half.CMPA = Tinv[1];
   	EPwm6Regs.CMPA.half.CMPA = Tinv[2];

    period_count++;
    if (period_count == 10000) period_count = 0;

    last[0] = Tinv[0];
    last[1] = Tinv[1];
    last[2] = Tinv[2];

   // Clear INT flag for this timer
    EPwm4Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

double roundn(double input)
{
	double temp;
	temp = input * digit;
	temp = floor(temp);
	temp = temp / digit;
	return temp;
}
