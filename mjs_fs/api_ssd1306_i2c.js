/**
 * Copyright 2018 Manfred Mueller-Spaeth <fms1961@gmail.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 * A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **/
 
load('api_i2c.js');
load('api_sys.js');
load('api_config.js');

let SSD1306 = {

  // some constant values
  SSD1306_COLOR_TRANSPARENT: -1,	//< Transparent (not drawing)
  SSD1306_COLOR_BLACK: 0,    			//< Black (pixel off)
  SSD1306_COLOR_WHITE: 1,    			//< White (or blue, yellow, ... -> pixel is on)
  SSD1306_COLOR_INVERT: 2,   			//< Invert pixel (XOR)

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
  _drawString: ffi('int mgos_ssd1306_draw_string(void *, int, int, char *)'),
  _drawStringColor: ffi('int mgos_ssd1306_draw_string_color(void *, int, int, char *, int, int)'),
  _measureString: ffi('int mgos_ssd1306_measure_string(void *, char *)'),
  _getFontHeight: ffi('int mgos_ssd1306_get_font_height(void *)'),
  _getFontC: ffi('int mgos_ssd1306_get_font_c(void *)'),
  _invertDisplay: ffi('void mgos_ssd1306_invert_display(void *, bool)'),
  _flipDisplay: ffi('void mgos_ssd1306_flip_display(void *, bool, bool)'),
  _rotateDisplay: ffi('void mgos_ssd1306_rotate_display(void *, int)'),
  _updateBuffer: ffi('void mgos_ssd1306_update_buffer(void *, void *, int)'),
  _command: ffi('void mgos_ssd1306_command(void *, int)'),
  _start: ffi('void mgos_ssd1306_start(void *)'),
  
  /**
   * @brief Init function, need to be called before using the api
   */
  init: function() {
    let myI2C = I2C.get_default();
    I2C.write(myI2C, Cfg.get('ssd1306.address'), "\0x0", 1, 1);
    this._oled = this._getGlobal();
    this._start(this._oled);
    this.clear();
  },

  /**
   * @brief Clears the screen
   */
  clear: function() {
    this._clear(this._oled);
  },

  /**
   * @brief Get screen width.
   *
   * @return Screen width, in pixels.
   */
  getWidth: function() {  
    return this._getWidth(this._oled);
  },
  
  /**
   * @brief Get screen height
   *
   * @return Screen height, in pixels.
   */
  getHeight: function() { 
    return this._getHeight(this._oled);
  },

  /**
   * @brief Refresh the display, sending any dirty regions to the OLED controller for display.
   * Call this after you are finished calling any drawing primitives.
   *
   * @param force Redraw the entire bitmap, not just dirty regions.
   */
  refresh: function(force) {
    this._refresh(this._oled, force);
  },

  /**
   * @brief Draw a single pixel.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param color Pixel color.
   */
  drawPixel: function(x, y, color) {
    this._drawPixel(this._oled, x, y, color);
  },

  /**
   * @brief Draw a horizontal line.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Line length.
   * @param color Line color.
   */
  drawHLine: function(x, y, w, color) {
    this._drawHLine(this._oled, x, y, w, color);
  },

  /**
   * @brief Draw a vertical line.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param h Line length.
   * @param color Line color.
   */
  drawVLine: function(x, y, h, color) {
    this._drawVLine(this._oled, x, y, h, color);
  },

  /**
   * @brief Draw an unfilled rectangle.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Rectangle width.
   * @param h Rectangle height.
   * @param color Line color.
   */
  drawRectangle: function(x, y, w, h, color) {
    this._drawRectangle(this._oled, x, y, w, h, color);
  },

  /**
   * @brief Draw a filled rectangle.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Rectangle width.
   * @param h Rectangle height.
   * @param color Line and fill color.
   */
  fillRectangle: function(x, y, w, h, color) {
    this._fillRectangle(this._oled, x, y, w, h, color);
  },

  /**
   * @brief Draw an unfilled circle.
   *
   * @param x0 Center X coordinate .
   * @param y0 Center Y coordinate.
   * @param r Radius.
   * @param color Line color.
   */
  drawCircle: function(x, y, r, color) {
    this._drawCircle(this._oled, x, y, r, color);
  },

  /**
   * @brief Draw a filled circle.
   *
   * @param x0 Center X coordinate.
   * @param y0 Center Y coordinate.
   * @param r Radius.
   * @param color Line and fill color.
   */
  fillCircle: function(x, y, r, color) {
    this._fillCircle(this._oled, x, y, r, color);
  },

  /**
   * @brief Select active font ID.
   *
   * @param font Font index; see `fonts.h`.
   */
  selectFont: function(font) {
    this._selectFont(this._oled, font);
  },

  /**
   * @brief Draw a single character using the active font and selected colors.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param c Character to draw.
   * @param foreground Foreground color.
   * @param background Background color.
   *
   * @return Character width in pixels
   */
  drawChar: function(x, y, chr, fg, bg) {
		return this._drawChar(this._oled, x, y, chr, fg, bg);
  },

  /**
   * @brief Draw a string using the active font and selected colors.
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param str String to draw.
   * @param foreground Foreground color.
   * @param background Background color.
   *
   * @return String witdth in pixels.
   */
  drawStringColor: function(x, y, txt, fg, bg) {
    return this._drawStringColor(this._oled, x, y, txt, fg, bg);
  },

  /**
   * @brief Draw a string using the active font and default colors (white on transparent)
   *
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param str String to draw.
   *
   * @return String width in pixels.
   */
  drawString: function(x, y, txt) {
    return this._drawStringColor(this._oled, x, y, txt);
  },

  /**
   * @brief Measure on-screen width of string if drawn using active font.
   *
   * @param str String to measure.
   *
   * @return String width in pixels.
   */
  measureString: function(txt) {
    return this._measureString(this._oled, txt);
  },

  /**
   * @brief Get the height of the active font.
   *
   *
   * @return Font height in pixels.
   */
  getFontHeight: function() {
    return this._getFontHeight(this._oled);
  },

  /**
   * @brief Get the inter-character font spacing.
   *
   *
   * @return Font inter-character spacing in pixels.
   */
  getFontC: function() {
    return this._getFontC(this._oled);
  },

  /**
   * @brief Invert or restore the display. Inverting the display flips the bitmask -
   * 1 is black (off) and 0 is white (on).
   *
   * @param invert Enable inversion.
   */
  invertDisplay: function(invert) {
    this._invertDisplay(this._oled, invert);
  },

  /**
   * @brief Flip the display render order. May not do exactly what you expect, depending on
   * the display's segment remap configuration.
   *
   * @param horizontal Enable horizontal flipping.
   * @param vertical Enable vertical flipping
   */
  flipDisplay: function(horizontal, vertical) {
    this._flipDisplay(this._oled, horizontal, vertical);
  },

  /**
   * @brief Rotates the display with an amount of 180 degrees
   *
   * @param boolean alt, if the current way won't work, try the second one
   */
  rotateDisplay: function(alt) {
  	this._rotateDisplay(this._oled, alt);
  },

  /**
   * @brief Copy pre-rendered bytes directly into the bitmap.
   *
   * @param data Array containing bytes to copy into buffer.
   * @param length Length to copy (unchecked) from source buffer.
   */
  updateBuffer: function(data, length) {
    this._updateBuffer(this._oled, data, length);
  },
  
  /**
   * @brief Sends a command without parameters to the display
   *
   * @param int command
   */
  command: function(cmd) {
    this._command(this._oled, cmd);
  },

};