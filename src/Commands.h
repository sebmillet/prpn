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

BuiltinCommandDescriptor builtinCommands[] = {
	  // Base
	{0, BC_RAW, CMD_PREFIX_NOSTD "HELP", &bc_help, 0, 0, 0, 0, _N("Display a help screen")},
	{0, BC_RAW, CMD_PREFIX_NOSTD "HELP_FLAGS", &bc_help_flags, 0, 0, 0, 0, _N("Display a help screen about flags")},
	{0, BC_RAW, CMD_PREFIX_NOSTD "EXIT", &bc_exit, 0, 0, 0, 0, _N("Quit program")},
	{0, BC_RAW, CMD_PREFIX_NOSTD "ABOUT", &bc_about, 0, 0, 0, 0, _N("Display information about this program")},
	  // Arithmetic
	{2, BC_FUNCTION_WRAPPER, "+", 0, 0, 0, &bc_add, 0, _N("Addition: x + y")},
	{2, BC_FUNCTION_WRAPPER, "-", 0, 0, 0, &bc_sub, 0, _N("Subtraction: x - y")},
	{2, BC_FUNCTION_WRAPPER, "*", 0, 0, 0, &bc_mul, 0, _N("Multiplication: x * y")},
	{2, BC_FUNCTION_WRAPPER, "/", 0, 0, 0, &bc_div, 0, _N("Division: x / y")},
	{2, BC_FUNCTION_WRAPPER, "^", 0, 0, 0, &bc_pow, 0, _N("Power: x ^ y")},
	{2, BC_FUNCTION_WRAPPER, "%", 0, 0, 0, &bc_percent, 0, _N("Percentage: (x * y) / 100")},
	{2, BC_FUNCTION_WRAPPER, "%CH", 0, 0, 0, &bc_percent_ch, 0, _N("Percentage change: 100 * (y - x) / x")},
	{2, BC_FUNCTION_WRAPPER, "%T", 0, 0, 0, &bc_percent_t, 0, _N("Percentage of total: 100 * y / x")},
	{2, BC_FUNCTION_WRAPPER, "MOD", 0, 0, 0, &bc_mod, 0, _N("Modulus: x - y * FLOOR(x / y)")},
	{2, BC_FUNCTION_WRAPPER, "MIN", 0, 0, 0, &bc_min, 0, _N("Minimum of two reals")},
	{2, BC_FUNCTION_WRAPPER, "MAX", 0, 0, 0, &bc_max, 0, _N("Maximum of two reals")},
	  // Real functions (extended to complex when the function is analytic)
	{1, BC_FUNCTION_WRAPPER, "COS", 0, 0, &bc_cos, 0, 0, _N("Cosine")},
	{1, BC_FUNCTION_WRAPPER, "SIN", 0, 0, &bc_sin, 0, 0, _N("Sine")},
	{1, BC_FUNCTION_WRAPPER, "TAN", 0, 0, &bc_tan, 0, 0, _N("Tangent")},
	{1, BC_FUNCTION_WRAPPER, "ACOS", 0, 0, &bc_acos, 0, 0, _N("Arccos")},
	{1, BC_FUNCTION_WRAPPER, "ASIN", 0, 0, &bc_asin, 0, 0, _N("Arcsin")},
	{1, BC_FUNCTION_WRAPPER, "ATAN", 0, 0, &bc_atan, 0, 0, _N("Arctan")},
	{1, BC_FUNCTION_WRAPPER, "LN", 0, 0, &bc_ln, 0, 0, _N("Logarihtm (e base)")},
	{1, BC_FUNCTION_WRAPPER, "EXP", 0, 0, &bc_exp, 0, 0, _N("Exponential")},
	{1, BC_FUNCTION_WRAPPER, "COSH", 0, 0, &bc_cosh, 0, 0, _N("Hyperbolic cosine")},
	{1, BC_FUNCTION_WRAPPER, "SINH", 0, 0, &bc_sinh, 0, 0, _N("Hyperbolic sine")},
	{1, BC_FUNCTION_WRAPPER, "TANH", 0, 0, &bc_tanh, 0, 0, _N("Hyperbolic tangent")},
	{1, BC_FUNCTION_WRAPPER, "ACOSH", 0, 0, &bc_acosh, 0, 0, _N("Arc-hyperbolic cosine")},
	{1, BC_FUNCTION_WRAPPER, "ASINH", 0, 0, &bc_asinh, 0, 0, _N("Arc-hyperbolic sine")},
	{1, BC_FUNCTION_WRAPPER, "ATANH", 0, 0, &bc_atanh, 0, 0, _N("Arc-hyperbolic tangent")},
	{1, BC_FUNCTION_WRAPPER, "NEG", 0, 0, &bc_neg, 0, 0, _N("Opposite")},
	{1, BC_FUNCTION_WRAPPER, "IP", 0, 0, &bc_ip, 0, 0, _N("Integer part")},
	{1, BC_FUNCTION_WRAPPER, "FP", 0, 0, &bc_fp, 0, 0, _N("Fractional part")},
	{1, BC_FUNCTION_WRAPPER, "FLOOR", 0, 0, &bc_floor, 0, 0, _N("Greatest integer lower than or equal to x")},
	{1, BC_FUNCTION_WRAPPER, "CEIL", 0, 0, &bc_ceil, 0, 0, _N("Lowest integer greater than or equal to x")},
	{1, BC_FUNCTION_WRAPPER, "ABS", 0, 0, &bc_abs, 0, 0, _N("Absolute of x")},
	{1, BC_FUNCTION_WRAPPER, "SIGN", 0, 0, &bc_sign, 0, 0, _N("Sign of x: can be -1, 0 or 1")},
	{1, BC_FUNCTION_WRAPPER, "MANT", 0, 0, &bc_mant, 0, 0, _N("Mantisse of x")},
	{1, BC_FUNCTION_WRAPPER, "XPON", 0, 0, &bc_xpon, 0, 0, _N("Exponent of x")},
	{1, BC_FUNCTION_WRAPPER, "INV", 0, 0, &bc_inv, 0, 0, _N("1/x")},
	{1, BC_FUNCTION_WRAPPER, "SQ", 0, 0, &bc_sq, 0, 0, _N("Square")},
	{1, BC_FUNCTION_WRAPPER, "SQR", 0, 0, &bc_sqr, 0, 0, _N("Square root")},
	{0, BC_FUNCTION_WRAPPER, "MINR", 0, &bc_minr, 0, 0, 0, _N("Smallest real greater than zero the calcultor can represent")},
	{0, BC_FUNCTION_WRAPPER, "MAXR", 0, &bc_maxr, 0, 0, 0, _N("Biggest real the calcultor can represent")},
	{1, BC_FUNCTION_WRAPPER, "->HMS", 0, 0, &bc_to_hms, 0, 0, _N("Convert a decimal to HMS (h.MMSSss)")},
	{1, BC_FUNCTION_WRAPPER, "HMS->", 0, 0, &bc_hms_to, 0, 0, _N("Convert a HMS (h.MMSSss) to a decimal")},
	{2, BC_FUNCTION_WRAPPER, "HMS+", 0, 0, 0, &bc_hms_add, 0, _N("Add two HMS (h.MMSSss)")},
	{2, BC_FUNCTION_WRAPPER, "HMS-", 0, 0, 0, &bc_hms_sub, 0, _N("Subtract two HMS (h.MMSSss)")},
	{1, BC_FUNCTION_WRAPPER, "D->R", 0, 0, &bc_d_to_r, 0, 0, _N("Convert from degrees to radians")},
	{1, BC_FUNCTION_WRAPPER, "R->D", 0, 0, &bc_r_to_d, 0, 0, _N("Convert from radians to degrees")},
	  // Complex-specific functions
	{2, BC_FUNCTION_WRAPPER, "R->C", 0, 0, 0, &bc_r_to_c, 0, _N("Real to complex")},
	{1, BC_RAW, "C->R", &bc_c_to_r, 0, 0, 0, 0, _N("Complex to real")},
	{1, BC_FUNCTION_WRAPPER, "RE", 0, 0, &bc_re, 0, 0, _N("Get real part of complex number")},
	{1, BC_FUNCTION_WRAPPER, "IM", 0, 0, &bc_im, 0, 0, _N("Get imaginary part of complex number")},
	{1, BC_FUNCTION_WRAPPER, "CONJ", 0, 0, &bc_conj, 0, 0, _N("Get conjugate of complex number")},
	{1, BC_FUNCTION_WRAPPER, "ARG", 0, 0, &bc_arg, 0, 0, _N("Get argument of a complex number")},
	{1, BC_FUNCTION_WRAPPER, "R->P", 0, 0, &bc_r_to_p, 0, 0, _N("Rectangular to polar coordinates")},
	{1, BC_FUNCTION_WRAPPER, "P->R", 0, 0, &bc_p_to_r, 0, 0, _N("Polar to rectangular coordinates")},
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
	{3, BC_FUNCTION_WRAPPER, "SUB", 0, 0, 0, 0, &bc_cmdsub, _N("Extract content from a string or a list")},
	  // Lists and arrays
	{1, BC_RAW, "->LIST", &bc_to_list, 0, 0, 0, 0, _N("Items to list")},
	{1, BC_RAW, "LIST->", &bc_list_to, 0, 0, 0, 0, _N("Extract items from list")},
	{2, BC_RAW, "GET", &bc_get, 0, 0, 0, 0, _N("Get an item from a list or array")},
	{2, BC_RAW, "GETI", &bc_geti, 0, 0, 0, 0, _N("Get an item from a list or array by increments")},
	{3, BC_RAW, "PUT", &bc_put, 0, 0, 0, 0, _N("Put an item into a list or array")},
	{3, BC_RAW, "PUTI", &bc_puti, 0, 0, 0, 0, _N("Put an item into a list or array by increments")},
	{1, BC_FUNCTION_WRAPPER, "SIZE", 0, 0, &bc_size, 0, 0, _N("Dimension of a list or array")},
	{1, BC_RAW, "ARRY->", &bc_arry_to, 0, 0, 0, 0, _N("Replace an array by all its elements")},
	{1, BC_RAW, "->ARRY", &bc_to_arry, 0, 0, 0, 0, _N("Create an array from a list of elements")},
	{2, BC_RAW, "CON", &bc_con, 0, 0, 0, 0, _N("Create a constant array")},
	{1, BC_FUNCTION_WRAPPER, "TRN", 0, 0, &bc_trn, 0, 0, _N("Transpose a matrix-type array")},
	{2, BC_RAW, "RDM", &bc_rdm, 0, 0, 0, 0, _N("Modify the dimension of an array")},
	{1, BC_FUNCTION_WRAPPER, "IDN", 0, 0, &bc_idn, 0, 0, _N("Create an identity matrix")},
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
	{0, BC_RAW, "HALT", &bc_halt, 0, 0, 0, 0, _N("Halt program execution")},
	{0, BC_RAW, "SST", &bc_sst, 0, 0, 0, 0, _N("Execute one step in the program")},
	{0, BC_RAW, "ABORT", &bc_abort, 0, 0, 0, 0, _N("Abort current program execution")},
	{0, BC_RAW, "KILL", &bc_kill, 0, 0, 0, 0, _N("Abort all programs execution")},
	{0, BC_RAW, "CONT", &bc_cont, 0, 0, 0, 0, _N("Resume program execution")},
	  // Misc
	{0, BC_RAW, "STD", &bc_std, 0, 0, 0, 0, _N("Set reals display to 'standard'")},
	{1, BC_COMMAND_WRAPPER, "SCI", 0, 0, &bc_sci, 0, 0, _N("Set reals display to 'scientific'")},
	{1, BC_COMMAND_WRAPPER, "FIX", 0, 0, &bc_fix, 0, 0, _N("Set reals display to 'fixed number of decimals'")},
	{1, BC_COMMAND_WRAPPER, "ENG", 0, 0, &bc_eng, 0, 0, _N("Set reals display to 'engineering'")},
	{0, BC_RAW, "CLLCD", &bc_cllcd, 0, 0, 0, 0, _N("Clear the screen")},
	{0, BC_RAW, "CLMF", &bc_clmf, 0, 0, 0, 0, _N("Clear the message flag => unfreeze stack display")},
	{2, BC_COMMAND_WRAPPER, "DISP", 0, 0, 0, &bc_disp, 0, _N("Display an item on a line of the screen")},
	{1, BC_RAW, CMD_PREFIX_NOSTD "READ", &bc_read, 0, 0, 0, 0, _N("Read file and puts its content in the stack")},
	{2, BC_COMMAND_WRAPPER, CMD_PREFIX_NOSTD "WRITE", 0, 0, 0, &bc_write, 0, _N("Write the second item into the file named by the first item")},
	{1, BC_COMMAND_WRAPPER, CMD_PREFIX_NOSTD "HACK-REAL-MGMT", 0, 0, &bc_hack_mgmt_std, 0, 0, _N("Internal: tune real 'standard' display management")},
	{1, BC_COMMAND_WRAPPER, CMD_PREFIX_NOSTD "HACK-REAL-DOT", 0, 0, &bc_hack_remove_trailing_dot, 0, 0, _N("Internal: set whether or not to keep a trailing dot in reals display")},
	{0, BC_RAW, "UNDO", &bc_undo, 0, 0, 0, 0, _N("Undo last command, 50 levels by default")},
	{1, BC_RAW, CMD_PREFIX_NOSTD "UNDO_LEVELS", &bc_undo_levels, 0, 0, 0, 0, _N("Define number of undo levels")},
	{0, BC_RAW, CMD_PREFIX_NOSTD "UNDO_LEVELS?", &bc_undo_levels_get, 0, 0, 0, 0, _N("Get number of undo levels")}
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
		const char *d;
		for (int i = FL_TAG_IT_BEGIN; i <= FL_TAG_IT_END; i++) {
			d = F->get_description(i);
			if (string(d) == "" && c == "") {
				ostringstream o;
				o.width(2);
				o << i << " - ";
				c = o.str();
			} else if (string(d) != "") {
				ostringstream o;
				s.erase(); E->append_padr(s, integer_to_string(i), 12 - E->get_string_length(c.c_str())); o << s;
				sout << c + o.str();
				s.erase(); E->append_padr(s, _(d), 75); sout << s;
				sout << (F->get_default(i) ? _("Set") : _("Unset"));
				//if (i != FL_TAG_IT_END)
				sout << endl;
				c = "";
			}
		}
	}
	return sout.str();
}

