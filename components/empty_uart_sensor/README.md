```yaml
# example configuration:

sensor:
  - platform: empty_uart_sensor
    name: Empty UART sensor

uart:
  tx_pin: GPIO0
  rx_pin: GPIO1
  baud_rate: 9600
```