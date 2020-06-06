# SSD1306 OLED Display Driver Support

This library adds support for the Solomon Systech SSD1306 OLED display driver to Mongoose OS.

The default configuration and PIN mapping supports ESP32 boards with the driver hard-wired for I2C communication via GPIO pins 4 and 5. It should work on other platforms, but has not yet been tested.

This driver should support displays of any resolution supported by the SSD1306.

This driver does not yet support SPI operation.

## [LOLIN / WEMOS OLED shield](https://wiki.wemos.cc/products:d1_mini_shields:oled_shield)

It uses a 64x48 panel with column offset of 32, correct configuration for it is as follows:

```yaml
config_schema:
  - ["i2c.enable", true]
  - ["i2c.sda_gpio", 4]
  - ["i2c.scl_gpio", 5]
  - ["ssd1306.i2c.enable", false]  # Use system bus.
  - ["ssd1306.enable", true]
  - ["ssd1306.width", 64]
  - ["ssd1306.height", 48]
  - ["ssd1306.col_offset", 32]
```

## MakerHawk I2C OLED Display Module 0.91 Inch 128X32

```yaml
config_schema:
  - ["ssd1306.width", 128]
  - ["ssd1306.height", 32]
  - ["ssd1306.com_pins", 0x02]
```
