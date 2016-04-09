/* Help-related definitions for Xconq.
   Copyright (C) 1991-1994, 1996, 1998, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* \file kernel/help.h
 * \brief Help-related definitions for Xconq.
 */

/* \brief Node types. */
enum nodeclass {
    miscnode,   	/*!< Miscellaneous node. */
    utypenode,  	/*!< Unit node. */
    mtypenode,  	/*!< Material node. */
    ttypenode,  	/*!< Terrain node. */
    atypenode   	/*!< Advance node. */
};

/* Help output carbon copies. */
#define HELP_OUTPUT_CC_NONE	0
#define HELP_OUTPUT_CC_FILES	1

/* \brief Help output mode. */
/* Postscript and PDF can be gotten from TexInfo or LaTeX. */
/* The idea behind magic text is to allow an UI to give a better presentation 
   of the help output. An alternative would be for the UI to parse /a subset 
   of?\ HTML directly /see doc/PROJECTS: I58\ */
typedef enum help_output_mode {
    HELP_OUTPUT_PLAIN_TEXT = 0,	    /*!< Plain Text */
    HELP_OUTPUT_HTML,		    /*!< Hypertext Markup Language */
    HELP_OUTPUT_TEXI,		    /*!< GNU TexInfo */
    HELP_OUTPUT_XML,		    /*!< Extensible Markup Language */
    HELP_OUTPUT_LATEX,		    /*!< LaTeX */
    HELP_OUTPUT_MAGIC_TEXT	    /*!< Text with UI-Parseable Format Codes */
} HelpOutputMode;

/* \brief Help page enumeration. */
typedef enum help_page {
    HELP_PAGE_NONE = 0,
    HELP_PAGE_MASTER_INDEX,
    HELP_PAGE_TOC,
    HELP_PAGE_COPYRIGHT,
    HELP_PAGE_WARRANTY,
    HELP_PAGE_NEWS,
    HELP_PAGE_INSTRUCTIONS,
    HELP_PAGE_GAME_OVERVIEW,
    HELP_PAGE_SCORING,
    HELP_PAGE_MODULES,
    HELP_PAGE_GAME_SETUP,
    HELP_PAGE_WORLD,
    HELP_PAGE_UTYPE,
    HELP_PAGE_TTYPE,
    HELP_PAGE_MTYPE,
    HELP_PAGE_ATYPE,
    HELP_PAGE_CONCEPTS
} HelpPage;

/* \brief Help page description struct. */
typedef struct help_page_defn {
    const char *filebname;
    const char *name;
    HelpPage tocpage;
    HelpPage prevpage;
    HelpPage nextpage;
} HelpPageDefn;

#include "obstack.h"

/*! \brief TextBuffer. */
typedef struct a_textbuffer {
    char *text;             	/*!< text string. */
    int bufmax;             	/*!< Maximum buffer size. */
    struct obstack ostack;  	/*!< Object stack control data */
} TextBuffer;

/*! \brief Help Node. */
typedef struct a_helpnode {
    const char *key;		/*!< key. */ 
    /*!< Pointer to function. (hash???) */
    void (*fn)(int arg, const char *key, TextBuffer *buf);
    enum nodeclass nclass;	/*!< Node type. */
    int arg;			/*!< Argument. ??? */
    char *text;			/*!< text string. */
    int textend;		/*!< text length. ??? */
    struct a_helpnode *prev; 	/*!< Previous help node. */
    struct a_helpnode *next;   	/*!< Next help node. */
} HelpNode;

/*! \brief First Help Node. */
extern HelpNode *first_help_node;

/*! Pointer to "Copying" help node. */
extern HelpNode *copying_help_node;
/*! Pointer to "Warranty" help node. */
extern HelpNode *warranty_help_node;

extern void tbprintf(TextBuffer *buf, const char *str, ...);
extern void tbcat(TextBuffer *buf, const char *str);
extern void tbcat_si(TextBuffer *buf, const char *str);
extern void tbcatline(TextBuffer *buf, const char *str);
extern void tbcatline_si(TextBuffer *buf, const char *str);

extern void init_help(void);
extern HelpNode *create_help_node(void);
extern HelpNode *add_help_node(const char *key,
			       void (*fn)(int, const char *, TextBuffer *),
			       int arg, HelpNode *prevnode);
extern HelpNode *find_help_node(HelpNode *node, const char *str);
extern void create_game_help_nodes(void);
extern char *get_help_text(HelpNode *node);

extern void describe_topics(int arg, const char *key, TextBuffer *buf);
extern void describe_command(int ch, const char *name, const char *help, int onechar, 
			     TextBuffer *buf);
extern void append_blurb_strings(char *buf, Obj *notes);
extern void notify_instructions(void);

extern void print_any_news(void);
extern void print_game_description_to_file(FILE *fp);

extern void describe_copyright(int arg, const char *key, TextBuffer *buf);
extern void describe_warranty(int arg, const char *key, TextBuffer *buf);

extern void set_help_output_cc(int cctarget);
extern void set_help_output_mode(HelpOutputMode houtmode);
extern void set_help_output_dir(char *);
extern void set_help_toc_filep(FILE *htocfilep);

extern FILE *prep_help_file(char *hfilename);
extern void finish_help_file(FILE *hfilep);
extern void write_help_file_header(FILE *hfile, const char *headerdata);
extern void write_help_file_footer(FILE *hfile, const char *footerdata);
extern void write_help_toc_entry(const char *hfilebname, const char *sectionname,
				 int indentlvl);
extern char *help_file_brand(void);
extern const char *get_help_file_extension(void);
