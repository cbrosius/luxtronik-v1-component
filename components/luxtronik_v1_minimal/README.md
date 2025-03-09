```yaml
# example configuration:
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  fast_connect: True

web_server:

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
    components: [ luxtronik_v1_minimal ]
    refresh: 0s

sensor:
  - platform: luxtronik_v1_minimal
    name: Sensor mit minimaler Logik

  - platform: uptime
    name: "Uptime Sensor"

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