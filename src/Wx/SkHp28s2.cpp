// Wx/SkHp28s2.cpp
// wxWidgets UI implementation HP28-S skin, tiny version

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

extern unsigned char hp28sbg2[];
extern unsigned long nb_items_hp28sbg2;

#define HP28S_GRAY  (wxColour(0xC8, 0xDA, 0xD0))
//#define HP28S_GRAY  (wxColour(0xFF, 0x00, 0xFF))

//
// Fonts used in the buttons
//
static const font_t skin_btn_hp28s_fonts2[] = {
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
static skin_btn_t skin_btn_hp28s2[] = {
    // 1st row
  {{21,  225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU1", "_MENU1", 5, 0, NULL, NULL},
  {{65,  225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU2", "_MENU2", 5, 0, NULL, NULL},
  {{108,  225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU3", "_MENU3", 5, 0, NULL, NULL},
  {{151, 225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU4", "_MENU4", 5, 0, NULL, NULL},
  {{194, 225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU5", "_MENU5", 5, 0, NULL, NULL},
  {{237, 225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU6", "_MENU6", 5, 0, NULL, NULL},
  {{280, 225, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "", "_MENU7", "_MENU6", 5, 0, NULL, NULL},

    // 2nd row
  {{21, 265, 35, 29}, red_small_unsel_xpm, red_small_sel_xpm, "", "_SHIFT", "_NOOP", 0, 0, NULL, NULL},
  {{65, 265, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, " MENU", "_MENU-ROOT", "_MENU-MODE", 2, 0, NULL, NULL},
  {{108, 265, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "TRIG", "_MENU-TRIG", "_MENU-LOGS", 2, 0, NULL, NULL},
  {{151, 265, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "COPY", "_COPY", "_PASTE", 2, 0, NULL, NULL},
  {{194, 265, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "USER", "_MENU-USER", "VARS", 2, 0, NULL, NULL},
  {{237, 265, 35, 29}, white_small_unsel_xpm, white_small_sel_xpm, "NEXT", "_MENU-NEXT", "_MENU-PREV", 3, 0, NULL, NULL},
  {{280, 265, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "EVAL", "EVAL", "->", 2, 0, NULL, NULL},

    // 3rd row
  {{21, 305, 79, 29}, enter_unsel_xpm, enter_sel_xpm, "ENTER", "", "_EDIT", 4, -2, NULL, NULL},
  {{108, 305, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "CHS", "_NEG", "_VUP", 0, -1, NULL, NULL},
  {{151, 305, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "EEX", "_E", "_VDOWN", 0, -1, NULL, NULL},
  {{194, 305, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "DROP", "DROP", "ROLL", 2, 0, NULL, NULL},
  {{237, 305, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "SWAP", "SWAP", "ROLLD", 2, 0, NULL, NULL},
  {{280, 305, 35, 29}, gray_small_unsel_xpm, gray_small_sel_xpm, "ON", "_ON", "_EXIT", 0, -1, NULL, NULL}
};

//
// The skin
//
struct skin_t skin_hp28s2 = {

    // Frame
  _N("HP 28S (tiny characters, fewer buttons, 37x8)"),
  _N("Use a fixed-size skin inspired of the HP 28S scientific calculator, size: 37 (width) x 8 (lines), few buttons"),
  MB_ENFORCE_MENU,

// FIXME FIXME FIXME
/*
#ifdef PROG_UNIXLIKE
//  316,
  337,
  372,
#endif
#ifdef PROG_WINDOWS
  341,
  403,
#endif
*/
  337,
  350,

  NULL,
  hp28sbg2, nb_items_hp28sbg2, wxBITMAP_TYPE_PNG,

    // Stack area
  8,
  37,

    // path
  {19, 41, 253, 12},
  HP28S_GRAY, {8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // status
  {19, 21, 253, 20},
  HP28S_GRAY, (*wxBLACK),
    // Status window bitmaps
  exec_norun_s1_xpm, exec_run_s1_xpm,
  shiftsel_s1_xpm, shiftuns_s1_xpm,
  angle_deg_s1_xpm, angle_rad_s1_xpm,
  unit_bin_s1_xpm, unit_oct_s1_xpm, unit_dec_s1_xpm, unit_hex_s1_xpm,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    // stack
  {19, 53, 298, 16},
  16,       // Stack step (= height of each element)
  HP28S_GRAY, (*wxBLACK), // Bg/fg color in normal mode
  (*wxBLACK), HP28S_GRAY, // Bg/fg color in inverted mode
  {10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // typein
  {-1, -1, -1, 16},
  HP28S_GRAY,
  {10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, *wxBLACK, false},

    // Buttons
  4, 5,
  0, 0,
  skin_btn_hp28s2,
  sizeof(skin_btn_hp28s2) / sizeof(*skin_btn_hp28s2),
  5,
  skin_btn_hp28s_fonts2,
  sizeof(skin_btn_hp28s_fonts2) / sizeof(skin_btn_hp28s_fonts2)
};

