/* RMT transmit example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "esp_vfs_dev.h"
#include "esp_intr_alloc.h"
#include "HTTPServer.h"
#include "http_post.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO 33

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define GPIO_GRANTED 27
#define GPIO_NGRANTED 32


#define GPIO_BUTTON 4
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_BUTTON)
#define ESP_INTR_FLAG_DEFAULT 0

static const int RX_BUF_SIZE = 1024;
static const char *TX_TASK_TAG = "TX_TASK";
const char* IR_pin = "1234";
const char* fob_ID = "T0";
const char* hub_ID = "0";


static int send_flag = 0;

static int debounce = 0;
static int RXreceived = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
      send_flag = 1;
}

void gpio_def(){      //CONFIGURATION OF GPIO - 1 interrupt + 4 OUTPUT
  gpio_config_t io_conf;

  //interrupt of rising edge
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);

  gpio_intr_enable(GPIO_BUTTON);

  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_BUTTON, gpio_isr_handler, (void*) GPIO_BUTTON);

  gpio_set_direction(GPIO_GRANTED, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NGRANTED, GPIO_MODE_OUTPUT);

  gpio_set_pull_mode(GPIO_GRANTED, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(GPIO_NGRANTED, GPIO_PULLUP_ONLY);
}

/*
 * Initialize the RMT Tx channel
 */
static void rmt_tx_init()
{
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = RMT_TX_CHANNEL;
    config.gpio_num = RMT_TX_GPIO;
    config.mem_block_num = 1;
    config.tx_config.loop_en = 1;
    // enable the carrier to be able to hear the Morse sound
    // if the RMT_TX_GPIO is connected to a speaker
    config.tx_config.carrier_en = 1;
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = 1;
    config.tx_config.carrier_duty_percent = 50;
    // set audible career frequency of 611 Hz
    // actually 611 Hz is the minimum, that can be set
    // with current implementation of the RMT API
    config.tx_config.carrier_freq_hz = 38000;
    config.tx_config.carrier_level = 1;

    config.clk_div = 1;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_set_line_inverse(UART_NUM_1,UART_INVERSE_RXD);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task()
{
    while (1) {
      if(send_flag == 1){

        char *dataTX[6];
        strcat(dataTX, fob_ID);
        strcat(dataTX,  "&");
        strcat(dataTX, IR_pin);
        static const char *TX_TASK_TAG = "TX_TASK";
        esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
        printf("dataTX = %s\n", dataTX[0]);
        sendData(TX_TASK_TAG, dataTX);
        send_flag = 0;
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
static windowControl()
{

}

static thermistorRead()
{


}

static lightControl()
{

}



static void rx_task()
{
    static const char *RX_TASK_TAG = "RX_TASK";
    static int starting = 1;
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* dataR = (uint8_t*) malloc(RX_BUF_SIZE+1);

    char* fob="fobID=";
    char* hub="hubID=";
    char* code="code=";

    char* verifier="T";

    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, dataR, 7, 1000 / portTICK_RATE_MS);

        int readlen = rxBytes;

        memset(data_post_hub,'\0', sizeof(data_post_hub));

        if (rxBytes > 0 && RXreceived < 1) {
            dataR[rxBytes] = 0;
            // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, dataR);
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, dataR, rxBytes, ESP_LOG_INFO);

            if(dataR[0] == 0x54){
              strcat(data_post_hub,fob);
              data_post_hub[6] = dataR[1];
              data_post_hub[7] = dataR[2];
              strcat(data_post_hub,code);
              data_post_hub[13] = dataR[3];
              data_post_hub[14] = dataR[4];
              data_post_hub[15] = dataR[5];
              data_post_hub[16] = dataR[6];
              data_post_hub[17] = dataR[2];
              strcat(data_post_hub, hub);
              strcat(data_post_hub, hub_ID);

              printf("data = %s\n", data_post_hub);

              RXreceived = 1;
            }

        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    free(dataR);

}

void Post_HTTP_hub(){

    while(1){
      if(RXreceived == 1){
        http_rest_with_url();
        RXreceived = 0;
      }
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }

}

void accessGranted(){

    while(1){
      if(AccessValue[0] == 1){
          gpio_set_level(GPIO_GRANTED, 1);
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          gpio_set_level(GPIO_GRANTED, 0);
          AccessValue[0] = 2;
      }
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void NotaccessGranted(){

    while(1){
      if(AccessValue[0] == 0){
          gpio_set_level(GPIO_NGRANTED, 1);
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          gpio_set_level(GPIO_NGRANTED, 0);
          AccessValue[0] = 2;
      }
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // // Start web server
    static httpd_handle_t server = NULL;
    // // ESP_ERROR_CHECK(nvs_flash_init());
    // initialise_wifi(&server);


    app_wifi_initialise(&server);
    app_wifi_wait_connected();


    gpio_def();
    uart_init();
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    printf("start.....\n");
    rmt_tx_start(RMT_TX_CHANNEL, true);
    printf("Init stop.....\n");
    rmt_tx_stop(RMT_TX_CHANNEL);
    printf("Init RMT.....\n");
    rmt_tx_init();
    printf("OK\n");


    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);




    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(Post_HTTP_hub, "Post_HTTP_hub", 1024*4, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(accessGranted, "accessGranted", 1024, NULL, configMAX_PRIORITIES-2, NULL);
    xTaskCreate(NotaccessGranted, "NotaccessGranted", 1024, NULL, configMAX_PRIORITIES-2, NULL);





    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
