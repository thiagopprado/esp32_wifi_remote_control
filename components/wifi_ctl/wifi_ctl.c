#include "wifi_ctl.h"

#include <stdbool.h>
#include <string.h>

// ESP
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "lwip/ip4_addr.h"

#define WIFI_CTL_TAG     "WIFI_CTL"

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(WIFI_CTL_TAG, "retry to connect to the AP");

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_CTL_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_ctl_setup(void) {
    esp_netif_t *sta_netif;

	esp_netif_init();
	sta_netif = esp_netif_create_default_wifi_sta();

	wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&init_cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_RC_WIFI_SSID,
            .password = CONFIG_WIFI_RC_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

	esp_netif_dhcpc_stop(sta_netif);

    esp_netif_ip_info_t ip_info;

    ip_info.ip.addr = ipaddr_addr(CONFIG_WIFI_RC_IP);
    ip_info.gw.addr = ipaddr_addr(CONFIG_WIFI_RC_GATEWAY);
    ip_info.netmask.addr = ipaddr_addr(CONFIG_WIFI_RC_NETMASK);
    esp_netif_set_ip_info(sta_netif, &ip_info);

	esp_wifi_start();
}
