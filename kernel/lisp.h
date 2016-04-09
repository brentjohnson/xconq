/* Definitions for Lisp objects in Xconq.
   Copyright (C) 1989, 1991-2000 Stanley T. Shebs.
   Copyright (C) 2004-2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#ifndef LISP_H
#define LISP_H

/*! \file kernel/lisp.h
 * \brief Definitions for Lisp objects in Xconq.
 *
 * Lisp objects in Xconq are pretty basic, since they are used only for
 * game definition and (usually) not the core computations. */

#include "parambox.h"

/*! \brief Lisp object types.
 *
  * The different types of Lisp |Object. */
enum lisptype {
  NIL,      	/*!< NULL object. */
  CONS,     	/*!< CONS object. */
  NUMBER,	/*!< number object. */
  STRING, 	/*!< string object. */
  SYMBOL,	/*!< symbol object. */
  UTYPE,    	/*!< Unit type. ??? */
  MTYPE,   	/*!< Material type. ??? */
  TTYPE,    	/*!< Terrain type. ??? */
  ATYPE,    	/*!< Advance type. ??? */
  POINTER,	/*!< pointer object. */
  EOFOBJ	/*!< end-of_file object. ??? */
  };

/*! \brief Cons object.
 *
 * Declaration of a cons cell(List of objects). */
struct a_cons {
    struct a_obj *car;  /*!< Pointer to first \Object of list. */
    struct a_obj *cdr;  /*!< Pointer to remaining \Objects on list. */
};

/*! \brief Symbol object.
 *
 * A symbol includes its index and a pointer to its binding. */
struct a_symbol {
    struct a_symentry *symentry;	/*!< pointer to \ref a_symentry "symbol table" entry */
    struct a_obj *value;            	/*!< pointer to \Object */
};

/*! \brief Pointer object.
 *
 * A pointer is an address with associated name.  Interpretation
 * and usage is up to the context. */
struct a_pointer {
    struct a_obj *sym;  /*!< Pointer to lisp \Object. */
    char *data;         	/*!< Pointer to data for pointer. */
};

/*! \brief Lisp object.
 *
 * The basic Lisp object.  This should be small. */
typedef struct a_obj {
    enum lisptype type;     	/*!< Type of the \Object */
    union {
	int num;                	/*!< numeric value */
	const char *str;              	/*!< string value */
	struct a_symbol sym;	/*!< \ref a_symbol "Symbol Object". */
	struct a_cons cons;     	/*!< \ref a_cons "list" object */
	struct a_pointer ptr;	/*!< \ref a_pointer "pointer" object */
    } v;                    		/*!< the "content" of the \Object */
} Obj;

/*! \brief Symbol table.
 *
 * The symbol table is the way to map names into symbols. */
typedef struct a_symentry {
    char *name;             		/*!< name */
    struct a_obj *symbol;   		/*!< pointer to the \Object. */
    char constantp;         		/*!< constant flag */
    struct a_symentry *next;		/*!< next entry in symbol table */
} Symentry;


/*! \brief context buffer size. */
#define CONTEXTSIZE 129

/*! \brief Stream type.
 *
 * Type of the stream.
 */
enum strmtype { 
    stringstrm,     /*!< string pointer */
    filestrm        	/*!< file pointer */
};

/*! \brief Stream.
 *
 * A stream is just a union of string pointer and file pointer. */
typedef struct a_strm {
    	enum strmtype type;         		/*!< Type of stream. */
    	union {
		const char *sp;        		/*!< String pointer */
		FILE *fp;                   		/*!< file pointer */
	} ptr;
	char lastread[CONTEXTSIZE];	/*!< Last read buffer */
	int numread;                			/*!< Amount read into buffer? */
} Strm;

#undef  DEF_KWD
#define DEF_KWD(name,CODE)  CODE,

/*! Keywords.
 * Enum of all the random keywords. */
enum keywords {

#include "keyword.def"

    LAST_KEYWORD    /*!< end of keywords */
};

/*! \brief Match keyword.
 *
 * Attempt to match symbol name with keyword. 
 * \param ob is \ref a_obj "lisp object".
 * \param key is keywords.
 * \return
 *    - true if matches;
 *    - false otherwise.
 */
#define match_keyword(ob,key) \
  (symbolp(ob) && strcmp(c_string(ob), keyword_name(key)) == 0)

/*! \brief Iterate list.
 *
 * for header for iterating through lisp lists. 
 * \param lis is the list iteration variable.
 * \param rest is the cdr part of the list (remainder, rest).
 */
#define for_all_list(lis,rest)  \
  for (rest = (lis); rest != lispnil; rest = cdr(rest))

/*! \brief Iterate through two lists.
 *
 * for header to interate through two lisp lists in parallel.
 * \param lis1 is the first list iteration variable.
 * \param lis2 is the second list iteration variable.
 * \param rest1 is the rest of the first list (cdr).
 * \param rest2 is the rest of the second list (cdr).
 */
#define for_both_lists(lis1,lis2,rest1,rest2)  \
   for (rest1 = (lis1), rest2 = (lis2);  \
	rest1 != lispnil && rest2 != lispnil;  \
	rest1 = cdr(rest1), rest2 = cdr(rest2))

/* All the Lisp interface declarations. */

/*! \brief Lisp nil \ref a_obj "object" pointer */
extern Obj *lispnil;
/*! \brief Lisp end-of-file. */
extern Obj *lispeof;

extern void init_lisp(void);
extern Obj *read_form(FILE *fp, int *p1, int *p2);
extern Obj *read_form_from_string(const char *str, int *p1, int *p2, const char **endstr);
extern int length(Obj *list);
extern Obj *new_string(const char *str);
extern Obj *new_number(int num);
extern Obj *new_utype(int u);
extern Obj *new_mtype(int r);
extern Obj *new_ttype(int t);
extern Obj *new_atype(int s);

extern Obj *new_pointer(Obj *sym, char *ptr);
extern Obj *cons(Obj *x, Obj *y);
extern void type_warning(char *funname, Obj *x, char *errtype, Obj *subst);
extern Obj *car(Obj *x);
extern Obj *cdr(Obj *x);
extern Obj *cadr(Obj *x);
extern Obj *cddr(Obj *x);
extern Obj *caddr(Obj *x);
extern Obj *cdddr(Obj *x);
extern void set_cdr(Obj *x, Obj *v);
extern const char *c_string(Obj *x);
extern int c_number(Obj *x);
extern Obj *intern_symbol(const char *str);
extern Obj *symbol_value(Obj *sym);
extern Obj *setq(Obj *sym, Obj *x);
extern void makunbound(Obj *sym);
extern void flag_as_constant(Obj *sym);
extern int constantp(Obj *sym);
extern int numberp(Obj *x);
extern int stringp(Obj *x);
extern int symbolp(Obj *x);
extern int consp(Obj *x);
extern int utypep(Obj *x);
extern int mtypep(Obj *x);
extern int ttypep(Obj *x);
extern int atypep(Obj *x);
extern int pointerp(Obj *x);
extern int boundp(Obj *sym);
extern int numberishp(Obj *x);
extern int listp(Obj *x);
extern int equal(Obj *x, Obj *y);
extern int member(Obj *x, Obj *lis);
extern Obj *elt(Obj *lis, int n);
extern Obj *reverse(Obj *lis);
extern Obj *find_at_key(Obj *lis, const char *key);
extern Obj *replace_at_key(Obj *lis, const char *key, Obj *newval);
extern int is_quoted_lisp(Obj *x);

namespace Xconq {

//! Get Type 1 dice spec from internal dice representation.
extern void dice1_desc(char *buf, DiceRep dice);
//! Get Type 2 dice spec from internal dice representation.
extern void dice2_desc(char *buf, DiceRep dice);

}

//! Print dice spec or a straight integer.
extern void fprint_num_or_dice(FILE *fp, int x, int valtype);
extern void fprinttable(FILE *fp, int n);
extern void fprintlisp(FILE *fp, Obj *oobj);
extern void fprint_list(FILE *fp, Obj *obj);
extern void sprintlisp(char *buf, Obj *obj, int maxlen);
extern void sprint_list(char *buf, Obj *obj, int maxlen);
extern void dlisp(Obj *x);
extern void print_form_and_value(FILE *fp, Obj *form);
extern Obj *append_two_lists(Obj *x1, Obj *x2);
extern Obj *append_lists(Obj *lis);
extern Obj *remove_from_list(Obj *element, Obj *lis);
extern Obj *remove_list_from_list(Obj *rlist, Obj *slist);
extern void push_binding(Obj **lis, Obj *key, Obj *val);
extern void push_cdr_binding(Obj **lis, Obj *key, Obj *val);
extern void push_int_binding(Obj **lis, Obj *key, int val);
extern void push_key_binding(Obj **lis, int key, Obj *val);
extern void push_key_cdr_binding(Obj **lis, int key, Obj *val);
extern void push_key_int_binding(Obj **lis, int key, int val);
extern Obj *eval(Obj *x);
extern Obj *eval_symbol(Obj *x);
extern Obj *eval_list(Obj *x);
extern Obj* eval_boolean_expression(Obj *expr);
extern int eval_boolean_expression(Obj *expr, int (*fn)(Obj *), int dflt);
extern int eval_boolean_expression(Obj *expr, int (*fn)(Obj *, ParamBox *), 
				   int dflt, ParamBox *pbox);
extern Obj* eval_arithmetic_expression(Obj *expr);
extern Obj* eval_arithmetic_comparison_expression(Obj *expr);
extern Obj *choose_from_weighted_list(Obj *lis, int *totalweightp, int flat);
extern int interpolate_in_list(int val, Obj *lis, int *rslt);
extern int interpolate_in_list_ext(int val, Obj *lis,
			int mindo, int minval, int minrslt,
			int maxdo, int maxval, int maxrslt,
			int *rslt);

extern const char *escaped_symbol(const char *str);
extern char *escaped_string(const char *str);
extern char *safe_escaped_string(const char *str, int len);
extern void interp_short_array(short *arr, Obj *lis, int n);
extern void interp_long_array(long *arr, Obj *lis, int n);
extern int eval_number(Obj *val, int *isnumber);
extern const char *get_string(Obj *lis);

/* Functions that the Lisp code needs to have defined. */

extern void init_warning(const char *str, ...);
extern void low_init_warning(char *str);
extern void init_error(const char *str, ...);
extern void low_init_error(const char *str);
extern void run_warning(const char *str, ...);
extern void low_run_warning(char *str);
extern void run_error(const char *str, ...);
extern void low_run_error(char *str);
extern void announce_read_progress(void);
extern int keyword_code(const char *str);
extern const char *keyword_name(enum keywords k);
extern int lazy_bind(Obj *sym);
extern void init_predefined_symbols(void);
extern void syntax_error(Obj *x, const char *msg);

#endif /* LISP_H */
