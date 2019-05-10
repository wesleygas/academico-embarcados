#include <asf.h>
#include <string.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "io_defines.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/


/************************************************************************/
/* Flags                                                                */
/************************************************************************/
volatile int but1_flag = 0;
volatile int up_flag = 0;
volatile int displ_value = 0;
volatile int incr_sec = 0;

/************************************************************************/
/* handlers/callbacks                                                   */
/************************************************************************/

void but1_callback(void){
	but1_flag = 1;
	up_flag = 1;
}

/**
*  Interrupt handler for TC1 interrupt.
*/
void TC0_Handler(void){
	volatile uint32_t ul_dummy;

	/****************************************************************
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/** Muda o estado do LED */
	//but1_flag = 1;
	up_flag = 1;
	displ_value+=0.1;
}


/**
* \brief Interrupt handler for the RTC. Refresh the display.
*/
void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
		incr_sec = 1;
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR); //Avisa q foi handled
			
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void timeToString(char *str, Horario tempo){
	if(tempo.hora < 10){
		str[0] = '0';
		str[1] = tempo.hora + 48;
	}else{
		str[0] = tempo.hora/10 + 48;
		str[1] = tempo.hora%10 + 48;
	}
	str[2] = ':';
	if(tempo.minuto < 10){
		str[3] = '0';
		str[4] = tempo.minuto + 48;
	}else{
		str[3] = tempo.minuto/10 + 48;
		str[4] = tempo.minuto%10 + 48;
	}
	str[5] = ':';
	if(tempo.segundo < 10){
		str[6] = '0';
		str[7] = tempo.segundo + 48;
	}else{
		str[6] = tempo.segundo/10 + 48;
		str[7] = tempo.segundo%10 + 48;
	}
	str[8] = 0;
	
}

void updateWatch(Horario eta){
	Horario c_time;
	char date_string[9];
	rtc_get_time(RTC,&c_time.hora,&c_time.minuto,&c_time.segundo);
	timeToString(date_string, c_time);
	gfx_mono_draw_string(date_string,10,0,&sysfont);
	if(c_time.segundo > eta.segundo){
		eta.minuto --;
		eta.segundo += 60;
	}
	eta.segundo = eta.segundo - c_time.segundo;
	
	if(c_time.minuto > eta.minuto){
		eta.hora --;
		eta.minuto += 60;
	}
	eta.minuto = eta.minuto - c_time.minuto;
	eta.hora = eta.hora - c_time.hora;
	timeToString(date_string,eta);
	gfx_mono_draw_string(date_string,10,15,&sysfont);
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

/**
* Configura os IOs definidos la em cima
* Ativa os botoes do OLED e aciona a interrupcao no LED1
*/
void io_init(void){
	pmc_enable_periph_clk(ID_PIOA);
	//pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	pio_configure(BUT3_PIO, PIO_INPUT,BUT3_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT,BUT2_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT,BUT1_MASK,PIO_DEBOUNCE|PIO_PULLUP);
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_MASK, PIO_DEFAULT);
	
	pio_handler_set(BUT1_PIO,ID_PIOD,BUT1_MASK,PIO_IT_FALL_EDGE, but1_callback);
	// Ativa interrup��o no hardware
	pio_enable_interrupt(PIOD, BUT1_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(ID_PIOD);
	NVIC_SetPriority(ID_PIOD, 4); // Prioridade 4
}

/**
* Configura TimerCounter (TC) para gerar uma interrupcao no canal (ID_TC e TC_CHANNEL)
* na taxa de especificada em freq.
*/
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	uint32_t channel = 1;

	/* Configura o PMC
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  4Mhz e interrup�c�o no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrup�c�o no TC canal 0 */
	/* Interrup��o no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}


/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MONTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 3);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_SECEN);

}

int main (void)
{
	
	int displ_value = 0;
	char string_cache[16];
	board_init();
	//TC_init(TC0,ID_TC0,0,500);
	sysclk_init();
	io_init();
	delay_init();
	RTC_init();
	Horario finish_time;
	finish_time.hora = 13;
	finish_time.minuto = 1;
	finish_time.segundo = 0;
	gfx_mono_ssd1306_init();
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_generic_draw_filled_rect(0,0,128,32,GFX_PIXEL_CLR);
	//gfx_mono_draw_string("mundo", 50,16, &sysfont);


  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if(but1_flag){
			pin_toggle(LED_PIO,LED_MASK);
			but1_flag = 0;
			displ_value+= 1;
		}
		if(up_flag){
			up_flag=0;
			displ_value++;
			itoa(displ_value,string_cache,10);
			gfx_mono_draw_string(string_cache,110,5,&sysfont);
		}if(incr_sec){
			updateWatch(finish_time);
			incr_sec=0;
		}
		pmc_sleep(SLEEPMGR_SLEEP_WFI);
		
		
	}
}
