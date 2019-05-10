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

//RTC constants
//Date
#define YEAR	2019
#define MONTH	03
#define DAY		30
#define WEEK	12
//Time
#define HOUR	11
#define MINUTE	59
#define SECOND	50

typedef struct Horario{
	int hora;
	int minuto;
	int segundo;
} Horario;