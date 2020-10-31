// Wx/SkHp28s.cpp
// wxWidgets UI implementation HP28-S skin

// RPN calculator

// Sébastien Millet
// March, April 2012

#include "../Ui.h"

// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <wx/wx.h>
// turn the warnings back on
#pragma GCC diagnostic pop

#include "../MyIntl.h"

#include "../platform/os_generic.h"
#include "SkBase.h"

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

extern unsigned char hp28sbg[];
extern unsigned long nb_items_hp28sbg;

#define HP28S_GRAY  (wxColour(0xC8, 0xDA, 0xD0))
//#define HP28S_GRAY  (wxColour(0xFF, 0x00, 0xFF))

//
// Fonts used in the buttons
//
static const font_t skin_btn_hp28s_fonts[] = {
  {8, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 0
  {9, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 1
  {6, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 2
  {6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 3
  {9, wxFONTFAMILY_SWISS,   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxWHITE, false},   // 4
#ifdef PROG_UNIXLIKE
  {5, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 5
#endif
#ifdef PROG_WINDOWS
  {6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 5
#endif
  {11, wxFONTFAMILY_SWISS,  wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false},   // 6
  {12, wxFONTFAMILY_SWISS,  wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,     *wxBLACK, false}    // 7
};

//
// Button layer
//
static skin_btn_t skin_btn_hp28s[] = {
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
  {{187, 359, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "9", "9", "_HELP_FLAGS", 1, 0, NULL, NULL},
  {{240, 359, 45, 29}, white_div_unsel_xpm, white_div_sel_xpm, "", "/", "INV", 6, -1, NULL, NULL},

    // 5th row
  {{32, 399, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "STO", "STO", "RCL", 0, -1, NULL, NULL},
  {{81, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "4", "4", "PURGE", 1, 0, NULL, NULL},
  {{134, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "5", "5", "_NOOP", 1, 0, NULL, NULL},
  {{187, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "6", "6", "_NOOP", 1, 0, NULL, NULL},
  {{240, 399, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "x", "*", "^", 6, -2, NULL, NULL},

    // 6th row
  {{32, 440, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "EVAL", "EVAL", "->", 2, 0, NULL, NULL},
  {{81, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "1", "1", "CONT", 1, 0, NULL, NULL},
  {{134, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "2", "2", "%", 1, 0, NULL, NULL},
  {{187, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "3", "3", "%CH", 1, 0, NULL, NULL},
  {{240, 440, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "-", "-", "SQR", 6, -1, NULL, NULL},

    // 7th row
  {{32, 481, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "ON", "_ON", "_EXIT", 0, -1, NULL, NULL},
  {{81, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "0", "0", "CLEAR", 1, 0, NULL, NULL},
  {{134, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, ".", ".", "3.14159265359", 7, -6, NULL, NULL},
  {{187, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, ",", ",", "_NOOP", 7, -7, NULL, NULL},
  {{240, 481, 45, 29}, white_large_unsel_xpm, white_large_sel_xpm, "+", "+", "SQ", 6, -1, NULL, NULL}
};

//
// The skin
//
struct skin_t skin_hp28s = {

    // Frame
  _N("HP 28S (regular, 23x4)"),
  _N("Use a fixed-size skin inspired of the HP 28S scientific calculator, size: 23 (width) x 4 (lines)"),
  MB_ENFORCE_MENU,

// FIXME FIXME FIXME
/*
#ifdef PROG_UNIXLIKE
  316,
  555,
#endif
#ifdef PROG_WINDOWS
  320,
  584,
#endif
*/
  316,
  532,

  NULL,
  hp28sbg, nb_items_hp28sbg, wxBITMAP_TYPE_PNG,

    // Stack area
  4,
  23,

    // path
  {29, 58, 253, 16},
  HP28S_GRAY, {10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // status
  {29, 38, 253, 20},
  HP28S_GRAY, (*wxBLACK),
    // Status window bitmaps
  exec_norun_s1_xpm, exec_run_s1_xpm,
  shiftsel_s1_xpm, shiftuns_s1_xpm,
  angle_deg_s1_xpm, angle_rad_s1_xpm,
  unit_bin_s1_xpm, unit_oct_s1_xpm, unit_dec_s1_xpm, unit_hex_s1_xpm,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    // stack
  {29, 74, 253, 22},
  22,       // Stack step (= height of each element)
  HP28S_GRAY, (*wxBLACK), // Bg/fg color in normal mode
  (*wxBLACK), HP28S_GRAY, // Bg/fg color in inverted mode
  {14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // typein
  {-1, -1, -1, 22},
  HP28S_GRAY,
  {14, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // Buttons
  4, 5,
  0, 0,
  skin_btn_hp28s,
  sizeof(skin_btn_hp28s) / sizeof(*skin_btn_hp28s),
  5,
  skin_btn_hp28s_fonts,
  sizeof(skin_btn_hp28s_fonts) / sizeof(skin_btn_hp28s_fonts)
};

