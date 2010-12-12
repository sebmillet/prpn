// UiImplWx.cpp
// wxWidgets UI implementation

// RPN calculator

// Sébastien Millet
// August 2009 - March 2010

#include "Ui.h"
#include <wx/wx.h>

#include "platform/os_generic.h"

  // To display HTML pages
#include "wx/wxhtml.h"
#include "wx/statline.h"
#include "wx/clipbrd.h"

#include "MyIntl.h"

#include <vector>

#ifdef DEBUG
#include <iostream>
#include <sstream>
#endif

#include "shiftuns.xpm"
#include "shiftsel.xpm"
#include "unit_bin.xpm"
#include "unit_oct.xpm"
#include "unit_dec.xpm"
#include "unit_hex.xpm"
#include "exec_norun.xpm"
#include "exec_run.xpm"
#include "angle_deg.xpm"
#include "angle_rad.xpm"

#include "prpn.xpm"

static const char *next_instruction_prefix = "";

using namespace std;

struct color_codes_t {
	wxColor fg;
	wxColor bg;
	color_codes_t(const wxColour& f, const wxColour& g) : fg(f), bg(g) { }
};

static const wxString const_char_to_wxString(const char *sz) {
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
bool const_char_is_empty(const char *sz) {
	return sz[0] == '\0';
}


//
// UI SETTINGS CONSTANTS
//

  // Common
#define MY_CALCULATOR_BACKGROUND_COLOR	(wxColour(0xFA, 0xFA, 0xFA))
  // Status area
#define MY_STATUS_BORDER				0
#define MY_STATUS_BORDER_STYLE			wxBORDER_NONE
#define MY_STATUS_BACKGROUND_COLOUR		(wxColour(0xFA, 0xFA, 0xFA))
#define MY_STATUS_WINDOW_ITEM_H_MARGIN	4
#define MY_STATUS_WINDOW_ITEM_V_MARGIN	2
  // Path
#define MY_PATH_BORDER					0
#define MY_PATH_BORDERSTYLE 			wxBORDER_NONE
#define MY_PATH_FONTSIZE				10
#define MY_PATH_FONTWEIGHT				wxFONTWEIGHT_NORMAL
#define MY_PATH_BACKGROUND_COLOUR		MY_STATUS_BACKGROUND_COLOUR
#define MY_PATH_FOREGROUND_COLOUR		(*wxBLACK)
  // Stack area
#define MY_STACK_BORDERSTYLE 			wxBORDER_NONE
#define MY_STACK_BORDERSIZE  			0
#define MY_STACK_FONTSIZE				14
#define MY_STACK_FONTWEIGHT				wxFONTWEIGHT_NORMAL
#define STACK_MIN_LINES					1
  // wxTextCtrl to type in values
#define MY_TYPEIN_BACKGROUND_COLOUR		(*wxWHITE)
#define MY_TYPEIN_FOREGROUND_COLOUR		(*wxBLACK)
#define MY_TYPEIN_BORDERSTYLE 			wxBORDER_NONE
#define MY_TYPEIN_BORDERSIZE  			0
#define MY_TYPEIN_FONTSIZE				14
#define MY_TYPEIN_FONTWEIGHT			wxFONTWEIGHT_NORMAL
  // Buttons layout
#define MY_BTN_FONTSIZE					11
#define MY_BTNCOLUMN_BORDER				4
#define MY_AROUND_BUTTONS_AREA_BORDER	4
#define MY_PROP_BTN_SMALL_LABELS		10
#define MY_PROP_BTN_BUTTONS				20
#define MY_BTN_FONTFAMILY				wxFONTFAMILY_MODERN
#define MY_BTN_FONTSTYLE				wxFONTSTYLE_NORMAL
#define MY_BTN_FONTWEIGHT				wxFONTWEIGHT_NORMAL
#define MY_ALTBTN_FONTRATIO				0.8
#define MY_ALTBTN_FONTFAMILY			wxFONTFAMILY_MODERN
#define MY_ALTBTN_FONTSTYLE				wxFONTSTYLE_ITALIC
#define MY_ALTBTN_FONTWEIGHT			wxFONTWEIGHT_NORMAL
  // Help screens
#define MY_HELP_FONTSIZE				10
#define MY_HELP_FONTWEIGHT				wxFONTWEIGHT_NORMAL
#define MY_HELP_OKBUTTON_MARGIN			5
#define MY_HELP_OKBUTTON_ALIGN			wxALIGN_RIGHT

  // 0: black and white
  // 1: a few colors... Well, really a little bit!
int my_actual_color_code_set = 0;
color_codes_t color_codes[2][SLCC_NB_CODES] = {{
// my_actual_color_code_set = 0
	color_codes_t(wxColour(*wxBLACK),	wxColour(*wxWHITE)),	// SLCC_NORMAL
	color_codes_t(wxColour(*wxWHITE),	wxColour(*wxBLACK)),	// SLCC_EDITED_ITEM
	color_codes_t(wxColour(*wxWHITE),	wxColour(*wxBLACK)),	// SLCC_NEXT_INSTRUCTION
	color_codes_t(wxColour(*wxBLACK),	wxColour(*wxWHITE))		// SLCC_ERROR
}, {
// my_actual_color_code_set = 1
	color_codes_t(*wxBLACK,	*wxWHITE),	// SLCC_NORMAL
	color_codes_t(*wxWHITE,	*wxBLACK),	// SLCC_EDITED_ITEM
	color_codes_t(*wxBLACK,	*wxGREEN),	// SLCC_NEXT_INSTRUCTION
	color_codes_t(*wxRED,	*wxWHITE)	// SLCC_ERROR
}};

static const wxColor slcc_to_bg_wxColor(const slcc_t& color_code) {
	return color_codes[my_actual_color_code_set][color_code].bg;
}
static const wxColor slcc_to_fg_wxColor(const slcc_t& color_code) {
	return color_codes[my_actual_color_code_set][color_code].fg;
}


//
// GLOBAL
//

static int quit_requested = false;
static bool xy_set = false;
static int my_x0 = -1;
static int my_w0 = -1;
static int my_h0 = -1;
static int my_y0 = -1;
static int my_y1 = -1;
//static int my_ty = -1;
static int my_char_width = -1;
static int my_initial_stack_lines = -1;
static int my_initial_stack_width = -1;
static int my_initial_client_width = -1;
static int my_initial_client_height = -1;
static int my_min_client_width = -1;
static int my_min_client_height = -1;
static wxSize my_frame_initial_size;

static wxDialog *dlg = NULL;

#ifdef DEBUG
/*static void debug_disp_and_size(const char *what, wxWindow* wxw) {
	int x, y, w, h;
	wxw->GetPosition(&x, &y);
	wxw->GetSize(&w, &h);
	debug_write(what);
	debug_write_i("  x = %i", x);
	debug_write_i("  y = %i", y);
	debug_write_i("  w = %i", w);
	debug_write_i("  h = %i", h);
}*/
#endif

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


//
// STATUSWINDOW
//

class StatusWindow: public wxScrolledWindow {
	wxWindow *my_parent;
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
public:
    StatusWindow(wxWindow *, wxWindowID, const wxPoint&, const wxSize&);
    void OnPaint(wxPaintEvent&);
	void OnDblClick(wxMouseEvent&);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(StatusWindow, wxScrolledWindow)
    EVT_PAINT(StatusWindow::OnPaint)
	EVT_LEFT_DCLICK(StatusWindow::OnDblClick)
END_EVENT_TABLE()

StatusWindow::StatusWindow(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
		: wxScrolledWindow(parent, id, pos, size, MY_STATUS_BORDER_STYLE),
		my_parent(parent),
		horoffset_exec(0),
		exec_norun(exec_norun_xpm), exec_run(exec_run_xpm),
		horoffset_arrow(0),
		shiftsel(shiftsel_xpm), shiftuns(shiftuns_xpm),
		horoffset_angle(0),
		angle_deg(angle_deg_xpm), angle_rad(angle_rad_xpm),
		horoffset_unit(0),
		unit_bin(unit_bin_xpm), unit_oct(unit_oct_xpm), unit_dec(unit_dec_xpm), unit_hex(unit_hex_xpm) {
    SetBackgroundColour(MY_CALCULATOR_BACKGROUND_COLOR);
	int w = exec_run.GetWidth();
	int w2 = shiftsel.GetWidth();
	int h = shiftsel.GetHeight();
	int w3 = angle_deg.GetWidth();
	int w4 = unit_bin.GetWidth();
	horoffset_exec = MY_STATUS_WINDOW_ITEM_H_MARGIN;
	horoffset_arrow = w + 2 * MY_STATUS_WINDOW_ITEM_H_MARGIN;
	horoffset_angle = w + w2 + 3 * MY_STATUS_WINDOW_ITEM_H_MARGIN;
	horoffset_unit = w + w2 + w3 + 4 * MY_STATUS_WINDOW_ITEM_H_MARGIN;
	SetSize(wxSize(horoffset_unit + w4, h + 2 * MY_STATUS_WINDOW_ITEM_V_MARGIN));
}

void StatusWindow::OnPaint(wxPaintEvent&) {
    wxPaintDC dc(this);
    PrepareDC(dc);

	dc.SetBackgroundMode(wxSOLID);
	SetBackgroundColour(MY_STATUS_BACKGROUND_COLOUR);

	if (ui_is_a_program_halted())
		dc.DrawBitmap(exec_run, horoffset_exec, MY_STATUS_WINDOW_ITEM_V_MARGIN);
	else
		dc.DrawBitmap(exec_norun, horoffset_exec, MY_STATUS_WINDOW_ITEM_V_MARGIN);
	if (ui_dsl.get_status_shift())
		dc.DrawBitmap(shiftsel, horoffset_arrow, MY_STATUS_WINDOW_ITEM_V_MARGIN);
	else
		dc.DrawBitmap(shiftuns, horoffset_arrow, MY_STATUS_WINDOW_ITEM_V_MARGIN);
	if (F->get_angle_mode() == ANGLE_DEG)
		dc.DrawBitmap(angle_deg, horoffset_angle, MY_STATUS_WINDOW_ITEM_V_MARGIN);
	else if (F->get_angle_mode() == ANGLE_RAD)
		dc.DrawBitmap(angle_rad, horoffset_angle, MY_STATUS_WINDOW_ITEM_V_MARGIN);
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
	dc.DrawBitmap(*u, horoffset_unit, MY_STATUS_WINDOW_ITEM_V_MARGIN);
}


//
// MYAPP and MYFRAME
//

class MyFrame;
class StatusWindow;

class MyApp: public wxApp {
	int what_am_i_to_do;
	MyFrame *frame;
	virtual bool OnInit();
	virtual int OnRun();
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
	wxSizer *topSizer;

	wxBtnAll wxAllButtons;

	const MenuDescription *menus_descriptions;
	int nb_menus_descriptions;
	const BtnDescription* btn_descriptions;
	int nb_btn_descriptions;

	StatusWindow *stwin;
	wxWindow *w_path;
	wxStaticText *path;
	int path_width;
	int path_width_associated_stack_width_in_pixels;

	vector<stack_1line> dispStack;
	wxTextCtrl *textTypein;
	void build_dispStack();
	void check_path_width();
	void display_help(const int&);
public:
	friend class UiImplWx;
	MyFrame(const wxString&, const wxPoint&, const wxSize&, const MenuDescription*&, int&, const BtnDescription*&, int&);
	void OnPaint(wxPaintEvent&);
	void OnSize(wxSizeEvent&);
//    void OnQuit(wxCommandEvent&);
	void OnAbout(wxCommandEvent&);
	void OnButton(wxCommandEvent&);
	void OnMenu(wxCommandEvent&);
	void OnChar(wxKeyEvent&);
	void OnDblClick(wxMouseEvent&);
	void resize_frame_to_initial();

	DECLARE_EVENT_TABLE()
};

void StatusWindow::OnDblClick(wxMouseEvent& ev) {
	dynamic_cast<MyFrame *>(GetParent())->resize_frame_to_initial();
	ev.Skip();
}

void MyFrame::OnDblClick(wxMouseEvent& ev) {
	resize_frame_to_initial();
	ev.Skip();
}

void MyFrame::resize_frame_to_initial() {
	if (xy_set) {
		debug_write("Resetting frame to its initial size");
		SetClientSize(my_frame_initial_size);
		textTypein->SetFocus();
	}
}

enum {ID_TEXTTYPEIN,
	ID_START_BUTTONS,	// ID_START_BUTTONS is used to enumerate buttons, so
						// values above ID_START_BUTTONS are reserved, that's why
						// ID_START_MENUS is set to 10,000: it enables the interface
						// to manage up to 9,999 buttons!
	ID_START_MENUS = 10000
};

static void build_menu_bar(const MenuDescription* const md, const int& nb, wxMenuBar*& menuBar) {

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
					menuBar->Append(menus[actual_menu], const_char_to_wxString(_(md[actual_menu].text)));
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
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

MyApp::MyApp() : what_am_i_to_do(PROG_INIT_EXIT) { }

MyFrame* MyApp::get_frame() const {
	return frame;
}

  // Calculate the width of the path area (to be able to end the string with "..." if needed)
void MyFrame::check_path_width() {
	int l = 0;
	wxStaticText *tmp;
	int wtmp, htmp, w, h;
	dispStack[0].t->GetSize(&w, &h);
	debug_write_i("wref = %i", w);
	if (path_width_associated_stack_width_in_pixels != w) {
		do {
			l++;
			tmp = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), l), wxPoint(0, 0), wxSize(wxDefaultSize),
				MY_PATH_BORDERSTYLE);
			tmp->SetFont(wxFont(MY_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, MY_PATH_FONTWEIGHT, 0));
			tmp->GetSize(&wtmp, &htmp);
			tmp->Destroy();
		} while (wtmp < path_width_associated_stack_width_in_pixels);
		path_width = l - 1;
		path_width_associated_stack_width_in_pixels = w;
	}
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,
	const MenuDescription*& md, int& nb_md, const BtnDescription*& bd, int& nb_bd)
		: wxFrame(NULL, -1, title, pos, size),
		menus_descriptions(md), nb_menus_descriptions(nb_md), btn_descriptions(bd), nb_btn_descriptions(nb_bd),
		path_width(-1), path_width_associated_stack_width_in_pixels(-1) {
    SetIcon(wxIcon(prpn_xpm));

	SetBackgroundColour(MY_CALCULATOR_BACKGROUND_COLOR);

	if (ui_has_menu_bar()) {
		wxMenuBar *menuBar;
		build_menu_bar(menus_descriptions, nb_menus_descriptions, menuBar);
		for (int i = 0; i < nb_menus_descriptions; i++) {
			Connect(ID_START_MENUS + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnMenu));
		}
		SetMenuBar(menuBar);
	}

//    CreateStatusBar();
//    SetStatusText(const_char_to_wxString(PACKAGE_NAME));

	topSizer = new wxBoxSizer(wxVERTICAL);

	  // Status line (red arrow to show shift pressed, ...)
	stwin = new StatusWindow(this, wxID_ANY, wxPoint(wxDefaultPosition), wxSize(wxDefaultSize));
	topSizer->Add(stwin, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, MY_STATUS_BORDER);

	  // Path
//    w_path = new wxWindow(this, wxID_ANY, wxPoint(wxDefaultPosition), wxSize(wxDefaultSize));
//    w_path->SetBackgroundColour(MY_PATH_BACKGROUND_COLOUR);
//    wxSizer *sxx = new wxBoxSizer(wxHORIZONTAL);
//    w_path->SetSizer(sxx);
	path = new wxStaticText(this, wxID_ANY, _T(""), wxPoint(wxDefaultPosition), wxSize(wxDefaultSize), MY_PATH_BORDERSTYLE);
	path->SetFont(wxFont(MY_PATH_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, MY_PATH_FONTWEIGHT, 0));
	path->SetForegroundColour(MY_PATH_FOREGROUND_COLOUR);
	topSizer->Add(path, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, MY_PATH_BORDER);
//    sxx->Add(path, 0, wxALL | wxEXPAND, 0);

	  // Stack items
	build_dispStack();

	  // Type-in area
	int w, h;
	dispStack[0].t->GetSize(&w, &h);
	textTypein = new wxTextCtrl(this, ID_TEXTTYPEIN, _T(""), wxPoint(wxDefaultPosition), wxSize(5, h),
						MY_TYPEIN_BORDERSTYLE | wxTE_PROCESS_ENTER | wxTE_MULTILINE);
	textTypein->SetFont(wxFont(MY_TYPEIN_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, MY_TYPEIN_FONTWEIGHT, 0));
	textTypein->SetBackgroundColour(MY_TYPEIN_BACKGROUND_COLOUR);
	textTypein->SetForegroundColour(MY_TYPEIN_FOREGROUND_COLOUR);
	topSizer->Add(textTypein, 0, wxALL | wxEXPAND, MY_TYPEIN_BORDERSIZE);

	  // Buttons
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
			my_id = ID_START_BUTTONS + i;
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

			cur_line->vl.push_back(bsl);
			cur_line->vb.push_back(b);
			b_font_size = MY_BTN_FONTSIZE;
			if (btn_descriptions[i].main_font_proportion != 0)
				b_font_size = static_cast<int>(b_font_size * 1.0 * btn_descriptions[i].main_font_proportion / 100.0);
			t_font_size = static_cast<int>(MY_BTN_FONTSIZE * MY_ALTBTN_FONTRATIO);

			if (btn_descriptions[i].alt_font_proportion != 0)
				t_font_size = static_cast<int>(t_font_size * 1.0 * btn_descriptions[i].alt_font_proportion / 100.0);
			if (bsl.t != NULL) {
				bsl.t->SetFont(wxFont(t_font_size, MY_ALTBTN_FONTFAMILY, MY_ALTBTN_FONTSTYLE, MY_ALTBTN_FONTWEIGHT, 0));
				bsl.t->SetForegroundColour(wxColour(*wxRED));
			}
			b->SetFont(wxFont(b_font_size, MY_BTN_FONTFAMILY, MY_BTN_FONTSTYLE, MY_BTN_FONTWEIGHT, 0));
			Connect(my_id, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyFrame::OnButton));
		}
	}

	textTypein->Connect(ID_TEXTTYPEIN, wxEVT_CHAR, wxKeyEventHandler(MyFrame::OnChar));

	  // To get a button area of the correct size (that just fits the upper area displaying the stack),
	  // we first (STEP 1) add the wxBoxSizers to the topSizer, with only one button inside. Then,
	  // SetSizeHints is called (STEP 2), so that the sizers containing buttons don't further enlarge the window.
	  // At last we add all other buttons (STEP 3.)
	  // This method works so long as a single button isn't larger than the upper sizer.

	  // STEP 1
	SetSizer(topSizer);
	topSizer->Add(1, MY_AROUND_BUTTONS_AREA_BORDER, 0);
	for (vector<wxBtnLine*>::iterator it = wxAllButtons.lines.begin(); it != wxAllButtons.lines.end(); it++) {
		for (int i = 0; i < 2; i++)
			(*it)->bSizer[i] = new wxBoxSizer(wxHORIZONTAL);
		topSizer->Add((*it)->bSizer[0], MY_PROP_BTN_SMALL_LABELS, wxEXPAND | wxLEFT | wxRIGHT, MY_AROUND_BUTTONS_AREA_BORDER);
		topSizer->Add((*it)->bSizer[1], MY_PROP_BTN_BUTTONS, wxEXPAND | wxLEFT | wxRIGHT, MY_AROUND_BUTTONS_AREA_BORDER);
		(*it)->bSizer[0]->Add((*it)->vl[0].s, btn_descriptions[(*it)->btns_index].proportion,
			wxALIGN_BOTTOM, 0);
		(*it)->vl[0].s->Add((*it)->vl[0].t, 1, wxALIGN_CENTER_HORIZONTAL, 0);
		(*it)->bSizer[1]->Add((*it)->vb[0], btn_descriptions[(*it)->btns_index].proportion, wxEXPAND, 0);
	}
	topSizer->Add(1, MY_AROUND_BUTTONS_AREA_BORDER, 0);
	  // STEP 2
	topSizer->SetSizeHints(this);
	  // STEP 3
	for (vector<wxBtnLine*>::iterator it = wxAllButtons.lines.begin(); it != wxAllButtons.lines.end(); it++) {
		for (int i = 1; i < static_cast<int>((*it)->vb.size()); i++) {
			(*it)->bSizer[0]->Add((*it)->vl[i].s, btn_descriptions[(*it)->btns_index + i].proportion,
				wxLEFT | wxALIGN_BOTTOM, MY_BTNCOLUMN_BORDER);
			(*it)->vl[i].s->Add((*it)->vl[i].t, 1, wxALIGN_CENTER_HORIZONTAL, 0);
			(*it)->bSizer[1]->Add((*it)->vb[i], btn_descriptions[(*it)->btns_index + i].proportion, wxEXPAND | wxLEFT, MY_BTNCOLUMN_BORDER);
		}
	}

	check_path_width();

	SetMinSize(wxSize(10, 10));

	textTypein->SetFocus();
}

void stack_line_set_font(wxStaticText *t) {
	t->SetFont(wxFont(MY_STACK_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, MY_STACK_FONTWEIGHT, 0));
}

#define	STACK_INDEX_0	2
void MyFrame::build_dispStack() {
	int n = my_get_max_stack();
	debug_write_i("build_dispStack(): n = %i", n);

	stack_1line l1;

	for (int i = static_cast<int>(dispStack.size()); i >= 1; i--) {
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
		} else {
			wxp = wxPoint(wxDefaultPosition);
			wxs = wxSize(wxDefaultSize);
		}
		l1.w = new wxWindow(this, wxID_ANY, wxp, wxs, MY_STACK_BORDERSTYLE);
		l1.t = new wxStaticText(l1.w, wxID_ANY, wxString(wxChar(' '), ui_dsl.get_width()),
				wxPoint(0, 0), wxs, MY_STACK_BORDERSTYLE);
		stack_line_set_font(l1.t);
		l1.w->SetBackgroundColour(slcc_to_bg_wxColor(SLCC_NORMAL));
		l1.t->SetForegroundColour(slcc_to_fg_wxColor(SLCC_NORMAL));
		dispStack.push_back(l1);
		topSizer->Insert(STACK_INDEX_0 + i, l1.w, 0, wxALL, MY_STACK_BORDERSIZE);
	}
}

void MyFrame::display_help(const int& dh) {
	wxSize mysize = wxSize(85 * MY_HELP_FONTSIZE, 46 * MY_HELP_FONTSIZE);
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
		topsizer->Add(btn, 0, wxALL | MY_HELP_OKBUTTON_ALIGN, MY_HELP_OKBUTTON_MARGIN);
		dlg->SetSizer(topsizer);
		topsizer->Fit(dlg);
	} else {
		wxBoxSizer *topsizer;
		topsizer = new wxBoxSizer(wxVERTICAL);
		wxTextCtrl *t = new wxTextCtrl(dlg, wxID_ANY, string_to_wxString(stack_get_help(dh)),
			wxPoint(wxDefaultPosition), mysize, wxTE_READONLY | wxTE_MULTILINE);
		t->SetFont(wxFont(MY_HELP_FONTSIZE, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, MY_HELP_FONTWEIGHT, 0));
		topsizer->Add(t, 1, wxALL, 10);
		string ok = _("OK");
		wxButton *btn = new wxButton(dlg, wxID_OK, string_to_wxString(ok));
		btn->SetDefault();
		topsizer->Add(btn, 0, wxALL | MY_HELP_OKBUTTON_ALIGN, MY_HELP_OKBUTTON_MARGIN);
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
	int btn_index = ev.GetId() - ID_START_BUTTONS;
	const char *sz = btn_descriptions[btn_index].main_cmd;
	if (ui_dsl.get_status_shift() && !const_char_is_empty(btn_descriptions[btn_index].alt_cmd))
		sz = btn_descriptions[btn_index].alt_cmd;
	ui_notify_button_pressed(sz);
	textTypein->SetFocus();
}

void MyFrame::OnMenu(wxCommandEvent& ev) {
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

	//debug_write_i("X1: key pressed = %i", wxk);

	int gm = event.GetModifiers();
	int uik;
	switch (wxk) {
		case WXK_UP:
			uik = UIK_UP;
			break;
		case WXK_DOWN:
			uik = UIK_DOWN;
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
//        my_ty = ty + my_h0 - 1;

//        debug_write_i("A1: my_x0 = %i", my_x0);
//        debug_write_i("A1: my_w0 = %i", my_w0);
//        debug_write_i("A1: my_h0 = %i", my_h0);
//        debug_write_i("A1: my_y0 = %i", my_y0);
//        debug_write_i("A1: my_y1 = %i", my_y1);
//        debug_write_i("A1: my_ty = %i", my_ty);

		wxStaticText *t1 = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), 1),
			wxPoint(0, 0), wxSize(wxDefaultSize), MY_STACK_BORDERSTYLE);
		stack_line_set_font(t1);
		wxStaticText *t2 = new wxStaticText(this, wxID_ANY, wxString(wxChar(' '), 2),
			wxPoint(0, 0), wxSize(wxDefaultSize), MY_STACK_BORDERSTYLE);
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
	
		xy_set = true;
	}
	ev.Skip();
}

void MyFrame::OnSize(wxSizeEvent& ev) {
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
		ui_dsl.redefine_geometry(target_stack_lines, target_stack_width);
	}
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
public:
	UiImplWx(MyFrame*);
	virtual ~UiImplWx();

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

UiImplWx::UiImplWx(MyFrame *ff) : UiImpl(), f(ff) { }

UiImplWx::~UiImplWx() { }

void UiImplWx::refresh_statuswin() { f->stwin->Refresh(); }

void UiImplWx::set_line(const int& line_number, const slcc_t& color_code, const string& s) {
	f->dispStack[line_number - 1].t->SetForegroundColour(slcc_to_fg_wxColor(color_code));
	f->dispStack[line_number - 1].w->SetBackgroundColour(slcc_to_bg_wxColor(color_code));
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

	debug_write_i("char = %i", static_cast<int>(c));

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
	//debug_write_i("idx = %i", static_cast<int>(idx));
	//debug_write_i("Character at the position of the cursor toward left = %i", static_cast<int>(c));
	if (replace)
		f->textTypein->Remove(idx, idx + 1);
	f->textTypein->SetInsertionPoint(idx);
	f->textTypein->WriteText(string_to_wxString(insert_string));
	f->textTypein->SetInsertionPoint(back_idx + (replace ? 0 : 1) + insert_string.length() - 1);
}

void UiImplWx::refresh_stack_height() {
	f->build_dispStack();
	f->check_path_width();

	int y0 = my_y0 + my_get_max_stack() * (my_y1 - my_y0);

	int x, y, w, h;
	f->textTypein->GetPosition(&x, &y);
	f->textTypein->GetSize(&w, &h);
	int bottom = y + my_h0 - 1;
	int new_h = bottom - y0 + 1;

	f->textTypein->SetSize(my_x0, y0, my_w0, new_h);

//    int ww, hh;
//    f->path->GetSize(&ww, &hh);
//    debug_write_i("ww = %i", ww);
//    debug_write_i("hh = %i", hh);

	//cout << "X5: setting size to " << my_w0 << " x " << h << endl;

	//f->topSizer->SetSizeHints(f);
}

void UiImplWx::quit() {
	quit_requested = true;
	f->Close(TRUE);
}

bool UiImplWx::want_to_refresh_display() { return false; }

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
		ui_impl = new UiImplWx(wxGetApp().get_frame());
		ui_post_init();
		int r = wxApp::OnRun();
		int c = prog_terminate();
		return c != 0 ? c : r;
	}
	return 1;
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

	const MenuDescription *menus_descriptions;
	int nb_menus_descriptions;
	ui_get_menus_descriptions(menus_descriptions, nb_menus_descriptions);
	const BtnDescription *btn_descriptions;
	int nb_btn_descriptions;
	ui_get_buttons_layout_description(btn_descriptions, nb_btn_descriptions);
	frame = new MyFrame(_T(PACKAGE_NAME), wxPoint(wxDefaultPosition), wxSize(wxDefaultSize),
		menus_descriptions, nb_menus_descriptions, btn_descriptions, nb_btn_descriptions);
	frame->Show(true);
	SetTopWindow(frame);

	return true;
}

