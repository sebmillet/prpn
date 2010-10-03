// UiImplConsole.cpp
// Console UI implementation

// RPN calculator

// Sébastien Millet
// August 2009 - March 2010

#include "Ui.h"
#include <string>
#include <iostream>

using namespace std;

extern bool opt_batch;
extern bool debug_tokeniser;
extern bool debug_itemiser;
extern bool debug_locale;

extern void debug_test_tokeniser();
extern void debug_test_itemiser();
extern void debug_display_locale();

static bool quit_requested = false;


//
// UIIMPLCONSOLE
//
// This part derivates the massive abstract UiImpl class
// and defines all its abstract classes
//

class UiImplConsole : public UiImpl {
	UiImplConsole(const UiImplConsole&);
public:
	UiImplConsole();
	virtual ~UiImplConsole();

	virtual void refresh_statuswin();
	virtual void ui_set_error_call_back(const string&, const string&);
	virtual void set_line(const int&, const string&);
	virtual void enforce_refresh();
	virtual void refresh_display_path(const string&, const bool&);
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
};


//
// UIIMPLCONSOLE
//

UiImplConsole::UiImplConsole() : UiImpl() { }

UiImplConsole::~UiImplConsole() { }

void UiImplConsole::refresh_statuswin() { }

void UiImplConsole::set_line(const int&, const string& s) { cout << s << endl; }

void UiImplConsole::enforce_refresh() { }

void UiImplConsole::refresh_display_path(const string& s, const bool&) {
	if (!opt_batch) {
		string s_mod = s;
		ui_string_trim(s_mod, ui_dsl.get_width(), &ui_dsl);
		cout << s_mod << endl;
	}
}

void UiImplConsole::set_syntax_error(const int& col_start, const int& col_end, const int& line, const int&) {
	cout << "Syntax error: " << col_start << "-" << col_end << "," << line << endl;
	ui_reset_is_displaying_error();
}

void UiImplConsole::ui_set_error_call_back(const string& l1, const string& l2) {
	cout << l1 << endl;
	cout << l2 << endl;
	ui_reset_is_displaying_error();
}

  // The functions below have little meaning in a console context
void UiImplConsole::erase_input() { }
int UiImplConsole::get_nb_newlines(const int&) { return -1; }
void UiImplConsole::insert_text(const char *) { }
void UiImplConsole::append_line(const char *) { }
void UiImplConsole::set_cursor_at_the_beginning() { }
int UiImplConsole::get_elevator_width() { return 0; }
const string UiImplConsole::get_string() { return ""; }
const string UiImplConsole::get_first_char() { return ""; }
bool UiImplConsole::space_at_the_left_of_the_cursor() { return false; }

void UiImplConsole::quit() { quit_requested = true; }

bool UiImplConsole::want_to_refresh_display() {
	return opt_batch ? quit_requested : true;
}

void UiImplConsole::display_help(const int& dh) { cout << stack_get_help(dh); }

void UiImplConsole::neg() { }

void UiImplConsole::refresh_stack_height() { }


//
// FUNCTIONS
//

void console_loop() {
	try {
		if (debug_itemiser || debug_tokeniser || debug_locale) {
			if (debug_itemiser)
				debug_test_itemiser();
			else if (debug_tokeniser)
				debug_test_tokeniser();
			else if (debug_locale)
				debug_display_locale();
			return;
		}

		ui_impl = new UiImplConsole();

		if (opt_batch)
			ui_dsl = DisplayStackLayout(0, 0);

		string input;
		ui_post_init();
		while(!quit_requested && cin.good()) {
			getline(cin, input);
			if (!cin.good())
				break;
			if (!opt_batch)
				cout << endl;
			ui_flush_input(input.c_str(), "");
			ui_refresh_display();
		}

		quit_requested = true;
		if (opt_batch)
			ui_refresh_display(ui_get_nb_items_in_stack());

	} catch (CalcFatal(c)) {
		std::cerr << c.get_description() << std::endl;
	}
}

