// Wx/SkBase.h
// wxWidgets UI implementation of skins, like HP28S's
// Include file used by skin files.

// RPN calculator

// Sébastien Millet
// March, April 2012

#ifndef WXSKIN_H
#define WXSKIN_H

#include "Common.h"

#include <wx/wx.h>

typedef enum {MB_DEFAULT, MB_ENFORCE_NO_MENU, MB_ENFORCE_MENU} menubar_t;

struct rectangle_t {
  int x;
  int y;
  int w;
  int h;
};

struct font_t {
  int pointSize;
  wxFontFamily family;
  int style;
  wxFontWeight weight;
  wxColour colour;
  bool underline;
};

struct skin_btn_t {
  rectangle_t rec;
  const char **char_released;
  const char **char_pressed;
  const char *text;
  const char *main_cmd;
  const char *alt_cmd;
  int font_index;
  int y_shift;

  wxBitmap *released;
  wxBitmap *pressed;

  void load_bitmaps();
  void delete_bitmaps();
  ~skin_btn_t();
};

struct skin_t {
  const char *menu_label;
  const char *menu_help;
  menubar_t menubar;
  int frame_w;
  int frame_h;
  wxBitmap *frame_bg_image;
  const unsigned char *raw_data_frame_bg_image;
  const unsigned long int sizeof_raw_data_frame_bg_image;
  wxBitmapType frame_bg_image_type;
  int stack_height;
  int stack_width;
  rectangle_t r_path;
  wxColour path_bg_colour;
  font_t f_path;
  rectangle_t r_status;
  wxColour status_bg_colour;
  wxColour status_fg_colour;

  const char **char_status_exec_norun;
  const char **char_status_exec_run;
  const char **char_status_shiftsel;
  const char **char_status_shiftuns;
  const char **char_status_angle_deg;
  const char **char_status_angle_rad;
  const char **char_status_unit_bin;
  const char **char_status_unit_oct;
  const char **char_status_unit_dec;
  const char **char_status_unit_hex;

  wxBitmap *status_exec_norun;
  wxBitmap *status_exec_run;
  wxBitmap *status_shiftsel;
  wxBitmap *status_shiftuns;
  wxBitmap *status_angle_deg;
  wxBitmap *status_angle_rad;
  wxBitmap *status_unit_bin;
  wxBitmap *status_unit_oct;
  wxBitmap *status_unit_dec;
  wxBitmap *status_unit_hex;

  rectangle_t r_stack;
  int stack_y_step;
  wxColour stack_bg_colour;
  wxColour stack_fg_colour;
  wxColour stack_inv_bg_colour;
  wxColour stack_inv_fg_colour;
  font_t f_stack;
  rectangle_t r_typein;
  wxColour typein_bg_colour;
  font_t f_typein;
  int y_released;
  int y_pressed;
  skin_btn_t *btns;
  int nb_btns;
  int menu_buttons_max_nb_chars;
  const font_t *btns_fonts;
  int nb_btns_fonts;

  void load_bitmaps();
  void delete_bitmaps();
  ~skin_t();
};

#endif

