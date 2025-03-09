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
      ref: Input-sensors
    components: [ luxtronik_v1_component ]
    refresh: 0s

luxtronik_v1_component:
  id: luxtronik_v1_component_1
  temperature_vorlauf:
    name: "Temperatur Vorlauf"
    id: temperature_vorlauf
  temperature_ruecklauf:
    name: "Temperatur Rücklauf"
    id: temperature_ruecklauf
  temperature_ruecklauf_soll:
    name: "Temperatur Rücklauf Soll"
    id: temperature_ruecklauf_soll
  temperature_heissgas:
    name: "Temperatur Heissgas"
    id: temperature_heissgas
  temperature_aussen:
    name: "Temperatur Aussen"
    id: temperature_aussen
  temperature_brauchwasser:
    name: "Temperatur Brauchwasser"
    id: temperature_brauchwasser
  temperature_brauchwasser_soll:
    name: "Temperatur Brauchwasser Soll"
    id: temperature_brauchwasser_soll
  temperature_waermequelle_ein:
    name: "Temperatur Wärmequelle Eingang"
    id: temperature_waermequelle_eingang
  temperature_kaeltekreis:
    name: "Temperatur Kältekreis"
    id: temperature_kaeltekreis
  temperature_mischkreis1_vorlauf:
    name: "Temperatur Mischkreis1 Vorlauf"
    id: temperature_mischkreis1_vorlauf
  temperature_mischkreis1_vorlauf_soll:
    name: "Temperatur Mischkreis1 Vorlauf Soll"
    id: temperature_mischkreis1_vorlauf_soll
  temperature_raumstation:
    name: "Temperatur Raumstation"
    id: temperature_raumstation
  eingang_abtau_soledruck_durchfluss:
    name: "Eingang Abtau Soledruck Durchfluss"
    id: eingang_abtau_soledruck_durchfluss
  eingang_sperrzeit_evu:
    name: "Eingang Sperrzeit EVU"
    id: eingang_sperrzeit_evu
  eingang_hochdruckpressostat:
    name: "Eingang Hochdruckpressostat"
    id: eingang_hochdruckpressostat
  eingang_motorschutz:
    name: "Eingang Motorschutz"
    id: eingang_motorschutz
  eingang_niederdruckpressostat:
    name: "Eingang Niederdruckpressostat"
    id: eingang_niederdruckpressostat
  eingang_fremdstromanode:
    name: "Eingang Fremdstromanode"
    id: eingang_fremdstromanode

sensor:
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