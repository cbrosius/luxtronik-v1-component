```yaml
esphome:
  name: luxtronik-test
  friendly_name: luxtronik-test

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  source: github://cbrosius/luxtronik-v1-component
  refresh: 0s

# Enable logging
logger:

# Enable Home Assistant API
api:
  encryption:
    key: "bLZheU0A9/psjJSwBSGbSuDoOimDGMMv1eioi6Zs/H8="

ota:
  - platform: esphome
    password: "93952becc227240faf59a239f20d83da"

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

uart:
  id: uart_bus
  rx_pin: GPIO20
  tx_pin: GPIO21
  baud_rate: 57600

sensor:
  - platform: luxtronik_v1_sensor
    id: luxtronik_v1
    uart_id: uart_bus
    temp_VL:
      name: "Vorlauftemperatur"
    temp_RL:
      name: "Rücklauftemperatur"
    temp_RL_Soll:
      name: "Rücklauf Solltemperatur"
    update_interval: 60s
```