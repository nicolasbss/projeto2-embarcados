#include "asf.h"
#include "main.h"
#include <string.h>
#include "bsp/include/nm_bsp.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"

#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- WINC1500 weather client example --"STRING_EOL	\
	"-- "BOARD_NAME " --"STRING_EOL	\
	"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL

/** IP address of host. */
uint32_t gu32HostIp = 0;

/** TCP client socket handlers. */
static SOCKET tcp_client_socket = -1;

/** Receive buffer definition. */
static uint8_t gau8ReceivedBuffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/** Wi-Fi status variable. */
static bool gbConnectedWifi = false;

/** Get host IP status variable. */
/** Wi-Fi connection state */
static uint8_t wifi_connected;


/** Instance of HTTP client module. */
static bool gbHostIpByName = false;

/** TCP Connection status variable. */
static bool gbTcpConnection = false;

/** Server host name. */
static char server_host_name[] = MAIN_SERVER_NAME;


#define TASK_WIFI_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_WIFI_STACK_PRIORITY        (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

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

// Bot�o
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

// WIFI
#define TASK_WIFI_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_WIFI_STACK_PRIORITY        (tskIDLE_PRIORITY)

//LED
#define TASK_LED_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY        (tskIDLE_PRIORITY)

//Semaphores
SemaphoreHandle_t xSemaphoreNEW;
SemaphoreHandle_t xSemaphoreBUT;

//QueueHandle
QueueHandle_t xQueueNew;


/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void)
{
	
}

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}


/**
 * \brief Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}


/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
 /* http://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/unpv12e/libfree/inet_aton.c */
int inet_aton(const char *cp, in_addr *ap)
{
  int dots = 0;
  register u_long acc = 0, addr = 0;

  do {
	  register char cc = *cp;

	  switch (cc) {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	        acc = acc * 10 + (cc - '0');
	        break;

	    case '.':
	        if (++dots > 3) {
		    return 0;
	        }
	        /* Fall through */

	    case '\0':
	        if (acc > 255) {
		    return 0;
	        }
	        addr = addr << 8 | acc;
	        acc = 0;
	        break;

	    default:
	        return 0;
    }
  } while (*cp++) ;

  /* Normalize the address */
  if (dots < 3) {
	  addr <<= 8 * (3 - dots) ;
  }

  /* Store it if requested */
  if (ap) {
	  ap->s_addr = _htonl(addr);
  }

  return 1;    
}


/**
 * \brief Callback function of IP address.
 *
 * \param[in] hostName Domain name.
 * \param[in] hostIp Server IP.
 *
 * \return None.
 */
static void resolve_cb(uint8_t *hostName, uint32_t hostIp)
{
	gu32HostIp = hostIp;
	gbHostIpByName = true;
	printf("resolve_cb: %s IP address is %d.%d.%d.%d\r\n\r\n", hostName,
			(int)IPV4_BYTE(hostIp, 0), (int)IPV4_BYTE(hostIp, 1),
			(int)IPV4_BYTE(hostIp, 2), (int)IPV4_BYTE(hostIp, 3));
}

/**
 * \brief Callback function of TCP client socket.
 *
 * \param[in] sock socket handler.
 * \param[in] u8Msg Type of Socket notification
 * \param[in] pvMsg A structure contains notification informations.
 *
 * \return None.
 */
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
  
	/* Check for socket event on TCP socket. */
	if (sock == tcp_client_socket) {
    
		switch (u8Msg) {
		case SOCKET_MSG_CONNECT:
		{
      printf("socket_msg_connect\n"); 
			if (gbTcpConnection) {
				memset(gau8ReceivedBuffer, 0, sizeof(gau8ReceivedBuffer));
				sprintf((char *)gau8ReceivedBuffer, "%s", MAIN_PREFIX_BUFFER);

				tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
				if (pstrConnect && pstrConnect->s8Error >= SOCK_ERR_NO_ERROR) {
          printf("send \n");
					send(tcp_client_socket, gau8ReceivedBuffer, strlen((char *)gau8ReceivedBuffer), 0);

					memset(gau8ReceivedBuffer, 0, MAIN_WIFI_M2M_BUFFER_SIZE);
					recv(tcp_client_socket, &gau8ReceivedBuffer[0], MAIN_WIFI_M2M_BUFFER_SIZE, 0);
				} else {
					printf("socket_cb: connect error!\r\n");
					gbTcpConnection = false;
					close(tcp_client_socket);
					tcp_client_socket = -1;
				}
			}
		}
		break;
    


		case SOCKET_MSG_RECV:
		{
			char *pcIndxPtr;
			char *pcEndPtr;

			tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
			if (pstrRecv && pstrRecv->s16BufferSize > 0) {
        //printf(pstrRecv->pu8Buffer);
		printf("resposta \n %s", pstrRecv->pu8Buffer);
		
		
				
				memset(gau8ReceivedBuffer, 0, sizeof(gau8ReceivedBuffer));
				recv(tcp_client_socket, &gau8ReceivedBuffer[0], MAIN_WIFI_M2M_BUFFER_SIZE, 0);
				
			} else {
				printf("socket_cb: recv error!\r\n");
				close(tcp_client_socket);
				tcp_client_socket = -1;
			}
		}
		break;

		default:
			break;
		}
	}
}

static void set_dev_name_to_mac(uint8_t *name, uint8_t *mac_addr)
{
	/* Name must be in the format WINC1500_00:00 */
	uint16 len;

	len = m2m_strlen(name);
	if (len >= 5) {
		name[len - 1] = MAIN_HEX2ASCII((mac_addr[5] >> 0) & 0x0f);
		name[len - 2] = MAIN_HEX2ASCII((mac_addr[5] >> 4) & 0x0f);
		name[len - 4] = MAIN_HEX2ASCII((mac_addr[4] >> 0) & 0x0f);
		name[len - 5] = MAIN_HEX2ASCII((mac_addr[4] >> 4) & 0x0f);
	}
}

/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType Type of Wi-Fi notification.
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters.
 *
 * \return None.
 */
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED:
	{
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			printf("wifi_cb: M2M_WIFI_CONNECTED\r\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("wifi_cb: M2M_WIFI_DISCONNECTED\r\n");
			gbConnectedWifi = false;
 			wifi_connected = 0;
		}

		break;
	}

	case M2M_WIFI_REQ_DHCP_CONF:
	{
		uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
		printf("wifi_cb: IP address is %u.%u.%u.%u\r\n",
				pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
		wifi_connected = M2M_WIFI_CONNECTED;
		
    /* Obtain the IP Address by network name */
		//gethostbyname((uint8_t *)server_host_name);
		break;
	}

	default:
	{
		break;
	}
	}
}

void but_callback(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreBUT, &xHigherPriorityTaskWoken);
}

void task_led(void *pvParameters){
	
	xQueueNew = xQueueCreate( 52, sizeof( char ) );
	
	xSemaphoreNEW = xSemaphoreCreateBinary();
	xSemaphoreBUT = xSemaphoreCreateBinary();
	
	int newvalue;
	
	int neworder = 1;
	int newbut = 0;
	
	char bufferNOME[8] = {'9','9','7','7'};
	
	while(1){
		if(xSemaphoreTake(xSemaphoreNEW, ( TickType_t ) 0) == pdTRUE ){
			neworder = !neworder;
		}
		
		if(xSemaphoreTake(xSemaphoreBUT, ( TickType_t ) 0) == pdTRUE ){
			newbut = 1;
		}
		
		if ( xQueueReceive(xQueueNew, &newvalue, 10)){
			bufferNOME[0] = newvalue;
		}
		
		if (neworder){
			allgreen(GREEN);
			allgreen(GREEN);
			allgreen(GREEN);
			allgreen(GREEN);
			newbut = 0;
		}
		
		if (newbut){
			
			int8_t size=0;
			
			for(int8_t i=0;i<=4;i++){
				if( (bufferNOME[i] >= '0' && bufferNOME[i] <= '9') || (bufferNOME[i] >= 'A' && bufferNOME[i] <= 'Z') || (bufferNOME[i] >= 'a' && bufferNOME[i] <= 'z')) size++;
				else break;
			}
			
			uint8_t cont = 0;
			
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
				size--;
				cont++;
			}
		}
		
		data_reset();
		delay_s(1);
		
	}
}

/**
 * \brief This task, when activated, send every ten seconds on debug UART
 * the whole report of free heap and total tasks status
 */
static void task_monitor(void *pvParameters)
{
	static portCHAR szList[256];
	UNUSED(pvParameters);

	for (;;) {
		printf("--- task ## %u", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		printf(szList);
		vTaskDelay(1000);
	}
}



static void task_wifi(void *pvParameters) {
	tstrWifiInitParam param;
	int8_t ret;
	uint8_t mac_addr[6];
	uint8_t u8IsMacAddrValid;
	struct sockaddr_in addr_in;
	
	/* Initialize the BSP. */
	nm_bsp_init();
	
	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
		while (1) {
		}
	}
	
	/* Initialize socket module. */
	socketInit();

	/* Register socket callback function. */
	registerSocketCallback(socket_cb, resolve_cb);

	/* Connect to router. */
	printf("main: connecting to WiFi AP %s...\r\n", (char *)MAIN_WLAN_SSID);
	m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = _htons(MAIN_SERVER_PORT);
	inet_aton(MAIN_SERVER_NAME, &addr_in.sin_addr);
	printf("Inet aton : %d", addr_in.sin_addr);
	
  while(1){
	  m2m_wifi_handle_events(NULL);

	  if (wifi_connected == M2M_WIFI_CONNECTED) {
		  /* Open client socket. */
		  if (tcp_client_socket < 0) {
			  printf("socket init \n");
			  if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				  printf("main: failed to create TCP client socket error!\r\n");
				  continue;
			  }

			  /* Connect server */
			  printf("socket connecting\n");
			  
			  if (connect(tcp_client_socket, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
				  close(tcp_client_socket);
				  tcp_client_socket = -1;
				  printf("error\n");
				  }else{
				  gbTcpConnection = true;
			  }
		  }
	  }
	  }
}
/**
 * \brief Main application function.
 *
 * Initialize system, UART console, network then start weather client.
 *
 * \return Program return value.
 */
int main(void)
{
	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	configure_console();
	printf(STRING_HEADER);
	
		// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

  // Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);

  // Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
	// Configura Bot�o
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_handler_set(BUT_PIO,BUT_PIO_ID,BUT_IDX_MASK,PIO_IT_FALL_EDGE,but_callback);
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 5);
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);

	// Configura led
	pmc_enable_periph_clk(PIN_DATA_ID);
	pio_configure(PIN_DATA, PIO_OUTPUT_0, PIN_DATA_IDX_MASK, PIO_DEFAULT);

	
	
	clearLEDs();
	
	if (xTaskCreate(task_wifi, "Wifi", TASK_WIFI_STACK_SIZE, NULL,
	TASK_WIFI_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Wifi task\r\n");
	}
	
	if (xTaskCreate(task_wifi, "Wifi", TASK_WIFI_STACK_SIZE, NULL,
	TASK_WIFI_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Wifi task\r\n");
	}

	vTaskStartScheduler();
	
	while(1) {};
	return 0;

}
