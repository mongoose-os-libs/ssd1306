/*
 * oled_fonts.c
 *
 */
#include "fonts.h"

extern const font_info_t glcd_5x7_font_info;
extern const font_info_t tahoma_8pt_font_info;

const font_info_t * fonts[NUM_FONTS] =
{
    &glcd_5x7_font_info,
    &tahoma_8pt_font_info
};
