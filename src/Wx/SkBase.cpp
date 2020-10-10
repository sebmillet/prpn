// Wx/SkBase.cpp
// wxWidgets UI implementation of skins, general purpose file
// (= shared between skins)

// RPN calculator

// Sébastien Millet
// March, April 2012

#include "../Ui.h"

// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <wx/wx.h>
#include <wx/mstream.h>
// turn the warnings back on
#pragma GCC diagnostic pop

#include "../platform/os_generic.h"
#include "SkBase.h"

// Uncomment if you wish to test a background image file (found
// in program working directory) instead of png/*.h-based data.
//#define HACK_BACKGROUND_IMAGE "hp.bmp"

using namespace std;

void skin_t::delete_bitmaps() {
  if (status_exec_norun != NULL)
    delete status_exec_norun;
  if (status_exec_run != NULL)
    delete status_exec_run;
  if (status_shiftsel != NULL)
    delete status_shiftsel;
  if (status_shiftuns != NULL)
    delete status_shiftuns;
  if (status_angle_deg != NULL)
    delete status_angle_deg;
  if (status_angle_rad != NULL)
    delete status_angle_rad;
  if (status_unit_bin != NULL)
    delete status_unit_bin;
  if (status_unit_oct != NULL)
    delete status_unit_oct;
  if (status_unit_dec != NULL)
    delete status_unit_dec;
  if (status_unit_hex != NULL)
    delete status_unit_hex;
  if (frame_bg_image != NULL)
    delete frame_bg_image;
}

skin_t::~skin_t() {
  delete_bitmaps();
}

const wxString const_char_to_wxString(const char *);

void skin_t::load_bitmaps() {
  debug_write_v("load_bitmap() called, for the skin \"%s\"", menu_label);
  debug_write_v("Nb bytes in bitmap raw data = %lu", sizeof_raw_data_frame_bg_image);
  delete_bitmaps();

  wxImageHandler * pngLoader = new wxPNGHandler();
  wxImage::AddHandler(pngLoader);


#ifdef HACK_BACKGROUND_IMAGE
  frame_bg_image = new wxBitmap(const_char_to_wxString(HACK_BACKGROUND_IMAGE), wxBITMAP_TYPE_BMP);
#else
  wxMemoryInputStream mem(raw_data_frame_bg_image, sizeof_raw_data_frame_bg_image);
  wxImage img(mem, frame_bg_image_type);
  frame_bg_image = new wxBitmap(img);
#endif

  status_exec_norun = new wxBitmap(char_status_exec_norun);
  status_exec_run = new wxBitmap(char_status_exec_run);
  status_shiftsel = new wxBitmap(char_status_shiftsel);
  status_shiftuns = new wxBitmap(char_status_shiftuns);
  status_angle_deg = new wxBitmap(char_status_angle_deg);
  status_angle_rad = new wxBitmap(char_status_angle_rad);
  status_unit_bin = new wxBitmap(char_status_unit_bin);
  status_unit_oct = new wxBitmap(char_status_unit_oct);
  status_unit_dec = new wxBitmap(char_status_unit_dec);
  status_unit_hex = new wxBitmap(char_status_unit_hex);
  for (int i = 0; i < nb_btns; i++) {
    btns[i].load_bitmaps();
  }
}

void skin_btn_t::delete_bitmaps() {
  if (released != NULL)
    delete released;
  if (pressed != NULL)
    delete pressed;
}

skin_btn_t::~skin_btn_t() {
  delete_bitmaps();
}

void skin_btn_t::load_bitmaps() {
  delete_bitmaps();
  released = new wxBitmap(char_released);
  pressed = new wxBitmap(char_pressed);
}

