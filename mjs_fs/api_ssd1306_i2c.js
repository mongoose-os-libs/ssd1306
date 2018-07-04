load('api_i2c.js');

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
		_drawChar: ffi('int mgos_ssd1306_draw_char (void *, int, int, int, char, int)'),
		_drawStringColor: ffi('int mgos_ssd1306_draw_string_color(void *, int, int, char *, int, int)'),
		_drawString: ffi('int mgos_ssd1306_draw_string(void *, int, int, char *)'),
		_measureString: ffi('int mgos_ssd1306_measure_string(void *, char *)'),
		_getFontHeight: ffi('int mgos_ssd1306_get_font_height(void *)'),
		_getFontC: ffi('int mgos_ssd1306_get_font_c(void *)'),
		_invertDisplay: ffi('void mgos_ssd1306_invert_display(void *, bool)'),
		_flipDisplay: ffi('void mgos_ssd1306_flip_display(void *, bool, bool)'),
		_updateBuffer: ffi('void mgos_ssd1306_update_buffer(void *, int *, int)'),
		
		init: function() {
			let myI2C = I2C.get_default();
			I2C.write(myI2C, 0x3c, "\0x0", 1, 1);
			_oled = this._getGlobal();
		},

		clear: function() {
			this._clear(this._oled);
		},
		
		refresh: function() {
			this._refresh(this._oled);
		},

		selectFont: function(font) {
			this._selectFont(this._oled, font);
		},
		
		drawStringColor: function(x, y, txt, fg, bg) {
			this._drawStringColort(this._oled, x, y, txt, fg, bg);
		},

		drawString: function(x, y, txt) {
			this._drawStringt(this._oled, x, y);
		},
};