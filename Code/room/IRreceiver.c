/* RMT transmit example
 
 This example code is in the Public Domain (or CC0 licensed, at your option.)
 
 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "esp_attr.h"
#include "esp_err.h"
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
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "math.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO 33

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define GPIO_GRANTED 27
#define GPIO_NGRANTED 32

#define GPIO_BUTTON 4
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_BUTTON)
#define ESP_INTR_FLAG_DEFAULT 0

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (15)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_CH_NUM       (1)
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (3000)

#define SERVO_MIN_PULSEWIDTH 500 //Minimum pulse width in microsecond 500
#define SERVO_MAX_PULSEWIDTH 1900 //Maximum pulse width in microsecond 2500
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate

#define DEFAULT_VREF    1100                            //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64                              //Multisampling
static esp_adc_cal_characteristics_t *adc_chars;        //initializing attenuation variables
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static const adc_channel_t channel1 = ADC_CHANNEL_0;    //GPIO36 //thermistor_monitor adc

static const int RX_BUF_SIZE = 1024;
static const char *TX_TASK_TAG = "TX_TASK";
const char* IR_pin = "1234";
const char* fob_ID = "T0";
const char* hub_ID = "0";

static int send_flag = 0;

static int debounce = 0;
static int RXreceived = 0;

float light_percentage = 0.0;
bool window = 1;

ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    .freq_hz = 5000,                      // frequency of PWM signal
    .speed_mode = LEDC_HS_MODE,           // timer mode
    .timer_num = LEDC_HS_TIMER,            // timer index
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
};

ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
};

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

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 12);    //Set GPIO 12 as PWM0A, to which servo is connected
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

/**
 * @brief Configure MCPWM module
 */
void windowControl()
{
    uint32_t angle, count;
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();
    
    //2. initial mcpwm configuration
    //printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    //while (1) {
    char str[100];
    printf("open (0) or close (1)?:\n");
    gets(str);
    if (atoi(str) == 0) {  // open
        window = 0;
        angle = servo_per_degree_init(SERVO_MAX_DEGREE);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
    } else {  // closed
        window = 1;
        angle = servo_per_degree_init(0);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //}
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

static void adc_init() {
    
    //Configure ADC channels for each sensor
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten); //thermistor
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel1, atten); //thermistor
    }
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    
    print_char_val_type(val_type);
    
}

static void temperatureControl()
{
    //Sample ADC1
    uint32_t adc_reading = 0;
    //Multisampling
    while(1) {
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
        
        //convert raw input to a temperature
        /*
         uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);              //Convert adc_reading to voltage in mV
         double resistance = (33000.0/((double)voltage/1000.0)) - 10000.0;                   //calculate resistance across thermistor using voltage divider formula
         double temperatureKelvin = -(1 / ((log(10000.0/resistance)/3435.0) - (1/298.0)));   //convert resistance across thermistor to Kelvin (T0 = 298K, B = 3435, R0 = 10kohm)
         printf("%.2f\n", (temperatureKelvin - 273.15));                                                //convert Kelvin to Celsius
         vTaskDelay(500 / portTICK_RATE_MS);
         */
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);              //Convert adc_reading to voltage in mV
        double resistance = ((voltage/1000.000)*(20000.000))/(3.300*(1.000-(voltage/3300.000)));
        double temp = 1/((1.000/298.000) + (1.000/3435.000) * log(resistance/10000)) - 273.000;
        printf("%.2f\n", temp);                                                //convert Kelvin to Celsius
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

static void lightInit()
{
    int ch;
    
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    
    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    
    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }
    
    // Initialize fade service.
    ledc_fade_func_install(0);
}

static void lightControl()
{
    //while (1) {
    char str[100];
    printf("Enter number 0-9:\n");
    gets(str);
    light_percentage = atoi(str);
    ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_TEST_DUTY*light_percentage);
    ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
    vTaskDelay(100 / portTICK_RATE_MS);
    //}
}

static void update()
{
    while(1) {
        lightControl();
        windowControl();
    }
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

void app_main()
{
    /*
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
     */
    lightInit();
    adc_init();
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
    
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(lightControl, "lightControl", 1024*4, NULL, 1, NULL);
    //xTaskCreate(windowControl, "windowControl", 4096, NULL, 2, NULL);
    xTaskCreate(temperatureControl, "temperatureControl", 4096, NULL, 3, NULL);
    //xTaskCreate(Post_HTTP_hub, "Post_HTTP_hub", 1024*4, NULL, configMAX_PRIORITIES-1, NULL);
    //xTaskCreate(update, "update", 4096, NULL, 1, NULL);
    
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

