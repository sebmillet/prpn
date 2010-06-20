// MyIntl.h
// A few declarations to have gettext work fine

// RPN calculator

// Sébastien Millet
// June 2010

#ifdef _
#undef _
#endif
#ifdef _N
#undef _N
#endif
#ifdef gettext_noop
#undef gettext_noop
#endif

#include "gettext.h"

#ifndef _
#define _(s)	gettext(s)
#endif
#ifndef _N
#define _N(s)	s
#endif
