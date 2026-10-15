#include <Arduino.h>
#include <driver/spi_slave.h>
#include "freertos/queue.h"

#define GPIO_MOSI 27
#define GPIO_SCLK 18
#define GPIO_CS   5
#define CHUNK_SIZE 8912 
#define QUEUE_DEPTH 8   // 增加隊列深度以緩衝 Serial 延遲

// 使用 FreeRTOS 隊列傳遞已接收的資料指針
QueueHandle_t dataQueue;
uint8_t* spi_buffer[QUEUE_DEPTH];
spi_slave_transaction_t trans[QUEUE_DEPTH];
volatile bool is_running = false;
const uint8_t SYNC_WORD[] = {0xAA, 0x55};

void init_spi_slave() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CHUNK_SIZE
    };
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_CS,
        .flags = 0,
        .queue_size = QUEUE_DEPTH,
        .mode = 1,
    };
    spi_slave_free(VSPI_HOST); 
    spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
}

// 核心 0 任務：專職 SPI 高速接收
void spi_task(void *pvParameters) {
    while(1) {
        if (is_running) {
            spi_slave_transaction_t* ret_t;
            // 阻塞等待 SPI 接收完成
            esp_err_t ret = spi_slave_get_trans_result(VSPI_HOST, &ret_t, portMAX_DELAY);
            if (ret == ESP_OK) {
                // 將成功的結果送入隊列供核心 1 發送
                xQueueSend(dataQueue, &ret_t, portMAX_DELAY);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

// 核心 1 任務：專職 Serial 高速發送與指令檢查
void serial_task(void *pvParameters) {
    while(1) {
        // 檢查指令
        if (Serial.available() > 0) {
            uint8_t cmd = Serial.read();
            if (cmd == 0x09) {
                is_running = true;
                for (int i = 0; i < QUEUE_DEPTH; i++) {
                    spi_slave_queue_trans(VSPI_HOST, &trans[i], portMAX_DELAY);
                }
            } else if (cmd == 's') {
                is_running = false;
                init_spi_slave();
            }
        }

        // 從隊列取出資料發送
        if (is_running) {
            spi_slave_transaction_t* t_to_send;
            if (xQueueReceive(dataQueue, &t_to_send, pdMS_TO_TICKS(5))) {
                Serial.write(SYNC_WORD, 2);
                Serial.write((uint8_t*)t_to_send->rx_buffer, CHUNK_SIZE);
                // 發送完立刻重新排入 SPI 接收隊列
                spi_slave_queue_trans(VSPI_HOST, t_to_send, portMAX_DELAY);
            }
        }
        vTaskDelay(1); // 讓出 CPU 給系統
    }
}

void setup() {
    Serial.begin(1000000); // 建議提升至 1M
    dataQueue = xQueueCreate(QUEUE_DEPTH, sizeof(spi_slave_transaction_t*));
    
    for (int i = 0; i < QUEUE_DEPTH; i++) {
        spi_buffer[i] = (uint8_t*)heap_caps_malloc(CHUNK_SIZE, MALLOC_CAP_DMA);
        memset(&trans[i], 0, sizeof(spi_slave_transaction_t));
        trans[i].length = CHUNK_SIZE * 8;
        trans[i].rx_buffer = spi_buffer[i];
    }
    init_spi_slave();

    // 啟動雙核心任務
    xTaskCreatePinnedToCore(spi_task, "SPI_Task", 4096, NULL, 3, NULL, 0); // 核心 0
    xTaskCreatePinnedToCore(serial_task, "Serial_Task", 4096, NULL, 2, NULL, 1); // 核心 1
}

void loop() {
    // 空循環，邏輯已移至任務中
    vTaskDelete(NULL); 
}
