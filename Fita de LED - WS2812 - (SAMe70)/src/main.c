/************************************************************************
* 5 semestre - Eng. da Computao - Insper
* Rafael Corsi - rafael.corsi@insper.edu.br
*
* Material:
*  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
*
* Objetivo:
*  - Demonstrar configuraçao do PIO
*
* Periféricos:
*  - PIO
*  - PMC
*
* Log:
*  - 10/2018: Criação
************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"
/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// PIN_DATA
#define PIN_DATA      PIOD
#define PIN_DATA_ID   ID_PIOD
#define PIN_DATA_IDX  30
#define PIN_DATA_IDX_MASK (1 << PIN_DATA_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX      11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define COLOR_RED 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_WHITE 3
#define COLOR_BLACK 4

#define RED 100,0,0
#define GREEN 0,100,0
#define BLUE 0,0,100

#include "tipoDado.h"
#include "cores_letras.h"

//FLAGS
volatile Bool new_order = false;

//RTOS
#define TASK_LED_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY        (tskIDLE_PRIORITY)

//Semaphores
SemaphoreHandle_t xSemaphoreNEW;

//QueueHandle
QueueHandle_t xQueueNew;


/************************************************************************/
/* prototypes                                                           */
/************************************************************************/


/************************************************************************/
/* constants                                                            */
/************************************************************************/
uint32_t NOME = 0xFF00;
/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void but_callback(void)
{
	new_order = true;
}

void but_callback(void)
{
	new_order = true;
}

void new_order_callback(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreNEW, &xHigherPriorityTaskWoken);
}

/************************************************************************/
/* funções                                                              */
/************************************************************************/


/* TASKS                                                                */
/************************************************************************/

void task_led(void *pvParameters){
	
	xQueueNew = xQueueCreate( 52, sizeof( char ) );
	
	xSemaphoreNEW = xSemaphoreCreateBinary();
	
	int newvalue;

	char bufferNOME[8] = {'9','9','7','7'};
	
	while(1){
		if(xSemaphoreTake(xSemaphoreNEW, ( TickType_t ) 0) == pdTRUE ){
			allgreen(GREEN);
			allgreen(GREEN);
			allgreen(GREEN);
			allgreen(GREEN);
		}
		if ( xQueueReceive(xQueueNew, &newvalue, 10)){
			bufferNOME[0] = newvalue;
		}
			
	}		
}


/************************************************************************/
/* Main                                                                 */
/************************************************************************/
/* Funcao principal chamada na inicalizacao do uC.                      */






int main(void)
{
	// Flags
//	volatile Bool new_order = true;
	volatile Bool number_change = false;
	
	
	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

  // Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);

  // Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
	// Configura Botão
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_handler_set(BUT_PIO,BUT_PIO_ID,BUT_IDX_MASK,PIO_IT_FALL_EDGE,but_callback);
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 5);
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);

	// Configura led
	pmc_enable_periph_clk(PIN_DATA_ID);
	pio_configure(PIN_DATA, PIO_OUTPUT_0, PIN_DATA_IDX_MASK, PIO_DEFAULT);

	
	
	clearLEDs();
	
	
	char bufferNOME[8] = {'9','9','7','7'};
		
	
	new_order = false;	
			
	while(1){
		
	if (new_order == 0){
		allgreen(GREEN);
		allgreen(GREEN);
		allgreen(GREEN);
		allgreen(GREEN);
	}
	
	int8_t size=0;
	for(int8_t i=0;i<=4;i++)
	{
		if( (bufferNOME[i] >= '0' && bufferNOME[i] <= '9') || (bufferNOME[i] >= 'A' && bufferNOME[i] <= 'Z') || (bufferNOME[i] >= 'a' && bufferNOME[i] <= 'z')) size++;
		else break;
	}
	
	uint8_t cont = 0;
	if(new_order){
		while(size >= 0){
			if (cont < 2){
				if(bufferNOME[cont] == '1') numero1(BLUE);
				else if(bufferNOME[cont] == '2') numero2(BLUE);
				else if(bufferNOME[cont] == '3') numero3(BLUE);
				else if(bufferNOME[cont] == '4') numero4(BLUE);
				else if(bufferNOME[cont] == '5') numero5(BLUE);
				else if(bufferNOME[cont] == '6') numero6(BLUE);
				else if(bufferNOME[cont] == '7') numero7(BLUE);
				else if(bufferNOME[cont] == '8') numero8(BLUE);
				else if(bufferNOME[cont] == '9') numero9(BLUE);
				else if(bufferNOME[cont] == '0') numero0(BLUE);
			}
			else if (cont < 4) {
				if(bufferNOME[cont] == '1') numero1(RED);
				else if(bufferNOME[cont] == '2') numero2(RED);
				else if(bufferNOME[cont] == '3') numero3(RED);
				else if(bufferNOME[cont] == '4') numero4(RED);
				else if(bufferNOME[cont] == '5') numero5(RED);
				else if(bufferNOME[cont] == '6') numero6(RED);
				else if(bufferNOME[cont] == '7') numero7(RED);
				else if(bufferNOME[cont] == '8') numero8(RED);
				else if(bufferNOME[cont] == '9') numero9(RED);
				else if(bufferNOME[cont] == '0') numero0(RED);
			}	
			//else if(bufferNOME[cont] == 'a' || bufferNOME[cont] == 'A') letraA(BLUE);
			//else if(bufferNOME[cont] == 'b' || bufferNOME[cont] == 'B') letraB(RED);
			//else if(bufferNOME[cont] == 'c' || bufferNOME[cont] == 'C') letraC(BLUE);
			//else if(bufferNOME[cont] == 'd' || bufferNOME[cont] == 'D') letraD(RED);
			//else if(bufferNOME[cont] == 'e' || bufferNOME[cont] == 'E') letraE(BLUE);
			//else if(bufferNOME[cont] == 'f' || bufferNOME[cont] == 'F') letraF(BLUE);
			//else if(bufferNOME[cont] == 'g' || bufferNOME[cont] == 'G') letraG(BLUE);
			//else if(bufferNOME[cont] == 'h' || bufferNOME[cont] == 'H') letraH(BLUE);
			//else if(bufferNOME[cont] == 'i' || bufferNOME[cont] == 'I') letraI(GREEN);
			//else if(bufferNOME[cont] == 'j' || bufferNOME[cont] == 'J') letraJ(BLUE);
			//else if(bufferNOME[cont] == 'k' || bufferNOME[cont] == 'K') letraK(BLUE);
			//else if(bufferNOME[cont] == 'l' || bufferNOME[cont] == 'L') letraL(BLUE);
			//else if(bufferNOME[cont] == 'm' || bufferNOME[cont] == 'M') letraM(BLUE);
			//else if(bufferNOME[cont] == 'n' || bufferNOME[cont] == 'N') letraN(GREEN);
			//else if(bufferNOME[cont] == 'o' || bufferNOME[cont] == 'O') letraO(GREEN);
			//else if(bufferNOME[cont] == 'p' || bufferNOME[cont] == 'P') letraP(BLUE);
			//else if(bufferNOME[cont] == 'q' || bufferNOME[cont] == 'Q') letraQ(BLUE);
			//else if(bufferNOME[cont] == 'r' || bufferNOME[cont] == 'R') letraR(BLUE);
			//else if(bufferNOME[cont] == 's' || bufferNOME[cont] == 'S') letraS(BLUE);
			//else if(bufferNOME[cont] == 't' || bufferNOME[cont] == 'T') letraT(BLUE);
			//else if(bufferNOME[cont] == 'u' || bufferNOME[cont] == 'U') letraU(BLUE);
			//else if(bufferNOME[cont] == 'v' || bufferNOME[cont] == 'V') letraV(GREEN);
			//else if(bufferNOME[cont] == 'w' || bufferNOME[cont] == 'W') letraW(BLUE);
			//else if(bufferNOME[cont] == 'x' || bufferNOME[cont] == 'X') letraX(BLUE);
			//else if(bufferNOME[cont] == 'y' || bufferNOME[cont] == 'Y') letraY(BLUE);
			//else if(bufferNOME[cont] == 'z' || bufferNOME[cont] == 'Z') letraZ(BLUE);
			size--;
			cont++;
		}
	}
	
	data_reset();
	delay_s(1);
	
	
	}
	return 0;
}

















//TESTEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
/*
while(1){
	
	//colorTESTE();
	//delay_ms(250);
	data_reset();
	
	for(int i = 0; i<1 ; i++){
		colorRED();
		colorGREEN();
		colorBLUE();
	}
	
	for(int i = 0; i<250 ; i++){
		colorBLACK();
	}
	for(int i = 0; i<2; i++){
		colorWHITE();
	}
	
	
	data_reset();
	
	delay_ms(250);
	
	for(int i = 0; i<1 ; i++){
		colorBLACK();
		colorBLACK();
		colorBLACK();
	}
	
	for(int i = 0; i<250 ; i++){
		colorBLACK();
	}
	for(int i = 0; i<2; i++){
		colorBLACK();
	}
	
	//for(int i = 0; i<290 ; i++){
	//	colorBLACK();
	//}
	
	data_reset();
	
	delay_ms(250);
	
}*/