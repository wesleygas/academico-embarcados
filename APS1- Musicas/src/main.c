/** 
 ** Entrega realizada em parceria com:
 **  - Wesley Silva
 ** 
 **  - https://photos.app.goo.gl/A7Tno5xq8CrboTne8
 **/
/************************************************************************/
/* includes                                                             */
/************************************************************************/
#include "asf.h"
#include "notes.h"
#include "songs.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED1_PIO           PIOA
#define LED1_PIO_ID        ID_PIOA //É 10 pro A
#define LED1_PIO_IDX       0u
#define LED1_PIO_IDX_MASK  (1u << LED1_PIO_IDX)

#define LED2_PIO	PIOC
#define LED2_MASK	(1u << 30)

#define LED3_PIO    PIOB
#define LED3_MASK	(1u << 2)

#define  MUS_LED_PIO  PIOC
#define  MUS_LED_MASK (1u << 8u)

#define BUT_PIO			  PIOD	
#define BUT_PIO_ID		  ID_PIOD
#define BUT_PIO_IDX       28u
#define BUT_PIO_IDX_MASK  (1u << BUT_PIO_IDX)

#define BUT2_PIO		  PIOC
#define BUT2_MASK		  (1u << 31)

#define BUT3_PIO		  PIOA
#define BUT3_MASK		  (1u << 19)

#define BUT4_PIO		  PIOA
#define BUT4_MASK		  (1u << 11)

#define BUZZ_PIO		  PIOA
#define BUZZ_PIO_ID		  ID_PIOA
#define BUZZ_PIO_IDX       6u
#define BUZZ_PIO_IDX_MASK  (1u << BUZZ_PIO_IDX)

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	//Init board clock
	sysclk_init();
	
	//deactivate WDT
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	//PMC enable clock
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	//Outputs
	pio_set_output(LED1_PIO,LED1_PIO_IDX_MASK,1,0,0);
	pio_set_output(LED2_PIO,LED2_MASK,1,0,0);
	pio_set_output(LED3_PIO,LED3_MASK,1,0,0);
	
	pio_set_output(BUZZ_PIO,BUZZ_PIO_IDX_MASK,0,0,0);
	pio_set_output(PIOC,MUS_LED_MASK,1,0,0);
	
	//Inputs
	pio_set_input(BUT_PIO,BUT_PIO_IDX_MASK,PIO_DEFAULT);
	pio_pull_up(BUT_PIO,BUT_PIO_IDX_MASK,PIO_PULLUP);
	
	pio_set_input(BUT2_PIO,BUT2_MASK,PIO_DEFAULT);
	pio_pull_up(BUT2_PIO,BUT2_MASK,PIO_PULLUP);
	
	pio_set_input(BUT3_PIO,BUT3_MASK,PIO_DEFAULT);
	pio_pull_up(BUT3_PIO,BUT3_MASK,PIO_PULLUP);
	
	pio_set_input(BUT4_PIO,BUT4_MASK,PIO_DEFAULT);
	pio_pull_up(BUT4_PIO,BUT4_MASK,PIO_PULLUP);

	
}
/*
	Frequency (Hertz)
	Duration (ms)
*/
void buzz(int frequency, int duration){
	double i = duration*1000;
	long delay = 1000000.0/frequency;
	if(frequency != 0){
		pio_clear(PIOC, MUS_LED_MASK);
		while(i > 0){
			pio_clear(PIOA, BUZZ_PIO_IDX_MASK);
			delay_us(delay);
			pio_set(PIOA, BUZZ_PIO_IDX_MASK);
			delay_us(delay);
			i-= delay*2;
		}
		pio_set(PIOC, MUS_LED_MASK);	
	}else{
		delay_ms(duration);
	}
	
}



int mus_selector()
{
	if(!pio_get(BUT_PIO,PIO_INPUT,BUT_PIO_IDX_MASK)){
		current_mus = 0;
		pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
		pio_set(LED2_PIO,LED2_MASK);
		pio_set(LED3_PIO,LED3_MASK);
		return 0;
	}
	if(!pio_get(BUT2_PIO,PIO_INPUT,BUT2_MASK)){
		current_mus = 1;
		pio_clear(LED2_PIO, LED2_MASK);
		pio_set(LED3_PIO,LED3_MASK);
		pio_set(LED1_PIO,LED1_PIO_IDX_MASK);
		return 1;
	}
	if(!pio_get(BUT3_PIO,PIO_INPUT,BUT3_MASK)){
		current_mus = 2;
		pio_clear(LED3_PIO, LED3_MASK);
		pio_set(LED1_PIO,LED1_PIO_IDX_MASK);
		pio_set(LED2_PIO,LED2_MASK);
		return 2;
	}
	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/


// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();
  int current_mus = 0;
  int is_playing = 0;
  
  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1){	
	if(is_playing){
		switch(current_mus){
			case 0:
				for(int i = 0; i<203;i++){
					buzz(piratasDoCaribe[i],piratasDuration[i]);
					delay_ms(50);
				}
				is_playing = 0;
				break;
			
			case 1:
				for(int i = 0; i<14; i++){
					buzz(starWars[i],starWarsDuration[i]);
					delay_ms(50);
				}
				is_playing = 0;
				break;
			
			case 2:
				for(int i = 0; i<8; i++){
					buzz(melodia1[i], melodia1Durations[i]);
					delay_ms(50);
				}
				is_playing = 0;
				break;
		}
	}
	
	current_mus = mus_selector();
	if(!pio_get(BUT4_PIO,PIO_INPUT,BUT4_MASK)){
		is_playing = 1;
	}
	
	
  }
  return 0;
}
