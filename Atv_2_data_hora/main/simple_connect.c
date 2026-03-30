#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>

#define TAG "Atividade wifi data hora"

//inicializar sistema
void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Inicializando SNTP");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // Servidores brasileiros (ntp.br)
    sntp_setservername(0, "a.st1.ntp.br");
    sntp_setservername(1, "b.st1.ntp.br");
    sntp_setservername(2, "c.st1.ntp.br");

    sntp_init();
}

//obter tempo
void obtain_time(void)
{
    initialize_sntp();

    time_t now = 0;
    struct tm timeinfo = {0};

    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && retry < retry_count)
    {
        ESP_LOGI(TAG, "Aguardando sincronização... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        retry++;
    }
}

void app_main(void)
{
// System initialization
ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());

ESP_ERROR_CHECK(example_connect());

// Print out Access Point Information
wifi_ap_record_t ap_info;
ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));
ESP_LOGI(TAG, "--- Access Point Information ---");
ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);

ESP_LOGI(TAG, "============================SNTP iniciado======================");

// Ajustar timezone (Brasil)
setenv("TZ", "<-03>3", 1);
tzset();

// sincronizar hora
obtain_time();

// Loop para mostrar hora
    while (1)
    {
        time_t now;
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);

        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

        ESP_LOGI(TAG, "Data/Hora atual: %s", strftime_buf);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

}
