```yaml
# example configuration:

esphome:
  name: luxtronik-test
  friendly_name: luxtronik-test

esp32:
  board: esp32dev
  framework:
    type: esp-idf

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  fast_connect: True

# Enable logging
logger:
  level: VERBOSE
  # disable serial loggin
  baud_rate: 0

web_server:

api:

ota:
  platform: esphome

uart:
  id: uart_bus
  rx_pin: GPIO2
  tx_pin: GPIO4
  baud_rate: 57600
  data_bits: 8
  parity: NONE
  stop_bits: 1
  debug:
    direction: BOTH
    dummy_receiver: false

external_components:
  - source:
      type: git
      url: https://github.com/cbrosius/luxtronik-v1-component
      ref: restart
    components: [ luxtronik_v1_component ]
    refresh: 0s

luxtronik_v1_component:
  id: luxtronik_v1_component_1
  temp_vl:
    name: "Vorlauftemperatur"
    id: temp_vl
  temp_rl:
    name: "Rücklauftemperatur" 
    id: temp_rl

sensor:
  - platform: uptime
    name: "Uptime Sensor"
  - platform: template
    name: "Polling Interval"
    id: polling_interval
    unit_of_measurement: "s"
    accuracy_decimals: 0
    update_interval: 10s
    lambda: |-
      return id(luxtronik_v1_component_1).get_update_interval() / 1000.0;
    on_value:
      then:
        - lambda: |-
            id(luxtronik_v1_component_1).set_polling_interval(x * 1000);

button:
  - platform: template
    name: CTRL_CheckUART
    id: check_uart
    on_press:
      then:
        - uart.write:
            data: "\r\n"
  - platform: template
    name: CTRL_Ask for Values
    id: ask_for_values
    on_press:
      then:
        - uart.write:
            data: "1100\r\n"
```