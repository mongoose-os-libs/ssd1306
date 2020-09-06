/**
 * Copyright 2018 Brandon Davidson <brad@oatmail.org>
 * copyright 2018 Manfred Mueller-Spaeth <fms1961@gmail.com> (changes, additions)
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
#include "ssd1306.h"

#ifdef __GNUC__
#define UNUSED(x) x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

typedef struct mgos_ssd1306 {
  uint8_t address;              // I2C address
  uint8_t width;                // panel width
  uint8_t height;               // panel height
  uint8_t col_offset;           // some displays have panel's starting column
                                // connected to seg pin other than 0.
  uint8_t com_pins;             // COM pins configuration
  uint8_t refresh_top;          // 'Dirty' window corners
  uint8_t refresh_left;
  uint8_t refresh_right;
  uint8_t refresh_bottom;
  const font_info_t *font;      // current font
  struct mgos_i2c *i2c;         // i2c connection
  uint8_t buffer[0];            // display buffer (continues beyond the struct)
} mgos_ssd1306;

static struct mgos_ssd1306 *s_global_ssd1306;

static inline bool _command (struct mgos_ssd1306 *oled, uint8_t cmd) {
  return mgos_i2c_write_reg_b (oled->i2c, oled->address, 0x80, cmd);
}

struct mgos_ssd1306 *mgos_ssd1306_create (const struct mgos_config_ssd1306 *cfg) {
  struct mgos_ssd1306 *oled = NULL;
  oled = calloc (1, sizeof (*oled) + cfg->width * cfg->height / 8);
  if (oled == NULL)
    return NULL;

  oled->address = cfg->address;
  oled->width = cfg->width;
  oled->height = cfg->height;
  oled->col_offset = cfg->col_offset;
  oled->com_pins = cfg->com_pins;
  if (cfg->i2c.enable && cfg->i2c.scl_gpio != -1 && cfg->i2c.sda_gpio != -1) {
    LOG (LL_INFO, ("Using SSD1306 GPIO config"));
    const struct mgos_config_i2c i2c_cfg = {
      .enable = cfg->i2c.enable,
#ifdef MGOS_CONFIG_HAVE_I2C_UNIT_NO
      .unit_no = cfg->i2c.unit_no,
#endif
      .freq = cfg->i2c.freq,
      .debug = cfg->i2c.debug,
      .scl_gpio = cfg->i2c.scl_gpio,
      .sda_gpio = cfg->i2c.sda_gpio
    };
    oled->i2c = mgos_i2c_create (&i2c_cfg);
  } else {
    LOG (LL_INFO, ("Using global GPIO config"));
    oled->i2c = mgos_i2c_get_global ();
  }

  if (oled->i2c == NULL) {
    goto out_err;
  }

  LOG (LL_DEBUG, ("Sending controller startup sequence"));

  _command (oled, 0xae);        // SSD1306_DISPLAYOFF
  _command (oled, 0xa8);        // SSD1306_SETMULTIPLEX
  _command (oled, oled->height - 1);
  _command (oled, 0xd3);        // SSD1306_SETDISPLAYOFFSET
  _command (oled, 0x00);        // 0 no offset
  _command (oled, 0x40);        // SSD1306_SETSTARTLINE line #0
  _command (oled, 0xa1);        // SSD1306_SEGREMAP | 1
  _command (oled, 0xc8);        // SSD1306_COMSCANDEC
  _command (oled, 0xda);        // SSD1306_SETCOMPINS
  _command (oled, oled->com_pins);
  _command (oled, 0x81);        // SSD1306_SETCONTRAST
  _command (oled, 0x7f);        // default contrast ratio
  _command (oled, 0xa4);        // SSD1306_DISPLAYALLON_RESUME
  _command (oled, 0xa6);        // SSD1306_NORMALDISPLAY
  _command (oled, 0xd5);        // SSD1306_SETDISPLAYCLOCKDIV
  _command (oled, 0x80);        // Suggested value 0x80
  _command (oled, 0xdb);        // SSD1306_VCOMMDESELECT
  _command (oled, 0x20);        // 0.77 * Vcc (default)
  _command (oled, 0x8d);        // SSD1306_CHARGEPUMP
  _command (oled, 0x14);        // Charge pump on, 0x10 for external Vcc
  _command (oled, 0x20);        // SSD1306_MEMORYMODE
  _command (oled, 0x00);        // 0x0 act like ks0108
  _command (oled, 0xd9);        // SSD1306_SETPRECHARGE
  _command (oled, 0xF1);        // 0x22 for external Vcc

  LOG (LL_DEBUG, ("Clearing screen buffer"));
  mgos_ssd1306_clear (oled);
  mgos_ssd1306_refresh (oled, true);
  mgos_ssd1306_select_font (oled, 0);

  LOG (LL_DEBUG, ("Turning on display"));
  _command (oled, 0x2e);        // SSD1306_SCROLLSTOP
  _command (oled, 0xaf);        // SSD1306_DISPLAYON

  LOG (LL_INFO, ("SSD1306 init ok (width: %d, height: %d, address: 0x%02x)", oled->width, oled->height, oled->address));
  return oled;

out_err:
  LOG (LL_ERROR, ("SSD1306 setup failed"));
  free (oled);
  return NULL;
}

void mgos_ssd1306_close (struct mgos_ssd1306 *oled) {
  if (oled == NULL)
    return;

  _command (oled, 0xae);        // SSD_DISPLAYOFF
  _command (oled, 0x8d);        // SSD1306_CHARGEPUMP
  _command (oled, 0x10);        // Charge pump off

  if (oled->i2c)
    mgos_i2c_close (oled->i2c);

  if (oled->buffer)
    free (oled->buffer);

  free (oled);
}

uint8_t mgos_ssd1306_get_width (struct mgos_ssd1306 *oled) {
  if (oled == NULL)
    return 0;

  return oled->width;
}

uint8_t mgos_ssd1306_get_height (struct mgos_ssd1306 * oled) {
  if (oled == NULL)
    return 0;

  return oled->height;
}

void mgos_ssd1306_clear (struct mgos_ssd1306 *oled) {
  if (oled == NULL)
    return;

  memset (oled->buffer, 0, (oled->width * oled->height / 8));
  oled->refresh_right = oled->width - 1;
  oled->refresh_bottom = oled->height - 1;
  oled->refresh_top = 0;
  oled->refresh_left = 0;
}

void mgos_ssd1306_refresh (struct mgos_ssd1306 *oled, bool force) {
  uint8_t page_start, page_end;

  if (oled == NULL)
    return;

  if (force || (oled->refresh_top <= 0 && oled->refresh_bottom >= oled->height - 1 && oled->refresh_left <= 0 && oled->refresh_right >= oled->width - 1)) {
    _command (oled, 0x21);                                // SSD1306_COLUMNADDR
    _command (oled, oled->col_offset);                    // column start
    _command (oled, oled->col_offset + oled->width - 1);  // column end
    _command (oled, 0x22);                      // SSD1306_PAGEADDR
    _command (oled, 0);                         // page start
    _command (oled, (oled->height / 8) - 1);    // page end
    mgos_i2c_write_reg_n (oled->i2c, oled->address, 0x40, oled->height * oled->width / 8, oled->buffer);
  } else if ((oled->refresh_top <= oled->refresh_bottom)
             && (oled->refresh_left <= oled->refresh_right)) {
    page_start = oled->refresh_top / 8;
    page_end = oled->refresh_bottom / 8;
    _command (oled, 0x21);                                    // SSD1306_COLUMNADDR
    _command (oled, oled->col_offset + oled->refresh_left);   // column start
    _command (oled, oled->col_offset + oled->refresh_right);  // column end
    _command (oled, 0x22);        // SSD1306_PAGEADDR
    _command (oled, page_start);  // page start
    _command (oled, page_end);    // page end

    for (uint8_t i = page_start; i <= page_end; ++i) {
      uint16_t start = i * oled->width + oled->refresh_left;
      uint16_t len = oled->refresh_right - oled->refresh_left + 1;
      mgos_i2c_write_reg_n (oled->i2c, oled->address, 0x40, len, oled->buffer + start);
    }
  }
  // reset dirty area
  oled->refresh_top = 255;
  oled->refresh_left = 255;
  oled->refresh_right = 0;
  oled->refresh_bottom = 0;
}

void mgos_ssd1306_draw_pixel (struct mgos_ssd1306 *oled, int8_t x, int8_t y, mgos_ssd1306_color_t color) {
  uint16_t UNUSED (index) = x + (y / 8) * oled->width;
  if (oled == NULL)
    return;

  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;

  switch (color) {
  case SSD1306_COLOR_WHITE:
    oled->buffer[index] |= (1 << (y & 7));
    break;
  case SSD1306_COLOR_BLACK:
    oled->buffer[index] &= ~(1 << (y & 7));
    break;
  case SSD1306_COLOR_INVERT:
    oled->buffer[index] ^= (1 << (y & 7));
    break;
  default:
    break;
  }
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x)
    oled->refresh_right = x;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y)
    oled->refresh_bottom = y;
}

void mgos_ssd1306_draw_hline (struct mgos_ssd1306 *oled, int8_t x, int8_t y, uint8_t w, mgos_ssd1306_color_t color) {
  uint16_t UNUSED (index);
  uint8_t mask, t;

  if (oled == NULL)
    return;
  // boundary check
  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;
  if (w == 0)
    return;
  if (x + w > oled->width)
    w = oled->width - x;

  t = w;
  index = x + (y / 8) * oled->width;
  mask = 1 << (y & 7);
  switch (color) {
  case SSD1306_COLOR_WHITE:
    while (t--) {
      oled->buffer[index] |= mask;
      ++index;
    }
    break;
  case SSD1306_COLOR_BLACK:
    mask = ~mask;
    while (t--) {
      oled->buffer[index] &= mask;
      ++index;
    }
    break;
  case SSD1306_COLOR_INVERT:
    while (t--) {
      oled->buffer[index] ^= mask;
      ++index;
    }
    break;
  default:
    break;
  }
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x + w - 1)
    oled->refresh_right = x + w - 1;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y)
    oled->refresh_bottom = y;
}

void mgos_ssd1306_draw_vline (struct mgos_ssd1306 *oled, int8_t x, int8_t y, uint8_t h, mgos_ssd1306_color_t color) {
  uint16_t UNUSED (index);
  uint8_t mask, mod, t;

  if (oled == NULL)
    return;
  // boundary check
  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;
  if (h == 0)
    return;
  if (y + h > oled->height)
    h = oled->height - y;

  t = h;
  index = x + (y / 8) * oled->width;
  mod = y & 7;
  if (mod)                      // partial line that does not fit into byte at top
  {
    // Magic from Adafruit
    mod = 8 - mod;
    static const uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    mask = premask[mod];
    if (t < mod)
      mask &= (0xFF >> (mod - t));
    switch (color) {
    case SSD1306_COLOR_WHITE:
      oled->buffer[index] |= mask;
      break;
    case SSD1306_COLOR_BLACK:
      oled->buffer[index] &= ~mask;
      break;
    case SSD1306_COLOR_INVERT:
      oled->buffer[index] ^= mask;
      break;
    default:
      break;
    }
    if (t < mod)
      goto draw_vline_finish;
    t -= mod;
    index += oled->width;
  }
  if (t >= 8)                   // byte aligned line at middle
  {
    switch (color) {
    case SSD1306_COLOR_WHITE:
      do {
        oled->buffer[index] = 0xff;
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    case SSD1306_COLOR_BLACK:
      do {
        oled->buffer[index] = 0x00;
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    case SSD1306_COLOR_INVERT:
      do {
        oled->buffer[index] = ~oled->buffer[index];
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    default:
      break;
    }
  }
  if (t)                        // // partial line at bottom
  {
    mod = t & 7;
    static const uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    mask = postmask[mod];
    switch (color) {
    case SSD1306_COLOR_WHITE:
      oled->buffer[index] |= mask;
      break;
    case SSD1306_COLOR_BLACK:
      oled->buffer[index] &= ~mask;
      break;
    case SSD1306_COLOR_INVERT:
      oled->buffer[index] ^= mask;
      break;
    default:
      break;
    }
  }
draw_vline_finish:
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x)
    oled->refresh_right = x;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y + h - 1)
    oled->refresh_bottom = y + h - 1;
  return;
}

void mgos_ssd1306_draw_rectangle (struct mgos_ssd1306 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h, mgos_ssd1306_color_t color) {
  mgos_ssd1306_draw_hline (oled, x, y, w, color);
  mgos_ssd1306_draw_hline (oled, x, y + h - 1, w, color);
  mgos_ssd1306_draw_vline (oled, x, y, h, color);
  mgos_ssd1306_draw_vline (oled, x + w - 1, y, h, color);
}

void mgos_ssd1306_fill_rectangle (struct mgos_ssd1306 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h, mgos_ssd1306_color_t color) {
  // Can be optimized?
  uint8_t i;
  for (i = x; i < x + w; ++i)
    mgos_ssd1306_draw_vline (oled, i, y, h, color);
}

void mgos_ssd1306_draw_circle (struct mgos_ssd1306 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_ssd1306_color_t color) {
  // Refer to http://en.wikipedia.org/wiki/Midpoint_circle_algorithm for the algorithm

  int8_t x = r;
  int8_t y = 1;
  int16_t radius_err = 1 - x;

  if (oled == NULL)
    return;

  if (r == 0)
    return;

  mgos_ssd1306_draw_pixel (oled, x0 - r, y0, color);
  mgos_ssd1306_draw_pixel (oled, x0 + r, y0, color);
  mgos_ssd1306_draw_pixel (oled, x0, y0 - r, color);
  mgos_ssd1306_draw_pixel (oled, x0, y0 + r, color);

  while (x >= y) {
    mgos_ssd1306_draw_pixel (oled, x0 + x, y0 + y, color);
    mgos_ssd1306_draw_pixel (oled, x0 - x, y0 + y, color);
    mgos_ssd1306_draw_pixel (oled, x0 + x, y0 - y, color);
    mgos_ssd1306_draw_pixel (oled, x0 - x, y0 - y, color);
    if (x != y) {
      /* Otherwise the 4 drawings below are the same as above, causing
       * problem when color is INVERT
       */
      mgos_ssd1306_draw_pixel (oled, x0 + y, y0 + x, color);
      mgos_ssd1306_draw_pixel (oled, x0 - y, y0 + x, color);
      mgos_ssd1306_draw_pixel (oled, x0 + y, y0 - x, color);
      mgos_ssd1306_draw_pixel (oled, x0 - y, y0 - x, color);
    }
    ++y;
    if (radius_err < 0) {
      radius_err += 2 * y + 1;
    } else {
      --x;
      radius_err += 2 * (y - x + 1);
    }

  }
}

void mgos_ssd1306_fill_circle (struct mgos_ssd1306 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_ssd1306_color_t color) {
  int8_t x = 1;
  int8_t y = r;
  int16_t radius_err = 1 - y;
  int8_t x1;

  if (oled == NULL)
    return;

  if (r == 0)
    return;

  mgos_ssd1306_draw_vline (oled, x0, y0 - r, 2 * r + 1, color); // Center vertical line
  while (y >= x) {
    mgos_ssd1306_draw_vline (oled, x0 - x, y0 - y, 2 * y + 1, color);
    mgos_ssd1306_draw_vline (oled, x0 + x, y0 - y, 2 * y + 1, color);
    if (color != SSD1306_COLOR_INVERT) {
      mgos_ssd1306_draw_vline (oled, x0 - y, y0 - x, 2 * x + 1, color);
      mgos_ssd1306_draw_vline (oled, x0 + y, y0 - x, 2 * x + 1, color);
    }
    ++x;
    if (radius_err < 0) {
      radius_err += 2 * x + 1;
    } else {
      --y;
      radius_err += 2 * (x - y + 1);
    }
  }

  if (color == SSD1306_COLOR_INVERT) {
    x1 = x;                     // Save where we stopped

    y = 1;
    x = r;
    radius_err = 1 - x;
    mgos_ssd1306_draw_hline (oled, x0 + x1, y0, r - x1 + 1, color);
    mgos_ssd1306_draw_hline (oled, x0 - r, y0, r - x1 + 1, color);
    while (x >= y) {
      mgos_ssd1306_draw_hline (oled, x0 + x1, y0 - y, x - x1 + 1, color);
      mgos_ssd1306_draw_hline (oled, x0 + x1, y0 + y, x - x1 + 1, color);
      mgos_ssd1306_draw_hline (oled, x0 - x, y0 - y, x - x1 + 1, color);
      mgos_ssd1306_draw_hline (oled, x0 - x, y0 + y, x - x1 + 1, color);
      ++y;
      if (radius_err < 0) {
        radius_err += 2 * y + 1;
      } else {
        --x;
        radius_err += 2 * (y - x + 1);
      }
    }
  }
}

void mgos_ssd1306_select_font (struct mgos_ssd1306 *oled, uint8_t font) {
  if (oled == NULL)
    return;
  if (font < NUM_FONTS)
    oled->font = fonts[font];
}

// return character width
uint8_t mgos_ssd1306_draw_char (struct mgos_ssd1306 *oled, uint8_t x, uint8_t y, unsigned char c, mgos_ssd1306_color_t foreground, mgos_ssd1306_color_t background) {
  uint8_t i, j;
  const uint8_t UNUSED (*bitmap);
  uint8_t line = 0;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  // we always have space in the font set
  if ((c < oled->font->char_start) || (c > oled->font->char_end))
    c = ' ';
  c = c - oled->font->char_start;       // c now become index to tables
  bitmap = oled->font->bitmap + oled->font->char_descriptors[c].offset;
  for (j = 0; j < oled->font->height; ++j) {
    for (i = 0; i < oled->font->char_descriptors[c].width; ++i) {
      if (i % 8 == 0) {
        line = bitmap[(oled->font->char_descriptors[c].width + 7) / 8 * j + i / 8];     // line data
      }
      if (line & 0x80) {
        mgos_ssd1306_draw_pixel (oled, x + i, y + j, foreground);
      } else {
        switch (background) {
        case SSD1306_COLOR_TRANSPARENT:
          // Not drawing for transparent background
          break;
        case SSD1306_COLOR_WHITE:
        case SSD1306_COLOR_BLACK:
          mgos_ssd1306_draw_pixel (oled, x + i, y + j, background);
          break;
        case SSD1306_COLOR_INVERT:
          // I don't know why I need invert background
          break;
        }
      }
      line = line << 1;
    }
  }
  return (oled->font->char_descriptors[c].width);
}

uint8_t mgos_ssd1306_draw_string_color (struct mgos_ssd1306 * oled, uint8_t x, uint8_t y, const char *str,
                                        mgos_ssd1306_color_t foreground, mgos_ssd1306_color_t background) {
  uint8_t t = x;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  if (str == NULL)
    return 0;

  while (*str) {
    x += mgos_ssd1306_draw_char (oled, x, y, *str, foreground, background);
    ++str;
    if (*str)
      x += oled->font->c;
  }

  return (x - t);
}

uint8_t mgos_ssd1306_draw_string (struct mgos_ssd1306 * oled, uint8_t x, uint8_t y, const char *str) {
  return mgos_ssd1306_draw_string_color (oled, x, y, str, SSD1306_COLOR_WHITE, SSD1306_COLOR_TRANSPARENT);
}

// return width of string
uint8_t mgos_ssd1306_measure_string (struct mgos_ssd1306 * oled, const char *str) {
  uint8_t w = 0;
  unsigned char c;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  while (*str) {
    c = *str;
    // we always have space in the font set
    if ((c < oled->font->char_start) || (c > oled->font->char_end))
      c = ' ';
    c = c - oled->font->char_start;     // c now become index to tables
    w += oled->font->char_descriptors[c].width;
    ++str;
    if (*str)
      w += oled->font->c;
  }
  return w;
}

uint8_t mgos_ssd1306_get_font_height (struct mgos_ssd1306 * oled) {

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  return (oled->font->height);
}

uint8_t mgos_ssd1306_get_font_c (struct mgos_ssd1306 * oled) {

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  return (oled->font->c);
}

void mgos_ssd1306_invert_display (struct mgos_ssd1306 *oled, bool invert) {
  if (oled == NULL)
    return;

  if (invert)
    _command (oled, 0xa7);      // SSD1306_INVERTDISPLAY
  else
    _command (oled, 0xa6);      // SSD1306_NORMALDISPLAY
}

void mgos_ssd1306_rotate_display (struct mgos_ssd1306 *oled, bool alt) {
  if (oled == NULL)
    return;

  if (alt) {
    _command (oled, 0xA1);
    _command (oled, 0xC8);
  } else {
    _command (oled, 0xA0);
    _command (oled, 0xC0);
  }
}

void mgos_ssd1306_flip_display (struct mgos_ssd1306 *oled, bool horizontal, bool vertical) {
  if (oled == NULL)
    return;

  _command (oled, 0xda);
  _command (oled, oled->com_pins | (horizontal << 5));
  _command (oled, vertical ? 0xc0 : 0xc8);
}

void mgos_ssd1306_update_buffer (struct mgos_ssd1306 *oled, uint8_t * data, uint16_t length) {
  if (oled == NULL)
    return;

  memcpy (oled->buffer, data, (length < (oled->width * oled->height / 8)) ? length : (oled->width * oled->height / 8));
  oled->refresh_right = oled->width - 1;
  oled->refresh_bottom = oled->height - 1;
  oled->refresh_top = 0;
  oled->refresh_left = 0;
}

void mgos_ssd1306_command (struct mgos_ssd1306 *oled, uint8_t cmd) {
  if (oled == NULL)
    return;

  _command (oled, cmd);
}

void mgos_ssd1306_start (struct mgos_ssd1306 *oled) {
  int rstPin = mgos_sys_config_get_ssd1306_rst_gpio();
  if (rstPin >= 0) {
    LOG (LL_INFO, ("Found reset pin %d", rstPin));
    mgos_gpio_setup_output(rstPin, 0);
    mgos_msleep(200);
    mgos_gpio_write(rstPin, 1);
  }
  (void) oled;
}

bool mgos_ssd1306_init (void) {
  if (!mgos_sys_config_get_ssd1306_enable ())
    return true;

  s_global_ssd1306 = mgos_ssd1306_create (mgos_sys_config_get_ssd1306 ());

  return (s_global_ssd1306 != NULL);
}

struct mgos_ssd1306 *mgos_ssd1306_get_global (void) {
  return s_global_ssd1306;
}
