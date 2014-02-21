/* New Windows native main file for the Win32 tcltk version of 
   imfapp for Xconq. Copyright (C) 2003 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"

#include <tk.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <malloc.h>
#include <locale.h>

#include "xlibstuff.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int);
#ifdef __cplusplus
}
#endif

char spbuf[BUFSIZE];

char readerrbuf[1000];

extern int ui_init(void);
extern void imfapp_main(void);
extern void close_displays(void);

extern void extract_dirname (char **dirname, char *pathname);

static void setargv(int *argcPtr, char ***argvPtr);

extern char *xcq_program_name;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    char **argv;
    int argc;
    char buffer[MAX_PATH+1];
    char path[MAX_PATH+1];
    char *p;

    setlocale(LC_ALL, "C");
    setargv(&argc, &argv);
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    argv[0] = buffer;
    xcq_program_name = copy_string(buffer);
    for (p = buffer; *p != '\0'; p++) {
	if (*p == '\\') {
	    *p = '/';
	}
    }

    /* Tweak behavior of tcl/tk image handling. */
    use_clip_mask = FALSE;

    /* Look for the tcl script files in the xconq/tcltk folder if TCL_LIBRARY 
    was not set explicitly by the user. */
    p = getenv("TCL_LIBRARY");
    if (p == NULL || p[0] == '\0') {
	    extract_dirname(&p, xcq_program_name);
	    sprintf(path, "TCL_LIBRARY=%s/tcltk/tcl%s", p, TCL_VERSION);
	    /* Must be called before Tcl_PutEnv. */
	    Tcl_FindExecutable(xcq_program_name);
	    Tcl_PutEnv(path);    
    }

    ui_init();
    init_lisp();
    imfapp_main();
    
    return 0;
}

/* Parses the command line. From the tcltk 8.3.4 winMain.c file. */

static void setargv(int *argcPtr, char ***argvPtr)
{
    char *cmdLine, *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    
    cmdLine = GetCommandLine();

    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
	if ((*p == ' ') || (*p == '\t')) {
	    size++;
	    while ((*p == ' ') || (*p == '\t')) {
		p++;
	    }
	    if (*p == '\0') {
		break;
	    }
	}
    }
    argSpace = (char *) Tcl_Alloc(
	    (unsigned) (size * sizeof(char *) + strlen(cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = cmdLine;
    for (argc = 0; argc < size; argc++) {
	argv[argc] = arg = argSpace;
	while ((*p == ' ') || (*p == '\t')) {
	    p++;
	}
	if (*p == '\0') {
	    break;
	}

	inquote = 0;
	slashes = 0;
	while (1) {
	    copy = 1;
	    while (*p == '\\') {
		slashes++;
		p++;
	    }
	    if (*p == '"') {
		if ((slashes & 1) == 0) {
		    copy = 0;
		    if ((inquote) && (p[1] == '"')) {
			p++;
			copy = 1;
		    } else {
			inquote = !inquote;
		    }
                }
                slashes >>= 1;
            }

            while (slashes) {
		*arg = '\\';
		arg++;
		slashes--;
	    }

	    if ((*p == '\0')
		    || (!inquote && ((*p == ' ') || (*p == '\t')))) {
		break;
	    }
	    if (copy != 0) {
		*arg = *p;
		arg++;
	    }
	    p++;
        }
	*arg = '\0';
	argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
}

/* Need these so we can link in Lisp reader code. */

void
announce_read_progress(void)
{
}

void  
syntax_error(Obj *x, char *msg)
{
    sprintlisp(readerrbuf, x, BUFSIZE);
    init_warning("syntax error in `%s' - %s", readerrbuf, msg);
}

void
low_init_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
}

/* A warning just gets displayed, no other action is taken. */

void
low_init_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* A run error is fatal. */

void
low_run_error(char *str)
{
    fprintf(stderr, "Error: %s.\n", str);
    fflush(stderr);
    exit(1);
}

void
low_run_warning(char *str)
{
    fprintf(stdout, "Warning: %s.\n", str);
    fflush(stdout);
}

/* This is needed because error handling refers to it. */

void
close_displays(void)
{
}

int
write_entire_game_state(char *fname)
{
    return 0;
}

/* Fake definitions of unneeded routines called by lisp.c. */

int
keyword_code(char *str)
{
    run_warning("fake keyword_code being called");
    return 0;
}

/* Make the table so keyword lookup works. */

struct a_key {
    char *name;
} keywordtable[] = {

#undef  DEF_KWD
#define DEF_KWD(NAME,code)  { NAME },

#include "keyword.def"

    { NULL }
};

char *
keyword_name(enum keywords k)
{
    return keywordtable[k].name;
}

void
init_predefined_symbols(void)
{
}

int
lazy_bind(Obj *sym)
{
    run_warning("fake lazy_bind being called");
    return FALSE;
}

void
prealloc_debug(void)
{
}
