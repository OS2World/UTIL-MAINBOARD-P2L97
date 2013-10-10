/*	Small	hardware	monitor for the Asus P2L97 mainboard
 *	  OS/2-EMX
 *
 *	  bjorn@mork.no
 *	updated to add	CPU temp for Winbond 83782 chips, Trevor-Hemsley@dial.pipex.com
 */

#include	"p2l97.h"
#include	"version.h"
#include	<stdio.h>

void fanprint(char *title, short tick, short fandiv)
{
  if (tick!=255)
	 printf("%s	%d	RPM\n", title, LM78_RPM(tick,fandiv));
}

int main(int argc, char* argv[])
  {
  int	i,	j,	lread;
  float f, f1;

  printf("LMxx/Winbond W83782d monitor\nVersion %s\n\n", VERSION);
  LM78_INIT;
  if (LM78_EXIST)
	  {
	  fanprint("\tChassis fan\t:", LM78_READ(LM78_FAN1),	LM78_FANDIV1);
	  fanprint("\tCPU fan\t\t:", LM78_READ(LM78_FAN2), LM78_FANDIV2);
	  fanprint("\tPower fan\t:", LM78_READ(LM78_FAN3), LM78_FANDIV3);
	  i =	LM78_READ(LM78_TEMP);
	  printf("\n\tMB temperature\t:%3døC/%3døF\n",	i,	i*9/5+32	);

	  LM78_WRITE(LM78_BANKSEL, LM78_BANK0);					//	switch RAM bank
	  lread = LM78_READ(LM78_VENDOR);
	  if (lread	==	0x5c)						//	if	a Winbond chip
		  {
		  LM78_WRITE(LM78_BANKSEL, (LM78_BANK0 &	0x7f));	//	switch register halves
		  lread = LM78_READ(LM78_VENDOR);
		  if (lread	== 0xa3)						//	and check other half	of	sig
			  {
			  LM78_WRITE(LM78_BANKSEL, LM78_BANK1);			//	switch RAM bank
			  lread = LM78_READ(LM78_TEMP_CFG);
			  if ((lread & 0x01) == 0)						//	if	cpu monitor	not stopped
				  {
				  i =	LM78_READ(LM78_EXT_TEMP1);				//	read his	temp
				  if (i > 127)						//	cater	for 2's complement
					  i = (256 - i)*(-1);
				  printf("\tCPU0 temp.\t:%3døC/%3døF\n",	i,	i*9/5+32	);
				  }

			  LM78_WRITE(LM78_BANKSEL, LM78_BANK2);			//	switch to other bank
			  lread = LM78_READ(LM78_TEMP_CFG);
			  if ((lread & 0x01) == 0)						//	to	check	if	2nd cpu installed
				  {
				  i =	LM78_READ(LM78_EXT_TEMP2);
				  if (i > 127)
					  i = (256 - i)*(-1);
				  printf("\tCPU1 temp.\t:%3døC/%3døF\n",	i,	i*9/5+32	);
				  }

			  LM78_WRITE(LM78_BANKSEL, LM78_BANK0);
			  }
		  }

	  printf("\n\tVcore voltage\t:%6.2f V\n", P2L97_VCORE);
	  printf("\t+3.3V voltage\t:%6.2f V\n", P2L97_3_3V);
	  printf("\t+5V   voltage\t:%6.2f V\n", P2L97_5V);
	  printf("\t+12V  voltage\t:%6.2f V\n", P2L97_12V);
	  printf("\t-12V  voltage\t:%6.2f V\n", P2L97_M12V);
	  printf("\t-5V   voltage\t:%6.2f V\n", P2L97_M5V);
	  }
  else
	  {
	  printf("\nERROR: Cannot find an LMxx/Winbond	at	port %#hx\n", LM78_BASE	);
	  }
  return	0;
  }

