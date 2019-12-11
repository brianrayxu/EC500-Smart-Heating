//This is base code for controlling the hub and fob. We will probably need two different programs because of their differing purposes.
//NOTE: the delay on the RX loop must be SHORTER than the delay on the TX loop, or else the RX loop will receive multiple messages jammed together in the same buffer.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

static const int RX_BUF_SIZE = 1024;

#define FOB_ID "brian"

#define RMT_PIN (GPIO_NUM_26) //A0
#define TXD_PIN (GPIO_NUM_25) //A1
#define RXD_PIN (GPIO_NUM_34) //A2
#define BTN_PIN (GPIO_NUM_18) //MO
#define GRN_LED (GPIO_NUM_14) //A6
#define BLU_LED (GPIO_NUM_32) //A7
#define RED_LED (GPIO_NUM_15) //A8

#define GPIO_OUTPUT_IO_0    37
#define GPIO_OUTPUT_IO_1    38
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<BTN_PIN) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
//#define RMT_TX_GPIO_NUM  18     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
//#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

int sendData(const char* logName, const char* data);


int bounceCount; //number of bounces
int startTime; //for recording elapsed time to determine if debouncer has timed out
volatile int interrupt_enabled = 1; //flag to disable interrupts during debouncing

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void interrupt_lock_task() {
    
    if (interrupt_enabled) {
        interrupt_enabled = 0; //disable interrupts during the following commands
        
        //format and send new message
        char msgBuf[100];
        sprintf(msgBuf, "fob: %s", FOB_ID);
        sendData("TX_TASK", msgBuf);
        
        vTaskDelay(500/portTICK_RATE_MS);
        
        interrupt_enabled = 1; //reenable interrupts
        
    }
    
    vTaskDelete(NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            
            //separate task for interrupt action
            xTaskCreate(interrupt_lock_task, "interrupt_lock_task", 4096, NULL, 5, NULL);
            
        }
    }
}

static void btn_init() {
    
    gpio_config_t io_conf;
    printf("Checkpoint 3\n");
    
    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    //change gpio intrrupt type for one pin
    gpio_set_intr_type(BTN_PIN, GPIO_INTR_POSEDGE);
    printf("Checkpoint 4\n");
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
    
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BTN_PIN, gpio_isr_handler, (void*) BTN_PIN);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    
    //remove isr handler for gpio number.
    gpio_isr_handler_remove(BTN_PIN);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(BTN_PIN, gpio_isr_handler, (void*) BTN_PIN);
    
    bounceCount = 0;
}

int ledCounter = 0; //0 off, 1 red, 2 blue, 3 green

/*
 * @brief RMT transmitter initialization
 */

//Sets up the 38kHz signal for IR transmission
static void nec_tx_init(void)
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_PIN;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000; //IMPORTANT PART?
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    rmt_tx.tx_config.idle_level = 1;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

void led_init(void) {
    gpio_pad_select_gpio(RED_LED);
    gpio_pad_select_gpio(BLU_LED);
    gpio_pad_select_gpio(GRN_LED);
    
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLU_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GRN_LED, GPIO_MODE_OUTPUT);
}


void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_rts(UART_NUM_1, 1);
    uart_set_line_inverse(UART_NUM_1, UART_INVERSE_RXD);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

//Single-use function to send data over IR UART
int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

//Loop to wait for and read data from IR receiver
static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
        
        //if nonzero data
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            
            printf("%s\n", data);
            
            //if unlock code, light up green LED
            if (strcmp((char*)data, "unlocked") == 0) {
                gpio_set_level(GRN_LED, 1);
                vTaskDelay(1000/portTICK_RATE_MS);
                gpio_set_level(GRN_LED, 0);
            }
            
            
        }
        
    }
    free(data);
}

void app_main(void)
{
    btn_init();
    uart_init();
    led_init();
    nec_tx_init();
    xTaskCreate(rx_task, "uart_rx_task", 4096, NULL, configMAX_PRIORITIES, NULL);
}
