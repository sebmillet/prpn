// Ui.cpp
// Generic UI management (not specific to UI implementation)

// RPN calculator

// Sébastien Millet
// August 2009 - March 2010

#include "Common.h"
#include "Stack.h"
#include "Parser.h"
#include "Ui.h"
#include "platform/os_generic.h"
#include <sstream>
#include <map>
#include <cstring>

#ifdef DEBUG
#include <iostream>
#endif

#include <fstream>

using namespace std;

#define HARD_GUI_MIN_HEIGHT	3

static void set_refresh_status_flag();
static void reset_refresh_status_flag();
static bool get_refresh_status_flag();
static void set_refresh_stack_flag();
static void reset_refresh_stack_flag();
static bool ui_get_refresh_stack_flag();
static void ui_set_recalc_stack_flag();
static void reset_recalc_stack_flag();
static bool get_recalc_stack_flag();

typedef enum {TYPEIN_EMPTY, TYPEIN_ANY, TYPEIN_STRING, TYPEIN_EXPRESSION, TYPEIN_PROGRAM} typein_t;

typedef enum {EM_DEFAULT, EM_EDIT_STACK} em_t;
em_t edit_mode = EM_DEFAULT;
int stack_item_number_being_edited;

UiImpl *ui_impl;

static TransStack *ts = 0;

static typein_t typein_status = TYPEIN_EMPTY;

const int default_disp_width = 23;
const int default_disp_height = 5;
DisplayStackLayout ui_dsl(-1, -1);
static void ui_set_status_shift(const bool& b) {
	if (b == ui_dsl.get_status_shift())
		return;
	ui_dsl.set_status_shift(b);
	set_refresh_status_flag();
}

static IntervalShift ui_shift = {0, 0, 0};

static void refresh_path();
static void refresh_status();
static void refresh_stack(const int&);

static bool refresh_status_flag = true;
static int status_actual_base = 0;
static void set_refresh_status_flag() { refresh_status_flag = true; }
static void reset_refresh_status_flag() { refresh_status_flag = false; }
static bool get_refresh_status_flag() { return refresh_status_flag; }

static bool is_displaying_error = false;
static string display_error_l1 = "";
static string display_error_l2 = "";
  // Uncomment the below if you wish the calculator to display
  // a message on start-up
/*static bool is_displaying_error = true;
static string display_error_l1 = "HELP command for help";
static string display_error_l2 = "";
*/

static bool refresh_stack_flag = true;
static void set_refresh_stack_flag() { refresh_stack_flag = true; }
static void reset_refresh_stack_flag() {
	reset_recalc_stack_flag();
	refresh_stack_flag = false;
}
static bool ui_get_refresh_stack_flag() { return refresh_stack_flag; }

static bool recalc_stack_flag = true;
static void ui_set_recalc_stack_flag() {
	set_refresh_stack_flag();
	recalc_stack_flag = true;
}
static void reset_recalc_stack_flag() { recalc_stack_flag = false; }
static bool get_recalc_stack_flag() { return recalc_stack_flag; }

static void set_error(const std::string&, const std::string&);
static void set_syntax_error(const int&, const int&, const int&, const int&);

static void redim(int nb_newlines = -1) {
	if (nb_newlines == -1)
		nb_newlines = ui_impl->get_nb_newlines(ui_dsl.get_height());
	if (ui_dsl.redim(nb_newlines)) {
		ui_impl->refresh_stack_height();
		ui_set_recalc_stack_flag();
	}
}

void ui_string_trim(string& s, const size_t& width, const DisplayStackLayout *dsl) {
	if (width >= 1 && E->get_string_length(s.c_str()) > width && dsl->get_max_stack() >= 1) {
		E->erase(s, width - dsl->get_to_be_continued_length());
		s.append(dsl->get_to_be_continued());
	}
}


//
// Buttons area
//

const BtnDescription btn_descriptions[] = {
	{1, "__FF0000__", 0, "_SHIFT", "", 0, ""},
	{2, "ENTER", 0, "", "EDIT", 0, "_EDIT"},
	{1, "+/-", 0, "_NEG", "VIEW ^", 0, "_VUP"},
	{1, "EEX", 0, "_E", "VIEW v", 0, "_VDOWN"},
	{-1, "", 0, "", "", 0, ""},
	{3, "'", 0, "'", "HELP", 0, "_HELP"},
	{4, "7", 0, "7", "", 0, ""},
	{4, "8", 0, "8", "UNDO", 0, "UNDO"},
	{4, "9", 0, "9", "", 0, ""},
	{4, "/", 0, "/", "1/x", 0, "INV"},
	{-1, "", 0, "", "", 0, ""},
	{3, "STO", 0, "STO", "RCL", 0, "RCL"},
	{4, "4", 0, "4", "PURGE", 0, "PURGE"},
	{4, "5", 0, "5", "", 0, ""},
	{4, "6", 0, "6", "", 0, ""},
	{4, "*", 0, "*", "^", 0, "^"},
	{-1, "", 0, "", "", 0, ""},
	{3, "EVAL", 80, "EVAL", "->NUM", 0, "->NUM"},
	{4, "1", 0, "1", "", 0, ""},
	{4, "2", 0, "2", "%", 0, "%"},
	{4, "3", 0, "3", "\%CH", 0, "\%CH"},
	{4, "-", 0, "-", "SQR", 0, "SQR"},
	{-1, "", 0, "", "", 0, ""},
	{3, "ON", 0, "_ON", "OFF", 0, "_OFF"},
	{4, "0", 0, "0", "CLEAR", 0, "CLEAR"},
	{4, ".", 0, ".", "PI", 0, "PI"},
	{4, ",", 0, ",", "->", 0, "->"},
	{4, "+", 0, "+", "x^2", 0, "SQ"}
};

static map<string, beval_t> cmd_evals;

static void readrc_1file(const string& filename_real, const string& filename_display) {
	string error_l1, error_l2;
	st_err_t c = read_rc_file(ts, TOSTRING_PORTABLE, filename_real, filename_display, true, error_l1, error_l2);
	if (c == ST_ERR_FILE_READ_ERROR) {
		if (os_file_exists(filename_real.c_str())) {
			set_error(filename_display + ":", "Could not open");
		}
	} else if (c != ST_ERR_OK && c != ST_ERR_EXIT)
		set_error(error_l1, error_l2);
}

static void readrc() {
	if (!opt_dry_run) {
		readrc_1file(osd->get_dir(OSF_VARSRC), osd->get_dir(OSF_SMALL_VARSRC));
		readrc_1file(osd->get_dir(OSF_STACKRC), osd->get_dir(OSF_SMALL_STACKRC));
	}
}

static void write_vars_dir(ostream& oss) {

// Part 1: output vars structure and content

	st_err_t c;
	const StackItem *csi;
	StackItem *si;
	Var *e;
	vector<string> *by_order;
	ts->vars.get_var_list(by_order);
	for (vector<string>::reverse_iterator it = by_order->rbegin(); it != by_order->rend(); it++) {
		c =  ts->rcl_const(*it, csi, e);
		if (c == ST_ERR_UNDEFINED_NAME)
			throw(CalcFatal(__FILE__, __LINE__, "write_vars_dir(): undefined name!!!??"));
		if (c == ST_ERR_DIRECTORY_NOT_ALLOWED) {
			oss << "'" + *it + "' CRDIR\n" + *it + "\n";
			si = const_cast<StackItem*>(csi);
			ts->rcl_for_eval(*it, si);
			if (si != NULL)
				throw(CalcFatal(__FILE__, __LINE__, "write_vars_dir(): StackItem* should be NULL"));
			write_vars_dir(oss);
			oss << "UP\n";
			ts->vars.up();
		} else if (c == ST_ERR_OK) {
			write_si(TOSTRING_PORTABLE, csi, oss);
			oss << "'" + *it + "' STO\n";
		} else
			throw(CalcFatal(__FILE__, __LINE__, "write_vars_dir(): unexpected return code from ts->rcl()"));
			
	}
	delete by_order;
}

static void writerc() {

	if (opt_dry_run)
		return;

// 1. Flags + stack content
// Written into ~/.prpn/stackrc or alike depending on OS

	fstream ofs(osd->get_dir(OSF_STACKRC).c_str(), fstream::out | fstream::trunc);

	if (ofs.good()) {
		  // Flags
		string f = get_rclf_portable_string();
		ofs << f << " STOF\n";
		  // Stack items
		const StackItem *csi;
		int n = ui_get_nb_items_in_stack();
		for (int i = n; i >= 1; i--) {
			csi = ts->transstack_get_const_si(i);
			write_si(TOSTRING_PORTABLE, csi, ofs);
		}
	} else {
		debug_write("Unable to open:");
		debug_write(osd->get_dir(OSF_STACKRC).c_str());
	}
	ofs.close();

// 2. Variables
// Written into ~/.prpn/varsrc or alike depending on OS

	fstream ofs2(osd->get_dir(OSF_VARSRC).c_str(), fstream::out | fstream::trunc);
	if (ofs2.good()) {
		VarDirectory *rec = ts->vars.save_pwd();
		ts->vars.home();
		write_vars_dir(ofs2);
		ts->vars.recall_pwd(rec);
		vector<string> *directories;
		ts->vars.get_path(directories);
		for (vector<string>::iterator it = directories->begin(); it != directories->end(); it++) {
			ofs2 << *it << " ";
		}
		delete directories;
	}
	ofs2.close();
}

void ui_init() {
	char sz[2];
	sz[1] = '\0';
	  // Buttons that trigger eval as per default EXCEPT in an expression
	string s("+-*/^");
	for (string::const_iterator it = s.begin(); it != s.end(); it++) {
		*sz = *it;
		cmd_evals[sz] = BEVAL_NOT_IN_EXPRESSION;
	}
	  // Buttons that NEVER trigger eval
	s = ".,{}[]()<>'\"0123456789";
	for (string::const_iterator it = s.begin(); it != s.end(); it++) {
		*sz = *it;
		cmd_evals[sz] = BEVAL_NEVER;
	}
	cmd_evals["->"] = BEVAL_NEVER;
	cmd_evals["<<"] = BEVAL_NEVER;
	cmd_evals[">>"] = BEVAL_NEVER;
	cmd_evals["_E"] = BEVAL_BTN_EEX;
	cmd_evals["_SHIFT"] = BEVAL_BTN_SHIFT;
	cmd_evals["_ON"] = BEVAL_BTN_ON;
	cmd_evals["_OFF"] = BEVAL_BTN_OFF;
	cmd_evals["_VUP"] = BEVAL_BTN_VUP;
	cmd_evals["_VDOWN"] = BEVAL_BTN_VDOWN;
	cmd_evals["_NEG"] = BEVAL_BTN_NEG;
	cmd_evals["_EDIT"] = BEVAL_BTN_EDIT;
	cmd_evals["_HELP"] = BEVAL_BTN_HELP;
	  // Special case: ENTER
	cmd_evals[""] = BEVAL_ALWAYS;	// Corresponds to the ENTER button
	  // Now that we've defined cmd_evals, we can go through btn_descriptions
	/*beval_t be;
	bevals.resize(sizeof(btn_descriptions) / sizeof(*btn_descriptions));
	for (int i = 0; i < static_cast<int>(sizeof(btn_descriptions) / sizeof(*btn_descriptions)); i++) {
		be = cmd_evals[btn_descriptions[i].main_cmd];
		if (be == BEVAL_NULL)
			be = BEVAL_DEFAULT;
		bevals[i] = be;
	}*/

	if (opt_height != -1 && opt_height < HARD_GUI_MIN_HEIGHT && has_gui)
		opt_height = HARD_GUI_MIN_HEIGHT;
	ui_dsl = DisplayStackLayout(opt_width == -1 ? default_disp_width : opt_width,
		opt_height == -1 ? default_disp_height : opt_height);
	if (opt_min_stack_height != -1)
		ui_dsl.set_min_stack_height(opt_min_stack_height);
	ts = new TransStack();
}

void ui_post_init() {
	readrc();
	ui_refresh_display();
}

void ui_get_buttons_layout_description(const BtnDescription*& b, int& nb) {
	b = btn_descriptions;
	nb = static_cast<int>(sizeof(btn_descriptions) / sizeof(*btn_descriptions));
}

  // Return the int made of the hex digits at sz[0] and sz[1]
  // ***   Assume ASCII coding!!!   ***
static int ascii_decode_2_hexdigits(const char *sz) {
	if (sz[0] == '\0' || sz[1] == '\0')
		return -1;
	char c;
	int r[2];
	for (int i = 0; i < 2; i++) {
		c = tolower(sz[i]);
		if (c >= 'a' && c <= 'f')
			r[i] = c - 'a' + 10;
		else if (c >= '0' && c <= '9')
			r[i] = c - '0';
		else
			r[i] = -1;
	}
	if (r[0] >= 0 && r[1] >= 0)
		return 16 * r[0] + r[1];
	else
		return -1;
}

void ui_decode_color_code(const char *sz, const char*& ret, int& r, int& g, int& b) {
	ret = sz;
	r = -1;
	g = -1;
	b = -1;
	if (strlen(sz) >= 10) {
		if (sz[0] == '_' && sz[1] == '_' && sz[8] == '_' && sz[9] == '_') {
			r = ascii_decode_2_hexdigits(sz + 2);
			g = ascii_decode_2_hexdigits(sz + 4);
			b = ascii_decode_2_hexdigits(sz + 6);
			if (r < 0 || g < 0 || b < 0) {
				r = -1;
				g = -1;
				b = -1;
			}
		}
	}
	if (r >= 0)
		ret = sz + 10;
}

void ui_terminate() {
	writerc();
	delete ui_impl;
	if (ts != NULL)
		delete ts;
}

void ui_display_help(const int& dh) { ui_impl->display_help(dh); }

static void erase_input(const bool& reset_everything) {

	//debug_write("erase_input()");
	//debug_write_i("typein_status = %i", typein_status);

	if (ui_dsl.get_status_shift()) {
		ui_set_status_shift(false);
		refresh_status();
	}

	if (typein_status == TYPEIN_EMPTY && ui_shift.actual == 0)
		return;
	ui_impl->erase_input();
	ui_shift.actual = 0;
	set_refresh_stack_flag();

	typein_status = TYPEIN_EMPTY;

	if (reset_everything) {
		edit_mode = EM_DEFAULT;
		redim(0);
	}
}

void ui_flush_input(const string& textin, const string& additional_command) {
	st_err_t c;
	bool eval_items = true;
	vector<SIO> vs;
	bool flag_erase_input = true;

	/*debug_write("textin:");
	debug_write(textin.c_str());
	debug_write("additional_command:");
	debug_write(additional_command.c_str());*/

// STEP 1 in 4: read items from inputs

	ParserError par;
	par.set = false;
	for (int i = (textin.empty() ? 1 : 0);
			!par.set && (i == 0 || (i == 1 && (!additional_command.empty() || textin.empty())));
			i++) {
		StackItem *si;
		istringstream* iss;
		const string* input = (i == 0 ? &textin : &additional_command);
		if (input->empty())
			iss = new istringstream("DUP");
		else
			iss = new istringstream(*input);
		Itemiser *itemise = new Itemiser(iss, TOSTRING_EDIT);

		bool r = true;
		while (r) {
			r = itemise->get_item(par, si);
			if (par.set) {
				set_syntax_error(par.col_start, par.col_end, par.line, par.c_index);
				r = false;
				flag_erase_input = false;
				eval_items = false;
			} else if (r)
				vs.push_back(SIO(TSO_OUTSIDE_TS, si));
		}
		delete iss;
		delete itemise;
	}

// STEP 2 in 4: evaluate read items in the stack

	c = ST_ERR_OK;
	if (eval_items) {
		if (edit_mode == EM_EDIT_STACK) {
			SIO s = ts->transstack_pop();
			s.cleanup();
			edit_mode = EM_DEFAULT;
		}
		string cmd_err;
		bool inside_undo_sequence = false;
		for (vector<SIO>::iterator it = vs.begin(); it != vs.end() && c == ST_ERR_OK; it++) {
			(*it).ownership = TSO_OWNED_BY_TS;
			c = ts->do_push_eval(*it, inside_undo_sequence, cmd_err);
			ui_set_recalc_stack_flag();
			if (c != ST_ERR_OK && c != ST_ERR_EXIT) {
				set_error(cmd_err + " Error:", get_error_string(c));
			}
			inside_undo_sequence = true;
		}
	}

// STEP 3 in 4: clean items in case read or evaluation has been aborted (syntax error or evaluate error)

	for (vector<SIO>::iterator it = vs.begin(); it != vs.end(); it++)
		(*it).cleanup();

// STEP 4 in 4: perform some tasks specifically linked to the user interface

	set_refresh_stack_flag();
	if (flag_erase_input)
		erase_input(true);
	if (c == ST_ERR_EXIT)
		ui_impl->quit();
}

static void refresh_path() {
	static string cache_path;
	bool has_changed = ts->vars.update_si_path();
	if (has_changed) {
		cache_path = ts->vars.get_si_path_string();
		//debug_write("A1: ********** PATH REFRESHED **********");
	}

	ui_impl->refresh_display_path(cache_path, has_changed);
}

static void refresh_status() {
	if (status_actual_base != get_base_from_flags()) {
		status_actual_base = get_base_from_flags();
		set_refresh_status_flag();
	}
	if (get_refresh_status_flag()) {
		ui_impl->refresh_statuswin();
		reset_refresh_status_flag();
	}
}

static inline const string ui_get_ts_display_line(const int& line_number, bool& recalc, bool& no_more_lines) {
	return ts->transstack_get_display_line(ui_dsl, line_number, ui_shift, recalc, no_more_lines);
}

static void refresh_stack(const int& enforced_nb_stack_elems_to_display) {

	//debug_write("A0: refresh_stack(): 0 (enter function)");

	if (!ui_impl->want_to_refresh_display() && !ui_get_refresh_stack_flag())
		return;

	//debug_write("A0: refresh_stack(): 1 (stack refreshed)");

	string l;

	int i_upper;
	if (enforced_nb_stack_elems_to_display < 0)
		i_upper = ui_dsl.get_max_stack();
	else {
		i_upper = enforced_nb_stack_elems_to_display;
		ui_set_recalc_stack_flag();
	}

	if (get_recalc_stack_flag()) {
		ui_shift.actual = 0;
		//debug_write("A0: refresh_stack(): 2 (get_recalc_stack_flag() == true)");
	}

	//debug_write("");

	bool recalc = get_recalc_stack_flag();
	bool no_more_lines = false;
	for (int i = 1; i <= i_upper && !no_more_lines; i++) {
		string* ps;
		if (is_displaying_error && i == 1)
			ps = &display_error_l1;
		else if (is_displaying_error && i == 2 && !display_error_l2.empty())
			ps = &display_error_l2;
		else {
			l = ui_get_ts_display_line(i, recalc, no_more_lines);
			ps = &l;
		}
		ui_impl->set_line(i, *ps);
	}
	reset_recalc_stack_flag();
	reset_refresh_stack_flag();
}

void ui_refresh_display(const int& enforced_nb_stack_elems_to_display) {
	refresh_status();
	refresh_path();
	refresh_stack(enforced_nb_stack_elems_to_display);
}

static void set_error(const std::string& l1, const std::string& l2) {
	is_displaying_error = true;
	display_error_l1 = l1;
	display_error_l2 = l2;
	ui_impl->ui_set_error_call_back(l1, l2);
}

static void set_syntax_error(const int& col_start, const int& col_end, const int& line, const int& c_index) {
	is_displaying_error = true;
	display_error_l1 = "Syntax Error";
	display_error_l2 = "";
	ui_impl->set_syntax_error(col_start, col_end, line, c_index);
}

int ui_get_nb_items_in_stack() { return ts->stack_get_count(); }

void ui_reset_is_displaying_error() {
	if (is_displaying_error) {
		is_displaying_error = false;
		ui_set_recalc_stack_flag();
	}
}

static void work_out_typein_status() {
	if (typein_status != TYPEIN_EMPTY)
		return;
	string s = ui_impl->get_first_char();
	if (s.length() >= 1) {
		char c = s.at(0);
		switch (c) {
			case '"':
				typein_status = TYPEIN_STRING;
				break;
			case '\'':
				typein_status = TYPEIN_EXPRESSION;
				break;
			case '<':
				typein_status = TYPEIN_PROGRAM;
				break;
			default:
				typein_status = TYPEIN_ANY;
		}
	}
}

static void enter_edit_mode() {
	if (edit_mode != EM_DEFAULT || !ui_impl->get_first_char().empty())
		return;

	  // One day, we may implement direct edition of any item in the stack, as with the HP-48
	const int n = 1;

	if (static_cast<int>(ts->stack_get_count()) < n)
		return;
	erase_input(false);
	const StackItem *csi = ts->transstack_get_const_si(n);
	edit_mode = EM_EDIT_STACK;
	stack_item_number_being_edited = n;
	  // Width - 2 because of the elevator at the right, that consumes this number of characters
	ToString tostr(TOSTRING_DISPLAY, 0, ui_dsl.get_width() - ui_impl->get_elevator_width());
	csi->to_string(tostr);
	tostr.lock();
	const char *cz = tostr.get_first_line();
	while (cz != NULL) {
		ui_impl->append_line(cz);
		cz = tostr.get_next_line();
		if (cz != NULL)
			ui_impl->append_line("\n");
	}
	tostr.unlock();
	ui_impl->set_cursor_at_the_beginning();
	redim();
}

void ui_notify_button_pressed(const char *c) {
	work_out_typein_status();
	ui_reset_is_displaying_error();

	beval_t beval = cmd_evals[c];
	if (beval == BEVAL_NULL)
		beval = BEVAL_DEFAULT;

	if (beval == BEVAL_BTN_NEG && !ui_impl->get_first_char().empty()) {
		ui_impl->neg();
		beval = BEVAL_NULL;
	} else if (beval == BEVAL_BTN_NEG) {
		c = "NEG";
		beval = BEVAL_DEFAULT;
	} else if (beval == BEVAL_BTN_EEX && !ui_impl->get_first_char().empty()) {
		c = "E";
		beval = BEVAL_NEVER;
	} else if (beval == BEVAL_BTN_EEX) {
		c = "1E";
		beval = BEVAL_NEVER;
	}

	if (beval == BEVAL_BTN_SHIFT)
		ui_set_status_shift(!ui_dsl.get_status_shift());
	else if (beval == BEVAL_BTN_ON)
		erase_input(true);
	else if (beval == BEVAL_BTN_OFF) {
		ui_set_status_shift(false);
		ui_impl->quit();
	} else if (beval == BEVAL_BTN_VUP) {
		ui_set_status_shift(false);
		ui_notify_key_pressed(UIK_UP);
	} else if (beval == BEVAL_BTN_VDOWN) {
		ui_set_status_shift(false);
		ui_notify_key_pressed(UIK_DOWN);
	} else if (beval == BEVAL_BTN_EDIT) {
		ui_set_status_shift(false);
		enter_edit_mode();
	} else if (beval == BEVAL_BTN_HELP) {
		ui_set_status_shift(false);
		ui_impl->display_help(DH_MAIN);
	} else if ((typein_status == TYPEIN_EMPTY || typein_status == TYPEIN_ANY || *c == '\0') && beval != BEVAL_NEVER &&
			beval != BEVAL_NULL && (beval != BEVAL_NOT_IN_EXPRESSION || typein_status != TYPEIN_EXPRESSION)) {
		ui_set_status_shift(false);
		ui_flush_input(ui_impl->get_string(), c);
	} else if (beval != BEVAL_NULL) {
		if (TYPEIN_EXPRESSION && beval != BEVAL_NEVER && !ui_impl->space_at_the_left_of_the_cursor())
			ui_impl->insert_text(" ");
		ui_set_status_shift(false);
		ui_impl->insert_text(c);
	}

	if (beval != BEVAL_NULL)
		ui_refresh_display();
}

bool ui_notify_key_pressed(const int& k) {
	bool event_stop = false;

	//debug_write_i("A1: ui_notify_key_pressed(): k = %i", static_cast<int>(k));

	work_out_typein_status();
	int delta = 0;
	if (typein_status == TYPEIN_EMPTY && (k == UIK_UP || k == UIK_DOWN)) {
		if (k == UIK_UP && ui_shift.actual > ui_shift.min) {
			delta = -1;
		} else if (k == UIK_DOWN && ui_shift.actual < ui_shift.max) {
			delta = 1;
		}
		event_stop = true;
	}
	ui_reset_is_displaying_error();
	char sz[2];
	switch (k) {
		case UIK_NEWLINE:
			  // FIXME
			  // We add 1 because the newline is not ALREADY taken into account
			  // It is a gruiiiik hack, as it simply ignores the fact that the ENTER key may
			  // happen while text is selected, text that itself includes newline characters.
			  // Any way this simplification works not too bad.
			redim(ui_impl->get_nb_newlines(ui_dsl.get_height()) + 1);
			break;
		case UIK_RETURN:
			ui_flush_input(ui_impl->get_string(), "");
			break;
		case UIK_ESCAPE:
			erase_input(true);
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			*sz = static_cast<char>(k);
			sz[1] = '\0';
			ui_notify_button_pressed(sz);
			event_stop = true;
			break;
		default:
			;
	}
	if (delta != 0) {
		ui_shift.actual += delta;
		set_refresh_stack_flag();
	}
	ui_refresh_display();

	//debug_write_i("typein_status = %i", typein_status);

	return event_stop;
}


//
// DISPLAYSTACKLAYOUT
//

static const char *DISPLAYSTACKLAYOUT_TO_BE_CONTINUED = "...";
static const int DISPLAYSTACKLAYOUT_TO_BE_CONTINUED_LENGTH = 3;

  // Constructor
DisplayStackLayout::DisplayStackLayout(const int& w, const int& h) :
	width(w), height(h), min_stack(h - 1), max_stack(h - 1), shift(0), bmenu(0),
	to_be_continued(DISPLAYSTACKLAYOUT_TO_BE_CONTINUED), to_be_continued_length(DISPLAYSTACKLAYOUT_TO_BE_CONTINUED_LENGTH),
	status_shift(false) {
	
}

  // Copy-constructor
DisplayStackLayout::DisplayStackLayout(const DisplayStackLayout& dsl) :
	width(dsl.width), height(dsl.height), min_stack(dsl.min_stack), max_stack(dsl.max_stack),
	shift(dsl.shift), bmenu(dsl.bmenu), to_be_continued(dsl.to_be_continued),
	to_be_continued_length(dsl.to_be_continued_length) {
}

void DisplayStackLayout::set_min_stack_height(const int& i) {
	min_stack = i;
}

int DisplayStackLayout::get_width() const { return width; }
int DisplayStackLayout::get_height() const { return height; }
int DisplayStackLayout::get_min_stack() const { return min_stack; }
int DisplayStackLayout::get_max_stack() const { return max_stack; }
int DisplayStackLayout::get_prompt_height() const { return height - max_stack - bmenu; }
const char *DisplayStackLayout::get_to_be_continued() const { return to_be_continued; }
int DisplayStackLayout::get_to_be_continued_length() const { return to_be_continued_length; }

bool DisplayStackLayout::redim(const int& nb_newlines) {
	int target = height - bmenu - nb_newlines - 1;
	target = (target < 1 ? 1 : target);

	//debug_write_i("height = %i", height);
	//debug_write_i("min_stack = %i", min_stack);
	//debug_write_i("max_stack = %i", max_stack);
	//debug_write_i("nb_newlines = %i", nb_newlines);
	//debug_write_i("target = %i", target);

	if (target != max_stack) {
		if (min_stack == max_stack)
			min_stack = target;
		max_stack = target;
		return true;
	}
	return false;
}

void DisplayStackLayout::set_to_be_continued(const char *tbc, const int& tbcl) {
	to_be_continued = tbc;
	to_be_continued_length = tbcl;
}

void DisplayStackLayout::set_status_shift(const bool& b) { status_shift = b; }

bool DisplayStackLayout::get_status_shift() const { return status_shift; }

int DisplayStackLayout::get_base() const { return get_base_from_flags(); }


//
// UIIMPL
//

UiImpl::UiImpl() {
	ui_dsl.set_to_be_continued(os_to_be_continued, os_to_be_continued_length);
}

UiImpl::~UiImpl() { }

void UiImpl::ui_set_error_call_back(const std::string&, const std::string&) { }

