#include <stdio.h>
#include <string.h>
#include <vector>
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class ActiveLook {
private:
    uint16_t connection_handle = 0;
    const std::vector<uint16_t> handles = {35, 56, 32};

public:
    void sendCommand(uint8_t cmd, const uint8_t* payload = nullptr, uint16_t len = 0) {
        uint8_t buf[128];
        uint16_t total_len = 4 + len + 1;
        buf[0] = 0xFF;
        buf[1] = cmd;
        buf[2] = (uint8_t)(total_len >> 8);
        buf[3] = (uint8_t)(total_len & 0xFF);
        if (payload && len > 0) memcpy(&buf[4], payload, len);
        buf[4 + len] = 0xAA;

        for (uint16_t h : handles) {
            ble_gattc_write_no_rsp_flat(connection_handle, h, buf, total_len);
        }
    }

    void displayHello(uint16_t conn_h) {
        connection_handle = conn_h;
        uint8_t vOn = 0x01, vFlip = 0x02;
        sendCommand(0x00, &vOn, 1);
        sendCommand(0x03, &vFlip, 1);
        sendCommand(0x01); 
        vTaskDelay(pdMS_TO_TICKS(50));

        const char* msg = "gvjv";
        uint8_t txt[64] = { 0x00, 0x99, 0x00, 0x60, 0x04, 0x02, 0x0F };
        memcpy(&txt[7], msg, strlen(msg) + 1);
        sendCommand(0x37, txt, 7 + strlen(msg) + 1);
    }
};

ActiveLook myGlasses;

static int gap_event_handler(struct ble_gap_event *event, void *arg) {
    if (event->type == BLE_GAP_EVENT_DISC) {
        struct ble_hs_adv_fields fields;
        ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
        if (fields.name && strncmp((char *)fields.name, "ENGO", 4) == 0) {
            ble_gap_disc_cancel();
            
           
            struct ble_gap_conn_params cp;
            memset(&cp, 0, sizeof(cp));
            cp.scan_itvl = 16; cp.scan_window = 16; cp.itvl_min = 24; cp.itvl_max = 40; cp.supervision_timeout = 500;
            
            ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr, 30000, &cp, gap_event_handler, NULL);
        }
    } else if (event->type == BLE_GAP_EVENT_CONNECT && event->connect.status == 0) {
        vTaskDelay(pdMS_TO_TICKS(1500));
        myGlasses.displayHello(event->connect.conn_handle);
    }
    return 0;
}

extern "C" void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nimble_port_init();
    
    ble_hs_cfg.sync_cb = []() {
       
        struct ble_gap_disc_params dp;
        memset(&dp, 0, sizeof(dp)); 
        ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &dp, gap_event_handler, NULL);
    };

    xTaskCreate([](void* p) { nimble_port_run(); }, "ble", 4096, nullptr, 5, nullptr);
}