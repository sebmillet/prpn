// Ui.h
// Generic UI management (not specific to UI implementation)
// Member functions are all pure virtuals and must be implemented in derived object

// RPN calculator

// Sébastien Millet
// August 2009 - March 2010

#ifndef UI_H
#define UI_H

#include "Common.h"
#include <string>

extern int opt_width;
extern int opt_height;
extern int opt_min_stack_height;
extern bool has_gui;
extern bool opt_dry_run;

void ui_clear_message_flag();
void ui_set_message_flag();
bool ui_get_message_flag();
void ui_disp(int, const std::string&);
void ui_set_error(const std::string&, const std::string&);
bool ui_is_a_program_halted();
void ui_cllcd();
//void ui_copy();
//void ui_paste();

const std::string stack_get_help(const int&);

void ui_string_trim(std::string&, const size_t&, const DisplayStackLayout*, const bool& = false);

extern std::string html_help_file;
extern bool html_help_found;

enum {UIK_ANY = -30000, UIK_RETURN, UIK_NEWLINE, UIK_ESCAPE, UIK_UP, UIK_DOWN};


//
// Buttons area
//

struct BtnDescription {
	int proportion;
	  // Main button
	const char *main_display;
	int main_font_proportion;
	const char *main_cmd;
	  // ALTernate -> command displayed above the button
	const char *alt_display;
	int alt_font_proportion;
	const char *alt_cmd;
};

typedef enum {BEVAL_NULL,
	BEVAL_DEFAULT,	// Default edition status
	BEVAL_NEVER,	// While editing a sting
	BEVAL_ALWAYS,	// Need always eval
	BEVAL_NOT_IN_EXPRESSION,	// While editing an expression
	BEVAL_BTN_EEX,		// UI-specific: EEX button
	BEVAL_BTN_SHIFT,	// UI-specific: shift button
	BEVAL_BTN_ON,		// UI-specific: ON button
	BEVAL_BTN_OFF,		// UI-specific: OFF button
	BEVAL_BTN_VUP,		// UI-specific: VIEW UP button
	BEVAL_BTN_VDOWN,	// UI-specific: VIEW DOWN button
	BEVAL_BTN_EDIT,		// UI-specific: EDIT button
	BEVAL_BTN_COPY,		// UI-specific: COPY button
	BEVAL_BTN_PASTE,	// UI-specific: PASTE button
	BEVAL_BTN_NEG,		// UI-specific: +/- button
	BEVAL_BTN_HELP		// UI-specific: HELP button
} beval_t;


//
// DISPLAYSTACKLAYOUT
//

class DisplayStackLayout {
	int width;
	int height;
	int min_stack;
	int max_stack;
	int shift;
	int bmenu;
	const char *to_be_continued;
	int to_be_continued_length;
	bool status_shift;
public:
	DisplayStackLayout(const int&, const int&);
	DisplayStackLayout(const DisplayStackLayout&);
	void set_min_stack_height(const int&);
	int get_width() const;
	int get_height() const;
	int get_max_stack() const;
	int get_min_stack() const;
	int get_prompt_height() const;
	bool redim(const int&);
	void set_to_be_continued(const char *, const int&);
	const char *get_to_be_continued() const;
	int get_to_be_continued_length() const;
	void set_status_shift(const bool&);
	bool get_status_shift() const;
	int get_base() const;
};

struct IntervalShift {
	int min;
	int max;
	int actual;
};

class UiImpl;
extern UiImpl *ui_impl;
extern DisplayStackLayout ui_dsl;

extern void console_loop();

void ui_init();
void ui_post_init();
void ui_terminate();

void ui_display_help(const int&);

int ui_get_nb_items_in_stack();
void ui_refresh_display(const int& = -1);
void ui_flush_input(const std::string&, const std::string&);
const std::string ui_get_ts_display_line(const DisplayStackLayout&, const int&, int&, int&, int&);

bool ui_notify_key_pressed(const int&);
void ui_reset_is_displaying_error();

void ui_notify_button_pressed(const char *);
void ui_get_buttons_layout_description(const BtnDescription*&, int&);

void ui_decode_color_code(const char*, const char*&, int&, int&, int&);
char ui_get_char_decimal_sep();


//
// UIIMPL
//

  // SLCC stands for "Set Line Color Code"
typedef enum {SLCC_NORMAL, SLCC_EDITED_ITEM, SLCC_NEXT_INSTRUCTION, SLCC_ERROR, SLCC_NB_CODES} slcc_t;

class UiImpl {
	UiImpl(const UiImpl&);
public:
	UiImpl();
	virtual ~UiImpl();

	virtual void ui_set_error_call_back(const std::string&, const std::string&);

	virtual void refresh_statuswin() = 0;
	virtual void set_line(const int&, const slcc_t&, const std::string&) = 0;
	virtual void enforce_refresh() = 0;
	virtual void refresh_display_path(const std::string&, const bool&) = 0;
	virtual void set_syntax_error(const int&, const int&, const int&, const int&) = 0;
	virtual const std::string get_string() = 0;
	virtual void erase_input() = 0;
	virtual const std::string get_first_char() = 0;
	virtual bool space_at_the_left_of_the_cursor() = 0;
	virtual int get_nb_newlines(const int&) = 0;
	virtual void insert_text(const char *) = 0;
	virtual void append_line(const char *) = 0;
	virtual void set_cursor_at_the_beginning() = 0;
	virtual int get_elevator_width() = 0;
	virtual void neg() = 0;
	virtual void refresh_stack_height() = 0;
	virtual void quit() = 0;
	virtual bool want_to_refresh_display() = 0;
	virtual void display_help(const int&) = 0;
	virtual const char *get_next_instruction_prefix() = 0;
	virtual void copy_text(const char *) = 0;
	virtual const char *paste_text() = 0;
};

#endif	// UI_H

