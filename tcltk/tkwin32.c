/* The main program of the Win32 tcltk interface to Xconq.
   Copyright (C) 2002 Hans Ronne.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */


#include "conq.h"
#include "kpublic.h"
#include "cmdline.h"
#include "tkconq.h"

#include <tk.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <malloc.h>
#include <locale.h>

#ifdef __cplusplus
extern "C" {
#endif
extern int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int);

#ifdef __cplusplus
}
#endif

extern int use_stdio;

int blinking_curunit = FALSE;

static void setargv(int *argcPtr, char ***argvPtr);

static void check_windows_version(void);

extern char *xcq_program_name;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    char **argv;
    int argc;
    char buffer[MAX_PATH+1];
    char path[MAX_PATH+1];
    char *p;

    /* Need to figure out if GDI resource memory is limited 
    and set the poor_memory flag accordingly. */
    check_windows_version();
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
    /* Setup output file for some messages since we cannot assume that 
       stdout is going to write to console or redirect to a file. */
    xcq_fstdout = open_file("Xconq_Output.txt", "w");

    use_stdio = FALSE;
    /* Tweak behavior of tcl/tk image handling. */
    use_clip_mask = FALSE;
    /* Dummy reference to get libraries pulled in */
    if (argc == -1)
      cmd_error(NULL, NULL);
    /* Shift to being the user that started the game. */
    init_library_path(NULL);
    /* Fiddle with game module structures. */
    clear_game_modules();
    /* Set up empty data structures. */
    init_data_structures();

    if (argc == 1)
      option_popup_new_game_dialog = TRUE;

    parse_command_line(argc, argv, general_options);
    
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
    initial_ui_init();
    /* If we're getting a startup dialog sequence, do it instead. */
    if (option_popup_new_game_dialog) {
	popup_game_dialog();
	ui_mainloop();
	/* Note that we never reach here. */
    }
    /* The straight-line setup sequence for a non-networked game set up
       using command-line arguments. */
    load_all_modules();
    check_game_validity();
    parse_command_line(argc, argv, variant_options);
    set_variants_from_options();
    parse_command_line(argc, argv, player_options);
    set_players_from_options();
    make_trial_assignments();
    /* Complain about anything that's left. */
    parse_command_line(argc, argv, leftover_options);
    launch_game();
    /* Go into the main play loop. */
    ui_mainloop();

    return 0;
}

void
check_windows_version(void)
{
    OSVERSIONINFO OS = { 0 };

    OS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OS);
    /* Primitive versions of Windows such as 98 and ME have 
	only 2M of GDI resource memory. */
    if (OS.dwPlatformId < VER_PLATFORM_WIN32_NT) {
	poor_memory = TRUE;
    }
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

/* The default (human) player is the current user on the current display. */

Player *
add_default_player(void)
{
    Player *player = add_player();
    
    player->name = getenv("USER");
    player->configname = getenv("XCONQCONFIG");
    player->displayname = "WinTCL";
    return player;
}

void
make_default_player_spec(void)
{
  default_player_spec = "WinTCL";
}
