// Commands.h
// Built-in commands

// RPN calculator

// Sébastien Millet
// August 2009 - May 2010

#ifdef COMMANDS_H
#error "Commands.h can be included only once"
#endif
#define COMMANDS_H

#include "Common.h"
#include <string>
#include <iostream>

  // Commands that do not exist as per standard start with this string
#ifndef PREFIX_NOSTD
#define PREFIX_NOSTD "_"
#endif

BuiltinCommandDescriptor builtinCommands[] = {
	  // Base
	{0, BC_RAW, PREFIX_NOSTD "HELP", &bc_help, 0, 0, 0, 0, _N("Display a help screen")},
	{0, BC_RAW, PREFIX_NOSTD "HELP_FLAGS", &bc_help_flags, 0, 0, 0, 0, _N("Display a help screen about flags")},
	{0, BC_RAW, PREFIX_NOSTD "EXIT", &bc_exit, 0, 0, 0, 0, _N("Quit program")},
	  // Arithmetic
	{2, BC_FUNCTION_WRAPPER, "+", 0, 0, 0, &bc_add, 0, _N("Addition")},
	{2, BC_FUNCTION_WRAPPER, "-", 0, 0, 0, &bc_sub, 0, _N("Subtraction")},
	{2, BC_FUNCTION_WRAPPER, "*", 0, 0, 0, &bc_mul, 0, _N("Multiplication")},
	{2, BC_FUNCTION_WRAPPER, "/", 0, 0, 0, &bc_div, 0, _N("Division")},
	{2, BC_FUNCTION_WRAPPER, "^", 0, 0, 0, &bc_pow, 0, _N("Power")},
	  // Real functions
	{1, BC_FUNCTION_WRAPPER, "COS", 0, 0, &bc_cos, 0, 0, _N("Cosine")},
	{1, BC_FUNCTION_WRAPPER, "SIN", 0, 0, &bc_sin, 0, 0, _N("Sine")},
	{1, BC_FUNCTION_WRAPPER, "TAN", 0, 0, &bc_tan, 0, 0, _N("Tangent")},
	{1, BC_FUNCTION_WRAPPER, "ACOS", 0, 0, &bc_acos, 0, 0, _N("Arccos")},
	{1, BC_FUNCTION_WRAPPER, "ASIN", 0, 0, &bc_asin, 0, 0, _N("Arcsin")},
	{1, BC_FUNCTION_WRAPPER, "ATAN", 0, 0, &bc_atan, 0, 0, _N("Arctan")},
	{1, BC_FUNCTION_WRAPPER, "LN", 0, 0, &bc_ln, 0, 0, _N("Logarihtm (e base)")},
	{1, BC_FUNCTION_WRAPPER, "EXP", 0, 0, &bc_exp, 0, 0, _N("Exponential")},
	{1, BC_FUNCTION_WRAPPER, "NEG", 0, 0, &bc_neg, 0, 0, _N("Opposite")},
	{1, BC_FUNCTION_WRAPPER, "IP", 0, 0, &bc_ip, 0, 0, _N("Integer part")},
	{1, BC_FUNCTION_WRAPPER, "INV", 0, 0, &bc_inv, 0, 0, _N("1/x")},
	{1, BC_FUNCTION_WRAPPER, "SQ", 0, 0, &bc_sq, 0, 0, _N("Square")},
	{1, BC_FUNCTION_WRAPPER, "SQR", 0, 0, &bc_sqr, 0, 0, _N("Square root")},
	{2, BC_FUNCTION_WRAPPER, "R->C", 0, 0, 0, &bc_r_to_c, 0, _N("Real to complex")},
	{1, BC_RAW, "C->R", &bc_c_to_r, 0, 0, 0, 0, _N("Complex to real")},
	  // Comparison functions
	{2, BC_FUNCTION_WRAPPER, "SAME", 0, 0, 0, &bc_same, 0, _N("Is identical")},
	{2, BC_FUNCTION_WRAPPER, "==", 0, 0, 0, &bc_equal, 0, _N("Equal to")},
	{2, BC_FUNCTION_WRAPPER, "<>", 0, 0, 0, &bc_notequal, 0, _N("Different from")},
	{2, BC_FUNCTION_WRAPPER, "<", 0, 0, 0, &bc_lower, 0, _N("Lower than")},
	{2, BC_FUNCTION_WRAPPER, "<=", 0, 0, 0, &bc_lower_or_equal, 0, _N("Lower than or equal to")},
	{2, BC_FUNCTION_WRAPPER, ">", 0, 0, 0, &bc_greater, 0, _N("Greater than")},
	{2, BC_FUNCTION_WRAPPER, ">=", 0, 0, 0, &bc_greater_or_equal, 0, _N("Greater than or equal to")},
	  // Stack manipulation commands
	{1, BC_RAW, "DUP", &bc_dup, 0, 0, 0, 0, _N("Copy the first item")},
	{2, BC_RAW, "SWAP", &bc_swap, 0, 0, 0, 0, _N("Exchange the first two items")},
	{1, BC_RAW, "DROP", &bc_drop, 0, 0, 0, 0, _N("Delete the first item")},
	{0, BC_RAW, "CLEAR", &bc_clear, 0, 0, 0, 0, _N("Clear the stack")},
	{2, BC_RAW, "OVER", &bc_over, 0, 0, 0, 0, _N("Copy the second item")},
	{2, BC_RAW, "DUP2", &bc_dup2, 0, 0, 0, 0, _N("Copy the first two items")},
	{2, BC_RAW, "DROP2", &bc_drop2, 0, 0, 0, 0, _N("Delete the first two items")},
	{3, BC_RAW, "ROT", &bc_rot, 0, 0, 0, 0, _N("Move the third item to the first position")},
	{1, BC_RAW, "ROLLD", &bc_rolld, 0, 0, 0, 0, _N("Move the first item to the Nth position")},
	{1, BC_RAW, "ROLL", &bc_roll, 0, 0, 0, 0, _N("Move the Nth item to the first position")},
	{1, BC_RAW, "PICK", &bc_pick, 0, 0, 0, 0, _N("Copy the Nth item")},
	{1, BC_RAW, "DUPN", &bc_dupn, 0, 0, 0, 0, _N("Copy the n first items")},
	{1, BC_RAW, "DROPN", &bc_dropn, 0, 0, 0, 0, _N("Delete the n first items")},
	{0, BC_RAW, "DEPTH", &bc_depth, 0, 0, 0, 0, _N("Get the number of items")},
	  // Flags
	{1, BC_COMMAND_WRAPPER, "SF", 0, 0, &bc_sf, 0, 0, _N("Set flag")},
	{1, BC_COMMAND_WRAPPER, "CF", 0, 0, &bc_cf, 0, 0, _N("Clear flag")},
	{1, BC_FUNCTION_WRAPPER, "FS?", 0, 0, &bc_fs_get, 0, 0, _N("Return 1 if flag is set, 0 otherwise")},
	{1, BC_FUNCTION_WRAPPER, "FC?", 0, 0, &bc_fc_get, 0, 0, _N("Return 0 if flag is set, 1 otherwise")},
	{1, BC_FUNCTION_WRAPPER, "FS?C", 0, 0, &bc_fs_get_clear, 0, 0, _N("Do FS? then clear flag")},
	{1, BC_FUNCTION_WRAPPER, "FC?C", 0, 0, &bc_fc_get_clear, 0, 0, _N("Do FC? then clear flag")},
	  // String
	{1, BC_FUNCTION_WRAPPER, "->STR", 0, 0, &bc_to_str, 0, 0, _N("Convert to string")},
	{0, BC_RAW, "STR->", &bc_str_to, 0, 0, 0, 0, _N("Convert from string")},
	  // Lists and arrays
	{1, BC_RAW, "->LIST", &bc_to_list, 0, 0, 0, 0, _N("Items to list")},
	{1, BC_RAW, "LIST->", &bc_list_to, 0, 0, 0, 0, _N("Extract items from list")},
	{2, BC_FUNCTION_WRAPPER, "GET", 0, 0, 0, &bc_get, 0, _N("Get an item from a list or array")},
	{2, BC_RAW, "GETI", &bc_geti, 0, 0, 0, 0, _N("Get an item from a list or array by increments")},
	  // Variables
	{0, BC_RAW, "EVAL", &bc_eval, 0, 0, 0, 0, _N("Evaluate item")},
	{2, BC_RAW, "STO", &bc_sto, 0, 0, 0, 0, _N("Store value in variable name")},
	{1, BC_RAW, "RCL", &bc_rcl, 0, 0, 0, 0, _N("Recall value from variable name")},
	{1, BC_RAW, "PURGE", &bc_purge, 0, 0, 0, 0, _N("Remove a variable")},
	{0, BC_RAW, "VARS", &bc_vars, 0, 0, 0, 0, _N("Get the list of variables in current directory")},
	{0, BC_RAW, "PATH", &bc_path, 0, 0, 0, 0, _N("Get the list from root to current directory")},
	{1, BC_RAW, "CRDIR", &bc_crdir, 0, 0, 0, 0, _N("Create a directory")},
	{0, BC_RAW, "HOME", &bc_home, 0, 0, 0, 0, _N("Set root directory as the current one")},
	{0, BC_RAW, "UP", &bc_up, 0, 0, 0, 0, _N("Make the parent directory the current one")},
	  // Binary
	{0, BC_RAW, "BIN", &bc_bin, 0, 0, 0, 0, _N("Use binary mode for binary integers")},
	{0, BC_RAW, "OCT", &bc_oct, 0, 0, 0, 0, _N("Use octal mode for binary integers")},
	{0, BC_RAW, "DEC", &bc_dec, 0, 0, 0, 0, _N("Use decimal mode for binary integers")},
	{0, BC_RAW, "HEX", &bc_hex, 0, 0, 0, 0, _N("Use hexadecimal mode for binary integers")},
	{1, BC_COMMAND_WRAPPER, "STWS", 0, 0, &bc_stws, 0, 0, _N("Define the number of bits in binary integers (1 to 64)")},
	{0, BC_FUNCTION_WRAPPER, "RCWS", 0, &bc_rcws, 0, 0, 0, _N("Get the number of bits in binary integers")},
	{1, BC_FUNCTION_WRAPPER, "B->R", 0, 0, &bc_b_to_r, 0, 0, _N("Convert a binary integer into a real")},
	{1, BC_FUNCTION_WRAPPER, "R->B", 0, 0, &bc_r_to_b, 0, 0, _N("Convert a real into a binary integer")},
	{0, BC_FUNCTION_WRAPPER, "RCLF", 0, &bc_rclf, 0, 0, 0, _N("Save flag states in a binary integer")},
	{1, BC_RAW, "STOF", &bc_stof, 0, 0, 0, 0, _N("Read flag states from a binary integer")},
	  // Programs
	{1, BC_COMMAND_WRAPPER, "WAIT", 0, 0, &bc_wait, 0, 0, _N("Wait a number of seconds")},
	  // Misc
	{1, BC_RAW, PREFIX_NOSTD "READ", &bc_read, 0, 0, 0, 0, _N("Read file and puts its content in the stack")},
	{2, BC_COMMAND_WRAPPER, PREFIX_NOSTD "WRITE", 0, 0, 0, &bc_write, 0, _N("Write the second item into the file named by the first item")},
	{0, BC_RAW, "UNDO", &bc_undo, 0, 0, 0, 0, _N("Undo last command, 50 levels by default")},
	{1, BC_COMMAND_WRAPPER, PREFIX_NOSTD "UNDO_LEVELS", 0, 0, &bc_undo_levels, 0, 0, _N("Define number of undo levels")},
	{0, BC_FUNCTION_WRAPPER, PREFIX_NOSTD "UNDO_LEVELS?", 0, &bc_undo_levels_get, 0, 0, 0, _N("Get number of undo levels")}
};
extern const unsigned int sizeof_builtinCommands = sizeof(builtinCommands) / sizeof(*builtinCommands);

const string stack_get_help(const int& dh) {
	ostringstream sout;
	if (dh == DH_MAIN) {
		sout << _("Command (number of arguments): description") << endl << endl;
		string s;
		for (size_t i = 0; i < sizeof_builtinCommands; i++) {
			sout.width(14);
			s.erase(); E->append_padr(s, builtinCommands[i].command, 14);
			sout << s << " (" << builtinCommands[i].nb_args << "): " <<
				_(builtinCommands[i].short_description) << endl;
		}
		sout << endl;

		s.erase(); E->append_padr(s, _("Object type"), 25); sout << s;
		s.erase(); E->append_padr(s, _("Symbol"), 15); sout << s << _("Example") << endl;
		s.erase(); E->append_padr(s, "------------------------", 25); sout << s;
		s.erase(); E->append_padr(s, "--------------", 15); sout << s << "-------------------------" << endl;
		sout << endl;
		sout << _("Data") << endl;
		s.erase(); E->append_padr(s, _("Real Number"), 25); sout << s;
		s.erase(); E->append_padr(s, _("1.23456e-25"), 15); sout << s << endl;
		s.erase(); E->append_padr(s, _("Complex Number"), 25); sout << s;
		s.erase(); E->append_padr(s, "( )", 15); sout << s << _("(123.45, 678.90)") << endl;
		s.erase(); E->append_padr(s, _("Binary Integer"), 25); sout << s;
		s.erase(); E->append_padr(s, "#", 15); sout << s << _("# 123AB") << endl;
		s.erase(); E->append_padr(s, _("String"), 25); sout << s;
		s.erase(); E->append_padr(s, "\" \"", 15); sout << s << _("\"RESULT\"") << endl;
		s.erase(); E->append_padr(s, _("Vector"), 25); sout << s;
		s.erase(); E->append_padr(s, "[ ]", 15); sout << s << _("[1.23 4.56 7.89]") << endl;
		s.erase(); E->append_padr(s, _("Matrix"), 25); sout << s;
		s.erase(); E->append_padr(s, "[[ ]]", 15); sout << s << _("[[1.23 4.56] [6.54 3.21]]") << endl;
		s.erase(); E->append_padr(s, _("List"), 25); sout << s;
		s.erase(); E->append_padr(s, "{ }", 15); sout << s << _("{1.23 \"ABC\" # 45d}") << endl;
		sout << endl;
		sout << _("Names") << endl;
		s.erase(); E->append_padr(s, _("Name"), 25); sout << s;
		s.erase(); E->append_padr(s, "' '", 15); sout << s << _("'CALC'") << endl;
		sout << endl;
		sout << _("Procedures") << endl;
		s.erase(); E->append_padr(s, _("Program"), 25); sout << s;
		s.erase(); E->append_padr(s, "<< >>", 15); sout << s << _("<< DUP + SWAP >>") << endl;
		s.erase(); E->append_padr(s, _("Algebraic"), 25); sout << s;
		s.erase(); E->append_padr(s, "' '", 15); sout << s << _("'X+2*Y=Z'") << endl;
		sout << endl;
	} else if (dh == DH_FLAGS) {
		string c, s;
		ostringstream o;
		s.erase(); E->append_padr(s, _("Flag #"), 12); sout << s;
		s.erase(); E->append_padr(s, _("Description"), 75); sout << s;
		sout << _("Status") << endl << endl;
		for (int i = FL_TAG_IT_BEGIN; i <= FL_TAG_IT_END; i++) {
			if (string(flags[i].description) == "" && c == "") {
				ostringstream o;
				o.width(2);
				o << i << " - ";
				c = o.str();
			} else if (string(flags[i].description) != "") {
				ostringstream o;
				s.erase(); E->append_padr(s, integer_to_string(i), 12 - E->get_string_length(c.c_str())); o << s;
				sout << c + o.str();
				s.erase(); E->append_padr(s, _(flags[i].description), 75); sout << s;
				sout << (flags[i].default_value ? _("Set") : _("Unset"));
				//if (i != FL_TAG_IT_END)
				sout << endl;
				c = "";
			}
		}
	}
	return sout.str();
}

