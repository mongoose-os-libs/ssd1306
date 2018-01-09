#ifndef FONTS_H
#define FONTS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  typedef struct _font_char_desc
  {
    uint8_t width;              // Character width in pixels
    uint16_t offset;            // Offset of this character in bitmap
  } font_char_desc_t;


  typedef struct _font_info
  {
    uint8_t height;             // Character height in pixels
    uint8_t c;                  // Space between adjacent characters
    char char_start;            // First character
    char char_end;              // Last character
    const font_char_desc_t *char_descriptors;   // descriptor for each character
    const uint8_t *bitmap;      // Character bitmap
  } font_info_t;


#define NUM_FONTS 2             // Number of built-in fonts

  extern const font_info_t *fonts[NUM_FONTS];   // Built-in fonts

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FONTS */
