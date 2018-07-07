load('api_i2c.js');
load('api_sys.js');
load('api_config.js');

let SSD1306 = {
  // Constants
  SSD1306_COLOR_TRANSPARENT: -1,     //< Transparent (not drawing)
  SSD1306_COLOR_BLACK: 0,    //< Black (pixel off)
  SSD1306_COLOR_WHITE: 1,    //< White (or blue,ellow, pixel on)
  SSD1306_COLOR_INVERT: 2,   //< Invert pixel (XOR)

  _oled: null,
  
  _getGlobal: ffi('void *mgos_ssd1306_get_global(void)'),
  _getWidth: ffi('int mgos_ssd1306_get_width(void *)'),
  _getHeight: ffi('int mgos_ssd1306_get_height (void *)'),
  _clear: ffi('void mgos_ssd1306_clear (void *)'),
  _refresh: ffi('void mgos_ssd1306_refresh (void *, bool)'),
  _drawPixel: ffi('void mgos_ssd1306_draw_pixel (void *, int, int, int)'),
  _drawHLine: ffi('void mgos_ssd1306_draw_hline (void *, int, int, int, int)'),
  _drawVLine: ffi('void mgos_ssd1306_draw_vline (void *, int, int, int, int)'),
  _drawRectangle: ffi('void mgos_ssd1306_draw_rectangle (void *, int, int, int, int, int)'),
  _fillRectangle: ffi('void mgos_ssd1306_fill_rectangle (void *, int, int, int, int, int)'),
  _drawCircle: ffi('void mgos_ssd1306_draw_circle (void *, int, int, int, int)'),
  _fillCircle: ffi('void mgos_ssd1306_fill_circle (void *, int, int, int, int)'),
  _selectFont: ffi('void mgos_ssd1306_select_font (void *, int)'),
  _drawChar: ffi('int mgos_ssd1306_draw_char (void *, int, int, int, int, int)'),
  _drawStringColor: ffi('int mgos_ssd1306_draw_string_color(void *, int, int, char *, int, int)'),
  _drawString: ffi('int mgos_ssd1306_draw_string(void *, int, int, char *)'),
  _measureString: ffi('int mgos_ssd1306_measure_string(void *, char *)'),
  _getFontHeight: ffi('int mgos_ssd1306_get_font_height(void *)'),
  _getFontC: ffi('int mgos_ssd1306_get_font_c(void *)'),
  _invertDisplay: ffi('void mgos_ssd1306_invert_display(void *, bool)'),
  _flipDisplay: ffi('void mgos_ssd1306_flip_display(void *, bool, bool)'),
  _rotateDisplay: ffi('void mgos_ssd1306_rotate_display(void *)'),
  _updateBuffer: ffi('void mgos_ssd1306_update_buffer(void *, void *, int)'),
  _command: ffi('void mgos_ssd1306_command(void *, int)');
  
  init: function() {
    let myI2C = I2C.get_default();
    I2C.write(myI2C, Cfg.get('ssd1306.address'), "\0x0", 1, 1);
    this._oled = this._getGlobal();
    let rstPin = Cfg.get('ssd1306.rst_pin');
    if (rstPin) {
	    GPIO.set_mode(rstPin, GPIO.MODE_OUTPUT);
	    GPIO.set_pull(rstPin, GPIO.PULL_UP);
	    GPIO.write(rstPin, 0);
	    Sys.usleep(100 * 1000);
	    GPIO.write(rstPin, 1);
		}    
    this.clear();
  },

  clear: function() {
    this._clear(this._oled);
  },

  getWidth: function() {  
    return this._getWidth();
  },
  
  getHeight: function() { 
    return this._getHeight();
  },

  refresh: function(force) {
    this._refresh(this._oled, force);
  },

  drawPixel: function(x, y, color) {
    this._drawPixel(this._oled, x, y, color);
  },

  drawHLine: function(x, y, w, color) {
    this._drawHLine(this._oled, x, y, w, color);
  },
  
  drawVLine: function(x, y, h, color) {
    this._drawVLine(this._oled, x, y, h, color);
  },
  
  drawRectangle: function(x, y, w, h, color) {
    this._drawRectangle(this._oled, x, y, w, h, color);
  },
  
  fillRectangle: function(x, y, w, h, color) {
    this._fillRectangle(this._oled, x, y, w, h, color);
  },
  
  drawCircle: function(x, y, r, color) {
    this._drawCircle(this._oled, x, y, r, color);
  },
  
  fillCircle: function(x, y, r, color) {
    this._fillCircle(this._oled, x, y, r, color);
  },

  selectFont: function(font) {
    this._selectFont(this._oled, font);
  },

  drawChar: function(x, y, chr, fg, bg) {
    this._drawChar(this._oled, x, y, chr, fg, bg);
  },
  
  drawStringColor: function(x, y, txt, fg, bg) {
    this._drawStringColor(this._oled, x, y, txt, fg, bg);
  },

  measureString: function(txt) {
    this._measureString(this._oled, txt);
  },
  
  getFontHeight: function() {
    this._getFontHeight(this._oled);
  },

  getFont: function() {
    this._getFont(this._oled);
  },

  invertDisplay: function(invert) {
    this._invertDisplay(this._oled, invert);
  },

  flipDisplay: function(horizontal, vertical) {
    this._flipDisplay(this._oled, horizontal, vertical);
  },

  rotateDisplay: function() {
    this._rotateDisplay(this._oled);
  },

  updateBuffer: function(data, length) {
    this._updateBuffer(this._oled, data, length);
  },
  
};