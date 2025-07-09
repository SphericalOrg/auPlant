#include "SysProvEvent.h"
#include <WiFiProv.h>
#include <WiFi.h>

// Variable para controlar el estado del provisioning
bool provisioningComplete = false;

void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nConnected IP address : ");
      Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      provisioningComplete = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("\nDisconnected. Connecting to the AP again... ");
      provisioningComplete = false;
      break;
    case ARDUINO_EVENT_PROV_START:
      Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
      break;
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("\nSoftAP started for provisioning");
      Serial.print("AP IP address: ");
      Serial.println(WiFi.softAPIP());
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      Serial.println("\nSoftAP stopped");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("\nDevice connected to provisioning AP");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("\nDevice disconnected from provisioning AP");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
      Serial.println("\nReceived Wi-Fi credentials");
      Serial.print("\tSSID : ");
      Serial.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
      Serial.print("\tPassword : ");
      Serial.println((const char *)sys_event->event_info.prov_cred_recv.password);
      break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL: {
      Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
      if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
        Serial.println("\nWi-Fi AP password incorrect");
      } else {
        Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
      }
      break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("\nProvisioning Successful");
      break;
    case ARDUINO_EVENT_PROV_END:
      Serial.println("\nProvisioning Ends");
      provisioningComplete = true;
      break;
    default:
      break;
  }
}
