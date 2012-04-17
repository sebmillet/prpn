// Wxskin.cpp
// wxWidgets UI implementation of skins, like HP28S's

// RPN calculator

// Sébastien Millet
// March, April 2012

#include "Ui.h"
#include <wx/wx.h>
#include <wx/mstream.h>

#include "platform/os_generic.h"

#include "Wxskin.h"

#include "xpm/shiftuns_s1.xpm"
#include "xpm/shiftsel_s1.xpm"
#include "xpm/unit_bin_s1.xpm"
#include "xpm/unit_oct_s1.xpm"
#include "xpm/unit_dec_s1.xpm"
#include "xpm/unit_hex_s1.xpm"
#include "xpm/exec_norun_s1.xpm"
#include "xpm/exec_run_s1.xpm"
#include "xpm/angle_deg_s1.xpm"
#include "xpm/angle_rad_s1.xpm"
#include "xpm/white-small-unsel.xpm"
#include "xpm/white-small-sel.xpm"
#include "xpm/gray-small-unsel.xpm"
#include "xpm/gray-small-sel.xpm"
#include "xpm/red-small-unsel.xpm"
#include "xpm/red-small-sel.xpm"
#include "xpm/enter-unsel.xpm"
#include "xpm/enter-sel.xpm"
#include "xpm/white-large-unsel.xpm"
#include "xpm/white-large-sel.xpm"
#include "xpm/white-div-unsel.xpm"
#include "xpm/white-div-sel.xpm"

#ifdef PROG_WINDOWS
#include "png/hp28s_win.h"
#endif

#ifdef PROG_UNIXLIKE
#include "png/hp28s_unix.h"
#endif

// Uncomment if you wish to test a background image file (found
// in program working directory) instead of png/*.h-based data.
//#define HACK_BACKGROUND_IMAGE "hp.bmp"

using namespace std;

#define HP28S_GRAY  (wxColour(0xC8, 0xDA, 0xD0))
//#define HP28S_GRAY  (wxColour(0xFF, 0xFF, 0xFF))

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
  debug_write_v("Call to ~skin_t()");
  delete_bitmaps();
}

const wxString const_char_to_wxString(const char *);

void skin_t::load_bitmaps() {
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
  debug_write_v("Call to ~skin_btn_t()");
  delete_bitmaps();
}

void skin_btn_t::load_bitmaps() {
  delete_bitmaps();
  released = new wxBitmap(char_released);
  pressed = new wxBitmap(char_pressed);
}

const font_t skin_btn_hp28s_fonts[] = {
  {8, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 0
  {9, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 1
  {6, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 2
  {6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 3
  {9, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 4
  {6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 5
  {11, wxFONTFAMILY_SWISS,  wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 6
  {12, wxFONTFAMILY_SWISS,  wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 7
  {5, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false}    // 8
};

#ifdef PROG_WINDOWS
skin_btn_t skin_btn_hp28s[] = {
    // 1st row
  {{32,  236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU1", "_MENU1", 5, 0, NULL, NULL},
  {{76,  236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU2", "_MENU2", 5, 0, NULL, NULL},
  {{120, 236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU3", "_MENU3", 5, 0, NULL, NULL},
  {{163, 236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU4", "_MENU4", 5, 0, NULL, NULL},
  {{207, 236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU5", "_MENU5", 5, 0, NULL, NULL},
  {{250, 236, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU6", "_MENU6", 5, 0, NULL, NULL},

    // 2nd row
  {{32, 277, 35, 29}, red_small_unsel_xpm, red_small_sel_xpm, "", "_SHIFT", "_NOOP", 0, 0, NULL, NULL},
  {{76, 277, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, " MENU", "_MENU-ROOT", "_MENU-MODE", 2, 0, NULL, NULL},
  {{120, 277, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "TRIG", "_MENU-TRIG", "_MENU-LOGS", 2, 0, NULL, NULL},
  {{163, 277, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "COPY", "_COPY", "_PASTE", 2, 0, NULL, NULL},
  {{207, 277, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "USER", "_MENU-USER", "VARS", 2, 0, NULL, NULL},
  {{250, 277, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "NEXT", "_MENU-NEXT", "_MENU-PREV", 3, 0, NULL, NULL},

    // 3rd row
  {{32, 318, 79, 29}, enter_unsel_xpm, enter_sel_xpm, "ENTER", "", "_EDIT", 4, -2, NULL, NULL},
  {{120, 318, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "CHS", "_NEG", "_VUP", 0, -1, NULL, NULL},
  {{163, 318, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "EEX", "_E", "_VDOWN", 0, -1, NULL, NULL},
  {{207, 318, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "DROP", "DROP", "ROLL", 2, 0, NULL, NULL},
  {{250, 318, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "SWAP", "SWAP", "ROLLD", 2, 0, NULL, NULL},

    // 4th row
  {{32, 359, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "'", "'", "UP", 0, 0, NULL, NULL},
  {{81, 359, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "7", "7", "HOME", 1, 0, NULL, NULL},
  {{134, 359, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "8", "8", "UNDO", 1, 0, NULL, NULL},
  {{187, 359, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "9", "9", "_NOOP", 1, 0, NULL, NULL},
  {{240, 359, 45, 29}, white_div_unsel_xpm, white_div_sel_xpm, "", "/", "INV", 6, -1, NULL, NULL},

    // 5th row
  {{32, 399, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "STO", "STO", "RCL", 0, -1, NULL, NULL},
  {{81, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "4", "4", "PURGE", 1, 0, NULL, NULL},
  {{134, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "5", "5", "_NOOP", 1, 0, NULL, NULL},
  {{187, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "6", "6", "_NOOP", 1, 0, NULL, NULL},
  {{240, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "x", "*", "^", 6, -2, NULL, NULL},

    // 6th row
  {{32, 440, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "EVAL", "EVAL", "_NOOP", 2, 0, NULL, NULL},
  {{81, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "1", "1", "CONT", 1, 0, NULL, NULL},
  {{134, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "2", "2", "%", 1, 0, NULL, NULL},
  {{187, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "3", "3", "%CH", 1, 0, NULL, NULL},
  {{240, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "-", "-", "SQR", 6, -1, NULL, NULL},

    // 7th row
  {{32, 481, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "ON", "_ON", "_EXIT", 0, -1, NULL, NULL},
  {{81, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "0", "0", "CLEAR", 1, 0, NULL, NULL},
  {{134, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, ".", ".", "3.14159265359", 7, -6, NULL, NULL},
  {{187, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, ",", ",", "_NOOP", 7, -7, NULL, NULL},
  {{240, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "+", "+", "SQ", 6, -1, NULL, NULL},
};
#endif

#ifdef PROG_UNIXLIKE
skin_btn_t skin_btn_hp28s[] = {
    // 1st row
  {{17,  246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU1", "_MENU1", 8, 0, NULL, NULL},
  {{65,  246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU2", "_MENU2", 8, 0, NULL, NULL},
  {{113, 246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU3", "_MENU3", 8, 0, NULL, NULL},
  {{160, 246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU4", "_MENU4", 8, 0, NULL, NULL},
  {{208, 246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU5", "_MENU5", 8, 0, NULL, NULL},
  {{255, 246, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU6", "_MENU6", 8, 0, NULL, NULL},

    // 2nd row
  {{17, 299, 45, 40}, red_small_unsel_xpm, red_small_sel_xpm, "", "_SHIFT", "_NOOP", 0, 0, NULL, NULL},
  {{65, 299, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, " MENU", "_MENU-ROOT", "_MENU-MODE", 2, 0, NULL, NULL},
  {{112, 299, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "TRIG", "_MENU-TRIG", "_MENU-LOGS", 2, 0, NULL, NULL},
  {{160, 299, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "COPY", "_COPY", "_PASTE", 2, 0, NULL, NULL},
  {{207, 299, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "USER", "_MENU-USER", "VARS", 2, 0, NULL, NULL},
  {{255, 299, 45, 40}, white_small_unsel_xpm, white_small_sel_xpm, "NEXT", "_MENU-NEXT", "_MENU-PREV", 3, 0, NULL, NULL},

    // 3rd row
  {{17, 352, 89, 40}, enter_unsel_xpm, enter_sel_xpm, "ENTER", "", "_EDIT", 4, -2, NULL, NULL},
  {{113, 352, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "CHS", "_NEG", "_VUP", 0, -1, NULL, NULL},
  {{160, 352, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "EEX", "_E", "_VDOWN", 0, -1, NULL, NULL},
  {{208, 352, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "DROP", "DROP", "ROLL", 2, 0, NULL, NULL},
  {{255, 352, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "SWAP", "SWAP", "ROLLD", 2, 0, NULL, NULL},

    // 4th row
  {{17, 405, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "'", "'", "UP", 0, 0, NULL, NULL},
  {{72, 405, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "7", "7", "HOME", 1, 0, NULL, NULL},
  {{130, 405, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "8", "8", "UNDO", 1, 0, NULL, NULL},
  {{187, 405, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "9", "9", "_NOOP", 1, 0, NULL, NULL},
  {{245, 405, 55, 40}, white_div_unsel_xpm, white_div_sel_xpm, "", "/", "INV", 6, -1, NULL, NULL},

    // 5th row
  {{17, 457, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "STO", "STO", "RCL", 0, -1, NULL, NULL},
  {{72, 457, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "4", "4", "PURGE", 1, 0, NULL, NULL},
  {{130, 457, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "5", "5", "_NOOP", 1, 0, NULL, NULL},
  {{187, 457, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "6", "6", "_NOOP", 1, 0, NULL, NULL},
  {{245, 457, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "x", "*", "^", 6, -2, NULL, NULL},

    // 6th row
  {{17, 510, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "EVAL", "EVAL", "_NOOP", 2, 0, NULL, NULL},
  {{72, 510, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "1", "1", "CONT", 1, 0, NULL, NULL},
  {{130, 510, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "2", "2", "%", 1, 0, NULL, NULL},
  {{187, 510, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "3", "3", "%CH", 1, 0, NULL, NULL},
  {{245, 510, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "-", "-", "SQR", 6, -1, NULL, NULL},

    // 7th row
  {{17, 563, 45, 40}, gray_small_unsel_xpm, gray_small_sel_xpm, "ON", "_ON", "_EXIT", 0, -1, NULL, NULL},
  {{72, 563, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "0", "0", "CLEAR", 1, 0, NULL, NULL},
  {{130, 563, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, ".", ".", "3.14159265359", 7, -6, NULL, NULL},
  {{187, 563, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, ",", ",", "_NOOP", 7, -7, NULL, NULL},
  {{245, 563, 55, 40}, white_large_unsel_xpm, white_large_sel_xpm, "+", "+", "SQ", 6, -1, NULL, NULL},
};
#endif

struct skin_t skin_hp28s = {

    // Frame
  "hp28s",
  MB_ENFORCE_MENU,

#ifdef PROG_WINDOWS
  320,
  584,
  NULL,
  hp28s_win, sizeof(hp28s_win), wxBITMAP_TYPE_PNG,
#endif
#ifdef PROG_UNIXLIKE
  320,
  645,
  NULL,
  hp28s_unix, sizeof(hp28s_unix), wxBITMAP_TYPE_PNG,
#endif

    // Stack area
  4,
  23,

    // path
  {29, 61, 253, 16},
  HP28S_GRAY, {10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // status
  {29, 41, 253, 20},
  HP28S_GRAY, (*wxBLACK),
    // Status window bitmaps
  exec_norun_s1_xpm, exec_run_s1_xpm,
  shiftsel_s1_xpm, shiftuns_s1_xpm,
  angle_deg_s1_xpm, angle_rad_s1_xpm,
  unit_bin_s1_xpm, unit_oct_s1_xpm, unit_dec_s1_xpm, unit_hex_s1_xpm,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    // stack
#ifdef PROG_UNIXLIKE
  {29, 77, 253, 22},
  22,       // Stack step (= height of each element)
#endif
#ifdef PROG_WINDOWS
  {29, 77, 253, 21},
  21,       // Stack step (= height of each element)
#endif
  4,        // Number of items displayed in the stack
  HP28S_GRAY, (*wxBLACK), // Bg/fg color in normal mode
  (*wxBLACK), HP28S_GRAY, // Bg/fg color in inverted mode
  {14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // typein
  {-1, -1, -1, 22},
  HP28S_GRAY,
  {14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // Buttons
  4, 5,
  skin_btn_hp28s,
  sizeof(skin_btn_hp28s) / sizeof(*skin_btn_hp28s),
  5,
  skin_btn_hp28s_fonts,
  sizeof(skin_btn_hp28s_fonts) / sizeof(skin_btn_hp28s_fonts)
};

