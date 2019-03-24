#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

//OLEDBoard
#define BUT1_PIO		  PIOD
#define BUT1_MASK		  (1u << 28u)

#define BUT2_PIO		  PIOC
#define BUT2_MASK		  (1u << 31)

#define BUT3_PIO		  PIOA
#define BUT3_MASK		  (1u << 19)

//Onboard LED
#define  LED_PIO  PIOC
#define  LED_MASK (1u << 8u)

/************************************************************************/
/* Flags                                                                */
/************************************************************************/
volatile int but1_flag = 0;
volatile int up_flag = 1;
volatile int displ_value = 0;

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
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/** Muda o estado do LED */
	//but1_flag = 1;
	up_flag = 1;
	displ_value++;
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

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
	// Ativa interrupção no hardware
	pio_enable_interrupt(PIOD, BUT1_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
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

	/** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrupçcão no TC canal 0 */
	/* Interrupção no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}


int main (void)
{

	int displ_value = 0;
	char string_cache[16];
	board_init();
	TC_init(TC0,ID_TC0,0,500);
	sysclk_init();
	io_init();
	delay_init();

	gfx_mono_ssd1306_init();
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_generic_draw_filled_rect(0,0,128,32,GFX_PIXEL_CLR);
	gfx_mono_draw_string("mundo", 50,16, &sysfont);


  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if(but1_flag){
			pin_toggle(LED_PIO,LED_MASK);
			but1_flag = 0;
		}
		if(up_flag){
			up_flag=0;
			displ_value++;
			itoa(displ_value,string_cache,10);
			
			gfx_mono_draw_string(string_cache,5,5,&sysfont);
		}
		//pmc_sleep(SLEEPMGR_SLEEP_WFI);
		
		
	}
}
