/* Support for Lisp-style data.
   Copyright (C) 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* (should have some deallocation support, since some game init data
   can be discarded) */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "game.h"

using namespace Xconq;

#ifdef MAC
extern int convert_mac_charcodes(int ch);
#endif

/* Declarations of local functions. */

static Obj *newobj(void);
static Symentry *lookup_string(const char *str);
static int hash_name(const char *str);

static int strmgetc(Strm *strm);
static void strmungetc(int ch, Strm *strm);
static void sprintf_context(char *buf, int n, int *start, int *end,
			    Strm *strm);
static Obj *read_form_aux(Strm *strm);
static Obj *read_list(Strm *strm);
static int read_delimited_text(Strm *strm, const char *delim, int spacedelimits,
			       int eofdelimits);
static void internal_type_error(const char *funname, Obj *x, const char *errtype);

/* Pointer to "nil", the empty list. */

Obj *lispnil;

/* Pointer to "eof", which is returned if no more forms in a file. */

Obj *lispeof;

/* Pointer to a "closing paren" object used only during list reading. */

Obj *lispclosingparen;

/* Pointer to an "unbound" object that indicates unbound variables. */

Obj *lispunbound;

/* Current number of symbols in the symbol table. */

int numsymbols = 0;

/* Pointer to the base of the symbol table itself. */

static Symentry **symboltablebase = NULL;

/* The number of Lisp objects allocated so far. */

int lispmalloc = 0;

/* This variable is used to track the depth of nested #| |# comments. */

int commentdepth = 0;

int actually_read_lisp = TRUE;

#define BIGBUF 1000

static char *lispstrbuf = NULL;

static int *startlineno;
static int *endlineno;
static char *linenobuf;

static char *escapedthingbuf;

namespace Xconq {

void
dice1_desc(char *buf, DiceRep dice)
{
    Z16u numdice = 0, die = 0;
    Z16 offset = 0;

    dice1(dice, numdice, die, offset);
    if (!numdice)
        sprintf(buf, "%d", dice);
    else if (!offset)
        sprintf(buf, "%dd%d", numdice, die);
    else
        sprintf(buf, "%dd%d%+d", numdice, die, offset);
}

void
dice2_desc(char *buf, DiceRep dice)
{
    Z16u die = 0;
    Z16 numdice = 0, offset = 0;

    dice2(dice, numdice, die, offset);
    if (!numdice)
        sprintf(buf, "%d", dice);
    else if (!offset)
        sprintf(buf, "%dd%d", numdice, die);
    else
        sprintf(buf, "%dd%d%+d", numdice, die, offset);
}

} // namespace Xconq

/* Allocate a new Lisp object, count it as such. */

static Obj *
newobj(void)
{
    lispmalloc += sizeof(Obj);
    return ((Obj *) xmalloc(sizeof(Obj)));
}

/* Pre-create some objects that should always exist. */

void
init_lisp(void)
{
    /* Allocate Lisp's NIL. */
    lispnil = newobj();
    lispnil->type = NIL;
    /* Do this so car/cdr of nil is nil, might cause infinite loops though. */
    lispnil->v.cons.car = lispnil;
    lispnil->v.cons.cdr = lispnil;
    /* We use the eof object to recognize eof when reading a file. */
    lispeof = newobj();
    lispeof->type = EOFOBJ;
    /* The "closing paren" object just flags closing parens while reading. */
    lispclosingparen = newobj();
    /* The "unbound" object is for unbound variables. */
    lispunbound = newobj();
    /* Set up the symbol table. */
    symboltablebase = (Symentry **) xmalloc(256 * sizeof(Symentry *));
    numsymbols = 0;
    init_predefined_symbols();
    escapedthingbuf = (char *)xmalloc(BUFSIZE);
}

/* Ultra-simple "streams" that can be stdio FILEs or strings. */

static int
strmgetc(Strm *strm)
{
    int ch;

    if (strm->type == stringstrm) {
	if (*(strm->ptr.sp) == '\0')
	  ch = EOF;
	else
	  ch = *((strm->ptr.sp)++);
    } else {
	ch = getc(strm->ptr.fp);
#ifdef MAC
	ch = convert_mac_charcodes(ch);
#endif
    }
    if (ch != EOF) {
	++(strm->numread);
	strm->lastread[(strm->numread % (CONTEXTSIZE - 1))] = ch;
	strm->lastread[((strm->numread + 1) % (CONTEXTSIZE - 1))] = '\0';
	/* This is redundant unless we're at the end of the buffer. */
	strm->lastread[(strm->numread % (CONTEXTSIZE - 1)) + 1] = '\0';
    }
    return ch;
}

static void
strmungetc(int ch, Strm *strm)
{
    if (strm->type == stringstrm) {
	--strm->ptr.sp;
    } else {
	ungetc(ch, strm->ptr.fp);
    }
    --(strm->numread);
}

/* El cheapo Lisp reader.  Lisp objects are generally advertised by
   their first characters, but lots of semantics actions happen while
   reading, so this isn't really a regular expression reader. */

Obj *
read_form(FILE *fp, int *p1, int *p2)
{
    Obj *rslt;
    Strm tmpstrm;

    commentdepth = 0;
    startlineno = p1;
    endlineno = p2;
    tmpstrm.type = filestrm;
    tmpstrm.ptr.fp = fp;
    tmpstrm.numread = 0;
    rslt = read_form_aux(&tmpstrm);
    if (rslt == lispclosingparen) {
	if (linenobuf == NULL)
	  linenobuf = (char *)xmalloc(BUFSIZE);
	sprintf_context(linenobuf, BUFSIZE, startlineno, endlineno, &tmpstrm);
	init_warning("extra close paren, substituting nil%s", linenobuf);
	rslt = lispnil;
    }
    return rslt;
}

Obj *
read_form_from_string(const char *str, int *p1, int *p2, const char **endstr)
{
    Obj *rslt;
    Strm tmpstrm;

    commentdepth = 0;
    startlineno = p1;
    endlineno = p2;
    tmpstrm.type = stringstrm;
    tmpstrm.ptr.sp = str;
    tmpstrm.numread = 0;
    rslt = read_form_aux(&tmpstrm);
    if (rslt == lispclosingparen) {
	if (linenobuf == NULL)
	  linenobuf = (char *)xmalloc(BUFSIZE);
	sprintf_context(linenobuf, BUFSIZE, startlineno, endlineno, &tmpstrm);
	init_warning("extra close paren, substituting nil%s", linenobuf);
	rslt = lispnil;
    }
    /* Record the next character to read from the string if possible. */
    if (endstr != NULL)
      *endstr = tmpstrm.ptr.sp;
    return rslt;
}

static void
sprintf_context(char *buf, int n, int *start, int *end, Strm *strm)
{
    int printedlineno = FALSE;

    strcpy(buf, "(");
    if (start != NULL && end != NULL) {
	if (*start == *end) {
	    sprintf(buf + strlen(buf), "at line %d", *start);
	} else {
	    sprintf(buf + strlen(buf), "lines %d to %d", *start, *end);
	}
	printedlineno = TRUE;
    }
    if (!empty_string(strm->lastread)) {
	if (printedlineno)
	  strcat(buf, ", ");
	strcat(buf, "context \"");
	if (strm->numread > (CONTEXTSIZE - 1) && (strm->numread % (CONTEXTSIZE - 1)) > 0) {
	    strncpy(buf + strlen(buf), strm->lastread + (strm->numread % (CONTEXTSIZE - 1)), n - strlen(buf) - 1);
	}
	strncpy(buf + strlen(buf), strm->lastread, n - strlen(buf) - 1);
	buf[n - 1] = '\0';
	strcat(buf, "\"");
    }
    strcat(buf, ")");
}

/* The main body of the the Lisp reader, works from a stream and returns
   an object. */

static Obj *
read_form_aux(Strm *strm)
{
    int minus, factor, commentclosed, ch, ch2, ch3, ch4, num;
    int numdice, dice, indice, badnumdice = FALSE, minus2 = FALSE;

    while ((ch = strmgetc(strm)) != EOF) {
	/* Recognize nested comments specially. */
	if (ch == '#') {
	    if ((ch2 = strmgetc(strm)) == '|') {
		commentclosed = FALSE;
		++commentdepth;
		while ((ch3 = strmgetc(strm)) != EOF) {
		    if (ch3 == '|') {
			/* try to recognize # */
			if ((ch4 = strmgetc(strm)) == '#') {
			    --commentdepth;
			    if (commentdepth == 0) {
				commentclosed = TRUE;
				break;
			    }
			} else {
			    strmungetc(ch4, strm);
			}
		    } else if (ch3 == '#') {
			if ((ch4 = strmgetc(strm)) == '|') {
			    ++commentdepth;
			} else {
			    strmungetc(ch4, strm);
			}
		    } else if (ch3 == '\n') {
			if (endlineno != NULL)
			  ++(*endlineno);
			announce_read_progress();
		    }
		}
		if (!commentclosed) {
		    init_warning("comment not closed at eof");
		}
		/* Always pick up the next char. */
		ch = strmgetc(strm);
	    } else {
		strmungetc(ch2, strm);
	    	return intern_symbol("#");
	    }
	}
	/* Regular lexical recognition. */
	if (isspace(ch)) {
	    /* Nothing to do here except count lines. */
	    if (ch == '\n') {
		if (endlineno != NULL)
		  ++(*endlineno);
		if (startlineno != NULL)
		  ++(*startlineno);
		announce_read_progress();
	    }
	} else if (ch == ';') {
	    /* Discard all from here to the end of this line. */
	    while ((ch = strmgetc(strm)) != EOF && ch != '\n')
	    	;
	    if (endlineno != NULL)
	      ++(*endlineno);
	    announce_read_progress();
	} else if (ch == '(') {
	    /* Jump into a list-reading mode. */
	    return read_list(strm);
	} else if (ch == ')') {
	    /* This is just to flag the end of the list for read_list. */
	    return lispclosingparen;
	} else if (ch == '"') {
	    read_delimited_text(strm, "\"", FALSE, FALSE);
	    if (!actually_read_lisp)
	      return lispnil;
	    return new_string(copy_string(lispstrbuf));
	} else if (ch == '|') {
	    read_delimited_text(strm, "|", FALSE, FALSE);
	    if (!actually_read_lisp)
	      return lispnil;
	    return intern_symbol(lispstrbuf);
	} else if (strchr("`'", ch)) {
	    if (!actually_read_lisp)
	      return lispnil;
	    return cons(intern_symbol("quote"),
			cons(read_form_aux(strm), lispnil));
	} else if (isdigit(ch) || ch == '-' || ch == '+' || ch == '.') {
	    numdice = dice = 0;
	    indice = FALSE;
	    minus = (ch == '-');
	    factor = (ch == '.' ? 100 : 1);
	    num = 0;
	    if (isdigit(ch))
	      num = ch - '0';
	    // Handle '+' and '-' operators.
	    if (('-' == ch) || ('+' == ch)) {
		ch2 = strmgetc(strm);
		if (isspace(ch2) || (ch2 == ')')) {
		    strmungetc(ch2, strm);
		    if ('-' == ch)
		      return intern_symbol("-");
		    else
		      return intern_symbol("+");
		}
		strmungetc(ch2, strm);
	    }
	    // Build the numeric representation.
	    while ((ch = strmgetc(strm)) != EOF) {
	    	if (isdigit(ch)) {
	    	    /* should ignore decimal digits past second one */
		    num = num * 10 + ch - '0';
		    if (factor > 1)
		      factor /= 10;
		} else if (ch == 'd') {
		    numdice = num;
		    num = 0;
		    indice = TRUE;
		} else if (ch == '+' || ch == '-') {
		    if (indice && minus && ('+' == ch))
			badnumdice = TRUE;
		    if ('-' == ch)
			minus2 = TRUE;
		    dice = num;
		    num = 0;
		    indice = FALSE;
		} else if (ch == '.') {
		    factor = 100;
		} else {
		    break;
		}
	    }
	    // Discard '%'. Return any other cahr back to the stream.
	    if (ch != '%' && ch != EOF)
	      strmungetc(ch, strm);
	    // If no offset in dice spec, then finalize dice parts.
	    if (indice) {
		dice = num;
		num = 0;
	    }
	    // If not dice, then negate.
	    if (minus && !numdice)
	      num = 0 - num;
	    // Construct dice representation, if necessary.
	    if (numdice > 0) {
		// Warn about out-of-bounds values.
		if (badnumdice && minus)
		    init_warning(
"Negative number of dice not allowed with positive offset in -%dd%d+%d",
				 numdice, dice, num);
		if (!between(1, numdice, 8))
		    init_warning(
"Number of dice in %dd%d+%d is %d, not between 1 and 8",
			         numdice, dice, num, numdice);
		if (!between(2, dice, 17))
		    init_warning(
"Dice size in %dd%d+%d is %d, not between 2 and 17",
			         numdice, dice, num, dice);
		if ((minus || !minus2) && !between(0, num, 127))
		    init_warning(
"Dice addon in %dd%d%c%d is %d, not between 0 and 127",
			         numdice, dice, num, (minus2 ? '-' : '+'), num);
		if ((!minus && minus2) && !between(1, num, 128))
		    init_warning(
"Dice addon in %dd%d-%d is -%d, not between -1 and -128",
				 numdice, dice, num, num);
		if (!minus && minus2)
		    num = num - 1;
		num = 
		    (short)(
			((minus | minus2) << 15) 
			| (((minus | minus2) ? 0 : 1) << 14) 
			| ((numdice - 1) << 11) | ((dice - 2) << 7) 
			| (num & 0x7f));
	    }
	    // "Integerize" decimal spec, if neccesary.
	    else {
	    	num = factor * num;
	    }
	    if (!actually_read_lisp)
	      return lispnil;
	    return new_number(num);
	} else {
	    /* Read a regular symbol. */
	    /* The char we just looked will be the first char. */
	    strmungetc(ch, strm);
	    /* Now read until any special char seen. */
	    ch = read_delimited_text(strm, "();\"'`#", TRUE, TRUE);
	    /* Undo the extra char we read in order to detect the end
	       of the symbol. */
	    strmungetc(ch, strm);
	    /* Need to recognize nil specially here. */
	    if (strcmp("nil", lispstrbuf) == 0) {
		return lispnil;
	    } else if (!actually_read_lisp) {
		/* Recognize boundaries of non-reading specially. */
	    	if (strcmp("else", lispstrbuf) == 0)
		  return intern_symbol(lispstrbuf);
	        if (strcmp("end-if", lispstrbuf) == 0)
		  return intern_symbol(lispstrbuf);
		return lispnil;    
	    } else {
		return intern_symbol(lispstrbuf);
	    }
	}
    }
    return lispeof;
}

/* Read a sequence of expressions terminated by a closing paren.  This
   works by looping; although recursion is more elegant, if the
   compiler does not turn tail-recursion into loops, long lists can
   blow the stack.  (This has happened with real saved games.) */

static Obj *
read_list(Strm *strm)
{
    Obj *thecar, *thenext, *lis, *endlis;

    thecar = read_form_aux(strm);
    if (thecar == lispclosingparen) {
	return lispnil;
    } else if (thecar == lispeof) {
	goto at_eof;
    } else {
	lis = cons(thecar, lispnil);
	endlis = lis;
	while (TRUE) {
	    thenext = read_form_aux(strm);
	    if (thenext == lispclosingparen) {
		break;
	    } else if (thenext == lispeof) {
		goto at_eof;
	    } else {
		set_cdr(endlis, cons(thenext, lispnil));
		endlis = cdr(endlis);
	    }
	}
	return lis;
    }
  at_eof:
    if (linenobuf == NULL)
      linenobuf = (char *)xmalloc(BUFSIZE);
    sprintf_context(linenobuf, BUFSIZE, startlineno, endlineno, strm);
    init_warning("missing a close paren, returning EOF%s", linenobuf);
    return lispeof;
}

/* Read a quantity of text delimited by a char from the given string,
   possibly also by whitespace or EOF. */

static int
read_delimited_text(Strm *strm, const char *delim, int spacedelimits,
		    int eofdelimits)
{
    int ch, octch, j = 0, warned = FALSE;

    if (lispstrbuf == NULL)
      lispstrbuf = (char *) xmalloc(BIGBUF);
    while ((ch = strmgetc(strm)) != EOF
	   && (!spacedelimits || !isspace(ch))
	   && !strchr(delim, ch)) {
	/* Handle escape char by replacing with next char,
	   or maybe interpret an octal sequence. */
	if (ch == '\\') {
	    ch = strmgetc(strm);
	    /* Octal chars introduced by a leading zero. */
	    if (ch == '0') {
		octch = 0;
		/* Soak up numeric digits (don't complain about 8 or 9,
		   sloppy but traditional). */
		while ((ch = strmgetc(strm)) != EOF && isdigit(ch)) {
		    octch = 8 * octch + ch - '0';
		}
		/* The non-digit char is actually next one in the string. */
		strmungetc(ch, strm);
		ch = octch;
	    }
	}
	if (j >= BIGBUF) {
	    /* Warn about buffer overflow, but only once per string,
	       then still read chars but discard them. */
	    if (!warned) {
		init_warning(
		 "exceeded max sym/str length (%d chars), ignoring rest",
			     BIGBUF);
		warned = TRUE;
	    }
	} else {
	    lispstrbuf[j++] = ch;
	}
    }
    lispstrbuf[j] = '\0';
    return ch;
}

/* The usual list length function. */

int
length(Obj *list)
{
    int rslt = 0;

    while (list != lispnil) {
	list = cdr(list);
	++rslt;
    }
    return rslt;
}


/* Basic allocation routines. */

Obj *
new_string(const char *str)
{
    Obj *N = newobj();

    N->type = STRING;
    N->v.str = str;
    return N;
}

Obj *
new_number(int num)
{
    Obj *N = newobj();

    N->type = NUMBER;
    N->v.num = num;
    return N;
}

Obj *
new_utype(int u)
{
    Obj *N = newobj();

    N->type = UTYPE;
    N->v.num = u;
    return N;
}

Obj *
new_mtype(int m)
{
    Obj *N = newobj();

    N->type = MTYPE;
    N->v.num = m;
    return N;
}

Obj *
new_ttype(int t)
{
    Obj *N = newobj();

    N->type = TTYPE;
    N->v.num = t;
    return N;
}

Obj *
new_atype(int a)
{
    Obj *N = newobj();

    N->type = ATYPE;
    N->v.num = a;
    return N;
}

Obj *
new_pointer(Obj *sym, char *ptr)
{
    Obj *N = newobj();

    N->type = POINTER;
    N->v.ptr.sym = sym;
    N->v.ptr.data = ptr;
    return N;
}

Obj *
cons(Obj *x, Obj *y)
{
    Obj *N = newobj();

    N->type = CONS;  
    N->v.cons.car = x;  N->v.cons.cdr = y;
    /* Xconq's Lisp does not include dot notation for consing, so this
       can only happen if there is an internal error somewhere. */
    if (!listp(y))
      run_error("cdr of cons is not a list");
    return N;
}

void
type_warning(const char *funname, Obj *x, const char *errtype, Obj *subst)
{
    char buf1[BUFSIZE], buf2[BUFSIZE];

    sprintlisp(buf1, x, BUFSIZE);
    sprintlisp(buf2, subst, BUFSIZE);
    run_warning("%s of non-%s `%s' being taken, returning `%s' instead",
                funname, errtype, buf1, buf2);
}

/* This routine reports fatal errors with handling objects. */

static void
internal_type_error(const char *funname, Obj *x, const char *errtype)
{
    char buf1[BUFSIZE];

    sprintlisp(buf1, x, BUFSIZE);
    run_error("%s of non-%s `%s'", funname, errtype, buf1);
}

/* The usual suspects. */

Obj *
car(Obj *x)
{
    if (x->type == CONS || x->type == NIL) {
	return x->v.cons.car;
    } else {
    	internal_type_error("car", x, "list");
	return lispnil;
    }
}

Obj *
cdr(Obj *x)
{
    if (x->type == CONS || x->type == NIL) {
	return x->v.cons.cdr;
    } else {
    	internal_type_error("cdr", x, "list");
	return lispnil;
    }
}

Obj *
cadr(Obj *x)
{
    return car(cdr(x));
}

Obj *
cddr(Obj *x)
{
    return cdr(cdr(x));
}

Obj *
caddr(Obj *x)
{
    return car(cdr(cdr(x)));
}

Obj *
cdddr(Obj *x)
{
    return cdr(cdr(cdr(x)));
}

void
set_cdr(Obj *x, Obj *v)
{
    if (x->type == CONS) {
	x->v.cons.cdr = v;
    } else {
    	internal_type_error("set_cdr", x, "cons");
    }
}

/* Return the string out of both strings and symbols. */

const char *
c_string(Obj *x)
{
    if (x->type == STRING) {
	return x->v.str;
    } else if (x->type == SYMBOL) {
	return x->v.sym.symentry->name;
    } else {
	/* (should be internal_type_error?) */
     	type_warning("c_string", x, "string/symbol", lispnil);
	return "";
   }
}

/* Return the actual number in a number object. */

int
c_number(Obj *x)
{
    if (x->type == NUMBER
	|| x->type == UTYPE
	|| x->type == MTYPE
	|| x->type == TTYPE
	|| x->type == ATYPE) {
	return x->v.num;
    } else {
	/* (should be internal_type_error?) */
     	type_warning("c_number", x, "number", lispnil);
	return 0;
    }
}

Obj *
intern_symbol(const char *str)
{
    int n;
    Symentry *se;
    Obj *new1;

    se = lookup_string(str);
    if (se) {
	return se->symbol;
    } else {
	new1 = newobj();
	new1->type = SYMBOL;
	se = (Symentry *) xmalloc(sizeof(Symentry));
	new1->v.sym.symentry = se;
	/* Declare a newly created symbol to be unbound. */
	new1->v.sym.value = lispunbound;
	se->name = copy_string(str); 
	se->symbol = new1;
	se->constantp = FALSE;
	n = hash_name(str);
	/* Push the symbol entry onto the front of its hash bucket. */
	se->next = symboltablebase[n];
	symboltablebase[n] = se;
	++numsymbols;
	return new1;
    }
}

/* Given a string, try to find a symbol entry with that as its name. */

static Symentry *
lookup_string(const char *str)
{
    Symentry *se;

    for (se = symboltablebase[hash_name(str)]; se != NULL; se = se->next) {
	if (strcmp(se->name, str) == 0)
	  return se;
    }
    return NULL;
}

static int
hash_name(const char *str)
{
    int rslt = 0;

    while (*str != '\0')
      rslt ^= *str++;
    return (ABS(rslt) & 0xff);
}

Obj *
symbol_value(Obj *sym)
{
    Obj *val = sym->v.sym.value;

    if (val == lispunbound) {
	run_warning("unbound symbol `%s', substituting nil", c_string(sym));
	val = lispnil;
    }
    return val;
}

Obj *
setq(Obj *sym, Obj *x)
{
    /* All the callers check for symbolness, but be careful. */
    if (!symbolp(sym)) {
	internal_type_error("setq", sym, "symbol");
    }
    if (constantp(sym)) {
    	run_warning("Can't alter the constant `%s', ignoring attempt",
		    c_string(sym));
    	return x;
    }
    sym->v.sym.value = x;
    return x;
}

void
makunbound(Obj *sym)
{
    sym->v.sym.value = lispunbound;
}

void
flag_as_constant(Obj *sym)
{
    sym->v.sym.symentry->constantp = TRUE;
}

int
constantp(Obj *sym)
{	
    return (sym->v.sym.symentry->constantp);
}

int
numberp(Obj *x)
{
    return (x->type == NUMBER);
}

int
stringp(Obj *x)
{
    return (x->type == STRING);
}

int
symbolp(Obj *x)
{
    return (x->type == SYMBOL);
}

int
consp(Obj *x)
{
    return (x->type == CONS);
}

int
utypep(Obj *x)
{
    return (x->type == UTYPE);
}

int
mtypep(Obj *x)
{
    return (x->type == MTYPE);
}

int
ttypep(Obj *x)
{
    return (x->type == TTYPE);
}

int
atypep(Obj *x)
{
    return (x->type == ATYPE);
}

int
pointerp(Obj *x)
{
    return (x->type == POINTER);
}

int
boundp(Obj *sym)
{
    return (sym->v.sym.value != lispunbound);
}

int
numberishp(Obj *x)
{
    return (x->type == NUMBER
	    || x->type == UTYPE
	    || x->type == MTYPE
	    || x->type == TTYPE
	    || x->type == ATYPE);
}

int
listp(Obj *x)
{
    return (x->type == NIL || x->type == CONS);
}

/* General structural equality test.  Assumes that it is not getting
   passed any circular structures, which should never happen in Xconq. */

int
equal(Obj *x, Obj *y)
{
    /* Objects of different types can never be equal. */
    if (x->type != y->type)
      return FALSE;
	/* Identical objects are always equal. */
    if (x == y)
      return TRUE;
    switch (x->type) {
      case NUMBER:
      case UTYPE:
      case MTYPE:
      case TTYPE:
      case ATYPE:
	return (c_number(x) == c_number(y));
      case STRING:
	return (strcmp(c_string(x), c_string(y)) == 0);
      case SYMBOL:
	return (strcmp(c_string(x), c_string(y)) == 0);
      case CONS:
	return (equal(car(x), car(y)) && equal(cdr(x), cdr(y)));
      case POINTER:
	return FALSE;
      default:
	case_panic("lisp type", x->type);
	return FALSE;
    }
}

int
member(Obj *x, Obj *lis)
{
    if (lis == lispnil) {
	return FALSE;
    } else if (!consp(lis)) {
	/* should probably be an error of some sort */
	return FALSE;
    } else if (equal(x, car(lis))) {
	return TRUE;
    } else {
	return member(x, cdr(lis));
    }
}

/* Return the nth element of a list. */

Obj *
elt(Obj *lis, int n)
{
    while (n-- > 0) {
	lis = cdr(lis);
    }
    return car(lis);
}

Obj *
reverse(Obj *lis)
{
    Obj *rslt = lispnil;

    for (; lis != lispnil; lis = cdr(lis)) {
	rslt = cons(car(lis), rslt);
    }
    return rslt;
}

Obj *
find_at_key(Obj *lis, const char *key)
{
    Obj *rest, *bdgs, *bdg;

    for_all_list(lis, rest) {
	bdgs = car(rest);
	bdg = car(bdgs);
	if (stringp(bdg) && strcmp(key, c_string(bdg)) == 0) {
	    return cdr(bdgs);
	}
    }
    return lispnil;
}

Obj *
replace_at_key(Obj *lis, const char *key, Obj *newval)
{
    Obj *rest, *bdgs, *bdg;

    for_all_list(lis, rest) {
	bdgs = car(rest);
	bdg = car(bdgs);
	if (stringp(bdg) && strcmp(key, c_string(bdg)) == 0) {
	    set_cdr(bdgs, newval);
	    return lis;
	}
    }
    return cons(cons(new_string(key), newval), lis);
}

/* Is the object quoted? */

int
is_quoted_lisp(Obj *x)
{
    enum keywords code;
    Obj *specialform = lispnil;

    assert_error(x, "Attempted to access a NULL GDL object");
    if (lispnil == x)
      return FALSE;
    switch (x->type) {
      case CONS:
	specialform = car(x);
	if (symbolp(specialform)
	    && !boundp(specialform)
	    && (code = 
		(enum keywords)keyword_code(c_string(specialform))) >= 0) {
	    switch (code) {
	      case K_QUOTE:
		return TRUE;
	      default:
		return FALSE;
	    }
	}
	break;
      default:
	return FALSE;
    }
    return FALSE;
}

/* Print an integer or dice spec according to type. */

void
fprint_num_or_dice(FILE *fp, int x, int valtype)
{
    char valbuf [BUFSIZE];

    if (TABDICE1 == valtype)
	dice1_desc(valbuf, (DiceRep)x);
    else if (TABDICE2 == valtype)
	dice2_desc(valbuf, (DiceRep)x);
    else
	snprintf(valbuf, BUFSIZE, "%d", x);
    fprintf(fp, " %s", valbuf);
}

/* Print the contents of a table at the given index in the table definitions 
   array. */
/* (TODO: Should perhaps use something similar to 'write_table' to get 
    more compact output.) */
void
fprinttable(FILE *fp, int n)
{
    int i = -1, j = -1;
    int (*getter)(int i, int j) = NULL;
    int dim1 = -1, dim2 = -1, valtype = -1;

    getter = tabledefns[n].getter;
    dim1 = numtypes_from_index_type(tabledefns[n].index1);
    dim2 = numtypes_from_index_type(tabledefns[n].index2);
    valtype = tabledefns[n].valtype;
    fprintf(fp, "\n(");
    for (i = 0; i < dim1; ++i) {
	fprintf(fp, "\n\t(");
	for (j = 0; j < dim2; ++j) {
	    fprint_num_or_dice(fp, getter(i, j), valtype);
	}
	fprintf(fp, ")");
    }
    fprintf(fp, ")\n");
}

void
fprintlisp(FILE *fp, Obj *oobj)
{
    int needescape;
    const char *str, *tmp;
    Obj *obj = NULL;
    int i = -1;

    /* Doublecheck, just in case caller is not so careful. */
    if (oobj == NULL) {
	run_warning("Trying to print NULL as object, skipping");
	return;
    }
    obj = oobj;
#if (0)
    /* Evaluate symbol down to the object it represents. */
    if (symbolp(obj) && boundp(obj) && !pointerp(obj)) {
	while (symbolp(obj) && boundp(obj))
	  obj = eval_symbol(obj);
    }
#endif
    /* Decide what to print depending on type of object. */
    switch (obj->type) {
      case NIL:
	fprintf(fp, "nil");
	break;
      case NUMBER:
	fprintf(fp, "%d", obj->v.num);
	break;
      case STRING:
	if (strchr(obj->v.str, '"')) {
	    fprintf(fp, "\"");
	    for (tmp = obj->v.str; *tmp != '\0'; ++tmp) {
		if (*tmp == '"')
		  fprintf(fp, "\\");
		fprintf(fp, "%c", *tmp);
	    }
	    fprintf(fp, "\"");
	} else {
	    /* Just printf the whole string. */
	    fprintf(fp, "\"%s\"", obj->v.str);
	}
	break;
      case SYMBOL:
	needescape = FALSE;
	str = c_string(obj);
	if (isdigit(str[0])) {
	    needescape = TRUE;
	} else {
	    /* Scan the symbol's name looking for special chars. */
	    for (tmp = str; *tmp != '\0'; ++tmp) {
		if (strchr(" ()#\";|", *tmp)) {
		    needescape = TRUE;
		    break;
		}
	    }
	}
	if (needescape) {
	    fprintf(fp, "|%s|", str);
	} else {
	    fprintf(fp, "%s", str);
	}
	/* Check to see if we are dealing with a table. */
	for (i = 0; tabledefns[i].name != NULL; ++i) {
	    if (!strcmp(str, tabledefns[i].name)) {
		fprinttable(fp, i);
		break;
	    }
	}
	break;
      case CONS:
	fprintf(fp, "(");
	fprintlisp(fp, car(obj));
	/* Note that there are no dotted pairs in our version of Lisp. */
	fprint_list(fp, cdr(obj));
	break;
      case UTYPE:
	fprintf(fp, "u#%d", obj->v.num);
	break;
      case MTYPE:
	fprintf(fp, "m#%d", obj->v.num);
	break;
      case TTYPE:
	fprintf(fp, "t#%d", obj->v.num);
	break;
      case ATYPE:
	fprintf(fp, "a#%d", obj->v.num);
	break;
      case POINTER:
	fprintlisp(fp, obj->v.ptr.sym);
	fprintf(fp, " #|0x%lx|#", (long) obj->v.ptr.data);
	break;
      default:
	case_panic("lisp type", obj->type);
	break;
    }
}

void
fprint_list(FILE *fp, Obj *obj)
{
    Obj *tmp;

    for_all_list(obj, tmp) {
	fprintf(fp, " ");
	fprintlisp(fp, car(tmp));
    }	
    fprintf(fp, ")");
}

void
sprintlisp(char *buf, Obj *obj, int maxlen)
{
    if (maxlen < 10) {
	strcpy(buf, " ... ");
	return;
    }
    switch (obj->type) {
      case NIL:
	strcpy(buf, "nil");
	break;
      case NUMBER:
	sprintf(buf, "%d", obj->v.num);
	break;
      case STRING:
	if (maxlen < (strlen(obj->v.str) + 2)) {
	    strcpy(buf, " ... ");
	    return;
	}
	/* (should print escape chars if needed) */
	sprintf(buf, "\"%s\"", obj->v.str);
	break;
      case SYMBOL:
	if (maxlen < strlen(c_string(obj))) {
	    strcpy(buf, " ... ");
	    return;
	}
	/* (should print escape chars if needed) */
	sprintf(buf, "%s", c_string(obj));
	break;
      case CONS:
	strcpy(buf, "(");
	sprintlisp(buf + 1, car(obj), maxlen - 1);
	/* No dotted pairs allowed in our version of Lisp. */
	sprint_list(buf+strlen(buf), cdr(obj), maxlen - strlen(buf));
	break;
      case UTYPE:
	sprintf(buf, "u#%d", obj->v.num);
	break;
      case MTYPE:
	sprintf(buf, "m#%d", obj->v.num);
	break;
      case TTYPE:
	sprintf(buf, "t#%d", obj->v.num);
	break;
      case ATYPE:
	sprintf(buf, "a#%d", obj->v.num);
	break;
      case POINTER:
	sprintlisp(buf, obj->v.ptr.sym, maxlen);
	sprintf(buf+strlen(buf), " #|0x%lx|#", (long) obj->v.ptr.data);
	break;
      default:
	case_panic("lisp type", obj->type);
	break;
    }
}

void
sprint_list(char *buf, Obj *obj, int maxlen)
{
    Obj *tmp;

    buf[0] = '\0';
    for (tmp = obj; tmp != lispnil; tmp = cdr(tmp)) {
	if ((maxlen - strlen(buf)) < 10) {
	    strcpy(buf, " ... ");
	    break;
	}
	strcat(buf, " ");
	sprintlisp(buf+strlen(buf), car(tmp), maxlen - strlen(buf));
    }	
    strcat(buf, ")");
}

/* These two routines make sure that any symbols and strings can
   be read in again. */

const char *
escaped_symbol(const char *str)
{
    const char *tmp = str;

    if (str[0] == '|' && str[strlen(str)-1] == '|')
      return str;
    if (isdigit(str[0])) {
	sprintf(escapedthingbuf, "|%s|", str);
	return escapedthingbuf;
    }
    while (*tmp != '\0') {
	if (((char *) strchr(" ()#\";|", *tmp)) != NULL) {
	    sprintf(escapedthingbuf, "|%s|", str);
	    return escapedthingbuf;
	}
	++tmp;
    }
    return str;
}

/* Note that this works correctly on NULL strings, turning them into
   strings of length 0. */

char *
escaped_string(const char *str)
{
    const char *tmp = str;
    char *rslt = escapedthingbuf;

    *rslt++ = '"';
    if (str != NULL) {
	while (*tmp != 0) {
	    if ((*tmp == '"') || (*tmp == '\\'))
	      *rslt++ = '\\';
	    *rslt++ = *tmp++;
	}
    }
    *rslt++ = '"';
    *rslt = '\0';
    return escapedthingbuf;
}

/* The escaped_string() function makes a dangerous assumption about 
    the length of an escaped (or even unescaped) string being less than 
    the BUFSIZE, which is the size of escapedthingbuf. 
    safe_escaped_string() is slightly less efficient, but avoids this 
    assumption. Do note, however, that safe_escaped_string() can result 
    in a memory leak, if the caller does not free the result after 
    using it.
*/

char *
safe_escaped_string(const char *str, int len)
{
    const char *tmp = str;
    char *rsltbase = NULL;
    char *rslt = NULL;
    int amt = len*2 + 3;

    rsltbase = (char *)malloc(amt);
    if (NULL == rsltbase){
        Dprintf("Unable to allocate %d bytes.\n", amt);
        run_error("Memory exhausted!!");
        exit(1);    /* Just to make sure. */
    }
    rslt = rsltbase;
    *rslt++ = '"';
    if (str != NULL) {
        while (*tmp != 0) {
            if ((*tmp == '"') || (*tmp == '\\'))
              *rslt++ = '\\';
            *rslt++ = *tmp++;
        }
    }
    *rslt++ = '"';
    *rslt = '\0';
    return rsltbase;
}


#ifdef DEBUGGING
/* For calling from debuggers, at least that those that support output
   to stderr. */

void
dlisp(Obj *x)
{
    fprintlisp(stderr, x);
    fprintf(stderr, "\n");
}
#endif /* DEBUGGING */

void
print_form_and_value(FILE *fp, Obj *form)
{
    fprintlisp(fp, form);
    if (symbolp(form)) {
	if (boundp(form)) {
	    fprintf(fp, " -> ");
	    fprintlisp(fp, symbol_value(form));
	} else {
	    fprintf(fp, " <unbound>");
	}
    }
    fprintf(fp, "\n");
}

Obj *
append_two_lists(Obj *x1, Obj *x2)
{
    if (!listp(x1))
      x1 = cons(x1, lispnil);
    if (!listp(x2))
      x2 = cons(x2, lispnil);
    if (x2 == lispnil) {
	return x1;
    } else if (x1 == lispnil) {
	return x2;
    } else {
    	return cons(car(x1), append_two_lists(cdr(x1), x2));
    }
}

Obj *
append_lists(Obj *lis)
{
    if (lis == lispnil) {
	return lispnil;
    } else if (!consp(lis)) {
    	return cons(lis, lispnil);
    } else {
    	return append_two_lists(car(lis), append_lists(cdr(lis)));
    }
}

/* Remove all occurrences of a single object from a given list. */

Obj *
remove_from_list(Obj *element, Obj *lis)
{
    Obj *tmp;

    if (lis == lispnil) {
	return lispnil;
    } else {
	tmp = remove_from_list(element, cdr(lis));
	if (equal(element, car(lis))) {
	    return tmp;
	} else {
	    return cons(car(lis), tmp);
	}
    }
}

/* Remove all occurrences of each object in a list of objects from a 
   given list. */

Obj *
remove_list_from_list(Obj *rlist, Obj *slist)
{
    Obj *tmp = lispnil;

    if (lispnil == rlist)
      return slist;
    if (lispnil == slist)
      return lispnil;
    if (!consp(rlist)) {
	run_warning("Attempted to 'remove-list' using a non-list.");
	return lispnil;
    }
    tmp = remove_list_from_list(cdr(rlist), slist);
    return remove_from_list(car(rlist), tmp);
}

void
push_binding(Obj **lis, Obj *key, Obj *val)
{
    *lis = cons(cons(key, cons(val, lispnil)), *lis);
}

void
push_cdr_binding(Obj **lis, Obj *key, Obj *val)
{
    *lis = cons(cons(key, val), *lis);
}

void
push_int_binding(Obj **lis, Obj *key, int val)
{
    *lis = cons(cons(key, cons(new_number(val), lispnil)), *lis);
}

void
push_key_binding(Obj **lis, int key, Obj *val)
{
    *lis = cons(cons(intern_symbol(keyword_name((enum keywords)key)), cons(val, lispnil)), *lis);
}

void
push_key_cdr_binding(Obj **lis, int key, Obj *val)
{
    *lis = cons(cons(intern_symbol(keyword_name((enum keywords)key)), val), *lis);
}

void
push_key_int_binding(Obj **lis, int key, int val)
{
    *lis = cons(cons(intern_symbol(keyword_name((enum keywords)key)), 
				   cons(new_number(val), lispnil)), *lis);
}

/* Our version of evaluation derefs symbols and evals through lists,
   unless the list car is a "special form". */

Obj *
eval(Obj *x)
{
    int code;
    Obj *specialform = lispnil;

    switch (x->type) {
      case SYMBOL:
	return eval_symbol(x);
      case CONS:
	specialform = car(x);
	if (symbolp(specialform)
	    && !boundp(specialform)
	    && (code = keyword_code(c_string(specialform))) >= 0) {
	    switch (code) {
	      case K_QUOTE:
		return cadr(x);
	      case K_LIST:
		return eval_list(cdr(x));
	      case K_APPEND:
		return append_lists(eval_list(cdr(x)));
	      case K_REMOVE:
	      	return remove_from_list(eval(cadr(x)), eval(caddr(x)));
	      case K_REMOVE_LIST:
		return remove_list_from_list(eval(cadr(x)), eval(caddr(x)));
	      case K_NOT: case K_AND: case K_OR:
		return eval_boolean_expression(x);
	      case K_PLUS: case K_MINUS: case K_MULTIPLY: case K_DIVIDE:
		return eval_arithmetic_expression(x);
	      case K_EQ: case K_NE: case K_LT: case K_LE: case K_GT: case K_GE:
		return eval_arithmetic_comparison_expression(x);
	      default:
		break;
	    }
	}
	/* A dubious default, but convenient. */
	return eval_list(x);
      default:
        /* Everything else evaluates to itself. */
	return x;
    }
}

/* Some symbols are lazily bound, meaning that they don't get a value
   until it is first asked for. */
	    
Obj *
eval_symbol(Obj *sym)
{
    if (boundp(sym)) {
	return symbol_value(sym);
    } else if (lazy_bind(sym)) {
    	return symbol_value(sym);
    } else {
	run_warning("`%s' is unbound, returning self", c_string(sym));
	/* kind of a hack */
	return sym;
    }
}

/* List evaluation just blasts straight through the list. */

Obj *
eval_list(Obj *lis)
{
    if (lis == lispnil) {
	return lispnil;
    } else {
	return cons(eval(car(lis)), eval_list(cdr(lis)));
    }
}

/* Evaluate a boolean expression as Common Lisp would. */

Obj *
eval_boolean_expression(Obj *expr)
{
    int code = -1;
    Obj *op = lispnil, *terms = lispnil, *tmpeval = lispnil;

    /* If empty list return empty list. */
    if (lispnil == expr)
      return lispnil;
    /*! \note Do we need to handle just the symbols by themselves to get 
		identity? */
    /* If a list, then try to do a boolean eval of it. */
    if (consp(expr)) {
	op = car(expr);
	if (symbolp(op) && !boundp(op)
	    && (code = keyword_code(c_string(op))) >= 0) {
	    switch (code) {
	      case K_NOT: /* Logical NOT. */
		if (lispnil == cdr(expr)) 
		  return new_number(TRUE);
		else
		  return lispnil;
		break;
	      case K_AND: /* Logical AND with short-circuit. */
		for_all_list(cdr(expr), terms) {
		    if (lispnil == eval(car(terms)))
		      return lispnil;
		    if (lispnil == cdr(terms))
		      return car(terms);
		}
		return lispnil;
	      case K_OR: /* Logical inclusive-OR with short-circuit. */
		for_all_list(cdr(expr), terms) {
		    tmpeval = eval(car(terms));
		    if (lispnil != tmpeval)
		      return tmpeval;
		}
		return lispnil;
	      default:
		run_warning("Illegal boolean keyword %s encountered",
			    c_string(op));
		return lispnil;
	    }
	}
    }
    /* We don't know how to deal with anything else. */
    else {
	run_warning(
"Boolean operator expected but none found in an expression");
	return expr;
    }
    return lispnil;
}

/* Evaluate a boolean expression with a single-parameter evaluator. */

int
eval_boolean_expression(Obj *expr, int (*fn)(Obj *), int dflt)
{
    const char *opname;

    if (expr == lispnil) {
	return dflt;
    } else if (consp(expr) && symbolp(car(expr))) {
	opname = c_string(car(expr));
	switch (keyword_code(opname)) {
	  case K_AND:
	    return (eval_boolean_expression(cadr(expr), fn, dflt)
		    && eval_boolean_expression(car(cddr(expr)), fn, dflt));
	  case K_OR:
	    return (eval_boolean_expression(cadr(expr), fn, dflt)
		    || eval_boolean_expression(car(cddr(expr)), fn, dflt));
	  case K_NOT:
	    return !eval_boolean_expression(cadr(expr), fn, dflt);
	  default:
	    return (*fn)(expr);
	}
    } else {
	return (*fn)(expr);
    }
}

/* Evaluate a boolean expression with using a parambox evaluator. */

int
eval_boolean_expression(Obj *expr, int (*fn)(Obj *, ParamBox *), int dflt, 
			ParamBox *pbox)
{
    const char *opname;

    if (expr == lispnil) {
	return dflt;
    } else if (consp(expr) && symbolp(car(expr))) {
	opname = c_string(car(expr));
	switch (keyword_code(opname)) {
	  case K_AND:
	    return (eval_boolean_expression(cadr(expr), fn, dflt, pbox)
		    && eval_boolean_expression(car(cddr(expr)), fn, dflt, 
					       pbox));
	  case K_OR:
	    return (eval_boolean_expression(cadr(expr), fn, dflt, pbox)
		    || eval_boolean_expression(car(cddr(expr)), fn, dflt, 
					       pbox));
	  case K_NOT:
	    return !eval_boolean_expression(cadr(expr), fn, dflt, pbox);
	  default:
	    return (*fn)(expr, pbox);
	}
    } else {
	return (*fn)(expr, pbox);
    }
}

int
eval_number(Obj *val, int *isnumber)
{
    /* (should have a non-complaining eval for this) */
    if (numberp(val)) {
	*isnumber = TRUE;
	return c_number(val);
    } else if (symbolp(val)
	       && boundp(val)
	       && numberp(symbol_value(val))) {
	*isnumber = TRUE;
	return c_number(symbol_value(val));
    } else {
	*isnumber = FALSE;
	return 0;
    }
}

/* Evaluate an arithmetic expression. (Operator precedence is not 
   considered because it is naturally enforced by the structure of the 
   lists.) */

#define CONSIFY_IF_NUMBER(lispeval, lisptmp) \
    (numberp((lisptmp) = (lispeval)) ? cons(lisptmp, lispnil) : lisptmp)

Obj *
eval_arithmetic_expression(Obj *expr)
{
    Obj *op = lispnil, *opnds = lispnil, *opnd = lispnil, *opnd2 = lispnil;
    Obj *tmprslt = lispnil, *rslt = lispnil;
    enum keywords opcode;
    int opndlength = 0;
    int operated = FALSE;

    if (lispnil == expr) {
	run_warning("Attempted to evaluate a bad arithmetic expression.");
	return lispnil;
    }
    /* Get the operator. */
    op = car(expr);
    if (!symbolp(op)) {
	run_warning("Attempted to evaluate a bad arithmetic expression.");
	return lispnil;
    }
    /* Determine which operator we are dealing with. */
    opcode = (enum keywords) keyword_code(c_string(op));
    /* Error, if an unexpected symbol is encountered. */
    if ((K_PLUS != opcode) && (K_MINUS != opcode) 
	&& (K_MULTIPLY != opcode) && (K_DIVIDE != opcode)) {
	run_warning("Attempted to evaluate a bad arithmetic expression.");
	return lispnil;
    }
    for_all_list(cdr(expr),opnds) {
	opnd2 = car(opnds);
	/* Skip empty operands. */
	if (lispnil == opnd2)
	  continue;
	/* Replace symbols with their values. May expand to lists or numbers. */
	if (symbolp(opnd2))
	  opnd2 = eval_symbol(opnd2);
	/* Evaluate any lists which may contain arithmetic expressions. */
	if (consp(opnd2) && symbolp(car(opnd2)))
          opnd2 = eval(opnd2);
	/* Error, if the second operand is neither a number nor a list. */
	if (!numberp(opnd2) && !consp(opnd2)) {
	    run_warning(
"Attempted to evaluate illegal operand in artithmetic expression.");
	    return lispnil;
	}
	/* Set the operand length, if not already set. */
	if (!opndlength && consp(opnd2))
	  opndlength = length(opnd2);
	/* Error, if there is a list length mismatch. */
	if (opndlength && consp(opnd2) && (opndlength != length(opnd2))) {
	    run_warning(
"Attempted to evaluate arithmetic expression with unmatched list lengths.");
	    return lispnil;
	}
	/* Shift the operands, if the first one is an empty list. */
	if (lispnil == opnd) {
	    opnd = opnd2;
	    continue;
	}
	/* Do the actual work. */
	operated = TRUE;
	if ((lispnil == rslt) && numberp(opnd))
	  rslt = new_number(c_number(opnd));
	/* Operating a number with... */
	if (numberp(rslt)) {
	    /* ...a number. */
	    if (numberp(opnd2)) {
		switch (opcode) {
		  case K_PLUS:
		    rslt->v.num += opnd2->v.num;
		    break;
		  case K_MINUS:
		    rslt->v.num -= opnd2->v.num;
		    break;
		  case K_MULTIPLY:
		    rslt->v.num *= opnd2->v.num;
		    break;
		  case K_DIVIDE:
		    if (opnd2->v.num)
			rslt->v.num /= opnd2->v.num;
		    /* Divide-by-zero error. */
		    else {
			run_warning(
"Attempted to evaluate arithmetic expression with division by zero.");
			return lispnil;
		    }
		    break;
		  default:
		    run_warning(
"Attempted to evaluate a bad arithmetic expression.");
		    return lispnil;
		}
	    }
	    /* ...a list. */
	    else {
		/* Hold the outermost result. */
		tmprslt = eval(cons(op, cons(rslt, cons(car(opnd2), lispnil))));
		/* Wittle down the list recursively in an order preserving 
		   manner. */
		if (lispnil != cdr(opnd2))
		  rslt = cons(tmprslt, 
			      cons(eval(cons(op, cons(rslt, cdr(opnd2)))), 
				   lispnil));
		/* Exit condition. */
		else
		  rslt = cons(tmprslt, lispnil);
	    }
	}
	/* Operating a list with... */
	else {
	    /* Do this so we don't have to do a deep copy of 'opnd' first 
	       time around. The deep copy will happen naturally, and we 
	       will end up with an initialized 'rslt'. Subsequently, 
	       'opnd' will be irrelevant, and we can thus clobber it with 
	       'rslt'. */
	    if (lispnil != rslt)
	      opnd = rslt;
	    /* ...a number. */
	    if (numberp(opnd2)) {
		/* Hold the outermost result. */
		tmprslt = eval(cons(op, cons(car(opnd), cons(opnd2, lispnil))));
		/* Wittle down the list recursively in an order preserving 
		   manner. */
		if (lispnil != cdr(opnd))
		  rslt = cons(tmprslt, 
			      eval(cons(op, 
					cons(cdr(opnd), 
					     cons(opnd2, lispnil)))));
		/* Exit condition. */
		else
		  rslt = cons(tmprslt, lispnil);
	    }
	    /* ...a list. */
	    else {
		/* Hold the outermost result. */
		tmprslt = eval(cons(op, 
				    cons(car(opnd), 
					 cons(car(opnd2), lispnil))));
		/* Wittle down the list recursively in an order preserving 
		   manner. */
		if (lispnil != cdr(opnd)) /* The same for 'opnd2'. */
		  rslt = cons(tmprslt, 
			      eval(cons(op, 
					cons(cdr(opnd), 
					     cons(cdr(opnd2), lispnil)))));
		else
		  rslt = cons(tmprslt, lispnil);
	    }
	}
    }
    if (!operated) {
	if (opnd == lispnil) {
	    /* Return identities for "+" and "*", per Common Lisp standard. */
	    if (K_PLUS == opcode)
	      rslt = new_number(0);
	    else if (K_MULTIPLY == opcode)
	      rslt = new_number(1);
	    /* Else, give error. */
	    else {
		run_warning(
"Attempted to evaluate an arithmetic expression with too few operands.");
		return lispnil;
	    }
	}
	/* (The behavior of divide differs from Common Lisp in this case; 
	    Clisp will take the reciprocal and return a rational number; 
	    GDL currently does not have rational numbers.) */
	if (opnd != lispnil) {
	    /* If plus, then return the operand unchanged. */
	    if (K_PLUS == opcode)
	      rslt = opnd;
	    /* If minus, then return the negation of the operand. */
	    else if (K_MINUS == opcode)
	      rslt = eval(cons(op, cons(new_number(0), cons(opnd, lispnil))));
	    /* Else, give error. */
	    else {
		run_warning(
"Attempted to evaluate an arithmetic expression with too few operands.");
		return lispnil;
	    }
	}
    }
    return rslt;
}

/* Evaluate an arithmetic expression like Common Lisp. */

Obj *
eval_arithmetic_comparison_expression(Obj *expr)
{
    enum keywords opcode;
    Obj *op = lispnil, *opnds = lispnil, *opnd = lispnil, *opnd2 = lispnil;
    Obj *retobj = lispnil;

    /* If expression is empty, then warn and get out. */
    if (lispnil == expr) {
	run_warning(
"Attempted to evaluate a bad arithmetic comparison expression.");
	return lispnil;
    }
    /* Get the operator. */
    op = car(expr);
    if (!symbolp(op) || boundp(op)) {
	run_warning(
"Attempted to evaluate a bad arithmetic comparison expression.");
	return lispnil;
    }
    /* Determine which operator we are dealing with. */
    opcode = (enum keywords) keyword_code(c_string(op));
    /* Error, if an unexpected symbol is encountered. */
    if ((K_EQ != opcode) && (K_NE != opcode) && (K_LT != opcode)
	&& (K_LE != opcode) && (K_GT != opcode) && (K_GE != opcode)) {
	run_warning(
"Attempted to evaluate a bad arithmetic comparison expression.");
	return lispnil;
    }
    /*! \todo Possibly make this as flexible as the arithmetic expression 
		evaluator. */
    for_all_list(cdr(expr), opnds) {
	opnd2 = car(opnds);
	/* Skip empty operands. */
	if (lispnil == opnd2)
	  continue;
	/* Expand any bound symbols. */
	if (symbolp(opnd2) && boundp(opnd2))
	  opnd2 = eval_symbol(opnd2);
	if (lispnil == opnd2)
	  continue;
	/* If list is encountered, attempt to evaluate it. */
	if (consp(opnd2))
	  opnd2 = eval(opnd2);
	if (lispnil == opnd2)
	  continue;
	/* If we do not have a number by this point, then we have a problem. */
	if (!numberp(opnd2)) {
	    run_warning("Illegal operand encountered in arithmetic expression");
	    return lispnil;
	}
	/* Compare, if we have something to compare against. */
	if (lispnil != opnd) {
	    switch (opcode) {
	      case K_EQ:
		if (c_number(opnd) != c_number(opnd2))
		  return lispnil;
		break;
	      case K_NE:
		if (c_number(opnd) == c_number(opnd2))
		  return lispnil;
		break;
	      case K_LT:
		if (c_number(opnd) >= c_number(opnd2))
		  return lispnil;
		break;
	      case K_LE:
		if (c_number(opnd) > c_number(opnd2))
		  return lispnil;
		break;
	      case K_GT:
		if (c_number(opnd) <= c_number(opnd2))
		  return lispnil;
		break;
	      case K_GE:
		if (c_number(opnd) < c_number(opnd2))
		  return lispnil;
		break;
	      default: break;
	    }
	}
	/* Load current operand into previous operand slot. */
	opnd = opnd2;
	/* Set the retobj, if necessary. */
	if (lispnil == retobj)
	  retobj = new_number(TRUE);
    }
    return retobj;
}

/* Choose from a list of weights and values, which can be formatted as
   a flat list of (n1 v1 n2 v2 ...), or as ((n1 v1) (n2 v2) ...) */

Obj *
choose_from_weighted_list(Obj *lis, int *totalweightp, int flat)
{
    int n, sofar, weight;
    char buf[BUFSIZE];
    Obj *rest, *head, *tail, *rslt;

    if (*totalweightp <= 0) {
	for_all_list(lis, rest) {
	    if (flat) {
		if (numberp(car(rest))) {
		    weight = c_number(car(rest));
		    rest = cdr(rest);
		} else {
		    weight = 1;
		}
	    } else {
		head = car(rest);
		weight = ((consp(head) && numberp(car(head)))
			  ? c_number(car(head)) : 1);
	    }
	    *totalweightp += weight;
	}
    }
    /* Warn about dubious weights - note that we can continue to
       execute, xrandom on 0 is still 0. */
    if (*totalweightp == 0) {
	sprintlisp(buf, lis, BUFSIZE);
	run_warning("Sum of weights in weighted list `%s' is 0", buf);
    }
    n = xrandom(*totalweightp);
    sofar = 0;
    rslt = lispnil;
    for_all_list(lis, rest) {
	if (flat) {
	    if (numberp(car(rest))) {
		sofar += c_number(car(rest));
		rest = cdr(rest);
	    } else {
		sofar += 1;
	    }
	    tail = car(rest);
	} else {
	    head = car(rest);
	    if (consp(head) && numberp(car(head))) {
		sofar += c_number(car(head));
		tail = cdr(head);
	    } else {
		sofar += 1;
		tail = head;
	    }
	}
	if (sofar > n) {
	    rslt = tail;
	    break;
	}
    }
    return rslt;
}

int
interpolate_in_list(int val, Obj *lis, int *rslt)
{
    int first, thisin, thisval, nextin, nextval;
    Obj *rest, *head, *next;

    first = TRUE;
    for_all_list(lis, rest) {
	head = car(rest);
	thisin = c_number(car(head));
	thisval = c_number(cadr(head));
	if (cdr(rest) != lispnil) {
	    next = cadr(rest);
	    nextin = c_number(car(next));
	    nextval = c_number(cadr(next));
	    first = FALSE;
	} else if (first) {
	    if (val == thisin) {
		*rslt = thisval;
		return 0;
	    } else if (val < thisin) {
		return (-1);
	    } else {
		return 1;
	    }
	} else {
	    /* We're at the end of a several-item list; the value
	       must be too high. */
	    return 1;
	}
	if (val < thisin) {
	    return (-1);
	} else if (between(thisin, val, nextin)) {
	    if (val == thisin) {
		*rslt = thisval;
	    } else if (val == nextin) {
		*rslt = nextval;
	    } else {
		*rslt = thisval;
		if (val != thisin && nextin != thisin) {
		    /* Add the linear interpolation. */
		    *rslt += ((nextval - thisval) * (val - thisin)) / (nextin - thisin);
		}
	    }
	    return 0;
	}
    }
    return (-1);
}

int
interpolate_in_list_ext(int val, Obj *lis, int mindo, int minval, int minrslt,
			int maxdo, int maxval, int maxrslt, int *rslt)
{
    int first, thisin, thisval, nextin, nextval;
    Obj *rest, *head, *next;

    /* (should use the additional parameters) */
    first = TRUE;
    for_all_list(lis, rest) {
	head = car(rest);
	thisin = c_number(car(head));
	thisval = c_number(cadr(head));
	if (cdr(rest) != lispnil) {
	    next = cadr(rest);
	    nextin = c_number(car(next));
	    nextval = c_number(cadr(next));
	    first = FALSE;
	} else if (first) {
	    if (val == thisin) {
		*rslt = thisval;
		return 0;
	    } else if (val < thisin) {
		return (-1);
	    } else {
		return 1;
	    }
	} else {
	    /* We're at the end of a several-item list; the value
	       must be too high. */
	    return 1;
	}
	if (val < thisin) {
	    return (-1);
	} else if (between(thisin, val, nextin)) {
	    if (val == thisin) {
		*rslt = thisval;
	    } else if (val == nextin) {
		*rslt = nextval;
	    } else {
		*rslt = thisval;
		if (val != thisin && nextin != thisin) {
		    /* Add the linear interpolation. */
		    *rslt += ((nextval - thisval) * (val - thisin)) / (nextin - thisin);
		}
	    }
	    return 0;
	}
    }
    return (-1);
}

void
interp_short_array(short *arr, Obj *lis, int n)
{
    int i = 0;
    Obj *rest, *head;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (i < n) {
	    	arr[i++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
		break;
	    }
	}
    }
}

void
interp_long_array(long *arr, Obj *lis, int n)
{
    int i = 0;
    Obj *rest, *head;

    /* Assume that if the destination array does not exist, there is
       probably a reason, and it's not our concern. */
    if (arr == NULL)
      return;
    for_all_list(lis, rest) {
    	head = car(rest);
    	if (numberp(head)) {
	    if (i < n) {
	    	arr[i++] = c_number(head);
	    } else {
		init_warning("too many numbers in list");
		break;
	    }
	}
    }
}

const char *
get_string(Obj *lis)
{
    const char *str = NULL;

    if (lis != lispnil) {
	if (stringp(lis)) {
	    str = c_string(lis);
	} else if (consp(lis)) {
	    if (stringp(car(lis))) {
		str = c_string(car(lis));
	    }
	}
    }
    return str;
}

