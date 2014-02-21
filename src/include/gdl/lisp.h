// xConq
// GDL Lisp.

// $Id: lisp.h,v 1.5 2006/06/02 16:58:33 eric_mcdonald Exp $

/*//////////////////////////// COPYRIGHT NOTICE ////////////////////////////////

  Copyright (C) 1987-1989   Stanley T. Shebs
  Copyright (C) 1991-2000   Stanley T. Shebs
  Copyright (C) 2004-2006   Eric A. McDonald

//////////////////////////////////// LICENSE ///////////////////////////////////

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//////////////////////////////////////////////////////////////////////////////*/

/*! \file
    \brief GDL Lisp.
    \ingroup grp_gdl

    Lisp objects in Xconq are pretty basic, since they are used only for
    game definition and (usually) not the core computations.
*/

#ifndef XCONQ_GDL_LISP_H
#define XCONQ_GDL_LISP_H

#include "gdl/base.h"

// Global Constant Macros

//! Integer.
#define GDLINT	    0
//! Type 1 dice.
#define GDLDICE1    1
//! Type 2 dice.
#define GDLDICE2    2

//! GDL Lisp stream buffer size.
#define CONTEXTSIZE 129

// Function Macros: Queries

//! Get keyword name from key value.
#define key(x) (keyword_name(x))

// Function Macros: Validation

//! Generalized syntax check and return on error.
#define SYNTAX(X,TEST,MSG)  \
    if (!(TEST)) {  \
      syntax_error((X), (MSG));  \
      return;  \
    }

//! Generalized syntax check and return designated value on error.
#define SYNTAX_RETURN(X,TEST,MSG,RET)  \
    if (!(TEST)) {  \
      syntax_error((X), (MSG));  \
      return (RET);  \
    }

// Function Macros: Parsing

//! Parse list of name-value pairs that most forms use.
#define PARSE_PROPERTY(BDG,NAME,VAL)  \
    SYNTAX(BDG, (consp(BDG) && symbolp(car(BDG))), "property binding");  \
    (NAME) = c_string(car(BDG));  \
    (VAL) = cadr(BDG);

//! Parse list of name-value pairs that most forms use, returning designated value.
#define PARSE_PROPERTY_RETURN(BDG,NAME,VAL,RET)  \
    SYNTAX_RETURN(BDG, (consp(BDG) && symbolp(car(BDG))), "property binding", (RET));  \
    (NAME) = c_string(car(BDG));  \
    (VAL) = cadr(BDG);

// Function Macros: Comparison

//! Match keyword.
/*! Attempt to match symbol name with keyword. */
#define match_keyword(ob,key) \
  (symbolp(ob) && strcmp(c_string(ob), keyword_name(key)) == 0)

// Iterator Macros

//! Iterate GDL list.
#define for_all_list(lis,rest)  \
    for (rest = (lis); rest != lispnil; rest = cdr(rest))

//! Iterate through two lists.
#define for_both_lists(lis1,lis2,rest1,rest2)  \
   for (rest1 = (lis1), rest2 = (lis2);  \
	rest1 != lispnil && rest2 != lispnil;  \
	rest1 = cdr(rest1), rest2 = cdr(rest2))

NAMESPACE_XCONQ_BEGIN
NAMESPACE_GDL_BEGIN

// Data Structures

//! GDL Lisp object types.
enum lisptype {
    //! NULL object.
    NIL,
    //! CONS object.
    CONS,
    //! Number object.
    NUMBER,
    //! String object.
    STRING,
    //! Symbol object.
    SYMBOL,
    //! Unit type object.
    UTYPE,
    //! Material type object.
    MTYPE,
    //! Terrain type object.
    TTYPE,
    //! Advance type object.
    ATYPE,
    //! Pointer object.
    POINTER,
    //! End-of-file object.
    EOFOBJ
};

//! GDL Lisp cons object.
/*! Declaration of a cons cell (list node). */
struct a_cons {
    //! Pointer to first GDL object of list.
    struct a_obj *car;
    //! Pointer to remaining GDL objects on list.
    struct a_obj *cdr;
};

//! GDL Lisp symbol object.
/*! A symbol includes its index and a pointer to its binding. */
struct a_symbol {
    //! Pointer to symbol table entry.
    struct a_symentry *symentry;
    //! Pointer to GDL object containig value.
    struct a_obj *value;
};

//! GDL Lisp pointer object.
/*!
    A pointer is an address with associated name.
    Interpretation and usage is up to the context.
*/
struct a_pointer {
    //! Pointer to GDL object containing symbol.
    struct a_obj *sym;
    //! Pointer to data for pointer.
    char *data;
};

//! Symbol table entry.
/*! The symbol table is the way to map names to symbols. */
typedef struct a_symentry {
    //! Name.
    char *name;
    //! Pointer to GDL object containing symbol.
    struct a_obj *symbol;
    //! Is symbol constant?
    char constantp;
    //! Next entry in symbol table.
    struct a_symentry *next;
} Symentry;

//! GDL Lisp container.
/*! The basic GDL Lisp object. This should be small. */
typedef struct a_obj {
    //! Type of GDL object.
    enum lisptype type;
    //! Content of GDL object.
    union {
	//! Integer.
	int num;
	//! String.
	char *str;
	//! Symbol.
	struct a_symbol sym;
	//! List.
	struct a_cons cons;
	//! Pointer.
	struct a_pointer ptr;
    } v;
} Obj;

//! Type of GDL Lisp stream object.
enum strmtype {
    //! String stream.
    stringstrm,
    //! File stream.
    filestrm
};

//! GDL Lisp stream.
/*! A stream is just a union of char pointer and file pointer. */
typedef struct a_strm {
    //! Type of stream.
    enum strmtype type;
    //! Pointer to underlying stream.
    union {
	//! String.
	char *sp;
	//! File Pointer
	FILE *fp;
    } ptr;
    //! Ring buffer of most recently read characters.
    char lastread[CONTEXTSIZE];
    //! Number of bytes read into ring buffer.
    int numread;
} Strm;

#ifdef DEF_KWD
#undef DEF_KWD
#endif
//! Define GDL keyword.
#define DEF_KWD(name,CODE)  CODE,

//! GDL keyword codes.
/*! Enum of all GDL keyword codes. */
enum keywords {

#include "gdl/keyword.def"

    //! End of keywords.
    LAST_KEYWORD

};

// Global Variables: Uniques

//! Lisp nil.
extern Obj *lispnil;
//! Lisp end-of-file.
extern Obj *lispeof;

// Global Variables: Behavior Options

//! Actually interpret GDL Lisp or not.
extern int actually_read_lisp;

// Global Variables: Buffers

//! Buffer for imprinting read errors.
extern char *readerrbuf;

// Notifications

//! Read-time warning with current module name and line number.
extern void read_warning(char *str, ...);

//! Complain about unknown property.
extern void unknown_property(char *type, char *inst, char *name);

//! Report syntax error with given GDL form.
extern void syntax_error(Obj *x, char *msg);

//! Report warning with given object.
/*! Report what substitution value will be used instead. */
extern void type_warning(char *funname, Obj *x, char *errtype, Obj *subst);

// Queries: GDL Keyword Lookups

//! Get keyword code from keyword name.
/*!
    Given a string, return the enum of the matching keyword,
    if found, else -1.
*/
extern int keyword_code(char *str);
//! Get keyword name from keyword code.
extern char *keyword_name(enum keywords k);

// Queries: Comparison

//! Is one GDL object equal to another?
/*!
    General structural equality test.  Assumes that it is not getting
    passed any circular structures, which should never happen in Xconq.
*/
extern int equal(Obj *x, Obj *y);

//! Is string not its default value?
extern int string_not_default(char *str, char *dflt);

// Queries: Attribution

//! Is GDL object constant?
extern int constantp(Obj *sym);

//! Is GDL object bound to a value?
extern int boundp(Obj *sym);

//! Is GDL object quoted?
extern int is_quoted_lisp(Obj *x);

//! Is GDL object a symbol?
extern int symbolp(Obj *x);
//! Is GDL object a list (nil or cons)?
extern int listp(Obj *x);
//! Is GDL object a number?
extern int numberp(Obj *x);
//! Is GDL object an unit type?
extern int utypep(Obj *x);
//! Is GDL object a material type?
extern int mtypep(Obj *x);
//! Is GDL object a terrain type?
extern int ttypep(Obj *x);
//! Is GDL object an advance type?
extern int atypep(Obj *x);
//! Is GDL object somewhat numberish (number or type)?
extern int numberishp(Obj *x);
//! Is GDL object a string?
extern int stringp(Obj *x);
//! Is GDL object a cons cell?
extern int consp(Obj *x);
//! Is GDL object a pointer?
extern int pointerp(Obj *x);

// Queries: GDL Lists

//! Length of list.
extern int length(Obj *list);
//! Check if length of values list is not same as types list.
extern int list_lengths_match(
    Obj *types, Obj *values, char *formtype, Obj *form);

// Accessors: GDL Object Data

//! Retrieve C integer from GDL object.
extern int c_number(Obj *x);
//! Retrieve C string from GDL object.
/*! Return the string out of both strings and symbols. */
extern char *c_string(Obj *x);

// Accessors: GDL Symbols

//! Retrieve GDL symbol value from GDL symbol.
extern Obj *symbol_value(Obj *sym);

//! Bind value to symbol.
extern Obj *setq(Obj *sym, Obj *x);
//! Bind certain symbols to incrementally growing lists.
/*! If a special symbol, we might not have to fail.

    \note Although this looks like Lisp-level code,
          it knows about unit types and so forth, so is higher-level.
*/
extern int lazy_bind(Obj *sym);

//! Unbind a symbol.
extern void makunbound(Obj *sym);

//! Imbue GDL symbol with constness.
extern void flag_as_constant(Obj *sym);

// Accessors: GDL Lists

//! Return payload of cons cell.
extern Obj *car(Obj *x);
//! Return list linked to cons cell.
extern Obj *cdr(Obj *x);
//! Return car(cdr(x)).
extern Obj *cadr(Obj *x);
//! Return cdr(cdr(x)).
extern Obj *cddr(Obj *x);
//! Return car(cdr(cdr(x))).
extern Obj *caddr(Obj *x);
//! Return cdr(cdr(cdr(x))).
extern Obj *cdddr(Obj *x);

//! Link list to cons cell.
extern void set_cdr(Obj *x, Obj *v);

//! Replace GDL list item where key is matched.
extern Obj *replace_at_key(Obj *lis, char *key, Obj *newval);

//! Remove element from list.
extern Obj *remove_from_list(Obj *element, Obj *lis);
//! Remove list from list.
/*!
    Remove all occurrences of each object in a list of objects
    from a given list.
*/
extern Obj *remove_list_from_list(Obj *rlist, Obj *slist);

// Lifecycle Management

//! Create new GDL number object.
extern Obj *new_number(int num);
//! Create new GDL unit type object.
extern Obj *new_utype(int u);
//! Create new GDL material type object.
extern Obj *new_mtype(int r);
//! Create new GDL terrain type object.
extern Obj *new_ttype(int t);
//! Create new GDL advance type object.
extern Obj *new_atype(int s);
//! Create new GDL string object.
extern Obj *new_string(char *str);
//! Create new GDL pointer object.
extern Obj *new_pointer(Obj *sym, char *ptr);

// Lifecycle Management: GDL Symbols

//! Push new symbol with given name onto symbol table.
extern Obj *intern_symbol(char *str);

// Lifecycle Management: GDL Lists

//! Construct cons cell.
extern Obj *cons(Obj *x, Obj *y);
//! Join two lists together.
extern Obj *append_two_lists(Obj *x1, Obj *x2);
//! Join lists in given list into one list.
extern Obj *append_lists(Obj *lis);

// GDL Lisp Evaluators

//! Return value of symbol.
/*!
    Some symbols are lazily bound, meaning that they don't get a value
    until it is first asked for.
*/
extern Obj *eval_symbol(Obj *x);
//! Return value of number, if it is a number.
extern int eval_number(Obj *val, int *isnumber);
//! Evaluate list.
/*! List evaluation just blasts straight through the list. */
extern Obj *eval_list(Obj *x);
//! Evaluate boolean expression.
/*! Evaluate a boolean expression as Common Lisp would. */
extern Obj* eval_boolean_expression(Obj *expr);
//! Evaluate boolean expression by applying a simple predicate.
extern int eval_boolean_expression(Obj *expr, int (*fn)(Obj *), int dflt);
//! Evaluate boolean expression by applying a parameterized predicate.
extern int eval_boolean_expression(
    Obj *expr, int (*fn)(Obj *, ParamBox *), int dflt, ParamBox *pbox);
//! Evaluate arithmetic expression.
/*! Evaluate an arithmetic expression.
    Operator precedence is not considered because it is naturally
    enforced by the structure of the lists.
*/
extern Obj* eval_arithmetic_expression(Obj *expr);
//! Evaluate arithmetic comparison expression.
/*! Evaluate an arithmetic expression like Common Lisp. */
extern Obj* eval_arithmetic_comparison_expression(Obj *expr);
//! Evaluate Lisp.
/*!
    Our version of evaluation derefs symbols and evals through lists,
    unless the list car is a "special form".
*/
extern Obj *eval(Obj *x);

// Game Setup

//! Initialize predefined symbols used in GDL Lisp.
extern void init_predefined_symbols(void);

//! Initialize GDL Lisp engine.
/*! Pre-create some objects that should always exist. */
extern void init_lisp(void);

// GDL I/O

//! Read GDL form from file buffer.
extern Obj *read_form(FILE *fp, int *p1, int *p2);
//! Read GDL form from string.
extern Obj *read_form_from_string(char *str, int *p1, int *p2, char **endstr);

//! Imprint GDL object into buffer up to a maximum length.
extern void sprintlisp(char *buf, Obj *obj, int maxlen);
//! Print any GDL object to file.
extern void fprintlisp(FILE *fp, Obj *oobj);
//! Imprint GDL list into buffer up to a maximum length.
extern void sprint_list(char *buf, Obj *obj, int maxlen);
//! Print GDL list to file.
extern void fprint_list(FILE *fp, Obj *obj);

//! Read up to a maximum number of shorts from a GDL list.
extern void interp_short_array(short *arr, Obj *lis, int n);
//! Read up to a maximum number of longs from a GDL list.
extern void interp_long_array(long *arr, Obj *lis, int n);

//! Convert symbol name to its GDL escaped form.
/*! \bug Potential buffer overflow. */
extern char *escaped_symbol(char *str);
//! Convert string to its GDL escaped form.
/*!
    Note that this works correctly on NULL strings, turning them into
    strings of length 0.
*/
/*! \bug Potential buffer overflow. */
extern char *escaped_string(char *str);
//! Safely convert string to its GDL escaped form.
/*!
    The escaped_string() function makes a dangerous assumption about
    the length of an escaped (or even unescaped) string being less than
    the BUFSIZE, which is the size of escapedthingbuf.
    safe_escaped_string() is slightly less efficient, but avoids this
    assumption. Do note, however, that safe_escaped_string() can result
    in a memory leak, if the caller does not free the result after
    using it.
*/
extern char *safe_escaped_string(char *str, int len);

//! Serialize start-of-form token to GDL.
extern void start_form(char *hd);
//! Serialize end-of-form token to GDL.
extern void end_form(void);
//! Serialize newline to GDL.
extern void newline_form(void);
//! Serialize space to GDL.
extern void space_form(void);

//! Append string to GDL form.
extern void add_to_form(char *x);
//! Append string to GDL form without space delimiter.
extern void add_to_form_no_space(char *x);
//! Append char to GDL form.
extern void add_char_to_form(int x);
//! Append number to GDL form.
extern void add_num_to_form(int x);
//! Append number to GDL form without space delimiter.
extern void add_num_to_form_no_space(int x);
//! Append number or dice spec to GDL form.
/*! Write either a normal value or a dice spec, as appropriate. */
extern void add_num_or_dice_to_form(int x, int valtype);
//! Append GDL form to GDL form.
extern void add_form_to_form(Obj *x);

//! Serialize boolean-valued property to GDL.
extern void write_bool_prop(
    char *name,
    int value,
    int dflt,
    int nodefaulting, int addnewline);
//! Serialize integer-valued property to GDL.
extern void write_num_prop(
    char *name,
    int value,
    int dflt,
    int nodefaulting, int addnewline);
//! Serialize string-valued property to GDL.
extern void write_str_prop(
    char *name,
    char *value,
    char *dflt,
    int nodefaulting, int addnewline);
//! Serialize GDL Lisp-valued property to GDL.
extern void write_lisp_prop(
    char *name,
    struct a_obj *value,
    struct a_obj *dflt,
    int nodefaulting, int as_cdr, int addnewline);

#if (0)

/* All the Lisp interface declarations. */

extern int member(Obj *x, Obj *lis);
extern Obj *elt(Obj *lis, int n);
extern Obj *reverse(Obj *lis);
extern Obj *find_at_key(Obj *lis, char *key);
extern void dlisp(Obj *x);
extern void print_form_and_value(FILE *fp, Obj *form);
extern void push_binding(Obj **lis, Obj *key, Obj *val);
extern void push_cdr_binding(Obj **lis, Obj *key, Obj *val);
extern void push_int_binding(Obj **lis, Obj *key, int val);
extern void push_key_binding(Obj **lis, int key, Obj *val);
extern void push_key_cdr_binding(Obj **lis, int key, Obj *val);
extern void push_key_int_binding(Obj **lis, int key, int val);
extern Obj *choose_from_weighted_list(Obj *lis, int *totalweightp, int flat);
extern int interpolate_in_list(int val, Obj *lis, int *rslt);
extern int interpolate_in_list_ext(int val, Obj *lis,
			int mindo, int minval, int minrslt,
			int maxdo, int maxval, int maxrslt,
			int *rslt);
extern char *get_string(Obj *lis);

#endif

NAMESPACE_GDL_END
NAMESPACE_XCONQ_END

#endif // ifndef XCONQ_GDL_LISP_H
