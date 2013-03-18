// Wx/UiImplWx.cpp
// wxWidgets UI implementation, master file

// RPN calculator

// Sébastien Millet
// August 2009 - March 2010

#include "../Ui.h"
#include <wx/wx.h>

#include "../platform/os_generic.h"

  // To display HTML pages
#include "wx/wxhtml.h"
#include "wx/statline.h"
#include "wx/clipbrd.h"

#include "../MyIntl.h"

#include <vector>

#ifdef DEBUG
#include <iostream>
#include <sstream>
#endif

#include "SkBase.h"

#include "xpm/shiftuns.xpm"
#include "xpm/shiftsel.xpm"
#include "xpm/unit_bin.xpm"
#include "xpm/unit_oct.xpm"
#include "xpm/unit_dec.xpm"
#include "xpm/unit_hex.xpm"
#include "xpm/exec_norun.xpm"
#include "xpm/exec_run.xpm"
#include "xpm/angle_deg.xpm"
#include "xpm/angle_rad.xpm"
#include "xpm/prpn.xpm"

using namespace std;

typedef enum {GUI_SIZER, GUI_SKIN} gui_t;

static const char *next_instruction_prefix = "";

struct sizer_color_codes_t {
  wxColor fg;
  wxColor bg;
  sizer_color_codes_t(const wxColour& f, const wxColour& g) : fg(f), bg(g) { }
};

static bool restart_gui = false;
static wxPoint frame_pos = wxPoint(wxDefaultPosition);

  // We manage two ui codes, 0 for sizer, 1 for graphic, HP-28S style, 2 for HP-28S (tiny)
  // 3 for HP-28S (tiny, regular buttons.)
extern struct skin_t skin_hp28s;
extern struct skin_t skin_hp28s2;
extern struct skin_t skin_hp28s3;
skin_t *skins[] = {&skin_hp28s, &skin_hp28s2, &skin_hp28s3};
#define MENU_LABEL_SWITCH_UI_SIZER  _N("&Basic (resizable)")
#define MENU_HELP_SWITCH_UI_SIZER   _N("Use WX 'sizer' mechanism to build the interface, can fit any size")

const int DEFAULT_WHEELDELTA_UNIT = 120;
const int MAX_WHEEL_STEPS_IN_1_EVENT = 5;



//
// UI SIZER SETTINGS CONSTANTS
//

  // Common
#define SIZER_CALCULATOR_BACKGROUND_COLOR (wxColour(0xFA, 0xFA, 0xFA))
  // Status area
#define SIZER_STATUS_BORDER               0
#define SIZER_STATUS_BORDER_STYLE         wxBORDER_NONE
#define SIZER_STATUS_BACKGROUND_COLOUR    (wxColour(0xFA, 0xFA, 0xFA))
#define SIZER_STATUS_WINDOW_ITEM_H_MARGIN 4
#define SIZER_STATUS_WINDOW_ITEM_V_MARGIN 2
  // Path
#define SIZER_PATH_BORDER                 0
#define SIZER_PATH_BORDERSTYLE            wxBORDER_NONE
#define SIZER_PATH_FONTSIZE               10
#define SIZER_PATH_FONTWEIGHT             wxFONTWEIGHT_NORMAL
#define SIZER_PATH_BACKGROUND_COLOUR      SIZER_STATUS_BACKGROUND_COLOUR
#define SIZER_PATH_FOREGROUND_COLOUR      (*wxBLACK)
  // Stack area
#define SIZER_STACK_BORDERSTYLE           wxBORDER_NONE
#define SIZER_STACK_BORDERSIZE            0
#define SIZER_STACK_FONTSIZE              14
#define SIZER_STACK_FONTWEIGHT            wxFONTWEIGHT_NORMAL
#define STACK_MIN_LINES                1
  // wxTextCtrl to type in values
#define SIZER_TYPEIN_BACKGROUND_COLOUR    (*wxWHITE)
#define SIZER_TYPEIN_FOREGROUND_COLOUR    (*wxBLACK)
#define SIZER_TYPEIN_BORDERSTYLE          wxBORDER_NONE
#define SIZER_TYPEIN_BORDERSIZE           0
#define SIZER_TYPEIN_FONTSIZE             14
#define SIZER_TYPEIN_FONTWEIGHT           wxFONTWEIGHT_NORMAL
  // Buttons layout
#define SIZER_BTN_FONTSIZE                10
#define SIZER_BTNCOLUMN_BORDER            4
#define SIZER_AROUND_BUTTONS_AREA_BORDER  4
#define SIZER_PROP_BTN_SMALL_LABELS       10
#define SIZER_PROP_BTN_BUTTONS            20
#define SIZER_BTN_FONTFAMILY              wxFONTFAMILY_SWISS
#define SIZER_BTN_FONTSTYLE               wxFONTSTYLE_NORMAL
#define SIZER_BTN_FONTWEIGHT              wxFONTWEIGHT_NORMAL
#define SIZER_ALTBTN_FONTRATIO            0.7
#define SIZER_ALTBTN_FONTFAMILY           wxFONTFAMILY_SWISS
#define SIZER_ALTBTN_FONTSTYLE            wxFONTSTYLE_ITALIC
#define SIZER_ALTBTN_FONTWEIGHT           wxFONTWEIGHT_NORMAL
#define SIZER_MENU_BUTTONS_MAX_NB_CHARS   6
  // Help screens
#define SIZER_HELP_FONTSIZE               10
#define SIZER_HELP_FONTWEIGHT             wxFONTWEIGHT_NORMAL
#define SIZER_HELP_OKBUTTON_MARGIN        5
#define SIZER_HELP_OKBUTTON_ALIGN         wxALIGN_RIGHT

  // 0: black and white
  // 1: a few colors... Well, really a little bit!
  // It is defined at compilation-time, no way to tune it in the UI.
const int sizer_actual_color_code_set = 1;
sizer_color_codes_t sizer_color_codes[2][SLCC_NB_CODES] = {{
// sizer_actual_color_code_set = 0
  sizer_color_codes_t(wxColour(*wxBLACK), wxColour(*wxWHITE)),  // SLCC_NORMAL
  sizer_color_codes_t(wxColour(*wxWHITE), wxColour(*wxBLACK)),  // SLCC_EDITED_ITEM
  sizer_color_codes_t(wxColour(*wxWHITE), wxColour(*wxBLACK)),  // SLCC_NEXT_INSTRUCTION
  sizer_color_codes_t(wxColour(*wxBLACK), wxColour(*wxWHITE))   // SLCC_ERROR
}, {
// sizer_actual_color_code_set = 1
  sizer_color_codes_t(*wxBLACK, *wxWHITE),   // SLCC_NORMAL
  sizer_color_codes_t(*wxWHITE, *wxBLACK),   // SLCC_EDITED_ITEM
  sizer_color_codes_t(*wxBLACK, *wxGREEN),   // SLCC_NEXT_INSTRUCTION
  sizer_color_codes_t(*wxRED, *wxWHITE)      // SLCC_ERROR
}};


//
// GLOBAL
//

static gui_t gui_type(const int& ui_code) {
  return ui_code == 0 ? GUI_SIZER : GUI_SKIN;
}

static const wxColor sizer_slcc_to_bg_wxColor(const slcc_t& sizer_color_code) {
  return sizer_color_codes[sizer_actual_color_code_set][sizer_color_code].bg;
}
static const wxColor sizer_slcc_to_fg_wxColor(const slcc_t& sizer_color_code) {
  return sizer_color_codes[sizer_actual_color_code_set][sizer_color_code].fg;
}

const wxString const_char_to_wxString(const char *sz) {
  if (E->get_actual_encoding() == MYENCODING_UTF8)
    return wxString::FromUTF8(sz);
  else if (E->get_actual_encoding() == MYENCODING_1BYTE)
    return wxString::From8BitData(sz);
  else
    throw(CalcFatal(__FILE__, __LINE__, "const_char_to_wxString(): unknown encoding returned by E->get_actual_encoding()"));

}

static const wxString string_to_wxString(const string& s) { return const_char_to_wxString(s.c_str()); }

static const string wxString_to_string(const wxString& wxs) {
  if (E->get_actual_encoding() == MYENCODING_UTF8)
    return string(wxs.mb_str(wxConvUTF8));
  else if (E->get_actual_encoding() == MYENCODING_1BYTE)
    return string(wxs.mb_str(wxConvLocal));
  else
    throw(CalcFatal(__FILE__, __LINE__, "wxString_to_string(): unknown encoding returned by E->get_actual_encoding()"));
}

  // Maybe we should use strlen() here for stability and portability...
  // Who knows?
static bool const_char_is_empty(const char *sz) {
  return sz[0] == '\0';
}

  // Usefule to extract the string from a wxTextCtrl -> when doing it directly, the
  // result is garbage. Only this type of invocation works...
  // FIXME: result in the copy of the control content
static const string wxTextCtrl_to_string(wxTextCtrl *t) {
  return wxString_to_string(t->GetValue());
}

  // Returns the number of newlines in a string.
static int count_newlines(const string& s, const int& max_lines) {
  int n = 0;
  for (string::const_iterator it = s.begin(); it != s.end() && n < max_lines; it++)
    if (*it == '\n')
      n++;
  return n;
}

static int my_get_max_stack() {
  int n = ui_dsl.get_max_stack();
  if (n < STACK_MIN_LINES)
    n = STACK_MIN_LINES;
  return n;
}

static void my_set_font(wxWindow *ctrl, const int& pointSize, const wxFontFamily& family, const int& style,
                  const wxFontWeight& weight, const bool& underline, font_t const *pf) {
  if (pf == NULL) {
    ctrl->SetFont(wxFont(pointSize, family, style, weight, underline));
    debug_write("  pf == NULL");
  } else {
    ctrl->SetFont(wxFont(pf->pointSize, pf->family, pf->style, pf->weight, pf->underline));
    debug_write("  pf != NULL");
    debug_write_i("  size = %i", pf->pointSize);
  }
}


//
// MYAPP and MYFRAME
//

class MyFrame;
class StatusWindow;
class myBitmapButton;

class MyApp: public wxApp {
  int what_am_i_to_do;
  MyFrame *frame;
  virtual bool OnInit();
  virtual int OnRun();
  void build_top_frame();
public:
  MyApp();
  virtual ~MyApp() { }
  virtual MyFrame* get_frame() const;
};

struct BtnSmallLabel {
  wxSizer *s;
  wxStaticText *t;
};

struct wxBtnLine {
  int btns_index;
  wxSizer *bSizer[2];
  vector<BtnSmallLabel> vl;
  vector<wxButton*> vb;
  wxBtnLine() : btns_index(-1) { }
  ~wxBtnLine() { }
};

struct wxBtnAll {
  vector<wxBtnLine*> lines;
  wxBtnAll() { }
  ~wxBtnAll() {
    for (vector<wxBtnLine*>::iterator it = lines.begin(); it != lines.end(); it++)
      delete *it;
  }
};

struct stack_1line {
  wxWindow *w;
  wxStaticText *t;
};

class MyFrame: public wxFrame {
  skin_t *skin;

  wxSizer *topSizer;

  wxBtnAll wxAllButtons;

  const MenuDescription *menus_descriptions;
  int nb_menus_descriptions;
  const BtnDescription *btn_descriptions;
  int nb_btn_descriptions;

  gui_t gui;

  bool xy_set;
  int my_x0;
  int my_w0;
  int my_h0;
  int my_y0;
  int my_y1;
  int my_char_width;
  int my_initial_stack_lines;
  int my_initial_stack_width;
  int my_initial_client_width;
  int my_initial_client_height;
  int my_min_client_width;
  int my_min_client_height;
  wxSize my_frame_initial_size;

  wxDialog *dlg;

  vector<wxButton*> sizer_menu_buttons;
  vector<myBitmapButton*> skin_menu_buttons;
  int skin_menu_button_1;

  StatusWindow *stwin;
  wxWindow *w_path;
  wxStaticText *path;
  int path_width;
  int last_read_width;

  vector<stack_1line> dispStack;
  wxTextCtrl *textTypein;
  void build_dispStack();
  void check_path_width();
  void display_help(const int&);
  void stack_line_set_font(wxStaticText*);
public:
  friend class UiImplWx;
  MyFrame(const wxString&, const wxPoint&, const wxSize&, const MenuDescription*&,
      int&, const BtnDescription*&, int&, const int&);
  void OnPaint(wxPaintEvent&);
  void OnSize(wxSizeEvent&);
  void OnAbout(wxCommandEvent&);
  void OnButton(wxCommandEvent&);
  void OnMenu(wxCommandEvent&);
  void OnChar(wxKeyEvent&);
  void OnDblClick(wxMouseEvent&);
  void OnMouseWheel(wxMouseEvent&);
  void resize_frame_to_initial();
  int get_nb_menu_buttons();
  void notify_ui_change();
  void textTypein_recalc_and_setsize();
  void textTypein_SetFocus();

  DECLARE_EVENT_TABLE()
};


//
// STATUSWINDOW
//

class StatusWindow: public wxScrolledWindow {
  MyFrame *my_parent;
  int horoffset_exec;
  wxBitmap exec_norun;
  wxBitmap exec_run;
  int horoffset_arrow;
  wxBitmap shiftsel;
  wxBitmap shiftuns;
  int horoffset_angle;
  wxBitmap angle_deg;
  wxBitmap angle_rad;
  int horoffset_unit;
  wxBitmap unit_bin;
  wxBitmap unit_oct;
  wxBitmap unit_dec;
  wxBitmap unit_hex;
  wxColour bg_color;
public:
  StatusWindow(MyFrame*, wxWindowID, const wxPoint&, const wxSize&, const wxColour&,
      const wxBitmap&, const wxBitmap&, const wxBitmap&, const wxBitmap&,
      const wxBitmap&, const wxBitmap&, const wxBitmap&, const wxBitmap&,
      const wxBitmap&, const wxBitmap&);
  void OnPaint(wxPaintEvent&);
  void OnDblClick(wxMouseEvent&);
  void OnMouseWheel(wxMouseEvent&);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(StatusWindow, wxScrolledWindow)
  EVT_PAINT(StatusWindow::OnPaint)
  EVT_LEFT_DCLICK(StatusWindow::OnDblClick)
  EVT_MOUSEWHEEL(StatusWindow::OnMouseWheel)
END_EVENT_TABLE()

StatusWindow::StatusWindow(MyFrame *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const wxColour& bgc,
      const wxBitmap& a_exec_norun, const wxBitmap& a_exec_run,
      const wxBitmap& a_shiftsel, const wxBitmap& a_shiftuns,
      const wxBitmap& a_angle_deg, const wxBitmap& a_angle_rad,
      const wxBitmap& a_unit_bin, const wxBitmap& a_unit_oct,
      const wxBitmap& a_unit_dec, const wxBitmap& a_unit_hex)
    : wxScrolledWindow(dynamic_cast<wxWindow*>(parent), id, pos, size, SIZER_STATUS_BORDER_STYLE),
    my_parent(parent),
    horoffset_exec(0),
    exec_norun(a_exec_norun), exec_run(a_exec_run),
    horoffset_arrow(0),
    shiftsel(a_shiftsel), shiftuns(a_shiftuns),
    horoffset_angle(0),
    angle_deg(a_angle_deg), angle_rad(a_angle_rad),
    horoffset_unit(0),
    unit_bin(a_unit_bin), unit_oct(a_unit_oct), unit_dec(a_unit_dec), unit_hex(a_unit_hex), bg_color(bgc) {
  SetBackgroundColour(bgc);
  int w = exec_run.GetWidth();
  int w2 = shiftsel.GetWidth();
  int h = shiftsel.GetHeight();
  int w3 = angle_deg.GetWidth();
  int w4 = unit_bin.GetWidth();
  horoffset_exec = SIZER_STATUS_WINDOW_ITEM_H_MARGIN;
  horoffset_arrow = w + 2 * SIZER_STATUS_WINDOW_ITEM_H_MARGIN;
  horoffset_angle = w + w2 + 3 * SIZER_STATUS_WINDOW_ITEM_H_MARGIN;
  horoffset_unit = w + w2 + w3 + 4 * SIZER_STATUS_WINDOW_ITEM_H_MARGIN;
  SetSize(wxSize(horoffset_unit + w4, h + 2 * SIZER_STATUS_WINDOW_ITEM_V_MARGIN));
}

void StatusWindow::OnPaint(wxPaintEvent&) {
  wxPaintDC dc(this);
  PrepareDC(dc);

  dc.SetBackgroundMode(wxSOLID);
  SetBackgroundColour(bg_color);

  if (ui_is_a_program_halted())
    dc.DrawBitmap(exec_run, horoffset_exec, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  else
    dc.DrawBitmap(exec_norun, horoffset_exec, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  if (ui_dsl.get_status_shift())
    dc.DrawBitmap(shiftsel, horoffset_arrow, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  else
    dc.DrawBitmap(shiftuns, horoffset_arrow, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  if (F->get_angle_mode() == ANGLE_DEG)
    dc.DrawBitmap(angle_deg, horoffset_angle, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  else if (F->get_angle_mode() == ANGLE_RAD)
    dc.DrawBitmap(angle_rad, horoffset_angle, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
  wxBitmap *u;
  switch (ui_dsl.get_base()) {
    case 2:
      u = &unit_bin;
      break;
    case 8:
      u = &unit_oct;
      break;
    case 10:
      u = &unit_dec;
      break;
    case 16:
      u = &unit_hex;
      break;
    default:
      throw(CalcFatal(__FILE__, __LINE__, "StatusWindow::OnPaint(): ui_dsl.get_base() returned an unknown value"));
  }
  dc.DrawBitmap(*u, horoffset_unit, SIZER_STATUS_WINDOW_ITEM_V_MARGIN);
}

void StatusWindow::OnDblClick(wxMouseEvent& ev) {
  dynamic_cast<MyFrame *>(GetParent())->resize_frame_to_initial();
  ev.Skip();
}

void StatusWindow::OnMouseWheel(wxMouseEvent& ev) {
  my_parent->OnMouseWheel(ev);
}

void MyFrame::OnDblClick(wxMouseEvent& ev) {
  resize_frame_to_initial();
  ev.Skip();
}

void MyFrame::resize_frame_to_initial() {
  if (xy_set && gui == GUI_SIZER) {
    debug_write("Resetting frame to its initial size");
    SetClientSize(my_frame_initial_size);
    textTypein->SetFocus();
  }
}

enum {ID_TEXTTYPEIN = 0,
  ID_START_BUTTONS_SIZER = 1,
  ID_START_BUTTONS_SKIN = 10000,
  ID_START_MENUS = 20000,
  ID_START_INTERFACE_CHOICE_MENUS = 30000
};


//
// MYBITMAPBUTTON
//

class myBitmapButton : public wxWindow {
  MyFrame *parent;
  wxBitmap released;
  wxBitmap pressed;
  const char *main_cmd;
  const char*alt_cmd;
  bool is_down;
public:
  myBitmapButton(MyFrame*, wxWindowID, wxBitmap*, wxBitmap*, wxPoint, wxSize, const char*, const char*);
  void OnPaint(wxPaintEvent&);
  void OnMouseButtonDown(wxMouseEvent&);
  void OnClick(wxMouseEvent&);
  void OnLeaveWindow(wxMouseEvent&);
  void mySetBitmapLabel(wxBitmap*);
  void mySetBitmapSelected(wxBitmap*);
  void change_button_status(const bool&);
  const wxBitmap *get_current_wxBitmap() const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(myBitmapButton, wxWindow)
  EVT_PAINT(myBitmapButton::OnPaint)
  EVT_LEFT_DOWN(myBitmapButton::OnMouseButtonDown)
  EVT_LEFT_UP(myBitmapButton::OnClick)
  EVT_LEAVE_WINDOW(myBitmapButton::OnLeaveWindow)
END_EVENT_TABLE()

myBitmapButton::myBitmapButton(MyFrame *p, wxWindowID id, wxBitmap *rel, wxBitmap *prs,
		wxPoint pos, wxSize size, const char *mcmd, const char *acmd)
			: wxWindow(p, id, pos, size, wxBORDER_NONE), parent(p), released(*rel), pressed(*prs),
				main_cmd(mcmd), alt_cmd(acmd), is_down(false) {
}

void myBitmapButton::OnPaint(wxPaintEvent& ev) {
  wxPaintDC dc(this);
  PrepareDC(dc);
  const wxBitmap *bm = get_current_wxBitmap();
  dc.DrawBitmap(*bm, 0, 0);
  ev.Skip();
}

void myBitmapButton::OnClick(wxMouseEvent& ev) {
  if (is_down) {
    const char *sz;
    sz = main_cmd;
    if (ui_dsl.get_status_shift() && !const_char_is_empty(alt_cmd))
      sz = alt_cmd;
    ui_notify_button_pressed(sz);
    parent->textTypein_SetFocus();
    debug_write_v("Button clicked!!! (%i)", ev.GetId());
    change_button_status(false);
  }
  ev.Skip();
}

void myBitmapButton::OnMouseButtonDown(wxMouseEvent& ev) {
  debug_write_v("MOUSE BUTTON DOWN!!! (%i)", ev.GetId());
  change_button_status(true);
  ev.Skip();
}

void myBitmapButton::OnLeaveWindow(wxMouseEvent& ev) {
  change_button_status(false);
  ev.Skip();
}

void myBitmapButton::mySetBitmapLabel(wxBitmap* bm) {
  released = *bm;
}

void myBitmapButton::mySetBitmapSelected(wxBitmap* bm) {
  pressed = *bm;
}

void myBitmapButton::change_button_status(const bool& pressed) {
  if (is_down != pressed) {
    is_down = pressed;
    Refresh();
  }
}

const wxBitmap* myBitmapButton::get_current_wxBitmap() const {
  return is_down ? &pressed : &released;
}

static void build_menu_bar(const MenuDescription* const md, const int& nb, wxMenuBar*& menuBar, const int& ui_code) {

  enum {MENUDESC_OPEN, MENUDESC_CLOSE, MENUDESC_SEPARATOR, MENUDESC_ITEM};

  const char *empty_sz = "";

  const MenuDescription *e = NULL;
  int e_type;
    menuBar = new wxMenuBar;
  vector<int> menu_stack;
  wxMenu **menus;
  menus = new wxMenu*[nb];
  int actual_menu;
  wxMenu *parent_wxMenu;
  string s;
  int builtin_id;
  for (int i = 0; i < nb || menu_stack.size() >= 1; i++) {
    if (i < nb) {
      e = &(md[i]);
      s = e->cmd;
      if (const_char_is_empty(e->text) && const_char_is_empty(e->cmd))
        e_type = MENUDESC_CLOSE;
      else if (const_char_is_empty(e->text) && s == "__SEPARATOR__")
        e_type = MENUDESC_SEPARATOR;
      else if (s == "__MENU__")
        e_type = MENUDESC_OPEN;
      else
        e_type = MENUDESC_ITEM;
    } else {
      e_type = MENUDESC_CLOSE;
    }

//        debug_write_i("i = %i", i);
//        debug_write("Command:");
//        debug_write(e->text);

    if (e_type == MENUDESC_OPEN || (e_type != MENUDESC_CLOSE && menu_stack.size() == 0)) {
      menu_stack.push_back(i);
      menus[i] = new wxMenu;
    }
    if (e_type == MENUDESC_CLOSE) {
      if (menu_stack.size() >= 1) {
        actual_menu = menu_stack.back();
        menu_stack.pop_back();
        if (menu_stack.size() >= 1) {
          parent_wxMenu = menus[menu_stack.back()];
          parent_wxMenu->AppendSubMenu(menus[actual_menu], const_char_to_wxString(_(md[actual_menu].text)));
        } else {
          string l = md[actual_menu].text;
          bool interface_menu = false;
          debug_write_v("Appending menu '%s'", l.c_str());
          if (l == _N("__INTERFACE__")) {
            interface_menu = true;
            l = "Interface";
          }
          menuBar->Append(menus[actual_menu], const_char_to_wxString(_(l.c_str())));
          if (interface_menu) {
            wxMenuItem *wxmi0 = menus[actual_menu]->AppendCheckItem(ID_START_INTERFACE_CHOICE_MENUS,
                const_char_to_wxString(_(MENU_LABEL_SWITCH_UI_SIZER)), const_char_to_wxString(_(MENU_HELP_SWITCH_UI_SIZER)));
            wxmi0->Check(ui_code == 0);
            wxmi0->Enable(ui_code != 0);
            for (int u = 0; static_cast<unsigned int>(u) < sizeof(skins) / sizeof(*skins); u++) {
              wxMenuItem *wxmi1 = menus[actual_menu]->AppendCheckItem(ID_START_INTERFACE_CHOICE_MENUS + u + 1,
                  const_char_to_wxString(_(skins[u]->menu_label)), const_char_to_wxString(_(skins[u]->menu_help)));
              wxmi1->Check(ui_code == u + 1);
              wxmi1->Enable(ui_code != u + 1);
            }
          }
        }
      }
    } else if (e_type == MENUDESC_ITEM && menu_stack.size() >= 1) {
      parent_wxMenu = menus[menu_stack.back()];
      string cmd = e->cmd;
      if (cmd.empty())
        cmd = e->text;
      builtin_id = identify_builtin_command(cmd);
      const char *help_sz;
      if (builtin_id >= 0)
        help_sz = _(get_builtin_command_short_description(builtin_id));
      else
        help_sz = empty_sz;
      string s = "";
      s.append(_(e->text));
      if (!const_char_is_empty(help_sz) && const_char_is_empty(e->cmd)) {
        s.append("        ");
        s.append(help_sz);
      }
      parent_wxMenu->Append(ID_START_MENUS + i, string_to_wxString(s), const_char_to_wxString(help_sz));
    } else if (e_type == MENUDESC_SEPARATOR && menu_stack.size() >= 1) {
      parent_wxMenu = menus[menu_stack.back()];
      parent_wxMenu->AppendSeparator();
    }
  }
  delete []menus;
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_PAINT(MyFrame::OnPaint)
  EVT_SIZE(MyFrame::OnSize)
  EVT_MOUSEWHEEL(MyFrame::OnMouseWheel)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

MyApp::MyApp() : what_am_i_to_do(PROG_INIT_EXIT), frame(NULL) { }

MyFrame* MyApp::get_frame() const {
  return frame;
}

  // Calculate the width of the path area (to be able to end the string with "..." if needed)
void MyFrame::check_path_width() {
  int l = 0;
  wxStaticText *tmp;
  int wtmp, htmp;
  wxSize s = GetClientSize();
  int read_width = s.GetWidth();
  debug_write_i("read_width = %i", read_width);
  if (read_width != last_read_width) {
    do {
      l++;
      tmp = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), l), wxPoint(0, 0), wxSize(wxDefaultSize),
        SIZER_PATH_BORDERSTYLE);
      my_set_font(tmp, SIZER_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_PATH_FONTWEIGHT, 0,
          (gui == GUI_SIZER ? NULL : &(skin->f_path)));
//      tmp->SetFont(wxFont(SIZER_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_PATH_FONTWEIGHT, 0));
      tmp->GetSize(&wtmp, &htmp);
      tmp->Destroy();
    } while (wtmp < read_width);
    path_width = l - 1;
    last_read_width = read_width;
  }
}

static void skin_read_typein_info(int& x, int& y, int& w, int& h, const skin_t *s) {
  x = s->r_typein.x;
  y = s->r_typein.y;
  w = s->r_typein.w;
  h = s->r_typein.h;
  if (x == -1)
    x = s->r_stack.x;
  if (y == -1)
    y = s->r_stack.y + s->stack_height * s->stack_y_step;
  if (w == -1)
    w = s->r_stack.w;
  if (h == -1)
    h = s->r_stack.h;
  debug_write_v("skin_read_typein_info()");
  debug_write_v("  s->r_typein.x = %i", s->r_typein.x);
  debug_write_v("  s->r_typein.y = %i", s->r_typein.y);
  debug_write_v("  s->r_typein.w = %i", s->r_typein.w);
  debug_write_v("  s->r_typein.h = %i", s->r_typein.h);
  debug_write_v("  s->r_stack.x = %i", s->r_stack.x);
  debug_write_v("  s->r_stack.y + s->stack_height * s->stack_y_step = %i",
    s->r_stack.y + s->stack_height * s->stack_y_step);
  debug_write_v("  s->r_stack.w = %i", s->r_stack.w);
  debug_write_v("  s->r_stack.h = %i", s->r_stack.h);
}

static void shape(wxWindow *ctrl, const int& x, const int& y, const int& w, const int& h,
    const wxColour& bg_color, const wxColour& fg_color) {
  ctrl->SetSize(x, y, w, h);
  ctrl->SetBackgroundColour(bg_color);
  ctrl->SetForegroundColour(fg_color);
}

static void draw_button_text(wxBitmap *released, wxBitmap *pressed,
        const char *t, const int& y_text_released, const int& y_text_pressed, const font_t& f_text) {
  wxMemoryDC memdc;
  memdc.SelectObject(*released);
  int w = released->GetWidth();
  memdc.SetTextForeground(f_text.colour);
  memdc.SetFont(wxFont(f_text.pointSize, f_text.family, f_text.style, f_text.weight, f_text.underline));
  wxSize s = memdc.GetTextExtent(const_char_to_wxString(t));
  memdc.DrawText(const_char_to_wxString(t), (w - s.GetWidth()) / 2, y_text_released);

  memdc.SelectObject(*pressed);
  memdc.DrawText(const_char_to_wxString(t), (w - s.GetWidth()) / 2, y_text_pressed);

}

myBitmapButton *build_bitmap_button(MyFrame *parent, wxWindowID id, const wxBitmap& bm_released,
    const wxPoint& pos, const wxSize& size, const wxBitmap& bm_pressed, const char *t, const int& y_text_released, const int& y_text_pressed,
    const font_t& f_text, const char*main_cmd, const char *alt_cmd)
{

  wxBitmap copy_bm_released = bm_released;
  wxBitmap copy_bm_pressed = bm_pressed;

  draw_button_text(&copy_bm_released, &copy_bm_pressed, t, y_text_released, y_text_pressed, f_text);

  myBitmapButton *b = new myBitmapButton(parent, id, &copy_bm_released, &copy_bm_pressed, pos, size, main_cmd, alt_cmd);
  return b;
}

static int get_menu_button_number(const char *cmd) {
  string s(cmd);

  if (s.substr(0, 5) == "_MENU" && s.length() >= 6) {
    if (isdigit(s.at(5))) {
      int r = string_to_integer(s.substr(5));
      debug_write_v("Command \"%s\": assigned number %i", cmd, r);
      return r;
    }
  }
  debug_write_v("Command \"%s\": no number assigned", cmd);
  return -1;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,
  const MenuDescription*& md, int& nb_md, const BtnDescription*& bd, int& nb_bd, const int& u)
    : wxFrame(NULL, -1, title, pos, size,
        gui_type(u) == GUI_SIZER ?  wxDEFAULT_FRAME_STYLE :
                        (wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX)),
        menus_descriptions(md), nb_menus_descriptions(nb_md), btn_descriptions(bd), nb_btn_descriptions(nb_bd),
        gui(gui_type(u)), xy_set(false), my_x0(-1), my_w0(-1), my_h0(-1), my_y0(-1), my_y1(-1),
        my_char_width(-1), my_initial_stack_lines(-1), my_initial_stack_width(-1),
        my_initial_client_width(-1), my_initial_client_height(-1),
        my_min_client_width(-1), my_min_client_height(-1),
        dlg(NULL),
        path_width(-1), last_read_width(-1) {

  debug_write_v("MyFrame::MyFrame() begin");
  debug_write_v("gui = %s", (gui == GUI_SIZER ? "sizer" : "skin"));
  debug_write_v("ui_code = %i", u);

  SetIcon(wxIcon(prpn_xpm));

  bool has_menu_bar;

  int ui_code = -1;

  if (gui == GUI_SIZER) {
    skin = NULL;
    has_menu_bar = ui_has_menu_bar();
  } else {
    ui_code = u;
    if (static_cast<unsigned int>(ui_code) > sizeof(skins) / sizeof(*skins))
      ui_code = sizeof(skins) / sizeof(*skins);
    debug_write_v("u = %i, ui_code = %i", u, ui_code);
    skin = skins[ui_code - 1];
    debug_write_v("Loadging bitmap images...");
    skin->load_bitmaps();
    debug_write_v("Finished loadging bitmap images...");
    if (skin->menubar == MB_DEFAULT)
      has_menu_bar = ui_has_menu_bar();
    else
      has_menu_bar = (skin->menubar == MB_ENFORCE_MENU);
      ui_dsl.redefine_geometry(skin->stack_height, skin->stack_width, true);
  }

  if (gui == GUI_SIZER)
    SetBackgroundColour(SIZER_CALCULATOR_BACKGROUND_COLOR);

  if (gui == GUI_SKIN)
    SetSize(wxSize(skin->frame_w, skin->frame_h));

  if (has_menu_bar) {
    wxMenuBar *menuBar;
    build_menu_bar(menus_descriptions, nb_menus_descriptions, menuBar, ui_code);
    for (int i = 0; i < nb_menus_descriptions; i++) {
      Connect(ID_START_MENUS + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnMenu));
    }
    for (int i = 0; static_cast<unsigned int>(i) <= sizeof(skins) / sizeof(*skins); i++) {
      Connect(ID_START_INTERFACE_CHOICE_MENUS + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnMenu));
    }
    SetMenuBar(menuBar);
  }

  if (gui == GUI_SIZER)
    topSizer = new wxBoxSizer(wxVERTICAL);
  else
    topSizer = NULL;

    // Status line (red arrow to show shift pressed, ...)
  if (gui == GUI_SIZER) {
    stwin = new StatusWindow(this, wxID_ANY, wxPoint(wxDefaultPosition), wxSize(wxDefaultSize), SIZER_STATUS_BACKGROUND_COLOUR,
        exec_norun_xpm, exec_run_xpm,
        shiftsel_xpm, shiftuns_xpm,
        angle_deg_xpm, angle_rad_xpm,
        unit_bin_xpm, unit_oct_xpm, unit_dec_xpm, unit_hex_xpm);
  } else if (gui == GUI_SKIN) {
    stwin = new StatusWindow(this, wxID_ANY, wxPoint(wxDefaultPosition), wxSize(wxDefaultSize), skin->status_bg_colour,
        *(skin->status_exec_norun), *(skin->status_exec_run),
        *(skin->status_shiftsel), *(skin->status_shiftuns),
        *(skin->status_angle_deg), *(skin->status_angle_rad),
        *(skin->status_unit_bin), *(skin->status_unit_oct), *(skin->status_unit_dec), *(skin->status_unit_hex));
  }

  if (gui == GUI_SIZER)
    topSizer->Add(stwin, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, SIZER_STATUS_BORDER);
  else
    stwin->SetSize(skin->r_status.x, skin->r_status.y, skin->r_status.w, skin->r_status.h);

    // Path
  path = new wxStaticText(this, wxID_ANY, _T(""), wxPoint(wxDefaultPosition), wxSize(wxDefaultSize), SIZER_PATH_BORDERSTYLE);
//  path->SetFont(wxFont(SIZER_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_PATH_FONTWEIGHT, 0));
  my_set_font(path, SIZER_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_PATH_FONTWEIGHT, 0,
      (gui == GUI_SIZER ? NULL : &(skin->f_path)));
  if (gui == GUI_SIZER) {
    path->SetForegroundColour(SIZER_PATH_FOREGROUND_COLOUR);
    topSizer->Add(path, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, SIZER_PATH_BORDER);
  } else if (gui == GUI_SKIN)
    shape(path, skin->r_path.x, skin->r_path.y, skin->r_path.w, skin->r_path.h, skin->path_bg_colour, skin->f_path.colour);

    // Stack items
  build_dispStack();

    // Type-in area
  int w, h;
  dispStack[0].t->GetSize(&w, &h);
  textTypein = new wxTextCtrl(this, ID_TEXTTYPEIN, _T(""), wxPoint(wxDefaultPosition), wxSize(5, h),
            SIZER_TYPEIN_BORDERSTYLE | wxTE_PROCESS_ENTER | wxTE_MULTILINE);
  my_set_font(textTypein, SIZER_TYPEIN_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_TYPEIN_FONTWEIGHT, 0,
                gui == GUI_SIZER ? NULL : &(skin->f_typein));
  if (gui == GUI_SIZER) {
    textTypein->SetBackgroundColour(SIZER_TYPEIN_BACKGROUND_COLOUR);
    textTypein->SetForegroundColour(SIZER_TYPEIN_FOREGROUND_COLOUR);
    topSizer->Add(textTypein, 0, wxALL | wxEXPAND, SIZER_TYPEIN_BORDERSIZE);
  } else if (gui == GUI_SKIN) {
    int x, y, w, h;
    skin_read_typein_info(x, y, w, h, skin);
    shape(textTypein, x, y, w, h, skin->typein_bg_colour, skin->f_typein.colour);
  }


    // Buttons
  if (gui == GUI_SIZER) {

    sizer_menu_buttons.clear();

    wxButton *b;
    BtnSmallLabel bsl;
    wxBtnLine *cur_line = 0;
    int my_id;
    int prop;
    int b_font_size;
    int t_font_size;

    for (int i = 0; i < nb_btn_descriptions; i++) {

      prop = btn_descriptions[i].proportion;
      if (i == 0 || prop == -1) {
          // Start a new line of buttons
        wxAllButtons.lines.push_back(new wxBtnLine());
        cur_line = wxAllButtons.lines.back();
      }
      if (prop != -1) {
          // Continue actual line of buttons
        if (cur_line->btns_index < 0)
          cur_line->btns_index = i;
        my_id = ID_START_BUTTONS_SIZER + i;
        bsl.t = new wxStaticText(this, wxID_ANY, const_char_to_wxString(btn_descriptions[i].alt_display),
                wxPoint(wxDefaultPosition), wxSize(wxDefaultSize), wxALIGN_CENTRE);
        bsl.s = new wxBoxSizer(wxVERTICAL);

        const char *sz;
        int cc_r; int cc_g; int cc_b;
        ui_decode_color_code(btn_descriptions[i].main_display, sz, cc_r, cc_g, cc_b);
        b = new wxButton(this, my_id, const_char_to_wxString(sz),
            wxPoint(wxDefaultPosition), wxSize(wxDefaultSize));
        if (cc_r >= 0 && cc_r <= 255 && cc_g >= 0 && cc_g <= 255 && cc_b >= 0 && cc_b <= 255)
          b->SetBackgroundColour(wxColour(cc_r, cc_g, cc_b));

        int mbn = get_menu_button_number(btn_descriptions[i].main_cmd);
        if (mbn >= 1) {
          sizer_menu_buttons.push_back(b);
        }

        cur_line->vl.push_back(bsl);
        cur_line->vb.push_back(b);
        b_font_size = SIZER_BTN_FONTSIZE;
        if (btn_descriptions[i].main_font_proportion != 0)
          b_font_size = static_cast<int>(b_font_size * 1.0 * btn_descriptions[i].main_font_proportion / 100.0);
        t_font_size = static_cast<int>(SIZER_BTN_FONTSIZE * SIZER_ALTBTN_FONTRATIO);

        if (btn_descriptions[i].alt_font_proportion != 0)
          t_font_size = static_cast<int>(t_font_size * 1.0 * btn_descriptions[i].alt_font_proportion / 100.0);
        if (bsl.t != NULL) {
          bsl.t->SetFont(wxFont(t_font_size, SIZER_ALTBTN_FONTFAMILY, SIZER_ALTBTN_FONTSTYLE, SIZER_ALTBTN_FONTWEIGHT, 0));
          bsl.t->SetForegroundColour(wxColour(*wxRED));
        }
        b->SetFont(wxFont(b_font_size, SIZER_BTN_FONTFAMILY, SIZER_BTN_FONTSTYLE, SIZER_BTN_FONTWEIGHT, 0));
        Connect(my_id, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyFrame::OnButton));
      }
    }
  } else if (gui == GUI_SKIN) {

    skin_menu_buttons.clear();

    skin_menu_button_1 = -1;

    int my_id;
    const skin_btn_t *bs;
    myBitmapButton *b;

    for (int i = 0; i < skin->nb_btns; i++) {
      my_id = ID_START_BUTTONS_SKIN + i;
      bs = &(skin->btns[i]);
      b = build_bitmap_button(this, static_cast<wxWindowID>(my_id), *(bs->released),
            wxPoint(bs->rec.x + skin->btn_shift_x, bs->rec.y + skin->btn_shift_y), wxSize(bs->rec.w, bs->rec.h),
            *(bs->pressed), bs->text, skin->y_released + bs->y_shift, skin->y_pressed + bs->y_shift,
            skin->btns_fonts[bs->font_index], bs->main_cmd, bs->alt_cmd);
      int mbn = get_menu_button_number(bs->main_cmd);
      if (mbn >= 1) {
        skin_menu_buttons.push_back(b);
        if (skin_menu_button_1 < 0)
          skin_menu_button_1 = i;
      }
    }

  }

  textTypein->Connect(ID_TEXTTYPEIN, wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnChar));
  textTypein->Connect(ID_TEXTTYPEIN, wxEVT_MOUSEWHEEL, wxMouseEventHandler(MyFrame::OnMouseWheel));

    // To get a button area of the correct size (that just fits the upper area displaying the stack),
    // we first (STEP 1) add the wxBoxSizers to the topSizer, with only one button inside. Then,
    // SetSizeHints is called (STEP 2), so that the sizers containing buttons don't further enlarge the window.
    // At last we add all other buttons (STEP 3.)
    // This method works so long as a single button isn't larger than the upper sizer.

    // STEP 1
  if (gui == GUI_SIZER) {
    SetSizer(topSizer);
    topSizer->Add(1, SIZER_AROUND_BUTTONS_AREA_BORDER, 0);
    for (vector<wxBtnLine*>::iterator it = wxAllButtons.lines.begin(); it != wxAllButtons.lines.end(); it++) {
      for (int i = 0; i < 2; i++)
        (*it)->bSizer[i] = new wxBoxSizer(wxHORIZONTAL);
      topSizer->Add((*it)->bSizer[0], SIZER_PROP_BTN_SMALL_LABELS, wxEXPAND | wxLEFT | wxRIGHT, SIZER_AROUND_BUTTONS_AREA_BORDER);
      topSizer->Add((*it)->bSizer[1], SIZER_PROP_BTN_BUTTONS, wxEXPAND | wxLEFT | wxRIGHT, SIZER_AROUND_BUTTONS_AREA_BORDER);
      (*it)->bSizer[0]->Add((*it)->vl[0].s, btn_descriptions[(*it)->btns_index].proportion,
        wxALIGN_BOTTOM, 0);
      (*it)->vl[0].s->Add((*it)->vl[0].t, 1, wxALIGN_CENTER_HORIZONTAL, 0);
      (*it)->bSizer[1]->Add((*it)->vb[0], btn_descriptions[(*it)->btns_index].proportion, wxEXPAND, 0);
    }
    topSizer->Add(1, SIZER_AROUND_BUTTONS_AREA_BORDER, 0);
      // STEP 2
    topSizer->SetSizeHints(this);
      // STEP 3
    for (vector<wxBtnLine*>::iterator it = wxAllButtons.lines.begin(); it != wxAllButtons.lines.end(); it++) {
      for (int i = 1; i < static_cast<int>((*it)->vb.size()); i++) {
        (*it)->bSizer[0]->Add((*it)->vl[i].s, btn_descriptions[(*it)->btns_index + i].proportion,
          wxLEFT | wxALIGN_BOTTOM, SIZER_BTNCOLUMN_BORDER);
        (*it)->vl[i].s->Add((*it)->vl[i].t, 1, wxALIGN_CENTER_HORIZONTAL, 0);
        (*it)->bSizer[1]->Add((*it)->vb[i], btn_descriptions[(*it)->btns_index + i].proportion, wxEXPAND | wxLEFT, SIZER_BTNCOLUMN_BORDER);
      }
    }
  }

  check_path_width();

  if (gui == GUI_SIZER)
    SetMinSize(wxSize(10, 10));

  textTypein->SetFocus();

  debug_write_v("MyFrame::MyFrame() end");
}

int MyFrame::get_nb_menu_buttons() {
  if (gui == GUI_SIZER) {
    debug_write_v("MyFrame::get_nb_menu_buttons(): return %lu", sizer_menu_buttons.size());
    return sizer_menu_buttons.size();
  } else if (gui == GUI_SKIN) {
    debug_write_v("MyFrame::get_nb_menu_buttons(): return %lu", skin_menu_buttons.size());
    return skin_menu_buttons.size();
	}
	// Never executed...
	return 0;
}

void MyFrame::notify_ui_change() {
  debug_write_v("MyFrame::notify_ui_change()");

  restart_gui = true;
  frame_pos = GetPosition();
  Close(TRUE);
}

void MyFrame::textTypein_recalc_and_setsize() {
  debug_write_v("textTypein_recalc_and_setsize() begin");

  int x, y, w, h;
  int y0 = my_y0 + my_get_max_stack() * (my_y1 - my_y0);

  debug_write_v("textTypein: x = %i, y = %i, w = %i, h = %i", x, y, w, h);

  textTypein->GetPosition(&x, &y);
  textTypein->GetSize(&w, &h);
  int bottom = y + h - 1;
  int new_h = bottom - y0 + 1;

  debug_write_v("textTypein: x = %i, y = %i, w = %i, h = %i", my_x0, y0, my_w0, new_h);

  textTypein->SetSize(my_x0, y0, my_w0, new_h);

  debug_write_v("textTypein_recalc_and_setsize() end");
}

void MyFrame::textTypein_SetFocus() {
  textTypein->SetFocus();
}

void MyFrame::stack_line_set_font(wxStaticText *t) {
  my_set_font(t, SIZER_STACK_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_STACK_FONTWEIGHT, 0,
                gui == GUI_SIZER ? NULL : &(skin->f_stack));
}

#define STACK_INDEX_0 2
void MyFrame::build_dispStack() {

  debug_write_v("build_dispStack() begin");

  Freeze();

  int n = my_get_max_stack();
  debug_write_v("  n = %i", n);

  stack_1line l1;

  for (int i = static_cast<int>(dispStack.size()); i >= 1; i--) {

    if (gui == GUI_SIZER)
      topSizer->Remove(STACK_INDEX_0 + i - 1);

    dispStack[i - 1].t->Destroy();
    dispStack[i - 1].w->Destroy();
  }
  dispStack.erase(dispStack.begin(), dispStack.end());

  for (int i = 0; i < n; i++) {
    wxPoint wxp;
    wxSize wxs;
    if (xy_set) {
      int y0 = my_y0 + i * (my_y1 - my_y0);
      wxp = wxPoint(my_x0, y0);
      wxs = wxSize(my_w0, my_h0);

      debug_write_v("  xy_set is true, i = %i, wxp.x = %i, wxp.y = %i, wxs.w = %i, wxs.h = %i",
        i, wxp.x, wxp.y, wxs.GetWidth(), wxs.GetHeight());

    } else {
      wxp = wxPoint(wxDefaultPosition);
      wxs = wxSize(wxDefaultSize);

      debug_write_v("  xy_set is false, i = %i, wxp.x = %i, wxp.y = %i, wxs.w = %i, wxs.h = %i",
        i, wxp.x, wxp.y, wxs.GetWidth(), wxs.GetHeight());

    }
    l1.w = new wxWindow(this, wxID_ANY, wxp, wxs, SIZER_STACK_BORDERSTYLE);
    l1.t = new wxStaticText(l1.w, wxID_ANY, wxString(wxChar(' '), ui_dsl.get_width()),
        wxPoint(0, 0), wxs, SIZER_STACK_BORDERSTYLE);
    stack_line_set_font(l1.t);
    if (gui == GUI_SIZER) {
      l1.w->SetBackgroundColour(sizer_slcc_to_bg_wxColor(SLCC_NORMAL));
      l1.t->SetForegroundColour(sizer_slcc_to_fg_wxColor(SLCC_NORMAL));
      topSizer->Insert(STACK_INDEX_0 + i, l1.w, 0, wxALL, SIZER_STACK_BORDERSIZE);
    } else if (gui == GUI_SKIN) {
      shape(l1.w, skin->r_stack.x, skin->r_stack.y + i * skin->stack_y_step, skin->r_stack.w, skin->r_stack.h,
          skin->stack_bg_colour, skin->stack_fg_colour);
    }
    dispStack.push_back(l1);
  }

  Thaw();

  debug_write_v("build_dispStack() end");
}

void MyFrame::display_help(const int& dh) {
  wxSize mysize = wxSize(85 * SIZER_HELP_FONTSIZE, 46 * SIZER_HELP_FONTSIZE);
  if (dlg != NULL)
    dlg->Destroy();
  string h = _("Help");
  dlg = new wxDialog(this, wxID_ANY, string_to_wxString(h), wxPoint(wxDefaultPosition), wxSize(wxDefaultSize),
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
  wxHtmlWindow *html = NULL;
  bool display_html = html_help_found;
  if (dh == DH_MAIN && display_html) {
    html = new wxHtmlWindow(dlg, wxID_ANY, wxDefaultPosition, mysize);
    html->SetBorders(0);
    display_html = html->LoadPage(string_to_wxString(html_help_file));
    if (!display_html)
      html->Destroy();
  }

  if (dh == DH_MAIN && display_html) {
    wxBoxSizer *topsizer;
    topsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(html, 1, wxALL, 10);

#if wxUSE_STATLINE
    topsizer -> Add(new wxStaticLine(dlg, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
#endif // wxUSE_STATLINE

    string ok = _("OK");
    wxButton *btn = new wxButton(dlg, wxID_OK, string_to_wxString(ok));
    btn->SetDefault();
    topsizer->Add(btn, 0, wxALL | SIZER_HELP_OKBUTTON_ALIGN, SIZER_HELP_OKBUTTON_MARGIN);
    dlg->SetSizer(topsizer);
    topsizer->Fit(dlg);
  } else {
    wxBoxSizer *topsizer;
    topsizer = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl *t = new wxTextCtrl(dlg, wxID_ANY, string_to_wxString(stack_get_help(dh)),
      wxPoint(wxDefaultPosition), mysize, wxTE_READONLY | wxTE_MULTILINE);
    t->SetFont(wxFont(SIZER_HELP_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, SIZER_HELP_FONTWEIGHT, 0));
    topsizer->Add(t, 1, wxALL, 10);
    string ok = _("OK");
    wxButton *btn = new wxButton(dlg, wxID_OK, string_to_wxString(ok));
    btn->SetDefault();
    topsizer->Add(btn, 0, wxALL | SIZER_HELP_OKBUTTON_ALIGN, SIZER_HELP_OKBUTTON_MARGIN);
    dlg->SetSizer(topsizer);
    t->ShowPosition(0);
    topsizer -> Fit(dlg);
  }
    dlg->Show();
}

void MyFrame::OnAbout(wxCommandEvent&) {
  string w = PACKAGE_NAME;
  wxMessageBox(string_to_wxString(w), _T("Information"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnButton(wxCommandEvent& ev) {
  const char *sz;
  if (ev.GetId() < ID_START_BUTTONS_SKIN) {
    int btn_index = ev.GetId() - ID_START_BUTTONS_SIZER;
    sz = btn_descriptions[btn_index].main_cmd;
    if (ui_dsl.get_status_shift() && !const_char_is_empty(btn_descriptions[btn_index].alt_cmd))
      sz = btn_descriptions[btn_index].alt_cmd;
  } else {
    int btn_index = ev.GetId() - ID_START_BUTTONS_SKIN;
    sz = skin->btns[btn_index].main_cmd;
    if (ui_dsl.get_status_shift() && !const_char_is_empty(skin->btns[btn_index].alt_cmd))
      sz = skin->btns[btn_index].alt_cmd;
  }
  ui_notify_button_pressed(sz);
  textTypein->SetFocus();
}

void MyFrame::OnMenu(wxCommandEvent& ev) {
  if (ev.GetId() >= ID_START_INTERFACE_CHOICE_MENUS) {
    int uc = ev.GetId() - ID_START_INTERFACE_CHOICE_MENUS;
    if (uc != ui_get_ui_code()) {
      ui_notify_key_pressed(UIK_ESCAPE);
      ui_uiset(uc);
    }
    return;
  }
  int menu_index = ev.GetId() - ID_START_MENUS;
//    debug_write_i("Menu cliqué : %i", menu_index);
  const char *sz = menus_descriptions[menu_index].cmd;

  if (const_char_is_empty(sz))
    sz = menus_descriptions[menu_index].text;
  ui_notify_button_pressed(sz);
  textTypein->SetFocus();
}

void MyFrame::OnChar(wxKeyEvent& event) {
  int wxk = event.GetKeyCode();

  debug_write_v("X1: key pressed = %i", wxk);

  int gm = event.GetModifiers();
  int uik;
  switch (wxk) {
    case WXK_UP:
      uik = UIK_UP;
      break;
    case WXK_DOWN:
      uik = UIK_DOWN;
      break;
    case WXK_PAGEUP:
      uik = UIK_PAGEUP;
      break;
    case WXK_PAGEDOWN:
      uik = UIK_PAGEDOWN;
      break;
    case WXK_HOME:
      uik = UIK_HOME;
      break;
    case WXK_END:
      uik = UIK_END;
      break;
    case WXK_RETURN:
    case '\n':
      if ((gm & (wxMOD_ALT | wxMOD_CONTROL)) != 0 || wxk == '\n') {
        uik = UIK_NEWLINE;
      } else {
        uik = UIK_RETURN;
      }
      break;
    case WXK_ESCAPE:
      uik = UIK_ESCAPE;
      break;
    default:
      uik = wxk;
  }
  bool event_stop = ui_notify_key_pressed(uik);
  if (!event_stop && uik != UIK_RETURN)
    event.Skip();

  /*MyFrame *f = dynamic_cast<MyFrame*>(this->GetParent());
  debug_disp_and_size("dispStack[0]", f->dispStack[0]);
  debug_disp_and_size("dispStack[1]", f->dispStack[1]);*/

}

void MyFrame::OnPaint(wxPaintEvent& ev) {

  debug_write_v("MyFrame::OnPaint() begin");

  if (!xy_set) {
    my_frame_initial_size = GetClientSize();
    int x, y, w, h, x2, y2;
    dispStack[0].w->GetPosition(&x, &y);
    dispStack[0].w->GetSize(&w, &h);
    dispStack[1].w->GetPosition(&x2, &y2);
    int tx, ty;
    textTypein->GetPosition(&tx, &ty);
    textTypein->GetSize(&my_w0, &my_h0);
    my_x0 = x;
    my_y0 = y;
    my_y1 = y2;
    wxStaticText *t1 = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), 1),
      wxPoint(0, 0), wxSize(wxDefaultSize), SIZER_STACK_BORDERSTYLE);
    stack_line_set_font(t1);
    wxStaticText *t2 = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), 2),
      wxPoint(0, 0), wxSize(wxDefaultSize), SIZER_STACK_BORDERSTYLE);
    stack_line_set_font(t2);
    wxSize s1 = t1->GetSize();
    wxSize s2 = t2->GetSize();
    t1->Destroy();
    t2->Destroy();
    my_char_width = s2.GetWidth() - s1.GetWidth();
    debug_write_i("PAINT::char width = %i", my_char_width);
    my_initial_stack_lines = my_get_max_stack();
    my_initial_stack_width = ui_dsl.get_width();
    wxSize s = GetClientSize();
    my_initial_client_width = s.GetWidth();
    my_initial_client_height = s.GetHeight();
    debug_write_i("PAINT::initial client width = %i", my_initial_client_width);
    debug_write_i("PAINT::initial client height = %i", my_initial_client_height);

    wxSize frame_size = GetSize();
    int fw = frame_size.GetWidth();
    int fh = frame_size.GetHeight();
    my_min_client_width = fw + (10 - my_initial_stack_width) * my_char_width;
    my_min_client_height = fh + (HARD_GUI_MIN_HEIGHT - 1 - my_initial_stack_lines) * (my_y1 - my_y0);

    int yy = my_y0 + (my_get_max_stack() + 1) * (my_y1 - my_y0);
    int hh = fh - yy;

    debug_write_i("PAINT::minimal client width = %i", my_min_client_width);
    debug_write_i("PAINT::minimal client height = %i", my_min_client_height);
    SetMinSize(wxSize(my_min_client_width, my_min_client_height - hh / 2));

    debug_write_v("  my_x0 = %i, my_w0 = %i, my_h0 = %i, my_y0 = %i, my_y1 = %i, "
      "my_char_width = %i", my_x0, my_w0, my_h0, my_y0, my_y1, my_char_width);

    xy_set = true;
  }

  if (gui == GUI_SKIN) {
    wxPaintDC dc(this);
    PrepareDC(dc);

    dc.DrawBitmap(*(skin->frame_bg_image), 0, 0);
    for (int i = 0; i < skin->nb_btns; i++) {
      dc.DrawBitmap(*(skin->btns[i].released), skin->btns[i].rec.x + skin->btn_shift_x, skin->btns[i].rec.y + skin->btn_shift_y);
    }
  }

  ev.Skip();

  debug_write_v("MyFrame::OnPaint() end");

}

void MyFrame::OnSize(wxSizeEvent& ev) {
  debug_write_v("MyFrame::OnSize() begin");

  if (gui == GUI_SKIN)
    return;

  wxSize s = GetClientSize();
  int new_w = s.GetWidth();
  int new_h = s.GetHeight();
  debug_write_i("SIZE::new client width  = %i", new_w);
  debug_write_i("SIZE::new client height = %i", new_h);
  debug_write_i("SIZE::my_w0 = %i", my_w0);
  if (xy_set) {
    int delta_w = new_w - my_initial_client_width;
    int delta_chars = delta_w / my_char_width;
    if (delta_chars < 0 && delta_w % my_char_width != 0)
      delta_chars -= 1;
    int target_stack_width = my_initial_stack_width + delta_chars;
    my_w0 = target_stack_width * my_char_width;
    debug_write_i("SIZE:: updated my_w0, now = %i", my_w0);
    debug_write_i("SIZE::New width in chars = %i", target_stack_width);
    int delta_h = new_h - my_initial_client_height;
    int delta_lines = delta_h / (my_y1 - my_y0);
    int target_stack_lines = my_initial_stack_lines + delta_lines;
    debug_write_i("SIZE::New lines = %i", target_stack_lines);
    debug_write_v("ui_dsl.redefine_geometry(): target_stack_lines = %i, target_stack_width = %i", target_stack_lines, target_stack_width);
    ui_dsl.redefine_geometry(target_stack_lines, target_stack_width, false);
  }
  ev.Skip();

  debug_write_v("MyFrame::OnSize() end");
}

void MyFrame::OnMouseWheel(wxMouseEvent& ev) {
  int taille_pas = ev.m_wheelDelta;
    // Avoid division by zero... :-)
  if (taille_pas == 0)
    taille_pas = DEFAULT_WHEELDELTA_UNIT;
  int mesure_pas = ev.GetWheelRotation();
  int nb_pas = mesure_pas / taille_pas;
  if (nb_pas < -MAX_WHEEL_STEPS_IN_1_EVENT)
    nb_pas = -MAX_WHEEL_STEPS_IN_1_EVENT;
  if (nb_pas > MAX_WHEEL_STEPS_IN_1_EVENT)
    nb_pas = MAX_WHEEL_STEPS_IN_1_EVENT;
  debug_write_v("MyFrame::OnMouseWheel(): function called, ::m_wheelDelta = %i"
      ", GetWheelDelta() = %i, GetWheelRotation() = %i, nb_pas = %i",
      ev.m_wheelDelta, ev.GetWheelDelta(), ev.GetWheelRotation(), nb_pas);
  ui_notify_key_pressed(UIK_WHEEL, nb_pas);
  ev.Skip();
}


//
// UIIMPLWX
//
// This part derivates the massive abstract UiImpl class
// and defines all its abstract classes
//

class UiImplWx : public UiImpl {
  MyFrame *f;
  UiImplWx(const UiImplWx&);
  bool first_refresh_done;
public:
  UiImplWx();
  virtual ~UiImplWx();

  void set_frame(MyFrame*);
  virtual void notify_ui_change();
  virtual void refresh_statuswin();
  virtual void set_line(const int&, const slcc_t&, const string&);
  virtual void enforce_refresh();
  virtual void refresh_display_path(const string&, const bool&, const int&);
  virtual void set_syntax_error(const int&, const int&, const int&, const int&);
  virtual const std::string get_string();
  virtual void erase_input();
  virtual const string get_first_char();
  virtual bool space_at_the_left_of_the_cursor();
  virtual int get_nb_newlines(const int&);
  virtual int get_nb_menu_buttons();
  virtual void set_menu_button(const int&, const menu_button_t&);
  virtual void insert_text(const char *);
  virtual void append_line(const char *);
  virtual void set_cursor_at_the_beginning();
  virtual int get_elevator_width();
  virtual void neg();
  virtual void refresh_stack_height();
  virtual void quit();
  virtual bool want_to_refresh_display();
  virtual void display_help(const int&);
  virtual const char *get_next_instruction_prefix();
  virtual void copy_text(const char *);
  virtual const char *paste_text();
};


//
// UIIMPLWX
//

UiImplWx::UiImplWx() : UiImpl(), first_refresh_done(false) { }

void UiImplWx::set_frame(MyFrame* ff) {
  first_refresh_done = false;
  f = ff;
}

UiImplWx::~UiImplWx() { }

void UiImplWx::notify_ui_change() {
  f->notify_ui_change();
}

void UiImplWx::refresh_statuswin() { f->stwin->Refresh(); }

static bool is_inverted(const slcc_t& color_code) {
  return (color_code == SLCC_EDITED_ITEM || color_code == SLCC_NEXT_INSTRUCTION);
}

void UiImplWx::set_line(const int& line_number, const slcc_t& color_code, const string& s) {
  if (f->gui == GUI_SIZER) {
    f->dispStack[line_number - 1].t->SetForegroundColour(sizer_slcc_to_fg_wxColor(color_code));
    f->dispStack[line_number - 1].w->SetBackgroundColour(sizer_slcc_to_bg_wxColor(color_code));
  } else if (f->gui == GUI_SKIN) {
    f->dispStack[line_number - 1].w->SetBackgroundColour(is_inverted(color_code) ?
        f->skin->stack_inv_bg_colour : f->skin->stack_bg_colour);
    f->dispStack[line_number - 1].t->SetForegroundColour(is_inverted(color_code) ?
        f->skin->stack_inv_fg_colour : f->skin->stack_fg_colour);
  }
  f->dispStack[line_number - 1].t->SetLabel(string_to_wxString(s));
  debug_write_i("Line number #%i set to:", line_number);
  debug_write(s.c_str());
}

void UiImplWx::enforce_refresh() { f->Update(); }

void UiImplWx::refresh_display_path(const string& s, const bool& modified, const int& when) {
  if (when == REFRESH_PATH_POST) {
    debug_write_i("PATH REFRESH = %i", static_cast<int>(modified));
    debug_write(s.c_str());
    if (modified) {
      string s_mod = s;
      ui_string_trim(s_mod, f->path_width, &ui_dsl, true);
      f->path->SetLabel(string_to_wxString(s_mod));
      debug_write("Path:");
      debug_write(s.c_str());
    }
  }
}

void UiImplWx::set_syntax_error(const int& col_start, const int& col_end, const int&, const int& c_index) {
  f->textTypein->SetSelection(c_index, c_index + col_end - col_start + 1);
  f->textTypein->ShowPosition(c_index);
}

void UiImplWx::erase_input() {
  f->textTypein->Clear();
}

const string UiImplWx::get_first_char() {
    // FIXME: copy of the complete type-in string
  string s = wxTextCtrl_to_string(f->textTypein);
  if (!s.empty())
    s.erase(1);
  return s;
}

bool UiImplWx::space_at_the_left_of_the_cursor() {
  int c;
  long pos = f->textTypein->GetInsertionPoint() - 1;
  if (pos < 0 || pos >= f->textTypein->GetLastPosition())
    c = '!';
  else
      // FIXME: copy of the complete type-in string
    c = wxTextCtrl_to_string(f->textTypein).at(pos);
  return (c == ' ' || c == '\n' || c == '\t');
}

int UiImplWx::get_nb_newlines(const int& max_lines) {
    // FIXME: copy of the complete type-in string
  return count_newlines(wxTextCtrl_to_string(f->textTypein), max_lines);
}

int UiImplWx::get_nb_menu_buttons() {
  return f->get_nb_menu_buttons();
}

void UiImplWx::set_menu_button(const int& n, const menu_button_t& mb) {
  if (f->gui == GUI_SIZER) {
    string s = mb.label.substr(0, SIZER_MENU_BUTTONS_MAX_NB_CHARS);
    f->sizer_menu_buttons[n]->SetLabel(string_to_wxString(s));
  } else {
    string s = mb.label.substr(0, f->skin->menu_buttons_max_nb_chars);
    wxBitmap copy_bm_released = *(f->skin->btns[f->skin_menu_button_1].released);
    wxBitmap copy_bm_pressed = *(f->skin->btns[f->skin_menu_button_1].pressed);

    draw_button_text(&copy_bm_released, &copy_bm_pressed, s.c_str(), f->skin->y_released, f->skin->y_pressed,
        f->skin->btns_fonts[f->skin->btns[f->skin_menu_button_1].font_index]);
    f->skin_menu_buttons[n]->mySetBitmapLabel(&copy_bm_released);
    f->skin_menu_buttons[n]->mySetBitmapSelected(&copy_bm_pressed);
    dynamic_cast<wxWindow*>(f->skin_menu_buttons[n])->Refresh();
  }
}

void UiImplWx::insert_text(const char *cz) { f->textTypein->WriteText(const_char_to_wxString(cz)); }
void UiImplWx::append_line(const char *cz) { f->textTypein->AppendText(const_char_to_wxString(cz)); }
void UiImplWx::set_cursor_at_the_beginning() { f->textTypein->SetInsertionPoint(0); }
int UiImplWx::get_elevator_width() { return 2; }

const string UiImplWx::get_string() {
    // FIXME: copy of the complete type-in string
  return wxTextCtrl_to_string(f->textTypein);
}

void UiImplWx::neg() {
  const string s = wxTextCtrl_to_string(f->textTypein);
  long back_idx = f->textTypein->GetInsertionPoint();
  long idx = back_idx;
  bool first_loop = true;
  long shift = 1;
  if (idx > static_cast<int>(s.length()) - 1) {
    idx = static_cast<int>(s.length()) - 1;
    first_loop = false;
    shift = 0;
  }

  char c = ' ';
  char char_decimal_sep = F->get_decimal_separator(true);
  bool number_seen = false;
  while (idx >= 0) {
    c = s.at(idx);
    if (isdigit(c) || c == char_decimal_sep) {
      number_seen = true;
      idx--;
    } else if (isspace(c)) {
      if (first_loop)
        idx--;
      else
        break;
    } else if (first_loop && c != '-' && c != '+') {
      idx--;
    } else {
      break;
    }
    first_loop = false;
  }

  debug_write_v("char = %i", static_cast<int>(c));

  bool replace = false;
  string insert_string;
  if (c == '-' || c == '+')
    number_seen = true;
  if (number_seen) {
    if (c == '-' || c == '+') {
      insert_string = (c == '-' ? "+" : "-");
      replace = true;
    } else {
      insert_string = "-";
      idx++;
    }
  } else {
    long j = idx - shift;
    if (j < 0)
      j = 0;
    if (isspace(s.at(j)))
      insert_string = "NEG";
    else
      insert_string = " NEG";
    idx = back_idx;
  }
  if (idx < 0)
    idx = 0;

  debug_write_v("idx = %i", static_cast<int>(idx));
  debug_write_v("Character at the position of the cursor toward left = %i", static_cast<int>(c));

  if (replace)
    f->textTypein->Remove(idx, idx + 1);
  f->textTypein->SetInsertionPoint(idx);
  f->textTypein->WriteText(string_to_wxString(insert_string));
  f->textTypein->SetInsertionPoint(back_idx + (replace ? 0 : 1) + insert_string.length() - 1);
}

void UiImplWx::refresh_stack_height() {
  debug_write_v("refresh_stack_height() begin");

  f->Freeze();

  f->build_dispStack();
  f->check_path_width();

  if (f->gui == GUI_SIZER) {
    f->textTypein_recalc_and_setsize();
  } else if (f->gui == GUI_SKIN) {
    int x, y, w, h;
    skin_read_typein_info(x, y, w, h, f->skin);
    int h2 = h * (f->skin->stack_height - my_get_max_stack() + 1);
    int y2 = y - h * (f->skin->stack_height - my_get_max_stack());
    f->textTypein->SetSize(x, y2, w, h2);
  }

  f->Thaw();

  debug_write_v("refresh_stack_height() end");
}

void UiImplWx::quit() {
  f->Close(TRUE);
}

bool UiImplWx::want_to_refresh_display() {
  if (!first_refresh_done) {
    first_refresh_done = true;
    return true;
  }
  return false;
}

void UiImplWx::display_help(const int& dh) { f->display_help(dh); }

const char *UiImplWx::get_next_instruction_prefix() { return next_instruction_prefix; }

void UiImplWx::copy_text(const char *sz) {
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(const_char_to_wxString(sz)));
    wxTheClipboard->Close();
  }
}

string paste_repository = "";
const char *empty_string = "";

const char *UiImplWx::paste_text() {
  const char *sz = NULL;
  if (wxTheClipboard->Open()) {
    if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
      wxTextDataObject data;
      wxTheClipboard->GetData(data);
      paste_repository = wxString_to_string(data.GetText());
      sz = paste_repository.c_str();
    }  
    wxTheClipboard->Close();
  }
  if (sz == NULL)
    sz = empty_string;
  return sz;
}


//
// MYAPP
//

int MyApp::OnRun() {
  if (what_am_i_to_do == PROG_INIT_START_CONSOLE) {
    console_loop();
    return prog_terminate();
  } else if (what_am_i_to_do == PROG_INIT_START_GUI) {
    ui_post_init_before_ui_build();
    ui_impl = new UiImplWx();
    int r;
    do {
      restart_gui = false;
      build_top_frame();
      dynamic_cast<UiImplWx*>(ui_impl)->set_frame(wxGetApp().get_frame());
      ui_post_init_after_ui_build();
      r = wxApp::OnRun();
    } while (restart_gui);
    int c = prog_terminate();
    return c != 0 ? c : r;
  }
  return 1;
}

void MyApp::build_top_frame() {

  debug_write_v("MyApp::build_top_frame() begin");

  const MenuDescription *menus_descriptions;
  int nb_menus_descriptions;
  ui_get_menus_descriptions(menus_descriptions, nb_menus_descriptions);
  const BtnDescription *btn_descriptions;
  int nb_btn_descriptions;
  ui_get_buttons_layout_description(btn_descriptions, nb_btn_descriptions);

  frame = new MyFrame(_T(PACKAGE_NAME), frame_pos, wxSize(wxDefaultSize),
    menus_descriptions, nb_menus_descriptions, btn_descriptions, nb_btn_descriptions, ui_get_ui_code());

  frame->Show(true);
  SetTopWindow(frame);

  debug_write_v("MyApp::build_top_frame() end");

}

bool MyApp::OnInit() {


    // FIXME - not all handlers are needed
  wxInitAllImageHandlers();


  char **copy_argv = 0;
  string t;
  int n;
  if (argc >= 1) {
    copy_argv = new char*[argc];
    for (int i = 0; i < argc; i++) {
      t = wxString(argv[i]).ToUTF8();
      n = t.size() + 1;
      copy_argv[i] = new char[n];
      strncpy(copy_argv[i], t.c_str(), n);
    }
  }

  what_am_i_to_do = prog_init(argc, copy_argv);
  if (argc >= 1) {
    for (int i = 0; i < argc; i++)
      delete []copy_argv[i];
    delete copy_argv;
  }

  if (what_am_i_to_do != PROG_INIT_START_GUI)
    return true;

  return true;
}

