//This is base code for controlling the hub and fob. We will probably need two different programs because of their differing purposes.
//NOTE: the delay on the RX loop must be SHORTER than the delay on the TX loop, or else the RX loop will receive multiple messages jammed together in the same buffer.


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_vfs_dev.h"

//networking
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_wifi.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static const int RX_BUF_SIZE = 1024;

#define RMT_PIN (GPIO_NUM_26) //A0
#define TXD_PIN (GPIO_NUM_25) //A1
#define RXD_PIN (GPIO_NUM_34) //A2
#define BTN_PIN (GPIO_NUM_18) //MO
#define GRN_LED (GPIO_NUM_14) //A6
#define BLU_LED (GPIO_NUM_32) //A7
#define RED_LED (GPIO_NUM_15) //A8

#define HEAT_BULB (GPIO_NUM_27) //27

#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
//#define RMT_TX_GPIO_NUM  18     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
//#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

//int ledCounter = 0; //0 off, 1 red, 2 blue, 3 green
bool present = 0; // present
float empty_temp = 15;

////WIFI & SOCKET SETUP///////////////////////////////////////////////////////////////////

//socket variables
#define HOST_IP_ADDR "192.168.1.102"                    //target server ip
#define PORT 3333                                       //target server port
char rx_buffer[128];
char addr_str[128];
int addr_family;
int ip_protocol;
int sock;                                               //socket id?
struct sockaddr_in dest_addr;                           //socket destination info

//wifi variables
#define EXAMPLE_ESP_WIFI_SSID "Group_2"
#define EXAMPLE_ESP_WIFI_PASS "smartkey"
#define EXAMPLE_ESP_MAXIMUM_RETRY 10                    //CONFIG_ESP_MAXIMUM_RETRY
static EventGroupHandle_t s_wifi_event_group;           //FreeRTOS event group to signal when we are connected
const int WIFI_CONNECTED_BIT = BIT0;                    //The event group allows multiple bits for each event, but we only care about one event - are we connected to the AP with an IP?
static const char *TAG = "wifi station";
static int s_retry_num = 0;
bool running = true;//false;

int sendData(const char* logName, const char* data);

//// ADC //////////////////////////////////////////////////////////////////////////

static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t channel1 = ADC1_CHANNEL_3;     // A3
static const adc_channel_t channel2 = ADC1_CHANNEL_0;     // A4

static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static void print_char_val_type(esp_adc_cal_value_t val_type);

static void adc_init() {
    
    //Configure ADC channels for each sensor
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten); //photoresistor
        adc1_config_channel_atten(channel2, atten); //thermistor
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel1, atten); //photoresistor
        adc2_config_channel_atten((adc2_channel_t)channel2, atten); //thermistor
    }
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    
    print_char_val_type(val_type);
    
}

float photoresistor() {
    //Continuously sample ADC1
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel1);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel1, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    
    float rphoto = 10000.000*((voltage/1000.000)/((3300.000/1000.000)-(voltage/1000.000)));
    
    return rphoto;
}

float thermistor() {
    //Continuously sample ADC1
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel2);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel2, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    //int temp = 1.000/((1.000/298.000) + (1.000/3435.000) * log((3134.000/voltage)-1.000)) - 273.000;
    float resistance = ((voltage/1000.000)*(20000.000))/(3.300*(1.000-(voltage/3300.000)));
    float temp = 1.000/((1.000/298.000) + (1.000/3435.000) * log(resistance/10000.000)) - 273.000;

    return temp;
}



static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


////WIFI SETUP/////////////////////////////////////////////////////////////////////

//wifi event handler
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

//connect to wifi
void wifi_init_sta(void) {
    
    //Initialize NVS
    printf("Wifi setup part 1\n");
    esp_err_t ret = nvs_flash_init();
    printf("Wifi setup part 2\n");
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    printf("Wifi setup part 3\n");
    
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    
}

////SOCKET SETUP/////////////////////////////////////////////////////////////////////

static void udp_init() {
    
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    
    //socket setup struct
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
    
    //establish socket connection
    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);
}

static void udp_client_receive() {
    
    while(1) {
        
        while(1) {
            
            printf("waiting for a message\n");
            
            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            
            printf("got one!\n");
            
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                //ESP_LOGI(TAG, "%s", rx_buffer);
                printf("received string: %s\n", rx_buffer);
                
                gpio_set_level(BLU_LED, 0);
                
                //detects unlock signal
                if (strcmp((char*)rx_buffer, "granted") == 0) {
                    printf("unlocking...\n");
                    sendData("TX_TASK", "unlocked");
                    
                    gpio_set_level(GRN_LED, 1);
                    vTaskDelay(1000/portTICK_RATE_MS);
                    gpio_set_level(GRN_LED, 0);
                } else {
                    printf("denied!\n");
                    
                    gpio_set_level(RED_LED, 1);
                    vTaskDelay(1000/portTICK_RATE_MS);
                    gpio_set_level(RED_LED, 0);
                }
                
            }
            
        }
        
        //shut down socket if anything failed
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            udp_init();
        }
        
    }
    
    vTaskDelete(NULL);
    
}

static void udp_client_send(char* message) {
    
    //assuming ip4v only
    printf("%s\n", message);
    
    //send message, and print error if anything failed
    int err = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}
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
    
    //init all LEDs
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

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            
            //printf("%s\n", data);
            
            if (strstr((char*)data, "fob: ") != NULL) {
                printf("received data!");
                
                char udpBuf[100];
                
                //from here: https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array
                char* split[4];
                int i = 0;
                
                split[i] = strtok((char*)data, " ");
                
                while(split[i] != NULL) {
                    split[++i] = strtok(NULL, " ");
                }
                
                if (i >= 2) {
                    sprintf(udpBuf, "{\"fob_id\": \"%s\"}", split[1]);
                    udp_client_send(udpBuf);
                    present = !present;
                    
                }
                
            }
            
        }
        
    }
    free(data);
}

static void sensor_task() {
    float setpoint = 22.0;
    gpio_pad_select_gpio(HEAT_BULB);
    gpio_set_direction(HEAT_BULB, GPIO_MODE_OUTPUT);
    while(1) {

        float photores = photoresistor();
        float temp = thermistor();
        printf("TEMP: %.2f\n", temp);

        //if (window) {
            // open window
        //} else  {
            // close window
            if (present) {
                if (temp > setpoint + 1) {
                    gpio_set_level(HEAT_BULB, 0);
                } else if (temp < setpoint - 1) {
                    gpio_set_level(HEAT_BULB, 1);
                }
            } else {
                if (temp > empty_temp + 1) {
                    gpio_set_level(HEAT_BULB, 0);
                } else if (temp < empty_temp - 1) {
                    gpio_set_level(HEAT_BULB, 1);
                }
            }
        //}

        vTaskDelay(1000/portTICK_RATE_MS);
    }
}

void app_main(void)
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
                                         256, 0, 0, NULL, 0) );
    
    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    
    adc_init();
    wifi_init_sta();
    udp_init();
    uart_init();
    led_init();
    nec_tx_init();
    xTaskCreate(sensor_task,"sensor_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 4096, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(udp_client_receive, "udp_client_receive", 4096, NULL, configMAX_PRIORITIES-2, NULL);
}
