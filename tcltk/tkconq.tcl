# The tcl/tk interface to Xconq.
# Copyright (C) 1998-2000 Stanley T. Shebs.
# Copyright (C) 2004, 2005 Eric A. McDonald.

# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.  See the file COPYING.

# Lose the original main window, we don't have a use for it.

wm title . "Xconq Main Window"
wm withdraw .

set pathlist [ split [ xconq_library_paths ] ";" ]

lappend auto_path [file join [file dirname [info script]] BWidget]
package require BWidget

# The regular background color.
#set bgcolor LemonChiffon
set bgcolor LightGray
# The "highlight" background color. 
# Fringe color around buttons and canvases on the mac.
#set hbcolor LemonChiffon
set hbcolor DarkGray
# The "active" backgound color. Unused on the mac.
#set abcolor LemonChiffon
set abcolor LightGray
# The troghcolor for scrollbars.
#set tcolor LemonChiffon2
set tcolor Gray
# The map background color.
#set mbcolor SandyBrown
set mbcolor DarkGray
# The listbox and scrollable canvas color.
#set lcolor azure
set lcolor AntiqueWhite
# The select color is on all the time on Windows ...
if { "$tcl_platform(platform)" == "windows" } {
	set scolor white
} else {
	set scolor MediumBlue
}
# ... but not for menu item checkmarks.
set micolor MediumBlue
# The foreground color.
set fgcolor black
# The "active" foreground color.
set afcolor SandyBrown
# The color of the notices panel.
set ncolor LightSkyBlue1

set lineheight 5

set utype_icon_size 32
set mtype_icon_size 16

# This flag is 1 if map displays are to use double-buffering for drawing.

set dblbuffer 1

# This flag 1 if the user is doing game design.

set designer 0

set any_elev_variation 0
set any_temp_variation 0
set any_wind_variation 0
set any_clouds 0

# Name of human or AI playing indepside, if any.
set indepside_player ""

set indepside_act 1
set indepside_build 0
set indepside_research 0
set indepside_economy 0
set indepside_treasury 0

set indepside_entry 0
set firstside_entry 0

set num_avail_side_names 0

# This global is the id/number of the side using the display.

set dside 0

# Name of dside's ai

set dside_ai "none"

# This global is the remote program id used when networking.

set my_rid 0

# This is the rid of the master.

set master_rid 0

set see_all 0

set may_set_show_all 0

set curunit 0

set endofgame 0

# Set this to 1 to get some handy bits for debugging the tcl/tk code.

set debug 0

set vary_world_size 0

set new_width 0
set new_height 0
set new_circumference 0
set new_latitude 0
set new_longitude 0

set vary_real_time 0

set new_time_for_game 0
set new_time_per_side 0
set new_time_per_turn 0

if { $debug } {
    puts stdout "Font families are [ font families ]"
    puts stdout "Paths are $pathlist"
    # Double buffering makes it hard to see what is going on.
    set dblbuffer 0
}

set last_world_width 0
set last_world_power -1

set ai_may_resign 0

# This is the list of all the map view options that are available.

set view_option_list { \
	power \
	grid \
	coverage \
	elevations \
	lighting \
	people \
	control \
	temperature \
	winds \
	clouds \
	unit_names \
	feature_names \
	feature_boundaries \
	meridians \
	meridian_interval \
	ai \
    }

foreach opt $view_option_list {
    set prefs($opt) 0
}

# The array of displayable names for view options.

set view_option_names(power) "Power"
set view_option_names(grid) "Grid"
set view_option_names(coverage) "Coverage"
set view_option_names(elevations) "Elevations"
set view_option_names(lighting) "Day/Night"
set view_option_names(people) "People"
set view_option_names(control) "Control"
set view_option_names(temperature) "Temperature"
set view_option_names(winds) "Winds"
set view_option_names(clouds) "Clouds"
set view_option_names(unit_names) "Unit Names"
set view_option_names(feature_names) "Feature Names"
set view_option_names(feature_boundaries) "Feature Boundaries"
set view_option_names(meridians) "Meridians"
set view_option_names(meridian_interval) "Meridian Interval"
set view_option_names(ai) "AI Info"

# The array of map command options corresponding to view options.

set view_option_flags(power) -power
set view_option_flags(grid) -grid
set view_option_flags(coverage) -coverage
set view_option_flags(elevations) -elevations
set view_option_flags(lighting) -lighting
set view_option_flags(people) -people
set view_option_flags(control) -control
set view_option_flags(temperature) -temperature
set view_option_flags(winds) -winds
set view_option_flags(clouds) -clouds
set view_option_flags(unit_names) -unitnames
set view_option_flags(feature_names) -featurenames
set view_option_flags(feature_boundaries) -featureboundaries
set view_option_flags(meridians) -meridians
set view_option_flags(meridian_interval) -meridianinterval
set view_option_flags(ai) -ai

set imagery_option_list { \
	terrain_images \
	terrain_patterns \
	transitions \
    }

foreach opt $imagery_option_list {
    set prefs($opt) 0
}

set imagery_option_names(terrain_images) "Terrain Images"
set imagery_option_names(terrain_patterns) "Terrain Patterns"
set imagery_option_names(transitions) "Terrain Transitions"

# Set some defaults that should be nonzero if no preferences found.

set prefs(power) 5
set prefs(meridian_interval) 600

# (should handle case of non-availability)

# Set platform-specific fonts for use in the startup dialogs. Also set 
# platform-specific default fonts in case no prefs file is loaded later.
# Finally set the default joinhost to either 127.0.0.1 or localhost.

if { "$tcl_platform(platform)" == "unix" } {
	set stdfont {-size 12 -weight normal -family helvetica }
	set boldfont {-size 12 -weight bold -family helvetica }
	set bigfont {-size 14 -weight normal -family helvetica }
	set titlefont {-size 14 -weight bold -family helvetica }
	set hugefont {-size 36 -weight bold -family helvetica }
	set mapfont {-size 12 -weight bold -family helvetica }
	set prefs(font_family) helvetica
	set prefs(font_size) 12
	set prefs(joinhost) localhost
	set scrollb 2
	set listb 2
	set textb 2
}

if { "$tcl_platform(platform)" == "macintosh" } {
	set stdfont {-size 12 -weight normal -family helvetica }
	set boldfont {-size 12 -weight bold -family helvetica }
	set bigfont {-size 14 -weight normal -family helvetica }
	set titlefont {-size 14 -weight bold -family helvetica }
	set hugefont {-size 36 -weight bold -family helvetica }
	set mapfont {-size 12 -weight normal -family gadget }
	set prefs(font_family) helvetica
	set prefs(font_size) 12
	# MacTCL doesn't understand what "localhost" is.
	set prefs(joinhost) 127.0.0.1
	set scrollb 0
	set listb 2
	set textb 2
}

if { "$tcl_platform(platform)" == "windows" } {
	set stdfont {-size 9 -weight normal -family arial }
	set boldfont {-size 9 -weight bold -family arial }
	set bigfont {-size 11 -weight normal -family arial }
	set titlefont {-size 11 -weight bold -family arial }
	set hugefont {-size 27 -weight bold -family arial }
	set mapfont {-size 9 -weight bold -family arial }
	set prefs(font_family) arial
	set prefs(font_size) 9
	# WinTCL doesn't understand what "127.0.0.1" is.
	set prefs(joinhost) localhost
	set scrollb 0
	set listb 2
	set textb 2
}

set prefs(joinport) 3075
set prefs(myname) "John Doe"

set prefs(want_checkpoints) 0
set prefs(checkpoint_interval) 0
set prefs(listallgames) 0

set prefs(terrain_images) 1
set prefs(transitions) 1

set prefs(unit_names) 1
set prefs(feature_names) 1

set prefs(temperature) 1
set prefs(clouds) 1
set prefs(winds) 1

# The preceding code is all executed during initial_ui_init(), and
# thus goes before any customization done by the game design or user
# preferences.

# The initial splash screen.  This proc also sets up the framing and
# buttons that later screens will use.

proc popup_splash_screen {} {
    if { "[ winfo exists .newgame ]" } {
	return
    }
    create_newgame_window
    add_splash_screen_dialog_items
}

proc create_newgame_window {} {
    global debug bigfont boldfont stdfont titlefont hugefont prefs 
    global bgcolor hbcolor abcolor fgcolor afcolor
    global my_rid master_rid

    toplevel .newgame
    wm title .newgame "Xconq Welcome"
    wm withdraw .newgame

    # Load the fonts from the preferences if present.
    set stdfont [ list "-family" $prefs(font_family) "-size" $prefs(font_size) ]
    set boldfont [ list "-family" $prefs(font_family) "-size" $prefs(font_size) "-weight" "bold" ]
    if { $prefs(font_size) < 10 } {
	    set bigfont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 1 ] ]
	    set titlefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 1 ] "-weight" "bold" ]
    } else {
	    set bigfont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 2 ] ]
	    set titlefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 2 ] "-weight" "bold" ]
    }
    set hugefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) * 3 ] "-weight" "bold" ]

    # Picking a fixed size is bad in the sense that we don't also
    # force the font to a known size (or provide scrollbars or
    # anything, not that scrollbars are a nice solution).  This shows
    # up in the variants dialog if the user has an 18 point font in
    # .Xdefaults, for example.
    frame .newgame.top -width 500 -height 370
    if { $debug } {
	.newgame.top config -bg green
    } else {
	.newgame.top config -bg $bgcolor
    }
    
    pack .newgame.top -side top -fill both -expand true
    # We're planning to reuse this frame, don't let its size bounce around.
    pack propagate .newgame.top false

    frame .newgame.bottom -bg $bgcolor
    pack .newgame.bottom -side bottom -fill both
    frame .newgame.bottom.dummy -bg $bgcolor
    pack .newgame.bottom.dummy -side left -padx 1
    frame .newgame.bottom.dummy2 -bg $bgcolor
    pack .newgame.bottom.dummy2 -side right -padx 1
 
   button .newgame.bottom.b1 -width 10 -height 2 -text "New Game" -font $boldfont \
	    -command { popup_game_dialog } -default active \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    button .newgame.bottom.b2 -width 10 -height 2 -text "Open File" -font $boldfont \
	    -command { popup_open_dialog } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    button .newgame.bottom.connect -width 10 -height 2 -text "Connect" -font $boldfont \
	    -command { popup_connect } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    button .newgame.bottom.quit -width 10 -height 2 -text "Quit" -font $boldfont \
	    -command { exit_xconq } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    pack .newgame.bottom.b1 .newgame.bottom.b2 .newgame.bottom.connect \
	    .newgame.bottom.quit -side left -padx 4 -pady 6

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.quit flash
		exit_xconq 
	} elseif  {"%K" == "Return"} {
		.newgame.bottom.b1 flash
		popup_game_dialog
	}
    }
    bind .newgame <n> { .newgame.bottom.b1 flash; popup_game_dialog }
    bind .newgame <N> { .newgame.bottom.b1 flash; popup_game_dialog }
    bind .newgame <o> { .newgame.bottom.b2 flash; popup_open_dialog }
    bind .newgame <O> { .newgame.bottom.b2 flash; popup_open_dialog }
    bind .newgame <c> { .newgame.bottom.connect flash; popup_connect }
    bind .newgame <C> { .newgame.bottom.connect flash; popup_connect }
    bind .newgame <q> { .newgame.bottom.quit flash; exit_xconq }
    bind .newgame <Q> { .newgame.bottom.quit flash; exit_xconq }

     center_on_screen .newgame
}

proc add_splash_screen_dialog_items {} {
    global stdfont
    global bgcolor hbcolor fgcolor

    frame .newgame.top.frame -bg $bgcolor
    pack .newgame.top.frame -side top -fill both
    canvas .newgame.top.frame.splash -width 500 -height 304 \
    	-bg $bgcolor -highlightbackground $hbcolor -borderwidth 1 -relief solid
    pack .newgame.top.frame.splash -side top -anchor nw -padx 4 -pady 4
    set splashname [ find_image_filename "splash.gif" ]
    # Only try to create the image if we found the file.
    if { "$splashname" != "" } {
    	image create photo splash -file $splashname -width 500 -height 320 
	.newgame.top.frame.splash create image 0 5 -image splash -anchor nw
    }

    label .newgame.top.version -text "Version [ version_string ]" -font $stdfont -bg $bgcolor -fg $fgcolor
    pack .newgame.top.version -side top -padx 4 -anchor nw
    label .newgame.top.copyright -text [ copyright_string ] -font $stdfont -bg $bgcolor -fg $fgcolor
    pack .newgame.top.copyright -side top -padx 4 -anchor nw
}

proc remove_splash_screen_dialog_items {} {
    pack forget .newgame.top.frame
    pack forget .newgame.top.version
    pack forget .newgame.top.copyright
}

# The new game dialog.

set list_all_games 0

proc popup_game_dialog {} {
    wm title .newgame "Xconq New Game Setup"

    remove_splash_screen_dialog_items

    # For now...
    global my_rid master_rid

    # Only solo players or masters may start the game load stage.
    if { $my_rid == 0 || ($my_rid > 0 && $my_rid == $master_rid) } {
	start_game_load_stage
    }
    if { $my_rid > 0 && $my_rid != $master_rid } {
	.newgame.bottom.b1 config -state disabled
	.newgame.bottom.b2 config -state disabled
	return
    }

    add_new_game_dialog_items
    .newgame.bottom.b1 config -text "New Game" -command { new_game }
    .newgame.bottom.b2 config -command { popup_open_dialog }

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.quit flash
		exit_xconq 
	} elseif  {"%K" == "Return" && ($my_rid == 0 || $my_rid == $master_rid) } {
		.newgame.bottom.b1 flash
		new_game
	}
    }
    bind .newgame <n> { .newgame.bottom.b1 flash; new_game }
    bind .newgame <N> { .newgame.bottom.b1 flash; new_game }
    bind .newgame <o> { .newgame.bottom.b2 flash; popup_open_dialog }
    bind .newgame <O> { .newgame.bottom.b2 flash; popup_open_dialog }

    update idletasks
}

proc add_new_game_dialog_items {} {
    global bigfont boldfont titlefont stdfont 
    global scrollb listb textb
    global bgcolor hbcolor abcolor tcolor lcolor scolor fgcolor afcolor
    global prefs list_all_games

    # The right side displays info about the selected game.

    frame .newgame.top.right -bg $bgcolor -width 260
    pack .newgame.top.right -side right -fill y
    pack propagate .newgame.top.right false

    # (should have a cool gif here instead)
    # label .newgame.top.right.banner -text Xconq -font $titlefont -bg $bgcolor -fg $fgcolor
    # pack .newgame.top.right.banner -side top -fill x -padx 12 -pady 4

    canvas .newgame.top.right.preview -width 240 -height 160 \
	-borderwidth 2 -relief sunken -bg $bgcolor -highlightbackground $hbcolor
    .newgame.top.right.preview create text 120 85 -tag label -anchor c -fill $fgcolor
    pack .newgame.top.right.preview -side top -padx 6 -pady 6 -expand false
    pack propagate .newgame.top.right.preview false

    label .newgame.top.right.version -text "??version??" -font $boldfont -bg $bgcolor -fg $fgcolor
    pack .newgame.top.right.version -side top -fill x -padx 6 -pady 0

    frame .newgame.top.right.blurb -relief sunken -borderwidth $textb -bg $bgcolor
    pack .newgame.top.right.blurb -side top -fill x -padx 10 -pady 0

    text .newgame.top.right.blurb.t -width 40 -height 10 -wrap word -font $stdfont \
    	-bg $lcolor -highlightbackground $lcolor -fg $fgcolor -relief flat
    pack .newgame.top.right.blurb.t -side left

    frame .newgame.top.left -bg $bgcolor
    pack .newgame.top.left -side left -fill y

    set list_all_games [ listallgames ]
    checkbutton .newgame.top.left.listall -text " Also list unfinished games" -font $boldfont -anchor w \
	    -variable list_all_games -command { fill_new_game_list } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor
    pack .newgame.top.left.listall -side bottom -anchor w -pady 3 -padx 12

    frame .newgame.top.left.f -bg $bgcolor
    pack .newgame.top.left.f -side top -expand true -fill both -padx 8 -pady 8

    listbox .newgame.top.left.f.games -selectmode browse -width 30 -font $bigfont \
	    -yscrollcommand ".newgame.top.left.f.yscroll set" \
	    -relief sunken -borderwidth $listb -bg $lcolor -fg $fgcolor
    scrollbar .newgame.top.left.f.yscroll -orient vert \
	    -command ".newgame.top.left.f.games yview" -width 16 -borderwidth $scrollb \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack .newgame.top.left.f.yscroll -side right -fill y
    pack .newgame.top.left.f.games -side left -expand true -fill y

    fill_new_game_list

    set numgames [ numgames ]

    if { $numgames == 0 } {
	.newgame.bottom.b1 config -state disabled
    }
    
    # A single click selects a game.
    bind .newgame.top.left.f.games <ButtonRelease-1> { select_game }
    # A double click opens the selected game.
    bind .newgame.top.left.f.games <Double-Button-1> { select_game; new_game}


    # Auto-pre-select the first game in the list.
    .newgame.top.left.f.games selection set 0
    select_game
}

proc fill_new_game_list {} {
    global list_all_games
    global game_title game_version game_base_name
    global game_index

    .newgame.top.left.f.games delete 0 end

    set numgames [ numgames ]

    set j 0
    for { set i 0 } { $i < $numgames } { incr i } {
	game_info $i
	if { !$list_all_games } {
	    if { ![ string match {[1-9]*} $game_version($i) ] } {
		continue
	    }
	}
	set entry "$game_base_name($i)$game_title($i)"
	.newgame.top.left.f.games insert end " $entry"
	set game_index($j) $i
	incr j
    }
}

proc remove_new_game_dialog_items {} {
    pack forget .newgame.top.left
    pack forget .newgame.top.right
}

# Adjust the right-side elements to reflect the currently-selected
# game.

# This global is used to title later startup dialogs.

set selected_game_title "?unknown?"

proc select_game {} {
    global selected_game_title
    global game_title game_version game_blurb game_name
    global game_index game_picturename
    global game_basemodule game_basemodule_picturename

    set j [ .newgame.top.left.f.games curselection ]
    if { $j == "" } {
    	return
    }
    set i $game_index($j)
    # First look for a picture with a defined name in the main module.
    set entry "$game_picturename($i)"
    set picname [ find_image_filename "$entry.gif" ]
    # Then look for a picture named like the main module.
    if { "$picname" == "" } {
	    set entry "$game_name($i)"
	    set picname [ find_image_filename "$entry.gif" ]
    }
    # The look for a picture with a defined name in the base module.
    if { "$picname" == "" } {
	    set entry "$game_basemodule_picturename($i)"
	    set picname [ find_image_filename "$entry.gif" ]
    }
    # Then look for a picture named like the base module.
    if { "$picname" == "" } {
	    set entry "$game_basemodule($i)"
	    set picname [ find_image_filename "$entry.gif" ]
    }
    # Finally look for the default picture.
    if { "$picname" == "" } {
	    set entry "default"
	    set picname [ find_image_filename "$entry.gif" ]
    }
    # Only try to create the image if we found the file.
    # Attempt to prevent image jumps - does not work.
    set names [ image names ]
    if { "$picname" != "" } {
    	if { [ lsearch $names "preview" ] != -1 } {
    		image delete preview
    		.newgame.top.right.preview config -width 0 -height 0
    	}
    	image create photo preview -file $picname
    	set w [ image width preview ]
    	if { $w > 240 } { set w 240 }
    	set h [ image height preview ]
    	if { $h > 160 } { set h 160 }
    	.newgame.top.right.preview config -width $w -height $h
	.newgame.top.right.preview create image 5 5 -image preview -anchor nw
    	.newgame.top.right.preview itemconfig label -text ""
    } elseif { [ lsearch $names "preview" ] != -1 } {
    	image delete preview
    	.newgame.top.right.preview config -width 240 -height 160
    	.newgame.top.right.preview itemconfig label -text "No picture available"
    }
    set version "$game_title($i) $game_version($i)"

    .newgame.top.right.version config -text $version
    .newgame.top.right.blurb.t configure -state normal
    .newgame.top.right.blurb.t delete 1.0 end
    .newgame.top.right.blurb.t insert end $game_blurb($i)
    .newgame.top.right.blurb.t configure -state disabled
    set selected_game_title $game_title($i)
}

proc new_game {} {
    global game_index

    set j [ .newgame.top.left.f.games curselection ]
    if { $j == "" } {
    	return
    }
    set i $game_index($j)
    start_new_game $i
    popup_variants_dialog
}

proc popup_open_dialog { } {
    set dname [ game_homedir ]
    set rslt [ tk_getOpenFile -initialdir $dname ]
    # Empty result cancels.
    if { "$rslt" == "" } {
	return
    }
    # Set the working directory manually to the game file location (needed 
    # in both MacOS and Windows).
    set home [ pwd ]
    set dname [ file dirname $rslt ]
    cd $dname
    start_saved_game "$rslt"
    cd $home
    popup_variants_dialog
}

proc popup_variants_dialog {} {
    global my_rid master_rid

    # We need to call both of these since Open Game may bypass the
    # New Game Dialog.
    remove_splash_screen_dialog_items
    remove_new_game_dialog_items
    
    interpret_variants
    set variants [ any_variants ]
    # Proceed to player setup if there are no variants.
    if { $variants == 0 } {
    	if { $my_rid == 0 || ( $my_rid > 0 && $my_rid == $master_rid ) } {
		.newgame.bottom.b1 config -text "OK" -command { set_players }
		.newgame.bottom.b2 config -text "" -state disabled
		set_variants
	}
		return  
    }

    # In case we double-clicked a saved game.
    if { !"[ winfo exists .newgame ]" } {
	create_newgame_window
    }    
    wm title .newgame "Xconq Variants Setup"
    
    #Make more room already here which is needed in the player list.
    # (this is for players thast bypassed the New Game dialog).
    .newgame.top config -width 500 -height 500

    add_variants_dialog_items
    .newgame.bottom.b1 config -text "OK" -command { set_variants }
    .newgame.bottom.b2 config -text "" -state disabled

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.quit flash
		exit_xconq 
	} elseif  {"%K" == "Return" && ($my_rid == 0 || $my_rid == $master_rid) } {
		.newgame.bottom.b1 flash
		set_variants
	}
    }
    # Disable New Game and Open File bindings to prevent error messages when 
    # pressing the n and o keys by mistake.
    bind .newgame <n> { }
    bind .newgame <N> { }
    bind .newgame <o> { }
    bind .newgame <O> { }

    # For now, always go along with host's changes.
    if { $my_rid > 0 && $my_rid != $master_rid } {
	.newgame.bottom.b1 config -state disabled
	.newgame.top.buttons.worldsize config -state disabled
	.newgame.top.buttons.realtime config -state disabled
    }
    update idletasks
}

proc add_variants_dialog_items {} {
    global selected_game_title
    global varianttext variantstate
    global vary_world_size vary_real_time
    global boldfont titlefont stdfont
    global textb
    global bgcolor hbcolor abcolor lcolor scolor fgcolor afcolor

    if { !"[ winfo exists .newgame.top.header ]" } {
	label .newgame.top.header \
	    -text "Setup Variants for $selected_game_title" \
	    -font $titlefont -bg $bgcolor -fg $fgcolor
    }
    pack .newgame.top.header -side top -pady 6

    if { !"[ winfo exists .newgame.top.vhelp ]" } {
	frame .newgame.top.vhelp -height 50 -relief sunken -borderwidth $textb \
	-bg $bgcolor
	pack propagate .newgame.top.vhelp false
	text .newgame.top.vhelp.text -wrap word -font $stdfont -state disabled \
	-bg $lcolor -highlightbackground $lcolor -fg $fgcolor -relief flat
	pack .newgame.top.vhelp.text -side top -fill both
    }
    pack .newgame.top.vhelp -side bottom -fill x -padx 10 -pady 10

    if { !"[ winfo exists .newgame.top.checks ]" } {
	frame .newgame.top.checks -bg $bgcolor
	set numactive 0
	for { set v 0 } { $v < 30 } {incr v } {
	    if { "$variantstate($v)" == "active" } {
	    	incr numactive
	    	if { $numactive > 14 } {
	    		set row [ expr $numactive - 15 ]
	    		set col 1
	    	} else {
			set row [ expr $numactive - 1 ]
	    		set col 0
	    	}
		checkbutton .newgame.top.checks.v$v -text " $varianttext($v)" \
		    -state normal -variable variantvalue($v) -font $boldfont \
		    -command [ list set_variant_value_cmd $v ] \
		    -bg $bgcolor -highlightbackground $bgcolor \
		    -activebackground $abcolor -selectcolor $scolor \
		    -fg $fgcolor -activeforeground $afcolor 
		grid .newgame.top.checks.v$v -sticky w -padx 10 -pady 2 \
		    -column $col -row $row
		bind .newgame.top.checks.v$v <Enter> \
			[ list show_variant_help $v ]
		bind .newgame.top.checks.v$v <Leave> \
			[ list clear_variant_help $v ]
	    }
	}
    }
    pack .newgame.top.checks -side left -fill y

    if { !"[ winfo exists .newgame.top.buttons ]" } {
	frame .newgame.top.buttons -bg $bgcolor

	button .newgame.top.buttons.worldsize -width 11 -text "World Size..." \
		 -command { popup_world_size_dialog } -font $boldfont \
		 -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		 -fg $fgcolor -activeforeground $afcolor
	pack .newgame.top.buttons.worldsize -side top -anchor c -padx 20 -pady 10
	button .newgame.top.buttons.realtime -width 11 -text "Real Time..." \
		-command { popup_real_time_dialog } -font $boldfont \
	    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    	-fg $fgcolor -activeforeground $afcolor
	pack .newgame.top.buttons.realtime -side top -anchor c -padx 20 -pady 10
	if { !$vary_world_size } {
	    .newgame.top.buttons.worldsize config -state disabled
	}
	if { !$vary_real_time } {
	    .newgame.top.buttons.realtime config -state disabled
	}
    }
    pack .newgame.top.buttons -side right -fill y
}

proc show_variant_help { var } {
    global varianthelp

    set msg $varianthelp($var)
    .newgame.top.vhelp.text configure -state normal
    .newgame.top.vhelp.text delete 1.0 end
    .newgame.top.vhelp.text insert end $msg
    .newgame.top.vhelp.text configure -state disabled
}

proc clear_variant_help { var } {
    .newgame.top.vhelp.text configure -state normal
    .newgame.top.vhelp.text delete 1.0 end
    .newgame.top.vhelp.text configure -state disabled
}

proc set_variant_value_cmd { n } {
    global new_width new_height new_circumference
    global new_time_for_game new_time_per_side new_time_per_turn
    global variantstate variantvalue

    if { $n == -1 } {
	set_variant_value $n $new_width $new_height $new_circumference
    } elseif { $n == -2 } {
	set_variant_value $n $new_time_for_game $new_time_per_side \
		$new_time_per_turn
    } else {
	set_variant_value $n $variantvalue($n) 0 0
    }
}

proc remove_variants_dialog_items {} {
    pack forget .newgame.top.header
    pack forget .newgame.top.vhelp
    pack forget .newgame.top.checks
    pack forget .newgame.top.buttons
}

proc set_variants {} {
    global my_rid master_rid

    remove_variants_dialog_items
    if { $my_rid == 0 || ( $my_rid > 0 && $my_rid == $master_rid ) } {
	start_player_pre_setup_stage
	start_player_setup_stage
    }
    popup_player_dialog
}

proc popup_world_size_dialog {} {
    global my_rid master_rid

    remove_variants_dialog_items
    add_world_size_dialog_items
    .newgame.bottom.b1 config -text "OK" \
	    -command { world_size_ok_cmd }
    .newgame.bottom.b2 config -text "Cancel" -state normal -borderwidth 2 \
	    -command { world_size_cancel_cmd }

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.b2 flash
		world_size_cancel_cmd 
	} elseif  {"%K" == "Return" && ($my_rid == 0 || $my_rid == $master_rid) } {
		.newgame.bottom.b1 flash
		world_size_ok_cmd
	}
    }
}

proc add_world_size_dialog_items {} {
    global new_width new_height new_circumference new_latitude new_longitude
    global bgcolor hbcolor fgcolor afcolor stdfont boldfont

    if { !"[ winfo exists .newgame.top.world ]" } {
	canvas .newgame.top.world -width 380 -height 240 -bg gray \
	-bg $bgcolor -highlightbackground $hbcolor
	set wtop [ expr 120 - 60 ]
	set wbot [ expr 120 + 60 ]
	set wleft [ expr 190 - $new_circumference / 2 ]
	set wright [ expr 190 + $new_circumference / 2 ]
	.newgame.top.world create rect $wleft $wtop $wright $wbot -fill white \
	    -outline $fgcolor
	.newgame.top.world create line $wleft 120 $wright 120 -fill $fgcolor
	set atop [ expr 120 - $new_height / 2 ]
	set abot [ expr 120 + $new_height / 2 ]
	set aleft [ expr 190 - $new_width / 2 ]
	set aright [ expr 190 + $new_width / 2 ]
	if { 1 } {
	    .newgame.top.world create rect $aleft $atop $aright $abot -fill blue -outline $fgcolor
	} else {
	    # (should draw hexagon)
	}
    }
    pack .newgame.top.world -side top -padx 10 -pady 10

    if { !"[ winfo exists .newgame.top.sizes ]" } {
	frame .newgame.top.sizes
	set base .newgame.top.sizes
	label $base.lwidth -text "Area Width:" -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fwidth -width 4 -fg $fgcolor -font $stdfont
	$base.fwidth insert end $new_width
	label $base.lheight -text "x Height:" -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fheight -width 4 -fg $fgcolor -font $stdfont
	$base.fheight insert end $new_height
	label $base.lcircumf -text "Circumference:" -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fcircumf -width 6 -fg $fgcolor -font $stdfont
	$base.fcircumf insert end $new_circumference
	grid $base.lwidth $base.fwidth $base.lheight $base.fheight \
		$base.lcircumf $base.fcircumf -sticky news
# these not supported currently
#	label $base.llon -text "Longitude:" -bg $bgcolor
#	entry $base.flon -width 6 -fg $fgcolor
#	$base.flon insert end $new_longitude
#	label $base.llat -text "Latitude:" -bg $bgcolor
#	entry $base.flat -width 6 -fg $fgcolor
#	$base.flat insert end $new_latitude
#	grid $base.llon $base.flon $base.llat $base.flat -sticky news
    }
    pack .newgame.top.sizes -side bottom
}

proc remove_world_size_dialog_items {} {
    pack forget .newgame.top.world
    pack forget .newgame.top.sizes
}

proc world_size_ok_cmd {} {
    global new_width new_height new_circumference new_latitude new_longitude

    set base .newgame.top.sizes
    set new_width [ $base.fwidth get ]
    set new_height [ $base.fheight get ]
    set new_circumference [ $base.fcircumf get ]
#    set new_latitude [ $base.flat get ]
#    set new_longitude [ $base.flon get ]
    set_variant_value_cmd -1
    # Use the cancellation steps to finish here.
    world_size_cancel_cmd
}

proc world_size_cancel_cmd {} {
    remove_world_size_dialog_items
    popup_variants_dialog
}

proc popup_real_time_dialog {} {
    global my_rid master_rid

    remove_variants_dialog_items
    add_real_time_dialog_items
    .newgame.bottom.b1 config -text "OK" \
	    -command { real_time_ok_cmd }
    .newgame.bottom.b2 config -text "Cancel" -state normal -borderwidth 2 \
	    -command { real_time_cancel_cmd }

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.b2 flash
		real_time_cancel_cmd 
	} elseif  {"%K" == "Return" && ($my_rid == 0 || $my_rid == $master_rid) } {
		.newgame.bottom.b1 flash
		real_time_ok_cmd
	}
    }
}

proc add_real_time_dialog_items {} {
    global new_time_for_game new_time_per_side new_time_per_turn
    global bgcolor fgcolor afcolor stdfont boldfont

    if { !"[ winfo exists .newgame.top.times ]" } {
	frame .newgame.top.times -bg $bgcolor
	set base .newgame.top.times
	label $base.lforgame -text "Total Time (mins:secs) " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fforgame_m -width 4 -fg $fgcolor -font $stdfont
	label $base.lforgame2 -text " : " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fforgame_s -width 4 -fg $fgcolor -font $stdfont
	$base.fforgame_m insert end [ expr $new_time_for_game / 60 ]
	$base.fforgame_s insert end [ expr $new_time_for_game % 60 ]
	label $base.lperside -text "Per Side (mins:secs) " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fperside_m -width 4 -fg $fgcolor -font $stdfont
	label $base.lperside2 -text " : " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fperside_s -width 4 -fg $fgcolor -font $stdfont
	$base.fperside_m insert end [ expr $new_time_per_side / 60 ]
	$base.fperside_s insert end [ expr $new_time_per_side % 60 ]
	label $base.lperturn -text "Per Turn (mins:secs) " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fperturn_m -width 4 -fg $fgcolor -font $stdfont
	label $base.lperturn2 -text " : " -bg $bgcolor -fg $fgcolor -font $boldfont
	entry $base.fperturn_s -width 4 -fg $fgcolor -font $stdfont
	$base.fperturn_m insert end [ expr $new_time_per_turn / 60 ]
	$base.fperturn_s insert end [ expr $new_time_per_turn % 60 ]
	grid $base.lforgame $base.fforgame_m $base.lforgame2 $base.fforgame_s \
		-sticky news -pady 10
	grid $base.lperside $base.fperside_m $base.lperside2 $base.fperside_s \
		-sticky news -pady 10
	grid $base.lperturn $base.fperturn_m $base.lperturn2 $base.fperturn_s \
		-sticky news -pady 10
    }
    pack .newgame.top.times -side top -fill both -expand true
}

proc remove_real_time_dialog_items {} {
    pack forget .newgame.top.times
}

proc real_time_ok_cmd {} {
    global new_time_for_game new_time_per_side new_time_per_turn

    set base .newgame.top.times
    set new_time_for_game \
	    [ expr [ $base.fforgame_m get ] * 60 + [ $base.fforgame_s get ] ]
    set new_time_per_side \
	    [ expr [ $base.fperside_m get ] * 60 + [ $base.fperside_s get ] ]
    set new_time_per_turn \
	    [ expr [ $base.fperturn_m get ] * 60 + [ $base.fperturn_s get ] ]
    set_variant_value_cmd -2
    real_time_cancel_cmd
}

proc real_time_cancel_cmd {} {
    remove_real_time_dialog_items
    popup_variants_dialog
}

proc popup_player_dialog {} {
    global my_rid master_rid

    # In case we double-clicked a saved game.
    if { !"[ winfo exists .newgame ]" } {
	create_newgame_window
    }    
    wm title .newgame "Xconq Player Setup"
    add_player_dialog_items
    .newgame.bottom.b1 config -text "OK" -command { set_players }
    .newgame.bottom.b2 config -text "" -state disabled

    bind .newgame <Key> {  
    	if {"%K" == "Escape"} { 
		.newgame.bottom.quit flash
		exit_xconq 
	} elseif  {"%K" == "Return" && ($my_rid == 0 || $my_rid == $master_rid) } {
		.newgame.bottom.b1 flash
		set_players
	}
    }

    # For now...
    if { $my_rid > 0 && $my_rid != $master_rid } {
	.newgame.bottom.b1 config -state disabled
    }
    update idletasks
}

set selected_player -1

set indepside_up 0

proc add_player_dialog_items {} {
    global num_avail_side_names
    global selected_player
    global indepside_entry firstside_entry
    global boldfont bigfont titlefont stdfont scrollb
    global bgcolor hbcolor abcolor tcolor lcolor fgcolor afcolor
    global indepside_up
    
    set nums [ numsides ]
    set maxs [ maxsides ]
    
    # Add one row for indepside if in game and not hidden.
    if { $indepside_up } {
	set maxs [ expr $maxs + 1 ]
    }
    frame .newgame.top.plbuttons -bg $bgcolor
    pack .newgame.top.plbuttons -side right -padx 4 -pady 3 -fill y -expand true
    
    frame .newgame.top.dummy -bg $bgcolor
    pack .newgame.top.dummy -side top -anchor nw -padx 10 -pady 3
    frame .newgame.top.listheadings -bg $bgcolor
    pack .newgame.top.listheadings -side top -anchor nw -padx 10 -pady 2

    label .newgame.top.listheadings.dummy -bg $bgcolor -fg $fgcolor
    label .newgame.top.listheadings.side -text "Side" -font $titlefont -bg $bgcolor -fg $fgcolor
    label .newgame.top.listheadings.player -text "Player" -font $titlefont -bg $bgcolor -fg $fgcolor
    label .newgame.top.listheadings.aitype -text "AI" -font $titlefont -bg $bgcolor -fg $fgcolor
    label .newgame.top.listheadings.adv -text "Adv" -font $titlefont -bg $bgcolor -fg $fgcolor

    grid  .newgame.top.listheadings.dummy \
    	   .newgame.top.listheadings.side \
    	   .newgame.top.listheadings.player  \
   	   .newgame.top.listheadings.aitype \
   	   .newgame.top.listheadings.adv -sticky w
   	   
    # Enforce the same pixel positions as for the text in sp_entry below.
    grid columnconfig .newgame.top.listheadings 0 -minsize 38 -weight 0
    grid columnconfig .newgame.top.listheadings 1 -minsize 126 -weight 0
    grid columnconfig .newgame.top.listheadings 2 -minsize 84 -weight 0
    grid columnconfig .newgame.top.listheadings 3 -minsize 68 -weight 0
    grid columnconfig .newgame.top.listheadings 4 -minsize 6 -weight 0

    set maxheight [ expr $maxs * (24 + 4 + 2) + 2 ]

    frame .newgame.top.f1 -bg $bgcolor
    pack .newgame.top.f1 -side left -fill both -anchor nw -padx 8

    scrollbar .newgame.top.f1.yscroll -orient vert \
	    -command { .newgame.top.f1.f11.c yview } -width 16 -borderwidth $scrollb \
   	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack .newgame.top.f1.yscroll -side right -fill y

    frame .newgame.top.f1.f11 -borderwidth 2 -relief sunken -bg $lcolor
    pack .newgame.top.f1.f11 -side left -fill both -expand false

    canvas .newgame.top.f1.f11.c -width 360 -height $maxheight \
	    -scrollregion [ list 0 0 360 $maxheight ] \
	    -yscrollcommand { .newgame.top.f1.yscroll set } \
	    -bg $lcolor -highlightbackground $lcolor
    pack .newgame.top.f1.f11.c -side left -fill both

   # Make sure the scrollbar is positioned at the top (this is needed under 
   # MacOS but not under Windows or Unix.
   .newgame.top.f1.f11.c yview moveto 0.0

    frame .newgame.top.f1.f11.c.f2  -bg $lcolor
    .newgame.top.f1.f11.c create window 0 0 -anchor nw \
	    -window .newgame.top.f1.f11.c.f2
	    
    # We make entries for all sides including the indepside, but we
    # choose whether to pack the indepside based on whether it is
    # active in the game or not.
    canvas .newgame.top.f1.f11.c.f2.dummy -width 360 -height -3 -borderwidth 0 \
	     -bg $lcolor -highlightbackground $lcolor
    pack .newgame.top.f1.f11.c.f2.dummy -side top -fill x
    for { set i 0 } { $i <= $maxs } { incr i } {
	set sp_entry .newgame.top.f1.f11.c.f2.s$i
	canvas $sp_entry -width 360 -height 24 -borderwidth 0 \
		 -bg $lcolor -highlightbackground $lcolor
	# Although indicating the current side/player by raising and
	# sinking relief seems good, it's visually confusing in practice;
	# so use a surrounding rect and make it thicker for selected side.
	$sp_entry create rect 32 3 350 23 -tag outline -outline gray
	$sp_entry create text 40 6 -tag side -anchor nw -text "" -font $bigfont -fill $fgcolor
	$sp_entry create text 166 6 -tag player -anchor nw -text "" -font $bigfont -fill $fgcolor
	$sp_entry create text 250 6 -tag aitype -anchor nw -text "" -font $bigfont -fill $fgcolor
	$sp_entry create text 326 6 -tag advantage -anchor nw -text "" -font $bigfont -fill $fgcolor
	imfsample $sp_entry.emblem -width 16 -height 16 -iwidth 16 -iheight 16 -bg $lcolor
	$sp_entry create window 6 6 -window $sp_entry.emblem -anchor nw
	if { $i <= $nums } {
	    $sp_entry itemconfig outline -width 1 -outline $fgcolor
	    update_player_entry $i
	}
	if { $i > 0 } {
	    pack $sp_entry -side top -fill x
	}
	if { $i == 0 } {
	    set indepside_entry $sp_entry
	}
	if { $i == 1 } {
	    set firstside_entry $sp_entry
	}
	bind $sp_entry <Button-1> "select_player $i"
    }
    
    label .newgame.top.plbuttons.advlabel -text "Advantage" -bg $bgcolor -fg $fgcolor -font $boldfont
    grid .newgame.top.plbuttons.advlabel -columnspan 2 -sticky ew -padx 4
    button .newgame.top.plbuttons.aplus -text "+" -state disabled \
	    -command { adjust_advantage_cmd 1 } -width 2 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .newgame.top.plbuttons.aminus -text "-" -state disabled \
	    -command { adjust_advantage_cmd -1 } -width 2 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.aplus .newgame.top.plbuttons.aminus -sticky ew -padx 4 -pady 2

    label .newgame.top.plbuttons.sideslabel -text "Sides" -bg $bgcolor -fg $fgcolor -font $boldfont
    grid .newgame.top.plbuttons.sideslabel -columnspan 2 -sticky ew -padx 4
    button .newgame.top.plbuttons.add -text "Add Side" \
	    -command { add_player_cmd } -width 8 \
 	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
 	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.add -columnspan 2 -sticky ew -padx 4 -pady 2
    button .newgame.top.plbuttons.remove -text "Remove" \
	    -command { remove_side_and_player_cmd } -width 8 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.remove -columnspan 2 -sticky ew -padx 4 -pady 2
    menubutton .newgame.top.plbuttons.sidelib -text "Rename" \
	    -menu .newgame.top.plbuttons.sidelib.menu \
	    -borderwidth 2 -relief raised -width 8 \
	    -bg $bgcolor -highlightbackground $hbcolor \
	    -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    menu .newgame.top.plbuttons.sidelib.menu -tearoff 0 \
 	    -bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.sidelib -columnspan 2 -sticky ew -padx 4 -pady 2
    button .newgame.top.plbuttons.rename -text "Random" \
	    -command { rename_side_for_player_cmd -1 } -width 8 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.rename -columnspan 2 -sticky ew -padx 4 -pady 2

    label .newgame.top.plbuttons.playerslabel -text "Players" -bg $bgcolor -fg $fgcolor -font $boldfont
    grid .newgame.top.plbuttons.playerslabel -columnspan 2 -sticky ew -padx 4
    button .newgame.top.plbuttons.computer -text "Switch AI" \
	    -command { cycle_ai_cmd } -width 8 -state normal \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.computer -columnspan 2 -sticky ew -padx 4 -pady 2
    button .newgame.top.plbuttons.exchange -text "Exchange" \
	    -command { exchange_players_cmd } -width 8 -state normal \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.exchange -columnspan 2 -sticky ew -padx 4 -pady 2

    label .newgame.top.plbuttons.indeplabel -text "Indep Side" -bg $bgcolor -fg $fgcolor -font $boldfont
    grid .newgame.top.plbuttons.indeplabel -columnspan 2 -sticky ew -padx 4
    button .newgame.top.plbuttons.indepside -text "Activate" \
	    -command { set_indepside toggle } -width 8 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.indepside -columnspan 2 -padx 4 -pady 2 -sticky ew
    button .newgame.top.plbuttons.indepconfig -text "Configure" \
	    -command { popup_indepside_config } -width 8 \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .newgame.top.plbuttons.indepconfig -columnspan 2 -padx 4 -pady 2 -sticky ew

    # Add names to the side library menu, disabling already-used names.
    set numsidelib [ side_lib_size ]
    for { set i 0 } { $i < $numsidelib } { incr i } {
	set colbreak 0
	if { $i % 25 == 0 } {
	    set colbreak 1
	}
	set sstate normal
	if { ![ side_lib_entry_available $i ] } {
	    set sstate disabled
	} else {
	    incr num_avail_side_names
	}
	.newgame.top.plbuttons.sidelib.menu add command \
		-label "[ side_lib_entry $i ]" -state $sstate \
		-command [ list rename_side_for_player_cmd $i ] \
		-columnbreak $colbreak
    }
    if { $num_avail_side_names == 0 } {
	.newgame.top.plbuttons.sidelib config -state disabled
    } else {
	.newgame.top.plbuttons.sidelib config -state normal    
    }
    if { !"[ side_ingame 0 ]" } {
	.newgame.top.plbuttons.indepside config -state disabled
	.newgame.top.plbuttons.indepconfig config -state disabled
    }
    set_indepside initial

    select_player 1
    update_allplayer_buttons
}

proc adjust_advantage_cmd { amt } {
    global selected_player

    adjust_advantage $selected_player $amt
    # Re-select so buttons get updated.
    select_player $selected_player
    update_player_entry $selected_player
}

proc add_player_cmd {} {
    set newsel [ add_side_and_player ]
    if { $newsel < 0 } {
	return
    }
    update_player_entry $newsel
    select_player $newsel
    update_allplayer_buttons
}

 proc remove_side_and_player_cmd {} {
    global selected_player
    
    remove_side_and_player $selected_player
    set nums [ numsides ]
    set maxs [ maxsides ]
    set newsel $selected_player
    if { $newsel > $nums } {
	set newsel $nums
    }
    for { set i $selected_player } { $i <= $nums } { incr i } {
    	update_player_entry $i
    }
    select_player $newsel
    update_allplayer_buttons

    # Finally erase the former last entry.
    set oldlast [ expr $nums + 1 ]
    if { $oldlast > $maxs } {
        return
    }
    set sp_entry .newgame.top.f1.f11.c.f2.s$oldlast
    $sp_entry.emblem replace imf "null"
    $sp_entry itemconfig outline -outline gray
    $sp_entry itemconfig side -text ""
    $sp_entry itemconfig player -text ""
    $sp_entry itemconfig aitype -text ""
    $sp_entry itemconfig advantage -text ""
}

proc rename_side_for_player_cmd { n } {
    global selected_player

    rename_side_for_player $selected_player $n
    update_player_entry $selected_player
}

proc cycle_ai_cmd {} {
    global selected_player

    set_ai_for_player $selected_player -cycle
    update_player_entry $selected_player
}

proc exchange_players_cmd {} {
    global selected_player

    set newsel [ exchange_players $selected_player -1 ]
    if { $newsel >= 0 } {
	update_player_entry $selected_player
	update_player_entry $newsel
	select_player $newsel
    }
}

proc select_player { newsel } {
    global num_avail_side_names
    global selected_player
    global fgcolor

    set nums [ numsides ]
    set mins [ minsides ]

    # De-highlight any previous selection.
    if { $selected_player != -1 } {
	set sp_entry .newgame.top.f1.f11.c.f2.s$selected_player
	$sp_entry itemconfig outline -width 1 -outline $fgcolor
    }
    if { $newsel <= $nums } {
	set emptysel 0
	set selected_player $newsel
	set sp_entry .newgame.top.f1.f11.c.f2.s$selected_player
	$sp_entry itemconfig outline -width 2 -outline $fgcolor
	set side [ assigned_side $selected_player ]
	set player [ assigned_player $selected_player ]
    } else {
	set emptysel 1
    }

    # Enable/disable advantage adjustment.
    if { !$emptysel && "[ player_advantage $player ]" < "[ max_advantage $side ]"} {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.aplus config -state $state
    if { !$emptysel && "[ player_advantage $player ]" > "[ min_advantage $side ]"} {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.aminus config -state $state
    
    # Disable the Remove button for indepside and human players, 
    # if minimum number of sides or if no player is selected.
    if { $emptysel } {
	set state disabled
    } else {
	set displayname [ player_displayname $player ]
	if { $side != 0 && $displayname == "" && $nums > $mins } {
	        set state normal
	} else {
	        set state disabled
	}
    }
    .newgame.top.plbuttons.remove config -state $state

    if { !$emptysel && "[ can_rename $side ]" } {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.sidelib config -state $state
    .newgame.top.plbuttons.rename config -state $state
    # Turn off the popup menu if it is empty.
    if { $num_avail_side_names <= 0 } {
    	.newgame.top.plbuttons.sidelib config -state disabled
    }
    
    # The other buttons are always active unless nothing selected.
    if { !$emptysel } {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.computer config -state $state
    .newgame.top.plbuttons.exchange config -state $state
}

# Given the index of an assignment, update the side and player info at
# that index.

proc update_player_entry { i } {
    global selected_player
    global fgcolor
    
    # Make sure we have a valid selected player
    set nums [ numsides ]
    if { $selected_player > $nums } {
    	select_player $nums    
    }
    # Nothing to do if command-line setup.
    if { !"[ winfo exists .newgame ]" } {
	return
    }
    set sp_entry .newgame.top.f1.f11.c.f2.s$i
    set side [ assigned_side $i ]
    if { $side == "?s?" } {
   	$sp_entry.emblem replace imf "null"
    	$sp_entry itemconfig outline -outline gray
    	$sp_entry itemconfig side -text ""
    	$sp_entry itemconfig player -text ""
    	$sp_entry itemconfig aitype -text ""
    	$sp_entry itemconfig advantage -text ""
    	return
    }
    # Ersatz images don't exist yet, so skip if none found.
    set ename [ side_emblem $side ]
    if { "$ename" != "null" } {
	$sp_entry.emblem replace imf $ename
    }
    set ingame [ side_ingame $side ]
    if { $ingame } {
	set color $fgcolor
	$sp_entry itemconfig outline -outline $fgcolor
    } else {
	set color gray
	$sp_entry itemconfig outline -outline gray
    }
    $sp_entry itemconfig side -text [ short_side_title $side ] -fill $color
    set player [ assigned_player $i ]
    $sp_entry itemconfig player -text [ simple_player_name $player ] -fill $color
    $sp_entry itemconfig aitype -text [ player_aitypename $player ] -fill $color
    set advantage [ player_advantage $player ]
    if { $advantage > 0 } {
	$sp_entry itemconfig advantage -text $advantage
    }
    if { "[ winfo exists .newgame.top.plbuttons.sidelib.menu ]" } {
	set numsidelib [ side_lib_size ]
	for { set i 0 } { $i < $numsidelib } { incr i } {
	    set sstate normal
	    if { ![ side_lib_entry_available $i ] } {
		set sstate disabled
	    }
	    .newgame.top.plbuttons.sidelib.menu entryconfig $i -state $sstate
	}
    }
}

# Set the state of buttons that affect the whole list of side/players.

proc update_allplayer_buttons {} {
    global selected_player

    # Nothing to do if command-line setup.
    if { !"[ winfo exists .newgame ]" } {
	return
    }
    set nums [ numsides ]
    set maxs [ maxsides ]
    set mins [ minsides ]

    if { $nums < $maxs } {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.add config -state $state

    set side [ assigned_side $selected_player ]
    set player [ assigned_player $selected_player ]
    set displayname [ player_displayname $player ]
    if { $side != 0 && $displayname == "" && $nums > $mins } {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.remove config -state $state

    if { $nums > 1 } {
	set state normal
    } else {
	set state disabled
    }
    .newgame.top.plbuttons.exchange config -state $state
}

# No removal needed for player dialog items, this is the last dialog
# in the sequence.

proc set_players {} {
    # Might have called during a command-line startup sequence...
    if { !"[ winfo exists .newgame ]" } {
	return
    }
    launch_game
}

proc set_indepside { mode } {
    global indepside_entry firstside_entry
    global indepside_up indepside_player

    set maxs [ maxsides ]

    if { [ side_ingame 0 ] } {
	set indepplayer [ assigned_player indepside ]
	# First test if indepside has a human player.
	set indepside_player [ player_displayname $indepplayer ]
	# If not, test if it has an AI player.
	if { "$indepside_player" == "" } {
		set indepside_player [ player_aitypename $indepplayer ]
	}
    } else {
	set indepside_player ""
    }
    # Decide whether indepside player entry should be "up" or "down".
    if { $mode == "initial" } {
	if { "$indepside_player" != "" } {
	    set indepside_up 1
	}
    } else {
	if { !$indepside_up } {
	    set indepside_up 1
	} else {
	    set indepside_up 0
	}
    }
    if { $indepside_up } {
	# Add indepside to player list if not hidden.
	set maxs [ expr $maxs + 1 ]
	set_indepside_options 1
	# Add the indepside entry
	update_player_entry 0
	pack $indepside_entry -before $firstside_entry -side top -fill x
	.newgame.top.plbuttons.indepside config -text "Deactivate"
    } else {
	# Make the indepside entry disappear
	set_indepside_options 0
	update_player_entry 0
	pack forget $indepside_entry
	.newgame.top.plbuttons.indepside config -text "Activate"
    }
    # Recompute the length of the player list.
    set maxheight [ expr $maxs * (24 + 4 + 2) + 2 ]
    .newgame.top.f1.f11.c config -width 360 -height $maxheight \
	    -scrollregion [ list 0 0 360 $maxheight ]
}

proc popup_indepside_config {} {
    global indepside_act indepside_build indepside_research \
	    indepside_economy indepside_treasury boldfont
    global new_indepside_act new_indepside_build new_indepside_research \
	    new_indepside_economy new_indepside_treasury
    global bgcolor hbcolor abcolor scolor fgcolor afcolor

    set new_indepside_act $indepside_act
    set new_indepside_build $indepside_build
    set new_indepside_research $indepside_research
    set new_indepside_economy $indepside_economy
    set new_indepside_treasury $indepside_treasury

    if { "[ winfo exists .indepside ]" } {
    	# Kick the window manager awake.
    	wm withdraw .indepside
	wm deiconify .indepside
	focus -force .indepside
	return
    }

    toplevel .indepside -bg $bgcolor -width 300 -height 200
    pack propagate .indepside true
    wm title .indepside "Independent Units Setup"
    wm withdraw .indepside

    label .indepside.lab1 -text "Choose what independent units may do:" \
    -bg $bgcolor -fg $fgcolor -font $boldfont
    pack .indepside.lab1 -side top -pady 8 -padx 6
    checkbutton .indepside.act -text " Act with any units" \
	    -variable new_indepside_act -offvalue 0 -onvalue 1 \
	    -bg $bgcolor -highlightbackground $bgcolor \
	    -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor \
	    -font $boldfont
    pack .indepside.act -side top -anchor nw -padx 30 -pady 4
    checkbutton .indepside.build -text " Build new units if possible" \
	    -variable new_indepside_build -offvalue 0 -onvalue 1 \
	    -bg $bgcolor -highlightbackground $bgcolor \
	    -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor \
	    -font $boldfont
    pack .indepside.build -side top -anchor nw -padx 30 -pady 4
    checkbutton .indepside.research -text " Research new advances" \
	    -variable new_indepside_research -offvalue 0 -onvalue 1 \
	    -bg $bgcolor -highlightbackground $bgcolor \
	    -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor \
	    -font $boldfont
    pack .indepside.research -side top -anchor nw -padx 30 -pady 4
    checkbutton .indepside.economy -text " Produce and consume materials" \
	    -variable new_indepside_economy -offvalue 0 -onvalue 1 \
	    -bg $bgcolor -highlightbackground $bgcolor \
	    -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor \
	    -font $boldfont
    pack .indepside.economy -side top -anchor nw -padx 30 -pady 4
    checkbutton .indepside.treasury -text " Use treasuries if available" \
	    -variable new_indepside_treasury -offvalue 0 -onvalue 1 \
	    -bg $bgcolor -highlightbackground $bgcolor \
	    -activebackground $abcolor \
	    -selectcolor $scolor -fg $fgcolor -activeforeground $afcolor \
	    -font $boldfont
    pack .indepside.treasury -side top -anchor nw -padx 30 -pady 4

    frame .indepside.bottom -bg $bgcolor
    pack .indepside.bottom -side bottom -anchor e -padx 20 -pady 4
    button .indepside.bottom.ok -text "OK" -width 8 \
	    -command { ok_indepside } -default active \
	    -bg $bgcolor -highlightbackground $hbcolor \
	    -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .indepside.bottom.cancel -text "Cancel" -width 8 \
	    -command { cancel_indepside } \
	    -bg $bgcolor -highlightbackground $hbcolor \
	    -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .indepside.bottom.cancel .indepside.bottom.ok -sticky ew \
	    -padx 6 -pady 4
    
    if { [ numatypes ] == 0 } {
    	.indepside.research config -state disabled
    }
    if { [ nummtypes ] == 0 } {
    	.indepside.treasury config -state disabled
    }
    if { [ numtreasury ] == 0 } {
    	.indepside.treasury config -state disabled
    }
    bind .indepside <Key> {  
    	if {"%K" == "Escape"} { 
		.indepside.bottom.cancel flash
		cancel_indepside 
	} elseif  {"%K" == "Return"} {
		.indepside.bottom.ok flash
		ok_indepside
	}
    }
    center_on_screen .indepside
}

proc ok_indepside {} {
    global indepside_act indepside_build indepside_research \
	    indepside_economy indepside_treasury
    global new_indepside_act new_indepside_build new_indepside_research \
	    new_indepside_economy new_indepside_treasury

    set indepside_act $new_indepside_act
    set indepside_build $new_indepside_build
    set indepside_research $new_indepside_research
    set indepside_economy $new_indepside_economy
    set indepside_treasury $new_indepside_treasury

    set_indepside_options $indepside_act $indepside_build \
	    $indepside_research $indepside_economy $indepside_treasury
    wm withdraw .indepside
}

proc cancel_indepside {} {
    wm withdraw .indepside
}

# Launch the periodically-executing procedures that will run during
# the game.

proc do_initial_setup {} {
    after 25 run_game_cmd
    after 50 animate_selection_cmd
    after 100 run_game_idle_cmd
}

proc popup_connect {} {
    popup_chat
}

proc join_game {} {
    global prefs

    make_default_player_spec $prefs(myname)
    set rslt [ try_join_game "$prefs(joinhost):$prefs(joinport)" ]
    if { $rslt == -2 } {
	bell
	insert_chat_string 0 0 "Cannot join ongoing game at $prefs(joinhost):$prefs(joinport)\n"
    } elseif { $rslt == 0 } {
	bell
	insert_chat_string 0 0 "No response from host at $prefs(joinhost):$prefs(joinport)\n"
    # Turn off the New Game and Open Game buttons and their key bindings
    # if we became a network client.
    } elseif { $rslt == 1 } {
	.newgame.bottom.b1 config -state disabled
	.newgame.bottom.b2 config -state disabled
	bind .newgame <Key> {  
		if {"%K" == "Escape"} { 
			.newgame.bottom.quit flash
			exit_xconq 
		} elseif  {"%K" == "Return"} {
		}
	}
	bind .newgame <n> { }
	bind .newgame <N> { }
	bind .newgame <o> { }
	bind .newgame <O> { }
    }
}

proc host_game {} {
    global prefs

    make_default_player_spec $prefs(myname)
    try_host_game "$prefs(joinhost):$prefs(joinport)"
}

# The pregame chat window may actually last throughout a game - it
# provides an "out-of-band" communications channel for human players.

proc popup_chat {} {
    global persons
    global env
    global debug
    global prefs
    global boldfont
    global stdfont
    global scrollb
    global listb
    global textb
    global bgcolor hbcolor abcolor tcolor lcolor fgcolor afcolor
    global my_rid master_rid
 
    if { "[ winfo exists .chat ]" } {
        	# Kick the window manager awake.
	wm withdraw .chat
	wm deiconify .chat
	focus -force .chat
	return
    }

    toplevel .chat
    wm title .chat "Xconq Chat"
    wm withdraw .chat

    frame .chat.top -bg $bgcolor
    pack .chat.top -side top -fill x

    frame .chat.top.parms -bg $bgcolor
    pack .chat.top.parms -side left -pady 4 -padx 4

    label .chat.top.parms.namelabel -text "Your Name :" -font $boldfont -bg $bgcolor -fg $fgcolor
    label .chat.top.parms.hostlabel -text "Game Host :" -font $boldfont -bg $bgcolor -fg $fgcolor
    label .chat.top.parms.portlabel -text "Port :" -font $boldfont -bg $bgcolor -fg $fgcolor

    grid .chat.top.parms.namelabel -sticky e -column 0 
    grid .chat.top.parms.hostlabel -sticky e -column 0 
    grid .chat.top.parms.portlabel -sticky e -column 0
    
    entry .chat.top.parms.nameentry -textvariable prefs(myname) \
	-relief sunken -width 15 -fg $fgcolor -font $stdfont
    entry .chat.top.parms.hostentry -textvariable prefs(joinhost) \
	-relief sunken -width 15 -fg $fgcolor -font $stdfont
    entry .chat.top.parms.portentry -textvariable prefs(joinport) \
	-relief sunken -width 4 -fg $fgcolor -font $stdfont

    grid .chat.top.parms.nameentry -sticky w -column 1 -row 0 
    grid .chat.top.parms.hostentry -sticky w -column 1 -row 1
    grid .chat.top.parms.portentry -sticky w -column 1 -row 2

    frame .chat.top.buttons -bg $bgcolor
    pack .chat.top.buttons -side bottom -padx 12 -pady 6 -anchor w

    button .chat.top.buttons.host -text "Host Game" -bg $bgcolor \
    	-command { host_game } -font $boldfont -pady 8 -width 10 \
    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
    	-fg $fgcolor -activeforeground $afcolor
    pack .chat.top.buttons.host -side left -padx 3
    button .chat.top.buttons.join -text "Join Game" -bg $bgcolor \
    	 -command { join_game } -font $boldfont -pady 8 -width 10 \
    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
    	-fg $fgcolor -activeforeground $afcolor
    pack .chat.top.buttons.join -side left -padx 3
    button .chat.top.buttons.close -text "Close" -bg $bgcolor \
    	-command { withdraw_window ".chat" } -font $boldfont -pady 8 -width 10 \
    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
    	-fg $fgcolor -activeforeground $afcolor
    pack .chat.top.buttons.close -side left -padx 3
    
    label .chat.top.status -text "Status : Not Connected" -bg $bgcolor -font $boldfont -fg $fgcolor
    pack .chat.top.status -side left -padx 12 -pady 3

    frame .chat.bottom -bg $bgcolor
    pack .chat.bottom -side bottom -fill x

    label .chat.bottom.label -text "Message :" -bg $bgcolor -width 14 -anchor e -font $boldfont -fg $fgcolor
    pack .chat.bottom.label -side left -padx 0 -pady 4 
    entry .chat.bottom.input -width 50 -state disabled -fg $fgcolor -font $stdfont
    pack .chat.bottom.input -side left -padx 22 -pady 4 -fill x -expand true

    frame .chat.mid -width 500 -height 330 -bg $bgcolor
    if { $debug } {
	.chat.mid config -bg green
    }
    pack .chat.mid -side top -fill both -expand true
    pack propagate .chat.mid false

    frame .chat.mid.left -borderwidth 2 -relief sunken -bg $bgcolor
    pack .chat.mid.left -side left -fill y

    label .chat.mid.left.label -text "Players" -bg $bgcolor -anchor w -font $boldfont -fg $fgcolor
    pack .chat.mid.left.label -side top
    listbox .chat.mid.left.persons -width 15 -relief sunken -borderwidth $listb \
    -bg $lcolor -fg $fgcolor -font $stdfont
    pack .chat.mid.left.persons -side left -fill y -expand true

    scrollbar .chat.mid.yscroll -orient vert \
	    -command ".chat.mid.right.t yview" -width 16 -borderwidth $scrollb \
    	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
   pack .chat.mid.yscroll -side right -fill y

    frame .chat.mid.right -borderwidth $textb -relief sunken -bg $lcolor
    pack .chat.mid.right -side right -fill both -expand true

    text .chat.mid.right.t -width 500 -height 500 -wrap word -font $stdfont -relief flat \
	    -yscrollcommand ".chat.mid.yscroll set"  -bg $lcolor -highlightbackground $lcolor -fg $fgcolor
    pack .chat.mid.right.t -side left -fill both -expand true
    # Don't let the user type into this window.
    .chat.mid.right.t config -state disabled
    # Bind Escape to the Close button.
    bind .chat <Key> {  
	if {"%K" == "Escape"} { 
		.chat.top.buttons.close flash
		wm withdraw .chat
	}
    }
    # Finally check if we were already connected and set items as required.
    if { $my_rid > 0 } {
	enable_chat
	.chat.top.buttons.host config -state disabled
	.chat.top.buttons.join config -state disabled
	.chat.top.parms.nameentry config -state disabled
	.chat.top.parms.hostentry config -state disabled
	.chat.top.parms.portentry config -state disabled
	if { $my_rid == $master_rid } {
		.chat.top.status config -text "Status : Accepting Connections"
	} else {
		.chat.top.status config -text "Status : Connected"	
	}
    }    	
    center_on_screen .chat
}

proc add_program { myrid rid str } {
    global persons
    global prefs
    global my_rid master_rid

    set my_rid $myrid
   
    set persons($rid,name) $str

    # -host/-join arg setup does not create chat window.
    if { !"[ winfo exists .chat ]" } {
	return
    }

    # Once the program is accepting connections, or connected, the
    # Host/Join buttons are no longer needed for anything.
    .chat.top.buttons.host config -state disabled
    .chat.top.buttons.join config -state disabled
    # Ditto for the name, host and port.
    .chat.top.parms.nameentry config -state disabled
    .chat.top.parms.hostentry config -state disabled
    .chat.top.parms.portentry config -state disabled
    if { $my_rid > 0 && $my_rid == $master_rid } {
	.chat.top.status config -text "Status : Accepting Connections"
    	# Always reenable the New Game button if .newgame exists.
	.newgame.bottom.b1 config -state normal
	# Only reenable the Open button if we have not yet selected a game.
	if { "[ numutypes ]" == 0 } {
		.newgame.bottom.b2 config -state normal
	}
    } else {
	.chat.top.status config -text "Status : Connected"
    }
    .chat.mid.left.persons insert end "$rid: $persons($rid,name)"
    enable_chat
    if { $rid == 1 } {
	set joinstr "is hosting a game at $prefs(joinhost)."
    } else {
	set joinstr "has joined the game."
    }
    insert_chat_string $myrid 0 "$persons($rid,name) $joinstr\n"
    # Connect button becomes a Chat button.
    if { "[ winfo exists .newgame.bottom.connect ]" } {
	.newgame.bottom.connect config -text "Chat"
    }
}

# This proc should be called when chatting becomes possible.

proc enable_chat {} {
    set cbuf .chat.bottom.input

    $cbuf config -state normal

    bind $cbuf <Key> { if {"%K" == "Return"} { send_chat_bdg ; break } }

    # This is now the main widget of interest in the window.
    focus $cbuf
}

proc send_chat_bdg {} {
    set cbuf .chat.bottom.input

    send_chat "[ $cbuf get ]\n"
    $cbuf delete 0 end
}

proc insert_chat_string { myrid rid str } {
    global persons
    global my_rid

    set cbuf .chat.mid.right.t

    if { !"[ winfo exists $cbuf ]" } {
	return
    }

    $cbuf config -state normal
    if { $rid != 0 } {
	$cbuf insert end "$persons($rid,name): "
    }
    $cbuf insert end "$str"
    # (should only do if already scrolled to end)
    $cbuf yview moveto 1.0
    # Don't let the user type into this window.
    $cbuf config -state disabled
}

proc popup_map { map } {

    if { "[ winfo exists $map ]" } {
	# Kick the window manager awake.
	wm withdraw $map
	wm deiconify $map
	focus -force $map
    }
}

proc close_map { map } {
    global nummaps
    
    # Count the number of open maps.
    set openmaps 0
    for { set i 1 } { $i <= $nummaps } {incr i } {
    	set map2 .m$i
    	if { "[ winfo ismapped $map2 ]" } {
    		incr openmaps
    	}
    }
    # Never close the last open map.
    if { $openmaps > 1 } {
    	wm withdraw $map
    }
}

# Create a map window, which is the main player interface to Xconq.
# The map window includes a map/view, a world map, plus info about
# individual units, lists of sides, unit types, etc.

set nummaps 0

proc create_map_window { mapn } {
    global bigfont boldfont stdfont titlefont hugefont mapfont
    global lineheight
    global utype_icon_size mtype_icon_size
    global nummaps map_widget map_number
    global dblbuffer
    global dside
    global prefs
    global debug
    global tcl_platform
    global scrollb
    global bgcolor hbcolor abcolor tcolor lcolor mbcolor fgcolor ncolor
    global indepside_player

    # For the benefit of users that bypassed the splash screen, we do this once again.
    set stdfont [ list "-family" $prefs(font_family) "-size" $prefs(font_size) ]
    set boldfont [ list "-family" $prefs(font_family) "-size" $prefs(font_size) "-weight" "bold" ]
    if { $prefs(font_size) < 10 } {
	    set bigfont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 1 ] ]
	    set titlefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 1 ] "-weight" "bold" ]
    } else {
	    set bigfont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 2 ] ]
	    set titlefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) + 2 ] "-weight" "bold" ]
    }
    set hugefont [ list "-family" $prefs(font_family) "-size" [ expr $prefs(font_size) * 3 ] "-weight" "bold" ]

    set asc [ font metrics $stdfont -ascent ]
    set dsc [ font metrics $stdfont -descent ]
    set lineheight [ expr $asc + $dsc + 1 ]

    set nums [ numsides ]
    set numscores [ numscorekeepers ]

    incr nummaps

    set map .m$mapn
    toplevel $map
    # Necessary to force focus here since xconq will have lost its focus to
    # Windows if the chat dialog was not up and could receive focus when 
    # the player setup dialog was dismissed.
    focus -force $map

    set map_widget($mapn) $map
    set map_number($map) $mapn

    wm title $map "Xconq Map $mapn"

   # Make sure the map is just withdrawn if the close box is used.
    wm protocol $map WM_DELETE_WINDOW "close_map $map"

    set_options_from_prefs $map

    # Set the main window to take up most of the screen.

    set mainwid [ winfo screenwidth . ]
    set mainhgt [ winfo screenheight . ]

    set mainwid [ expr int(0.95 * $mainwid) ]
    set mainhgt [ expr int(0.90 * $mainhgt) ]

    set geomspec ""
    set geomspec [ append geomspec $mainwid "x" $mainhgt ]

    # Under MacOS and Windows we also need to set the correct offset.
    if { "$tcl_platform(platform)" == "macintosh" } {
    	set geomspec [ append geomspec "+0+20" ]
    }
    if { "$tcl_platform(platform)" == "windows" } {
    	set geomspec [ append geomspec "+0+0" ]
    }
    
    wm geometry $map $geomspec

    create_map_menus $map

    for { set i 1 } { $i <= $nummaps } { incr i } {
    	set map2 .m$i
    	$map.menubar.windows add command -label "Map $i" \
    		-command "popup_map $map2"
    }	

    create_left_right_panes $map 0.80

    # Set up the left side's subpanes.

    create_top_bottom_panes $map.leftside 0.10

    # Notices window.  We don't want tcl to allow user input into this
    # window (thing get confused that way).  Thus, we keep it in
    # -state disabled except when adding text to it.
    frame $map.leftside.topside.notices -borderwidth 1 -bg $bgcolor
    pack $map.leftside.topside.notices -side top -expand true -fill both

    text $map.leftside.topside.notices.t -borderwidth 1 -height 1000 \
	    -font $stdfont \
	    -yscrollcommand "$map.leftside.topside.notices.yscroll set" \
	    -bg $ncolor -highlightbackground $ncolor -fg $fgcolor
    whelp $map.leftside.topside.notices.t "Notices of events and other info"
    scrollbar $map.leftside.topside.notices.yscroll -orient vert \
	    -command "$map.leftside.topside.notices.t yview" -width 16 \
	    -borderwidth $scrollb \
	    -bg $bgcolor -highlightbackground $hbcolor \
	    -activebackground $abcolor -troughcolor $tcolor
    
    whelp $map.leftside.topside.notices.yscroll \
	    "Notices of events and other info"
    pack $map.leftside.topside.notices.yscroll -side right -fill y
    pack $map.leftside.topside.notices.t -side left -fill both -expand true

    frame $map.leftside.botside.buttons -borderwidth 0 -bg $bgcolor
    pack $map.leftside.botside.buttons -side left -fill y

    fill_in_button_box $mapn $map.leftside.botside.buttons

    text $map.leftside.botside.mouseover -borderwidth 1 -height 1 \
	    -font $stdfont -bg $bgcolor -highlightbackground $hbcolor \
		-fg $fgcolor
    whelp $map.leftside.botside.mouseover \
	    "Description of what the mouse is over"
    pack $map.leftside.botside.mouseover -side top -fill x

    frame $map.leftside.botside.uf -borderwidth 0 -bg $bgcolor
    pack $map.leftside.botside.uf -side top -fill x
    
    canvas $map.leftside.botside.uf.unitinfo \
	    -height [ expr 5 * $lineheight + $lineheight / 2 ] -width 2000 \
	    -borderwidth 1 -relief sunken \
	    -bg $bgcolor -highlightbackground $hbcolor	    
    whelp $map.leftside.botside.uf.unitinfo "Details about the current unit"
    pack $map.leftside.botside.uf.unitinfo -side left -fill y -expand true

    frame $map.leftside.botside.mapf -bg $bgcolor
    pack $map.leftside.botside.mapf -side bottom -fill both

    # Ask for a frame larger than the window, so that it's guaranteed to
    # fill up its grid position.
    frame $map.leftside.botside.mapf.mapf2 -width 4000 -height 4000 -bg $mbcolor
    if { $debug } {
	$map.leftside.botside.mapf.mapf2 config -bg green
    }
    pack propagate $map.leftside.botside.mapf.mapf2 false
#    scrollbar $map.leftside.botside.mapf.xscroll -orient horiz \
#	    -command "$map.leftside.botside.mapf.mapf2.map xview" -width 16 -borderwidth $scrollb \
#	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor	    
#    scrollbar $map.leftside.botside.mapf.yscroll -orient vert \
#	    -command "$map.leftside.botside.mapf.mapf2.map yview" -width 16 -borderwidth $scrollb \
#	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor	    
#    grid $map.leftside.botside.mapf.mapf2 $map.leftside.botside.mapf.yscroll -sticky news
#    grid $map.leftside.botside.mapf.xscroll -sticky ew
    grid $map.leftside.botside.mapf.mapf2 -sticky news -padx 1 -pady 1
    grid rowconfigure $map.leftside.botside.mapf 0 -weight 1
    grid columnconfigure $map.leftside.botside.mapf 0 -weight 1

    map $map.leftside.botside.mapf.mapf2.map \
    	    -power $prefs(power) \
    	    -bg $mbcolor \
	    -grid $prefs(grid) \
	    -coverage $prefs(coverage) \
	    -elevations $prefs(elevations) \
	    -lighting $prefs(lighting) \
	    -people $prefs(people) \
	    -control $prefs(control) \
	    -temperature $prefs(temperature) \
	    -winds $prefs(winds) \
	    -clouds $prefs(clouds) \
	    -unitnames $prefs(unit_names) \
	    -featurenames $prefs(feature_names) \
	    -featureboundaries $prefs(feature_boundaries) \
	    -meridians $prefs(meridians) \
	    -meridianinterval $prefs(meridian_interval) \
	    -ai $prefs(ai) \
	    -terrainimages $prefs(terrain_images) \
	    -terrainpatterns $prefs(terrain_patterns) \
	    -transitions $prefs(transitions) \
	    -mainfont $mapfont \
	    -featurefont $mapfont \
	    -dbl $dblbuffer
    pack $map.leftside.botside.mapf.mapf2.map -expand true
    
    adjust_map_font_sizes $map

    # Set up the right side's subpanes.

    set rightwid [ expr (1.0 - 0.75) * $mainwid ]

    # Create the turn/date pane.

    frame $map.rightside.turnf -borderwidth 1 -relief sunken -bg $bgcolor
    pack $map.rightside.turnf -side top -fill x -expand true

    canvas $map.rightside.turnf.turn -height [ expr $lineheight + 4 ] \
    	-bg $bgcolor -highlightbackground $hbcolor
    whelp $map.rightside.turnf.turn "Current turn info"
    pack $map.rightside.turnf.turn -side top -fill x -expand true

    # Create the side list pane.

    frame $map.rightside.gamef -borderwidth 1 -relief sunken -bg $bgcolor
    pack $map.rightside.gamef -side top -fill x -expand true

    # (should be sized later, when actual side elements filled in)

    # We compute this again since things may have changed during the
    # player setup.
    if { [ side_ingame 0 ] } {
	set indepplayer [ assigned_player indepside ]
	# First test if indepside has a human player.
	set indepside_player [ player_displayname $indepplayer ]
	# If not, test if it has an AI player.
	if { "$indepside_player" == "" } {
		set indepside_player [ player_aitypename $indepplayer ]
	}
    } else {
	set indepside_player ""
    }
    set game_entry_height [ expr ( 2 + $numscores ) * $lineheight + 20 ]
    set game_win_height [ expr $nums * $game_entry_height ]
    # Add the indepside if necessary.
    if { "$indepside_player" != "" } {
	set game_win_height [ expr { $game_win_height + $game_entry_height } ]
    }
    set numtreas [ numtreasury ]
    incr game_win_height [ expr (($numtreas + 1) / 2) * $lineheight ]
    set actualheight $game_win_height
    # Limit side list space to 40% of main window height.
    set limitheight [ expr ( $mainhgt * 40 ) / 100 ]
    if { $actualheight > $limitheight } {
	set actualheight $limitheight
    }
    canvas $map.rightside.gamef.game -height $actualheight \
	-scrollregion [ list 0 0 $rightwid $game_win_height ] \
	-yscrollcommand "$map.rightside.gamef.yscroll set" \
    	-bg $bgcolor -highlightbackground $hbcolor
    whelp $map.rightside.gamef.game "List of sides in game"
    scrollbar $map.rightside.gamef.yscroll -orient vert \
	-command "$map.rightside.gamef.game yview" -width 16 -borderwidth $scrollb \
        -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    whelp $map.rightside.gamef.yscroll "List of sides in game"
    pack $map.rightside.gamef.yscroll -side right -fill y
    pack $map.rightside.gamef.game -side left -fill both -expand true

    # Make sure the scrollbar is positioned at the top (this is needed under 
    # MacOS but not under Windows or Unix).
    $map.rightside.gamef.game yview moveto 0.0

#    $map.rightside.gamef.game create line 1 1 10 100 -stipple ants

    # Create the world map pane.

    # A border of 1 is more consistent, but not a good idea until the
    # grip has been changed from a box to a stripe between panes.
    frame $map.rightside.worldf -borderwidth 2 -relief sunken -bg $bgcolor
    pack $map.rightside.worldf -side bottom -fill both -expand true
    frame $map.rightside.worldf.2 -bg $mbcolor
    pack $map.rightside.worldf.2 -side bottom -fill both -expand true

    set pow [ fit_map $rightwid ]
    # Limit world map space to 40% of main window height.
    set limitheight [ expr ( $mainhgt * 40 ) / 100 ]
    map $map.rightside.worldf.2.world -world 1 -power $pow -bg $mbcolor \
	    -maxheight $limitheight -dbl $dblbuffer
    whelp $map.rightside.worldf.2.world "Map of the whole world"
    # The pad here looks a little strange, but it serves two purposes:
    # first, to keep the grip from "leaking" into the world map when
    # scrolling, and second, to distinguish white terrain (such as ice)
    # from the sunken border of the frame.
    pack $map.rightside.worldf.2.world -padx 1 -pady 1

    global last_world_width last_world_power
    set last_world_width $rightwid
    set last_world_power $pow

    # Create the unit type list pane.  This comes last packingwise,
    # since it will usually need to scroll, so it's not so important
    # to give it all the space it would like.

    frame $map.rightside.listf -borderwidth 1 -relief sunken -bg $bgcolor
    pack $map.rightside.listf -side top -expand true -fill x

    set numu [ numutypes_available $dside ]
    set numm [ nummtypes ]
    set utype_entry_height [ expr $utype_icon_size + 4 ]
    set mtype_entry_height [ expr $mtype_icon_size + 4 ]
    set listwinheight [ expr $numu * $utype_entry_height ]
    if { $numm > 0 } {
    	incr listwinheight $utype_entry_height
    	incr listwinheight [ expr $numm * $mtype_entry_height ]
    }
    canvas $map.rightside.listf.unitlist -height $listwinheight \
	    -scrollregion [ list 0 0 $rightwid $listwinheight ] \
	    -yscrollcommand "$map.rightside.listf.yscroll set" \
	    -bg $bgcolor -highlightbackground $hbcolor
    whelp $map.rightside.listf.unitlist "List of unit types"
    scrollbar $map.rightside.listf.yscroll -orient vert \
	    -command "$map.rightside.listf.unitlist yview" -width 16 -borderwidth $scrollb \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor	    
    whelp $map.rightside.listf.yscroll "List of unit types"
    pack $map.rightside.listf.yscroll -side right -fill y
    pack $map.rightside.listf.unitlist -side left -fill both -expand true

    # Make sure the scrollbar is positioned at the top (this is needed under 
    # MacOS but not under Windows or Unix.
    $map.rightside.listf.unitlist yview moveto 0.0

    # Preload widget with tagged text and other items.

    # Pre-tag groups of blank chars for the ranges that we will use for
    # notification and interaction.
    $map.leftside.topside.notices.t insert end " " notices
    $map.leftside.topside.notices.t insert end " " prefix
    $map.leftside.topside.notices.t insert end " " prompt
    $map.leftside.topside.notices.t insert end "  " answer
    # Make the user interaction things stand out more.
    $map.leftside.topside.notices.t tag config prefix -font $boldfont
    $map.leftside.topside.notices.t tag config prompt -font $boldfont
    $map.leftside.topside.notices.t tag config answer -font $boldfont

    set unitinfo $map.leftside.botside.uf.unitinfo
#    imfsample $unitinfo.pic -width 44 -height 48 -iwidth 44 -iheight 48 \
#	-bg $bgcolor
    imfsample $unitinfo.pic -width 32 -height 32 -iwidth 32 -iheight 32 \
	-bg $bgcolor
    $unitinfo create window 4 6 -window $unitinfo.pic -anchor nw
    set col1 [ expr 6 + 44 ]
    set col2 250
    set ypos [ expr $lineheight + $lineheight / 2 ]
    $unitinfo create text $col1 $ypos -tag { handle textual } -fill $fgcolor
    $unitinfo create text $col2 $ypos -tag { hp textual col2 } -fill $fgcolor
    incr ypos $lineheight
    $unitinfo create text $col1 $ypos -tag { loc textual } -fill $fgcolor
    $unitinfo create text $col2 $ypos -tag { stack textual col2 } -fill $fgcolor
    incr ypos $lineheight
    $unitinfo create text $col1 $ypos -tag { occ textual } -fill $fgcolor
    $unitinfo create text $col2 $ypos -tag { s0 textual col2 } -fill $fgcolor
    incr ypos $lineheight
    $unitinfo create text $col1 $ypos -tag { plan textual } -fill $fgcolor
    $unitinfo create text $col2 $ypos -tag { s1 textual col2 } -fill $fgcolor
    incr ypos $lineheight
    $unitinfo create text $col1 $ypos -tag { t0 textual } -fill $fgcolor
    $unitinfo create text $col2 $ypos -tag { aux0 textual col2 } -fill $fgcolor
    incr ypos $lineheight

#    $unitinfo create text $col1 $ypos -tag { t1 textual }
#    $unitinfo create text $col2 $ypos -tag { aux0 textual col2 }

    # Make all the textual unit info items look the same.
    $unitinfo itemconfig textual -anchor sw -font $stdfont

    set turnpane $map.rightside.turnf.turn
    $turnpane create text 4 4 -tag the_date -anchor nw -font $boldfont -fill $fgcolor

    fill_in_side_list $map

    fill_in_unit_type_list $map

    make_normal_bindings $map
}

proc fill_in_button_box { mapn buttonbox } {
    global bgcolor hbcolor abcolor fgcolor afcolor

    button $buttonbox.move -bitmap shoot_cursor \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "survey" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.move "Switch between move and survey modes"
    pack $buttonbox.move -side top
    frame $buttonbox.divider1 -width 24 -height 8 -bg $bgcolor
    pack $buttonbox.divider1 -side top
    button $buttonbox.build -bitmap build \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "produce-unit" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.build "Build a type of unit"
    pack $buttonbox.build -side top
    button $buttonbox.return -bitmap return \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "return" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.return "Return for more supplies"
    pack $buttonbox.return -side top
    button $buttonbox.sleep -bitmap sleep \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "sleep" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.sleep "Sleep indefinitely"
    pack $buttonbox.sleep -side top
    button $buttonbox.reserve -bitmap reserve \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "reserve" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.reserve "Reserve unit for next turn"
    pack $buttonbox.reserve -side top
    button $buttonbox.delay -bitmap delay \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "delay" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.delay "Delay moving unit until others moved this turn"
    pack $buttonbox.delay -side top
    frame $buttonbox.divider2 -width 24 -height 8 -bg $bgcolor
    pack $buttonbox.divider2 -side top
    button $buttonbox.zoomin -bitmap closer \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "zoom-in" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.zoomin "Zoom in"
    pack $buttonbox.zoomin -side top
    button $buttonbox.zoomout -bitmap farther \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "zoom-out" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.zoomout "Zoom out"
    pack $buttonbox.zoomout -side top
    button $buttonbox.iso -bitmap iso \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "map iso" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.iso "Switch between isometric and overhead"
    pack $buttonbox.iso -side top
    button $buttonbox.rotl -bitmap rotl -state disabled \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "map rotl" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.rotl "Rotate view left"
    pack $buttonbox.rotl -side top
    button $buttonbox.rotr -bitmap rotr -state disabled \
	    -width 24 -height 24 \
	    -command [ list execute_long_command $mapn "map rotr" ] \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    whelp $buttonbox.rotr "Rotate view right"
    pack $buttonbox.rotr -side top
}

# This proc adds all the informational elements about each side in the
# side list.

proc fill_in_side_list { map } {
    global stdfont boldfont
    global lineheight
    global dside
    global bgcolor fgcolor
    global indepside_player

    set nums [ numsides ]
    set dside [ dside ]
    set numtreas [ numtreasury ]
    set numscores [ numscorekeepers ]

    # Turn on listing of indepside if either is true.
    if { "$indepside_player" == "" } {
	set firstside 1
    } else {
	set firstside 0
    }
    set sidelist $map.rightside.gamef.game
    set game_entry_height [ expr (2 + $numscores ) * $lineheight + 20 ]

    set sy 0
    for { set i $firstside } { $i <= $nums } { incr i } {
	set tsy [ expr $sy + $lineheight ]
	set tpy [ expr $sy + (2 * $lineheight) ]
	set msy [ expr $sy + floor($lineheight * .60) ]
	set rtop [ expr $sy + (2 * $lineheight) + 4 ]
	set rbot [ expr $sy + (2 * $lineheight) + 12 ]
	if { $i > $firstside } {
	    $sidelist create line 0 [ expr $sy - 4 ] 2000 [ expr $sy - 4 ] -fill gray
	}
	imfsample $sidelist.e$i -width 16 -height 16 -bg $bgcolor
	$sidelist.e$i add imf [ side_emblem $i ]
	$sidelist create window 4 [ expr $sy + 0 ] -window $sidelist.e$i \
		-anchor nw
	$sidelist create text 24 $tsy -tag s$i -anchor sw -font $stdfont -fill $fgcolor
	$sidelist create text 24 $tpy -tag player$i -anchor sw -font $stdfont -fill $fgcolor
	# (should make conditional on liveness of side)
	$sidelist create rect 23 [ expr $rtop + 1 ] 23 $rbot -tag left$i \
		-fill LawnGreen -outline ""
	$sidelist create rect 23 [ expr $rtop + 1 ] 23 $rbot -tag resv$i \
		-fill magenta -outline ""
	$sidelist create rect 23 $rtop 125 $rbot -tag frame$i -outline $fgcolor
	for { set j 1 } { $j <= $numscores } { incr j } {
		set scy [ expr $sy + (2 + $j ) * $lineheight + 15 ]
		$sidelist create text 24 $scy -tag score$j$i -text "" -anchor sw -font $stdfont -fill $fgcolor
	}
	# Start the loss line and victory laurels offscreen. 
	$sidelist create line 4000 $msy 5000 $msy -tag lost$i -fill $fgcolor
	$sidelist create bitmap 4000 [ expr $sy + 22 ] -bitmap laurels -tag won$i -anchor nw
	incr sy $game_entry_height
	# Possibly add treasury materials.
	if { $numtreas > 0 && $i == $dside } {
	    # Make two columns.
	    set rows [ expr ($numtreas + 1) / 2 ]
	    set j 0
	    for { set row 0 } { $row < $rows } { incr row } {
		$sidelist create text 40 [ expr $sy - 4 ] -text m$i,$j -tag m$i,$j -anchor ne -font $stdfont -fill $fgcolor
		$sidelist create text 45 [ expr $sy - 4 ] -text [ mtype_name [ mtype_actual $j ] ] -tag mt$i,$j \
			-anchor nw -font $stdfont -fill $fgcolor
		incr j
		# Only do second column if sufficient treasury materials.
		if { $j < $numtreas } {
		    $sidelist create text 120 [ expr $sy - 4 ] -text m$i,$j -tag m$i,$j -anchor ne -font $stdfont -fill $fgcolor
		    $sidelist create text 125 [ expr $sy - 4 ] -text [ mtype_name [ mtype_actual $j ] ] -tag mt$i,$j \
		    	-anchor nw -font $stdfont -fill $fgcolor
		    incr j
		}
		incr sy $lineheight
	    }
	}
    }

    $sidelist itemconfig s$dside -font $boldfont
}

# This proc adds all the informational elements about each unit type.

proc fill_in_unit_type_list { map } {
    global stdfont boldfont bigfont
    global lineheight
    global utype_icon_size mtype_icon_size
    global dside
    global bgcolor fgcolor

    set unitlist $map.rightside.listf.unitlist

    set sy 1
    set utype_entry_height [ expr $utype_icon_size + 4 ]
    set numu [ numutypes_available $dside ]
    for { set i 0 } { $i < $numu } { incr i } {
	# Compute the y position of text items.
	set tsy [ expr $sy + $utype_icon_size / 2 ]
	imfsample $unitlist.u$i -width 32 -iwidth 32 -height 32 -iheight 32 \
		-bg $bgcolor
#	whelp $unitlist.u$i "name$i"
	$unitlist.u$i add imf [ u_image_name [ utype_actual $i ] ]
	# Add the side emblem as a second image, but don't display it
	# as an image; instead declare as the "emblem".
	$unitlist.u$i add imf [ side_emblem $dside ]
	$unitlist.u$i emblem 1
	$unitlist create window 4 $sy -window $unitlist.u$i -anchor nw
	$unitlist create text [ expr $utype_icon_size + 12 ] $tsy -tag u$i \
		-anchor s -font $stdfont -fill $fgcolor
	$unitlist create text [ expr $utype_icon_size + 24 ] $tsy -tag name$i \
		-anchor sw -font $stdfont -fill $fgcolor
	set tsy [ expr $sy + $utype_icon_size / 2 + $lineheight ]
	$unitlist create text [ expr $utype_icon_size + 42 ] $tsy -tag n$i \
		-anchor se -font $stdfont -fill $fgcolor
	$unitlist create text [ expr $utype_icon_size + 44 ] $tsy -tag i$i \
		-anchor sw -font $stdfont -fill $fgcolor
	set rtop [ expr $sy - 1 ]
	set rbot [ expr $sy + $utype_icon_size + 1 ]
	$unitlist create rect 3 $rtop [ expr $utype_icon_size + 5 ] $rbot \
		-tag rect$i -outline $bgcolor
	incr sy $utype_entry_height
    }
    set mtype_entry_height [ expr $mtype_icon_size + 4 ]
    set numm [nummtypes ]
    if { $numm > 0 } {
	set tsy [ expr $sy + $utype_icon_size / 2 + 6 ]
	$unitlist create text 10 $tsy -tag material -anchor sw -font $boldfont -fill $fgcolor -text "Material:"
	$unitlist create text 110 $tsy -tag supply -anchor se -font $stdfont -fill $fgcolor -text "supply"
	$unitlist create text 145 $tsy -tag production -anchor se -font $stdfont -fill $fgcolor -text "prod"
	$unitlist create text 185 $tsy -tag capacity -anchor se -font $stdfont -fill $fgcolor -text "capac"
	incr sy $utype_entry_height
	for { set i 0 } { $i < $numm } { incr i } {
		set tsy [ expr $sy + $mtype_icon_size / 2  ]
		imfsample $unitlist.m$i -width 16 -iwidth 16 \
		    -height 16 -iheight 16 -bg $bgcolor
		$unitlist.m$i add imf [ m_image_name $i ]
		$unitlist create window 10 [ expr $sy - 8 ] \
		    -window $unitlist.m$i -anchor nw	
		$unitlist create text 30 $tsy -tag material$i \
			-anchor sw -font $stdfont -fill $fgcolor
		$unitlist create text 110 $tsy -tag supply$i \
			-anchor se -font $stdfont -fill $fgcolor
		$unitlist create text 145 $tsy -tag production$i \
			-anchor se -font $stdfont -fill $fgcolor
		$unitlist create text 185 $tsy -tag capacity$i \
			-anchor se -font $stdfont -fill $fgcolor
		incr sy $mtype_entry_height
	}
    }    	
}

# The following collection of flags govern what is enabled and disabled
# in the menus and other controls.  They have to be set/reset each time
# a different unit becomes the current one.

set can_act 0
set can_plan 0
set can_move 0
set can_return 0
set can_build 0
set can_repair 0
set can_change_type 0
set can_attack 0
set can_fire 0
set can_detonate 0
set can_give_take 0
set can_embark 0
set can_disembark 0
set can_disband 0
set can_add_terrain 0
set can_remove_terrain 0

set can_see_people 0
set can_see_control 0
set can_see_elev 0
set can_see_lighting 0
set can_see_temp 0
set can_see_winds 0
set can_see_clouds 0

set map_survey 0

proc set_options_from_prefs { map } {
    global view_option_list
    global prefs
    global map_options

    foreach opt $view_option_list {
	set map_options($map,$opt) $prefs($opt)
    }
}

# Create the complete menu bar for a given map window.

proc create_map_menus { map } {
    global view_option_names
    global map_options
    global map_number
    global tcl_platform
    global bgcolor abcolor scolor fgcolor afcolor micolor
    global dside_ai designer

    set mapn $map_number($map)

    set nums [ numsides ]

    menu $map.menubar -bg $bgcolor -activebackground $abcolor -fg $fgcolor \
  	    -activeforeground $afcolor
    $map config -menu $map.menubar

    # Add apple menu and about box support on the mac.
    if { "$tcl_platform(platform)" == "macintosh" } {
	    $map.menubar add cascade -menu $map.menubar.apple
	    menu $map.menubar.apple -bg $bgcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -tearoff 0
	    $map.menubar.apple add command -label "About Xconq..." -state normal
    }
    $map.menubar add cascade -label "File" -menu $map.menubar.file
    menu $map.menubar.file -postcommand [ list adjust_file_menu $map ] -tearoff 0 \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $map.menubar.file add command -label "New Game..." -state disabled
    $map.menubar.file add command -label "Open Game..." -state disabled
    $map.menubar.file add command -label "Chat" -state disabled \
	    -command { popup_chat }
    $map.menubar.file add separator
    $map.menubar.file add command -label "Close" -state disabled
    $map.menubar.file add command -label "Save Game" -accelerator "S" \
	    -command [ list execute_long_command $mapn "save 0" ]
    $map.menubar.file add command -label "Save Game As" \
	    -command [ list execute_long_command $mapn "save 1" ]
    $map.menubar.file add separator
    $map.menubar.file add command -label "Preferences..." \
	    -command { popup_preferences_dialog }
    $map.menubar.file add separator
    $map.menubar.file add command -label "Print..." -state disabled
    $map.menubar.file add separator
    $map.menubar.file add command -label Resign \
	    -command [ list execute_long_command $mapn "resign" ]
    $map.menubar.file add command -label Quit -accelerator "Q" \
	    -command [ list execute_long_command $mapn "quit" ]

    $map.menubar add cascade -label "Edit" -menu $map.menubar.edit
    menu $map.menubar.edit -postcommand [ list adjust_edit_menu $map ] \
	-tearoff 0 -bg $bgcolor -activebackground $abcolor -fg $fgcolor \
	-activeforeground $afcolor
    $map.menubar.edit add command -label "Can't Undo" -state disabled
    $map.menubar.edit add separator
    $map.menubar.edit add command -label "Cut" -state disabled
    $map.menubar.edit add command -label "Copy" -state disabled
    $map.menubar.edit add command -label "Paste" -state disabled
    $map.menubar.edit add command -label "Clear" -state disabled
    $map.menubar.edit add separator
    $map.menubar.edit add command -label "Select All" -state disabled
    $map.menubar.edit add separator
    $map.menubar.edit add check -label "Design" \
	    -command [ list execute_long_command $mapn "design" ] \
	    -variable designer -offvalue 0 -onvalue 1 -selectcolor $micolor

    $map.menubar add cascade -label "Find" -menu $map.menubar.find
    menu $map.menubar.find -postcommand [ list adjust_find_menu $map ] -tearoff 0 \
 	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $map.menubar.find add command -label "Previous" -state disabled
    $map.menubar.find add command -label "Next" -state disabled
    $map.menubar.find add command -label "Next Occupant" -accelerator "i" \
	    -command [ list execute_long_command $mapn "in" ]
    $map.menubar.find add separator
    $map.menubar.find add command -label "Location..." -state disabled
    $map.menubar.find add command -label "Unit by Name..." \
	    -command [ list popup_find_dialog $mapn ]
    $map.menubar.find add command -label "Distance" -accelerator "\#" \
	    -command [ list execute_long_command $mapn "distance" ]
    $map.menubar.find add separator
    $map.menubar.find add command -label "Current Unit" -accelerator "." \
	    -command [ list execute_long_command $mapn "recenter" ]

    $map.menubar add cascade -label "Orders" -menu $map.menubar.play
    menu $map.menubar.play -postcommand [ list adjust_play_menu $map ] -tearoff 0 \
 	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $map.menubar.play add command -label "Closeup" -state disabled
    $map.menubar.play add command -label "City Dialog" -state disabled
    $map.menubar.play add command -label "Move To" -accelerator "m" \
	    -command [ list execute_long_command $mapn "move-to" ]
    $map.menubar.play add command -label "Return" -accelerator "r" \
	    -command [ list execute_long_command $mapn "return" ]
    $map.menubar.play add command -label "Set Formation" -accelerator "F" \
	    -command [ list execute_long_command $mapn "formation" ]
    $map.menubar.play add separator
    $map.menubar.play add command -label "Wake" -accelerator "w" \
	    -command [ list execute_long_command $mapn "wake" ]
    $map.menubar.play add command -label "Wake All" -accelerator "W" \
	    -command [ list execute_long_command $mapn "wakeall" ]
    $map.menubar.play add command -label "Sleep" -accelerator "s" \
	    -command [ list execute_long_command $mapn "sleep" ]
    $map.menubar.play add command -label "Reserve" \
	    -command [ list execute_long_command $mapn "reserve" ]
    $map.menubar.play add command -label "Idle" -accelerator "I" \
	    -command [ list execute_long_command $mapn "idle" ]
    $map.menubar.play add command -label "Delay" -accelerator "d" \
	    -command [ list execute_long_command $mapn "delay" ]
    $map.menubar.play add separator
    $map.menubar.play add command -label "Build" -accelerator "P" \
	    -command [ list execute_long_command $mapn "produce-unit" ]
    $map.menubar.play add command -label "Repair" \
	    -command [ list execute_long_command $mapn "repair" ]
    $map.menubar.play add command -label "Change Type" -accelerator "c" \
	    -command [ list execute_long_command $mapn "change-type" ]
    $map.menubar.play add separator
    $map.menubar.play add command -label "Attack" -accelerator "a" \
	    -command [ list execute_long_command $mapn "attack" ]
    $map.menubar.play add command -label "Overrun" \
	    -command [ list execute_long_command $mapn "attack" ]
    $map.menubar.play add command -label "Fire" -accelerator "f" \
	    -command [ list execute_long_command $mapn "fire" ]
    $map.menubar.play add command -label "Fire Into" -accelerator "^f" \
	    -command [ list execute_long_command $mapn "fire-into" ]
    $map.menubar.play add command -label "Detonate" -accelerator "\!" \
	    -command [ list execute_long_command $mapn "detonate" ]

    $map.menubar add cascade -label "More Orders" -menu $map.menubar.more
    menu $map.menubar.more -postcommand [ list adjust_more_menu $map ] -tearoff 0 \
 	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $map.menubar.more add command -label "Give" -accelerator "g" \
	    -command [ list execute_long_command $mapn "give" ]
    $map.menubar.more add command -label "Take" -accelerator "t" \
	    -command [ list execute_long_command $mapn "take" ]
    $map.menubar.more add command -label "Collect" \
	    -command [ list execute_long_command $mapn "collect" ]
    $map.menubar.more add separator
    $map.menubar.more add command -label "Embark" -accelerator "e" \
	    -command [ list execute_long_command $mapn "embark" ]
    $map.menubar.more add command -label "Disembark" -accelerator "^e" \
	    -command [ list execute_long_command $mapn "disembark" ]
    $map.menubar.more add separator
    $map.menubar.more add command -label "Disband" -accelerator "D" \
	    -command [ list execute_long_command $mapn "disband" ]
    $map.menubar.more add separator
    $map.menubar.more add command -label "Add Terrain" -accelerator "A" \
	    -command [ list execute_long_command $mapn "add-terrain" ]
    $map.menubar.more add command -label "Remove Terrain" -accelerator "R" \
	    -command [ list execute_long_command $mapn "remove-terrain" ]
    $map.menubar.more add separator
    $map.menubar.more add cascade -label "Plan Type" \
	    -menu $map.menubar.more.plantype -state disabled
    menu $map.menubar.more.plantype -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    $map.menubar.more.plantype add command -label "None" \
	    -command [ list execute_long_command $mapn "map plan-none" ]
    $map.menubar.more.plantype add command -label "Passive" \
	    -command [ list execute_long_command $mapn "map plan-passive" ]
    $map.menubar.more.plantype add command -label "Defensive" \
	    -command [ list execute_long_command $mapn "map plan-defensive" ]
    $map.menubar.more.plantype add command -label "Exploratory" \
	    -command [ list execute_long_command $mapn "map plan-exploratory" ]
    $map.menubar.more.plantype add command -label "Offensive" \
	    -command [ list execute_long_command $mapn "map plan-offensive" ]
    $map.menubar.more.plantype add command -label "Random" \
	    -command [ list execute_long_command $mapn "map plan-random" ]
    $map.menubar.more add command -label "AI Control" \
	    -command [ list execute_long_command $mapn "auto" ]
    $map.menubar.more add separator
    $map.menubar.more add command -label "Rename..." \
	    -command [ list execute_long_command $mapn "name" ]
    $map.menubar.more add cascade -label "Give Unit" \
	    -menu $map.menubar.more.giveunit
    menu $map.menubar.more.giveunit -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    for { set i 0 } { $i <= $nums } { incr i } {
	$map.menubar.more.giveunit add command -label [ side_adjective $i ] \
		-command [ list execute_long_command 0 "$i give-unit" ]
    }

    $map.menubar add cascade -label "Side" -menu $map.menubar.side
    menu $map.menubar.side -postcommand [ list adjust_side_menu $map ] -tearoff 0 \
 	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $map.menubar.side add command -label "Closeup" -state disabled
    $map.menubar.side add command -label "End This Turn" \
	    -command [ list execute_long_command $mapn "end-turn" ]
    $map.menubar.side add separator
    $map.menubar.side add radio -label "Move Mode" -accelerator "z" \
	    -command [ list execute_long_command $mapn "survey" ] \
	    -variable map_survey -value 0 -selectcolor $micolor
    $map.menubar.side add radio -label "Survey Mode" -accelerator "z" \
	    -command [ list execute_long_command $mapn "survey" ] \
	    -variable map_survey -value 1 -selectcolor $micolor
    $map.menubar.side add separator
    $map.menubar.side add cascade -label "AI" \
	    -menu $map.menubar.side.ai
    menu $map.menubar.side.ai -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    $map.menubar.side.ai add radio -label "None" \
	    -command [ list execute_long_command $mapn "ai none" ] \
	    -variable dside_ai -value "none" -selectcolor $micolor
    $map.menubar.side.ai add radio -label "Mplayer" \
	    -command [ list execute_long_command $mapn "ai mplayer" ] \
	    -variable dside_ai -value "mplayer" -selectcolor $micolor
#    $map.menubar.side.ai add radio -label "Iplayer" \
#	    -command [ list execute_long_command $mapn "ai iplayer" ] \
#	    -variable dside_ai -value "iplayer" -selectcolor $micolor
    $map.menubar.side add check -label "AI May Resign" \
	    -state active \
	    -command { toggle_ai_may_resign } \
	    -variable ai_may_resign -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.side add separator
    $map.menubar.side add command -label "Doctrines" -state disabled
    $map.menubar.side add separator
    $map.menubar.side add command -label "Message" -accelerator "M" \
	    -command [ list execute_long_command $mapn "message" ]
# $map.menubar.side add command -label "Agreements" \
#	    -command { create_agreements_window }

    $map.menubar add cascade -label "Windows" -menu $map.menubar.windows
    menu $map.menubar.windows -postcommand [ list adjust_windows_menu $map ] \
 	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor \
	-tearoff 0
    $map.menubar.windows add command -label "Scores" \
	    -command { popup_scores }
    $map.menubar.windows add command -label "Research" \
	    -command { popup_research_dialog }
    $map.menubar.windows add separator
    $map.menubar.windows add command -label "New Map" \
	    -command [ list execute_long_command $mapn "new-map" ]
    $map.menubar.windows add separator

    $map.menubar add cascade -label "View" -menu $map.menubar.view
    menu $map.menubar.view -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    $map.menubar.view add command -label "Recenter" -accelerator "." \
	    -command [ list execute_long_command $mapn "recenter" ]
    $map.menubar.view add command -label "Closer" -accelerator "\}" \
	    -command [ list execute_long_command $mapn "zoom-in" ]
    $map.menubar.view add command -label "Farther" -accelerator "\{" \
	    -command [ list execute_long_command $mapn "zoom-out" ]
    $map.menubar.view add separator
    $map.menubar.view add check -label $view_option_names(grid) \
	    -command [ list set_map_view_option $map grid ] \
	    -variable map_options($map,grid) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(coverage) \
	    -command [ list set_map_view_option $map coverage ] \
	    -variable map_options($map,coverage) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(elevations) \
	    -command [ list set_map_view_option $map elevations ] \
	    -variable map_options($map,elevations) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add command -label "Contour Interval" \
	    -command [ list popup_contour_interval_dialog $map ]
    $map.menubar.view add check -label $view_option_names(lighting) \
	    -command [ list set_map_view_option $map lighting ] \
	    -variable map_options($map,lighting) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(people) \
	    -command [ list set_map_view_option $map people ] \
	    -variable map_options($map,people) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(control) \
	    -command [ list set_map_view_option $map control ] \
	    -variable map_options($map,control) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add cascade -label "Weather" \
	    -menu $map.menubar.view.weather
    menu $map.menubar.view.weather -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    $map.menubar.view.weather add check -label $view_option_names(temperature) \
	    -command [ list set_map_view_option $map temperature ] \
	    -variable map_options($map,temperature) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view.weather add check -label $view_option_names(winds) \
	    -command [ list set_map_view_option $map winds ] \
	    -variable map_options($map,winds) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view.weather add check -label $view_option_names(clouds) \
	    -command [ list set_map_view_option $map clouds ] \
	    -variable map_options($map,clouds) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(unit_names) \
	    -command [ list set_map_view_option $map unit_names ] \
	    -variable map_options($map,unit_names) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(feature_names) \
	    -command [ list set_map_view_option $map feature_names ] \
	    -variable map_options($map,feature_names) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(feature_boundaries) \
	    -command [ list set_map_view_option $map feature_boundaries ] \
	    -variable map_options($map,feature_boundaries) \
	    -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add check -label $view_option_names(meridians) \
	    -command [ list set_map_view_option $map meridians ] \
	    -variable map_options($map,meridians) -offvalue 0 -onvalue 1 -selectcolor $micolor
    $map.menubar.view add cascade -label $view_option_names(meridian_interval) \
	    -menu $map.menubar.view.mi
    menu $map.menubar.view.mi -bg $bgcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -tearoff 0
    $map.menubar.view.mi add radio -label "15 min" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 15 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "30 min" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 30 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "1 deg" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 60 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "2 deg" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 120 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "5 deg" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 300 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "10 deg" \
	    -command [ list set_map_view_option $map meridian_interval ] \
	    -variable map_options($map,meridian_interval) -value 600 -selectcolor $micolor
    $map.menubar.view.mi add radio -label "Other..." \
	    -command [ list popup_meridian_interval_dialog $mapn ] \
	    -variable map_options($map,meridian_interval) -value 1
    $map.menubar.view add check -label $view_option_names(ai) \
	    -command [ list set_map_view_option $map ai ] \
	    -variable map_options($map,ai) -offvalue 0 -onvalue 1 -selectcolor $micolor

    # Disable the help system on MacOS < 8.6 since it crashes there.
    if { "$tcl_platform(platform)" != "macintosh" || $tcl_platform(osVersion) >= 8.6 } {
	$map.menubar add cascade -label "Help" -menu $map.menubar.help
	menu $map.menubar.help -bg $bgcolor -activebackground $abcolor \
		    -fg $fgcolor -activeforeground $afcolor -tearoff 0
	$map.menubar.help add command -label "Xconq Help" -accelerator "?" \
	   		-command { popup_help_dialog }

   	init_help_menu $mapn
    }

    # Chop up the help menu if necessary (workaround for clunky 
    # menu support on Unix systems).
    if { "$tcl_platform(platform)" == "unix" } {
	set menu $map.menubar.help
	# Get the number of menu items.
	set numitems [ $menu index end ]
	# Get the max menu height in pixels (with some margin).
	set ymax [ expr [ lindex [ wm maxsize $menu ] 1 ] - 100 ]
	# Get the height of one item in pixels (Tcl should provide this).
	set spacing [ expr [ $menu yposition 2 ] - [ $menu yposition 1 ] ]
	# Get the number of columns (rounded down).
	set columns [ expr $numitems * $spacing / $ymax ]
	# Compute the protruding rest, if any.
	set rest [ expr $numitems * $spacing % $ymax ]
	# We have a rest.
	if { $rest > 0 } {
		# We need one more column.
		set columns [ expr $columns + 1 ]
		# Recompute ymax so that the columns are filled up.
		set ymax [ expr $numitems * $spacing / $columns ]
	}
	# Finally chop up the menu.
	for { set i 1 } { $i < $numitems } { incr i } {
		set ypos [ $menu yposition $i ]
		if { $ypos > $ymax } {
			$menu entryconfig $i -columnbreak 1
		}
	} 
    }
    
    # Add command key shortcuts on the Mac.
    if { "$tcl_platform(platform)" == "macintosh" } {

	$map.menubar.file entryconfig "New Game..." -accel Command-N
	$map.menubar.file entryconfig "Open Game..." -accel Command-O
	$map.menubar.file entryconfig Close -accel Command-W
	$map.menubar.file entryconfig "Save Game" -accel Command-S
	$map.menubar.file entryconfig Quit -accel Command-Q

	$map.menubar.edit entryconfig "Select All" -accel Command-A
	
	$map.menubar.find entryconfig "Current Unit" -accel Command-F
	
	$map.menubar.play entryconfig Closeup -accel Command-C
	$map.menubar.play entryconfig Return -accel Command-R
	$map.menubar.play entryconfig Build -accel Command-P
	$map.menubar.play entryconfig Attack -accel Command--
	$map.menubar.play entryconfig Detonate -accel Command-=

	$map.menubar.more entryconfig Disband -accel Command-K

	$map.menubar.side entryconfig "End This Turn" -accel Command-E
	
	$map.menubar.windows entryconfig Scores -accel Command-1
	$map.menubar.windows entryconfig Research -accel Command-2
	$map.menubar.windows entryconfig "New Map" -accel Command-M

    }

    global may_set_show_all
    global show_all_was_enabled

    set show_all_was_enabled($map) 0

    if { $may_set_show_all } {
	add_show_all_item $map
    }

    adjust_view_menu $map
}

# Given a help topic key, add it to the menu.  This is called
# from C code.

proc add_help_topic_menu { num key } {
    global tcl_platform

    set map .m$num
    $map.menubar.help add command -label "$key" \
	-command [ list get_help_on_topic "$key" ]
}

proc get_help_on_topic { key } {
    popup_help_dialog
    help_goto "$key"
}

proc add_show_all_item { map } {
    global show_all_was_enabled
    global map_number
    global micolor

    $map.menubar.view add check -label "Show All" \
	    -command [ list execute_long_command $map_number($map) "map show-all" ] \
	    -variable map_options($map,show_all) -offvalue 0 -onvalue 1 -selectcolor $micolor
    set show_all_was_enabled($map) 1
}

proc set_map_view_option { map opt } {
    global view_option_flags
    global map_options

    $map.leftside.botside.mapf.mapf2.map config $view_option_flags($opt) $map_options($map,$opt)
}

proc adjust_file_menu { map } {
    global my_rid
    global endofgame
    global designer

    adjust_menu_entry $map file "Chat" [ expr ($my_rid > 0) && !$endofgame ]
    adjust_menu_entry $map file "Save Game" [ expr !$endofgame ]
    adjust_menu_entry $map file "Save Game As" [ expr !$endofgame ]
    adjust_menu_entry $map file "Resign" [ expr !$endofgame && !$designer ]
}

proc adjust_edit_menu { map } {
    global endofgame

    adjust_menu_entry $map edit "Design" [ expr !$endofgame ]
}

# Enable/disable things on the find menu.

proc adjust_find_menu { map } {
    global curunit
    global can_act can_plan can_move can_return can_embark can_disembark
    global can_build can_repair can_change_type can_disband
    global can_add_terrain can_remove_terrain
    global can_give_take 
    global can_attack can_fire can_detonate

    # (should disable if no next occ)
    adjust_menu_entry $map find "Next Occupant" [ expr $curunit ]
    adjust_menu_entry $map find "Distance" [ expr $curunit ]
    adjust_menu_entry $map find "Current Unit" [ expr $curunit ]
}

# Enable/disable things on the play menu.

proc adjust_play_menu { map } {
    global curunit
    global can_act can_plan can_move can_return can_embark can_disembark
    global can_build can_repair can_change_type can_disband
    global can_add_terrain can_remove_terrain
    global can_give_take 
    global can_attack can_fire can_detonate

    adjust_menu_entry $map play "Move To" [ expr $curunit && $can_move ]
    adjust_menu_entry $map play "Return" [ expr $curunit && $can_return ]
    adjust_menu_entry $map play "Set Formation" [ expr $curunit && $can_move ]
    adjust_menu_entry $map play "Wake" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Wake All" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Sleep" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Reserve" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Idle" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Delay" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map play "Build" [ expr $curunit && $can_build ]
    adjust_menu_entry $map play "Repair" [ expr $curunit && $can_repair ]
    adjust_menu_entry $map play "Change Type" [ expr $curunit && $can_change_type ]
    adjust_menu_entry $map play "Attack" [ expr $curunit && $can_attack ]
    adjust_menu_entry $map play "Overrun" [ expr $curunit && $can_move && $can_attack ]
    adjust_menu_entry $map play "Fire" [ expr $curunit && $can_fire ]
    adjust_menu_entry $map play "Fire Into" [ expr $curunit && $can_fire ]
    adjust_menu_entry $map play "Detonate" [ expr $curunit && $can_detonate ]
}

# Enable/disable things on the more menu.

proc adjust_more_menu { map } {
    global curunit
    global can_act can_plan can_move can_return can_embark can_disembark
    global can_build can_repair can_change_type can_disband
    global can_add_terrain can_remove_terrain
    global can_give_take 
    global can_attack can_fire can_detonate
    global endofgame

    adjust_menu_entry $map more "Give" [ expr $curunit && $can_give_take ]
    adjust_menu_entry $map more "Take" [ expr $curunit && $can_give_take ]
    adjust_menu_entry $map more "Collect" [ expr $curunit && $can_move && $can_give_take ]
    adjust_menu_entry $map more "Embark" [ expr $curunit && $can_embark ]
    adjust_menu_entry $map more "Disembark" [ expr $curunit && $can_disembark ]
    adjust_menu_entry $map more "Disband" [ expr $curunit && $can_disband ]
    adjust_menu_entry $map more "Add Terrain" [ expr $curunit && $can_add_terrain ]
    adjust_menu_entry $map more "Remove Terrain" [ expr $curunit && $can_remove_terrain ]
#	This kills the submenu.
#   adjust_menu_entry $map more "Plan Type" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map more "AI Control" [ expr $curunit && $can_plan ]
    adjust_menu_entry $map more "Rename..." [ expr !$endofgame ]
#	This kills the submenu.
#    adjust_menu_entry $map more "Give Unit" [ expr !$endofgame ]
}

# Enable/disable things on the side menu.

proc adjust_side_menu { map } {
    global endofgame

    adjust_menu_entry $map side "End This Turn" [ expr !$endofgame ]
    adjust_menu_entry $map side "Move Mode" [ expr !$endofgame ]
    adjust_menu_entry $map side "Survey Mode" [ expr !$endofgame ]
#	This kills the submenu.
#    adjust_menu_entry $map side "AI" [ expr !$endofgame ]
}

# Enable/disable things on the windows menu.

proc adjust_windows_menu { map } {
    global endofgame nummaps
    
    set numa [ numatypes ]
    adjust_menu_entry $map windows "Research" [ expr !$endofgame && $numa ]
    # Alternatively, we could force newly-created maps to come up
    # in survey mode, but it seems unlikely that players would
    # ever have much use for creating new maps after the game is over.
    adjust_menu_entry $map windows "New Map" [ expr !$endofgame ]
    # Update the number of maps. First get the number of menu items (minus 1).
    set numitems [ $map.menubar.windows index end ]
    # Substract 4 to get the number of listed maps.
    set mapitems [ expr ( $numitems - 4 ) ]
    # Add popup entries for all unlisted maps.
    for { set i [ expr ( $mapitems + 1 ) ]  } { $i <= $nummaps } { incr i } {
    	set map2 .m$i
    	$map.menubar.windows add command -label "Map $i" \
    		-command "popup_map $map2"
    }	

}

# Enable/disable things on the view menu.

proc adjust_view_menu { map } {
    global see_all
    global can_see_people can_see_control can_see_elev can_see_lighting
    global can_see_temp can_see_winds can_see_clouds
    global view_option_names
    global designer

    adjust_menu_entry $map view $view_option_names(coverage) [ expr !$see_all ]
    adjust_menu_entry $map view $view_option_names(people) $can_see_people
    adjust_menu_entry $map view $view_option_names(control) $can_see_control
    adjust_menu_entry $map view $view_option_names(elevations) $can_see_elev
    adjust_menu_entry $map view "Contour Interval" $can_see_elev
    adjust_menu_entry $map view $view_option_names(lighting) $can_see_lighting
    set weather [ expr $can_see_temp | $can_see_winds | $can_see_clouds ]
    adjust_menu_entry $map view "Weather" $weather
    if { $weather } {
	adjust_menu_entry $map view.weather $view_option_names(temperature) \
		$can_see_temp
	adjust_menu_entry $map view.weather $view_option_names(winds) \
		$can_see_winds
	adjust_menu_entry $map view.weather $view_option_names(clouds) \
		$can_see_clouds
    }
    set feats [ expr ([ numfeatures ] > 0) || $designer ]
    if { $feats == 0 } {
	$map.menubar.view entryconfig $view_option_names(feature_names) -variable 0    
	$map.menubar.view entryconfig $view_option_names(feature_boundaries) -variable 0    
    }
    adjust_menu_entry $map view $view_option_names(feature_names) $feats
    adjust_menu_entry $map view $view_option_names(feature_boundaries) $feats
}

# Enable/disable a single specified menu entry.

proc adjust_menu_entry { map menu entryname val } {
    set state disabled
    if { $val } {
	set state active
    }
    $map.menubar.$menu entryconfig $entryname -state $state
}

proc toggle_ai_may_resign {} {
    global ai_may_resign

    if { $ai_may_resign } {
	execute_long_command 0 "ai +"
    } else {
	execute_long_command 0 "ai -"
    }
}

# A popup to set the elevation contour interval directly.

set contour_interval 0
set contour_interval_map foo

proc popup_contour_interval_dialog { map } {
    global contour_interval_map stdfont boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor

    set contour_interval_map $map

    if { "[ winfo exists .contour ]" } {
	# Kick the window manager awake.
	wm withdraw .contour
	wm deiconify .contour
	focus -force .contour
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .contour -bg $bgcolor -width 240 -height 160
    pack propagate .contour true
    wm withdraw .contour
    wm title .contour "Xconq Contour Interval"

    frame .contour.dummy
    pack .contour.dummy -pady 6
    label .contour.label -text "Spacing in meters:" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack .contour.label -side top -pady 6
    entry .contour.interval -textvariable contour_interval -fg $fgcolor \
    	-relief sunken -width 15 -font $stdfont
    pack .contour.interval -side top -pady 6
    
    frame .contour.bottom -bg $bgcolor
    pack .contour.bottom -side bottom -anchor center -padx 20 -pady 4
    button .contour.bottom.ok -text "OK" \
	    -command { ok_contour_interval } -default active \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont
    button .contour.bottom.cancel -text "Cancel" \
	    -command { withdraw_window ".contour" } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont	    
    grid .contour.bottom.cancel .contour.bottom.ok -sticky ew -padx 8 -pady 4
   
    bind .contour <Key> {  
    	if {"%K" == "Escape"} { 
		.contour.bottom.cancel flash
		withdraw_window ".contour" 
	} elseif  {"%K" == "Return"} {
		.contour.bottom.ok flash
		ok_contour_interval
	}
    }
    center_above_curmap .contour
}

proc ok_contour_interval {} {
    global contour_interval
    global contour_interval_map map_number

    execute_long_command $map_number($contour_interval_map) \
	    "map contour-interval=$contour_interval"

    withdraw_window ".contour"
}

set new_meridian_interval 0

set meridian_map_number 0

proc popup_meridian_interval_dialog { mapn } {
    global new_meridian_interval 
    global meridian_map_number
    global map_options
    global prefs
    global bgcolor hbcolor abcolor fgcolor afcolor boldfont stdfont

    # We need to set a global since the key binding doesn't know the
    # value of $mapn any longer when OK is pressed.
    set meridian_map_number $mapn

    set new_meridian_interval $prefs(meridian_interval)

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .meridian -bg $bgcolor -width 240 -height 160
    pack propagate .meridian true
    wm withdraw .meridian
    wm title .meridian "Xconq Meridian Interval"

    frame .meridian.dummy
    pack .meridian.dummy -pady 6
    label .meridian.label -text "Spacing in arc minutes:" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack .meridian.label -side top -pady 6
    entry .meridian.interval -textvariable new_meridian_interval \
	    -fg $fgcolor -relief sunken -width 15 -font $stdfont
    pack .meridian.interval -side top -pady 6

    frame .meridian.bottom -bg $bgcolor
    pack .meridian.bottom -side bottom -anchor center -padx 20 -pady 4
    button .meridian.bottom.ok -text "OK" \
	    -command [ list ok_meridian_interval $meridian_map_number ] -default active \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont	    
    button .meridian.bottom.cancel -text "Cancel" \
	    -command { .meridian.bottom.cancel flash;  destroy .meridian } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont	    
     grid .meridian.bottom.cancel .meridian.bottom.ok -sticky ew -padx 8 -pady 4
   
    bind .meridian <Key> {  
    	if {"%K" == "Escape"} { 
		.meridian.bottom.cancel flash
		wm withdraw .meridian
		destroy .meridian
	} elseif  {"%K" == "Return"} {
		.meridian.bottom.ok flash
		ok_meridian_interval $meridian_map_number
	}
    }
    center_above_curmap .meridian
}

proc ok_meridian_interval { mapn } {
    global new_meridian_interval
    global map_options
    global map_widget
    global newprefs

    if { $mapn > 0 } {
	set map $map_widget($mapn)
	set map_options($map,meridian_interval) $new_meridian_interval
	set_map_view_option $map meridian_interval
    } else {
	# If 0 is passed as $mapn it means that we are setting the prefs.
	set newprefs(meridian_interval) $new_meridian_interval
    }
    wm withdraw .meridian
    destroy .meridian
}

# Given a map window, set up all of its standard event bindings.

proc make_normal_bindings { map } {
    global dside

    bind $map <Key> {
    	    if {"%K" == "Return" \
	    	|| "%K" == "Delete" \
	    	|| "%K" == "Escape" \
	    	|| "%K" == "BackSpace" \
	    	|| "%K" == "Right" \
	    	|| "%K" == "Left" \
	    	|| "%K" == "Up" \
	    	|| "%K" == "Down" \
	    	|| "%K" == "Home" \
	    	|| "%K" == "Prior" \
	    	|| "%K" == "Next" \
	    	|| "%K" == "End" \
	    	|| "%K" == "KP_1" \
	    	|| [string length "%A"] <= 1} {
	    		handle_key_binding "%A" "%K" %W %X %Y
	    }
    }

    set mapview $map.leftside.botside.mapf.mapf2.map

    bind $mapview <ButtonPress> { mouse_down_cmd %W %x %y %b }
    bind $mapview <ButtonRelease> { mouse_up_cmd %W %x %y %b }
    bind $mapview <Motion> { mouse_over_cmd %W %x %y }
    bind $mapview <Leave> { mouse_over_cmd %W -1 -1 }

    bind $map <Enter> { update_widget_help "%W" }

    set numu [ numutypes_available $dside ]
    set unitlist $map.rightside.listf.unitlist
    for { set i 0 } { $i < $numu } { incr i } {
	bind $unitlist.u$i <ButtonPress> [ list select_unit_type $map $i ]
    }

    set worldview $map.rightside.worldf.2.world

    bind $worldview <ButtonPress> { world_mouse_down_cmd %W %x %y %b }
    bind $worldview <ButtonRelease> { world_mouse_up_cmd %W %x %y %b }
    bind $worldview <Motion> { world_mouse_over_cmd %W %x %y }
    bind $worldview <Leave> { world_mouse_over_cmd %W -1 -1 }

    bind $map.rightside.worldf <Configure> { resize_world_map %W %v %w %h }

    bind $map.leftside.botside.uf.unitinfo <Configure> \
	    { resize_unit_info %W %v %w %h }
}

# Handle a keystroke by passing it into C code along the current mouse
# position, and update the numeric prefix argument if the key was a
# digit adding to the prefix.

# The map_number($win) existence test is to handle the unusual but
# reproducible case of a non-toplevel window getting passed in here
# (according to the supposed event binding stack rules).  Rather than
# trying to figure out what's going on, just dig out the toplevel and
# go with it, since that's all we really care about.

proc handle_key_binding { stra strk win x y } {
    global map_number

    if { [ info exists map_number($win) ] } {
	set mwin $win
    } else {
	set mwin [ winfo toplevel $win ]
	if { ! [ info exists map_number($mwin) ] } {
	    low_notify "key binding screwup in $win, ignoring key"
	    return
	}
    }
    set prefix [ interp_key $map_number($mwin) "$stra" "$strk" $x $y ]
    $mwin.leftside.topside.notices.t configure -state normal
    $mwin.leftside.topside.notices.t delete prefix.first "prefix.last -1 chars"
    if { "$prefix" >= 0 } {
	$mwin.leftside.topside.notices.t insert prefix.first ":" prefix
	$mwin.leftside.topside.notices.t insert prefix.first $prefix prefix
    }
    $mwin.leftside.topside.notices.t configure -state disabled
}

proc run_game_cmd {} {
    set interval [ run_game 1 ]
    after $interval run_game_cmd
#    after 1 run_game_cmd
}

proc animate_selection_cmd {} {
    animate_selection
    after 100 animate_selection_cmd
}

proc run_game_idle_cmd {} {
    run_game_idle
#    after 100 run_game_idle_cmd
    after 50 run_game_idle_cmd
}

# Map zoom command.

proc zoom_in_out { mapn incr } {
    global map_widget

    set map $map_widget($mapn)
    set maxpower 6

    set power [ $map.leftside.botside.mapf.mapf2.map cget -power ]
    set newpower [ expr $power + $incr ]
    if { $newpower < 0 } {
	set newpower 0
    }
    if { $newpower > $maxpower } {
	set newpower $maxpower
    }
    if { $newpower != $power } {
	$map.leftside.botside.mapf.mapf2.map config -power $newpower
	# Update the states of various controls.
	if { $newpower < $maxpower } {
	    set newstate normal
	} else {
	    set newstate disabled
	}
	$map.menubar.view entryconfigure "Closer" -state $newstate
	$map.leftside.botside.buttons.zoomin config -state $newstate
	if { $newpower > 0 } {
	    set newstate normal
	} else {
	    set newstate disabled
	}
	$map.menubar.view entryconfigure "Farther" -state $newstate
	$map.leftside.botside.buttons.zoomout config -state $newstate
	
	adjust_map_font_sizes $map
    }
}

proc adjust_map_font_sizes { map } {
    global boldfont mapfont
    global tcl_platform

    set power [ $map.leftside.botside.mapf.mapf2.map cget -power ]
    
    # Pick sizes that look good on each platform
    if { "$tcl_platform(platform)" == "unix" } {
	if { $power < 4 } {
		set mainsize 10
		set featuresize 12
	} elseif { $power == 4 } {
		set mainsize 12
		set featuresize 16
	} elseif { $power == 5 } {
		set mainsize 14
		set featuresize 20
	} elseif { $power == 6 } {
		set mainsize 18
		set featuresize 32
	}
    }
    if { "$tcl_platform(platform)" == "macintosh" } {
	if { $power < 4 } {
		set mainsize 9
		set featuresize 12
	} elseif { $power == 4 } {
		set mainsize 10
		set featuresize 14
	} elseif { $power == 5 } {
		set mainsize 12
		set featuresize 20
	} elseif { $power == 6 } {
		set mainsize 18
		set featuresize 36
	}
    }
    if { "$tcl_platform(platform)" == "windows" } {
	if { $power < 4 } {
		set mainsize 7
		set featuresize 8
	} elseif { $power == 4 } {
		set mainsize 8
		set featuresize 11
	} elseif { $power == 5 } {
		set mainsize 10
		set featuresize 16
	} elseif { $power == 6 } {
		set mainsize 16
		set featuresize 24
	}
    }
    set tmpfont $mapfont
    append tmpfont [ list "-size" $mainsize ]
    $map.leftside.botside.mapf.mapf2.map config -mainfont $tmpfont
    set tmpfont $mapfont
    append tmpfont [ list "-size" $featuresize ]
    $map.leftside.botside.mapf.mapf2.map config -featurefont $tmpfont
}

# Turn isometric display on and off.

proc set_isometric { mapn flag scale } {
    global map_widget

    set map $map_widget($mapn)
    $map.leftside.botside.mapf.mapf2.map config -isometric $flag \
	    -verticalscale $scale
    if { $flag } {
	set newstate normal
    } else {
	set newstate disabled
    }
    $map.leftside.botside.buttons.rotl config -state $newstate
    $map.leftside.botside.buttons.rotr config -state $newstate
}

# Set the direction of viewing for an isometric display.

proc set_iso_dir { mapn dir } {
    global map_widget

    set map $map_widget($mapn)
    $map.leftside.botside.mapf.mapf2.map config -direction $dir
}

# Update routines called from C code.

proc update_game_state { str } {
    global debug
    global nummaps map_widget

    if { $debug } {
	set str "$str (Debug)"
    }
    for { set i 1 } { $i <= $nummaps } { incr i } {
	$map_widget($i).rightside.turnf.turn itemconfig the_date -text $str
    }
}

# (should optimize by remembering states and changing canvas items
# only once)

proc update_game_side_info { s str str2 everingame beforestart ingame status } {
    global nummaps 
    global map_widget
    global bgcolor fgcolor
    global dside dside_ai

    # Update the selected AI in the AI menu. 
    set dplayer [ assigned_player $dside ]
    set dside_ai [ player_aitypename $dplayer ]
    # The menu code cannot handle empty strings.
    if { $dside_ai == "" } {
    	set dside_ai "none"
    }
    # puts stdout "ugsi $s $str $everingame $ingame $status"
    for { set i 1 } { $i <= $nummaps } { incr i } {
	set sidelist $map_widget($i).rightside.gamef.game
	$sidelist itemconfig s$s -text $str
	$sidelist itemconfig player$s -text $str2
	if { !$everingame && !$beforestart } {
	    $sidelist itemconfig s$s -fill gray
	    $sidelist itemconfig left$s -fill $bgcolor
	    $sidelist itemconfig resv$s -fill $bgcolor
	    $sidelist itemconfig frame$s -outline $bgcolor
	} elseif { !$ingame } {
	    $sidelist itemconfig frame$s -outline gray
	    $sidelist itemconfig left$s -fill $bgcolor
	    $sidelist itemconfig resv$s -fill $bgcolor
	} else {
	    $sidelist itemconfig s$s -fill $fgcolor
	}
	if { $status > 0 } {
	    # Find the the victory laurels and move into visibility.
	    set lis [ $sidelist coords won$s ]
	    set xval [ lindex $lis 0 ]
	    if { $xval > 4 } {
		$sidelist move won$s [expr 4 - $xval ] 0
		$sidelist raise won$s
	    }
	}
	if { $status < 0 } {
	    # Find the the loss line and move into visibility.
	    set lis [ $sidelist coords lost$s ]
	    set xval [ lindex $lis 0 ]
	    if { $xval > 0 } {
		$sidelist move lost$s [expr 0 - $xval ] 0
		$sidelist raise lost$s
	    }
	}
    }
}

proc update_game_side_score { which str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set game $map_widget($i).rightside.gamef.game
	$game itemconfig $which -text $str
    }
}

# (should optimize by remembering states and changing canvas items
# only once)

proc update_side_progress { s acting left resv finished } {
    global lineheight
    global nummaps map_widget
    global fgcolor

    # puts stdout "progress $s $acting $left $resv $finished"
    for { set i 1 } { $i <= $nummaps } { incr i } {
	set game $map_widget($i).rightside.gamef.game
	set old [ $game coords left$s ]
	set rtop [ lindex $old 1 ]
	set rbot [ lindex $old 3 ]
	if { $acting } {
	    $game itemconfig frame$s -outline $fgcolor
	    $game coords left$s 23 $rtop [ expr 23 + $left  + 1 ] $rbot
	    $game coords resv$s 23 $rtop [ expr 23 + $resv ] $rbot
	    if { !$finished } {
		$game itemconfig left$s -fill LawnGreen
		$game itemconfig resv$s -fill magenta
		
	    } else {
		$game itemconfig left$s -fill violet
		$game itemconfig resv$s -fill violet
	    }
	} else {
	    $game itemconfig frame$s -outline gray
	    $game coords left$s 23 $rtop 23 $rbot
	    $game coords resv$s 23 $rtop 23 $rbot
	}
    }
}

proc update_side_treasury { s j amt } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set game $map_widget($i).rightside.gamef.game
	$game itemconfig m$s,$j -text $amt
    }
}

proc update_unitlist_char { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig u$n -text $str
    }
}

proc update_unitlist_count { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig n$n -text $str
    }
}

proc update_unitlist_incomplete { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig i$n -text $str
    }
}

proc update_unitlist_name { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig name$n -text $str
    }
}

proc update_unitlist_material { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig material$n -text $str
    }
}

proc update_unitlist_supply { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig supply$n -text $str
    }
}

proc update_unitlist_production { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig production$n -text $str
    }
}

proc update_unitlist_capacity { n str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set unitlist $map_widget($i).rightside.listf.unitlist
	$unitlist itemconfig capacity$n -text $str
     }
 }
 
proc update_unit_info { mapn tag str } {
    global curunit
    global map_widget

    if { "$tag" == "curunit" } {
	set curunit $str
    } else {
	$map_widget($mapn).leftside.botside.uf.unitinfo itemconfig $tag -text $str
    }
}

# Make sure the unit picture in the unit info window is showing the
# given image and emblem, switching if necessary.

set last_image "(no)"
set last_emblem "(no)"

proc update_unit_picture { mapn image emblem } {
    global map_widget
    global last_image last_emblem

    if { "$image" != "$last_image" } {
	set imsamp $map_widget($mapn).leftside.botside.uf.unitinfo.pic
	if { "$image" != "(no)" } {
	    $imsamp replace imf $image
	    if { "$emblem" != "$last_emblem" } {
		if { "$emblem" != "(no)" } {
		    $imsamp replace emblem $emblem
		    $imsamp emblem 1
		} else {
		    $imsamp remove emblem $last_emblem
		    $imsamp emblem -1
		}
	    }
	    set last_emblem $emblem
	} else {
	    $imsamp remove all
	    set last_emblem "(no)"
	}
	set last_image $image
    } elseif { "$emblem" != "$last_emblem" } {
	set imsamp $map_widget($mapn).leftside.botside.uf.unitinfo.pic
	if { "$emblem" != "(no)" } {
	    $imsamp replace emblem $emblem
	    $imsamp emblem 1
	} else {
	    $imsamp remove emblem $last_emblem
	    $imsamp emblem -1
	}
	set last_emblem $emblem
    }
}

# Update the appearance of any mode controls/displays.

proc update_mode { mapn mode } {
    global map_survey
    global map_widget

    if { "$mode" == "survey" } {
	set map_survey 1
	set relief raised
    } elseif { "$mode" == "move" } {
	set map_survey 0
	set relief sunken
    } else {
	# This will induce a distinctive tcl error shortly.
	set relief badmode
    }
    $map_widget($mapn).leftside.botside.buttons.move config -relief $relief
    update idletasks
}

proc whelp { widg str } {
    global widget_help_strings

    set widget_help_strings($widg) $str
}

# Given a widget, put any helpful info about it in the mouseover display.

proc update_widget_help { widg } {
    global widget_help_strings
    global nummaps

    if { [ info exists widget_help_strings($widg) ] } {
	set str $widget_help_strings($widg)
    } else {
	set str ""
    }
    # a hack, should extract map from widget
    if { $nummaps == 1 } {
	update_mouseover 1 $str
    }
}

# Replace the current mouseover text with the given version.
# Time-critical, called from C and tcl code.
# (should try to replace text string in one op if possible)

proc update_mouseover { mapn str } {
    global map_widget

    $map_widget($mapn).leftside.botside.mouseover configure -state normal
    $map_widget($mapn).leftside.botside.mouseover delete 1.0 end
    $map_widget($mapn).leftside.botside.mouseover insert insert "$str"
    $map_widget($mapn).leftside.botside.mouseover configure -state disabled
}

# Scroll the main or world map by the given amount.

proc autoscroll { mapn which xdelta ydelta } {
    global map_widget

    if { $which == 0 } {
	set widget $map_widget($mapn).leftside.botside.mapf.mapf2.map
    } else {
	set widget $map_widget($mapn).rightside.worldf.2.world
    }
    if { $xdelta != 0 } {
	$widget xview scroll $xdelta units
    }
    if { $ydelta != 0 } {
	$widget yview scroll $ydelta units
    }
    if { $xdelta != 0  || $ydelta != 0 } {
	update idletasks
    }
}

proc update_action_controls_info { a1 a2 a3 a4 a5 flags } {
    global can_act can_plan can_move can_return can_embark can_disembark
    global can_build can_repair can_change_type can_disband
    global can_add_terrain can_remove_terrain
    global can_give_take 
    global can_attack can_fire can_detonate
    global curunit
    global nummaps map_widget

    set can_act $a1
    set can_plan $a2
    set can_move $a3
    set can_build $a4
    set can_attack $a5
    set can_return 0
    set can_repair 0
    set can_change_type 0
    set can_fire 0
    set can_detonate 0
    set can_embark 0
    set can_disembark 0
    set can_disband 0
    set can_add_terrain 0
    set can_remove_terrain 0
    set can_give_take 0

    foreach flag $flags {
	set $flag 1
    }
    for { set i 1 } { $i <= $nummaps } { incr i } {
	set buttons $map_widget($i).leftside.botside.buttons
	set state normal
	if { !$curunit || !$can_build } {
	    set state disabled
	}
	$buttons.build config -state $state
	set state normal
	if { !$curunit || !$can_return } {
	    set state disabled
	}
	$buttons.return config -state $state
	set state normal
	if { !$curunit || !$can_plan } {
	    set state disabled
	}
	$buttons.sleep config -state $state
	$buttons.reserve config -state $state
	$buttons.delay config -state $state
    }
}

proc update_view_controls_info { a1 a3 a4 a5 a6 a7 a8 a9 } {
    global see_all
    global can_see_people can_see_control can_see_elev can_see_lighting
    global can_see_temp can_see_winds can_see_clouds
    global nummaps map_widget

    set see_all $a1
    set can_see_people $a3
    set can_see_control $a4
    set can_see_elev $a5
    set can_see_lighting $a6
    set can_see_temp $a7
    set can_see_winds $a8
    set can_see_clouds $a9
    for { set i 1 } { $i <= $nummaps } { incr i } {
	set map $map_widget($i)
	adjust_view_menu $map
    }
}

proc update_show_all_info { a1 } {
    global see_all
    global may_set_show_all
    global show_all_was_enabled
    global nummaps map_widget

    if { $see_all } {
	return
    }
    set may_set_show_all $a1
    # Make "Show All" menu item appear and disappear.
    if { $may_set_show_all } {
	for { set i 1 } { $i <= $nummaps } { incr i } {
	    set map $map_widget($i)
	    if { !$show_all_was_enabled($map) } {
		add_show_all_item $map
	    }
	}
    } else {
	for { set i 1 } { $i <= $nummaps } { incr i } {
	    set map $map_widget($i)
	    if { $show_all_was_enabled($map) } {
		$map.menubar.view delete "Show All"
		set show_all_was_enabled($map) 0
	    }
	}
    }
}

proc update_show_all { mapn value } {
    global see_all
    global map_widget
    global map_options

    if { $see_all } {
	return
    }
    set map $map_widget($mapn)
    set map_options($map,show_all) $value
}

proc low_notify { str } {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	set notices $map_widget($i).leftside.topside.notices.t
	$notices configure -state normal
	$notices insert notices.last $str notices
	$notices configure -state disabled
	# (should only do if already scrolled to end of notices)
	$notices yview moveto 1.0
    }
}

proc ask_bool_mode { mapn str dflt } { 
    global bgcolor fgcolor afcolor
    global map_widget
    global mapnum
    global titlefont

    set map $map_widget($mapn)
    # A new global is needed since bind will not pass on $mapn directly.
    set mapnum $mapn
    set color Gold

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .bool -bg $color -width 340 -height 160
    wm withdraw .bool
    wm title .bool "Xconq Query"
    pack propagate .bool false

    message .bool.msg -text "$str" -bg $color -fg $fgcolor -font $titlefont \
	-aspect 500
    frame .bool.buttons -bg $color
    pack .bool.buttons -side bottom -padx 0 -pady 8
    pack .bool.msg -side top -padx 4 -expand true

    button .bool.buttons.no -text No -command [ list bool_no $mapn ] \
	-bg $color -highlightbackground $color -activebackground $color \
	-fg $fgcolor -activeforeground $afcolor -width 6 -font $titlefont
    button .bool.buttons.yes -text Yes -command [ list bool_yes $mapn ] \
	-bg $color -highlightbackground $color -activebackground $color \
	-fg $fgcolor -activeforeground $afcolor -width 6 -default active -font $titlefont
    grid .bool.buttons.no .bool.buttons.yes -sticky e -padx 12 -pady 4

    bind .bool <Key> { 
    		if {"%K" == "Return"} {
    			.bool.buttons.yes flash
    			bool_yes $mapnum 
    		} elseif {"%K" == "Escape"} {
    			.bool.buttons.no flash
    			bool_no $mapnum 
    		}
    }
    bind .bool <Y> { .bool.buttons.yes flash; bool_yes $mapnum }
    bind .bool <y> { .bool.buttons.yes flash; bool_yes $mapnum }
    bind .bool <N> { .bool.buttons.no flash; bool_no $mapnum }
    bind .bool <n> { .bool.buttons.no flash; bool_no $mapnum }

    center_above_curmap .bool
}

proc bool_yes { mapn } {
    interp_key $mapn "y" "y" 0 0
}

proc bool_no { mapn } {
    interp_key $mapn "n" "n" 0 0
}

proc ask_bool_done { mapn } {
    wm withdraw .bool
	destroy .bool
}

proc ask_position_mode { mapn str } {
    global map_widget

    $map_widget($mapn).leftside.topside.notices.t configure -state normal
    $map_widget($mapn).leftside.topside.notices.t insert prompt.first "$str" prompt
    $map_widget($mapn).leftside.botside.mapf.mapf2.map config -cursor crosshair
    $map_widget($mapn).leftside.topside.notices.t configure -state disabled
    # (should only do if already scrolled to end of notices)
    $map_widget($mapn).leftside.topside.notices.t yview moveto 1.0
}

proc ask_position_done { mapn } {
    global map_widget

    clear_command_line $mapn
    # (should be restoring prev cursor) */
    $map_widget($mapn).leftside.botside.mapf.mapf2.map config -cursor arrow
}

proc ask_unit_type_mode { mapn str } {
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices insert prompt.first "$str" prompt
    $notices configure -state disabled
    # (should only do if already scrolled to end of notices)
    $notices yview moveto 1.0
}

proc enable_unitlist { mapn n flag } {
    global map_widget
    global bgcolor fgcolor

    set map $map_widget($mapn)
    if { $flag == 1 } {
	set color $fgcolor
    } else {
	set color $bgcolor
    }
    $map.rightside.listf.unitlist itemconfig rect$n -outline $color
}

proc select_unit_type { map n } {
    global map_number
    global fgcolor

    set color [ $map.rightside.listf.unitlist itemcget rect$n -outline ]
    if { "$color" == $fgcolor } {
	set_unit_type $map_number($map) [ utype_actual $n ]
	# Arguments are dummies, this is effectively a pseudo-event
	# that gets the modalhandler function to run.
	interp_key $map_number($map) a a 0 0
    }
}

proc ask_unit_type_done { mapn } {
    clear_command_line $mapn
}

proc ask_terrain_type_mode { mapn str } { 
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices insert prompt.first "$str" prompt
    $notices configure -state disabled
    # (should only do if already scrolled to end of notices)
    $notices yview moveto 1.0
}

proc ask_terrain_type_done { mapn } {
    clear_command_line $mapn
}

# Put a given string and default into the map's prompt and answer tags
# in its notices window.

proc ask_string_mode { mapn str dflt } { 
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices insert prompt.first "$str" prompt
    $notices insert "answer.first + 1 chars" "$dflt" answer
    $notices configure -state disabled
    # (should only do if already scrolled to end of notices)
    $notices yview moveto 1.0
}

# Replace the answer tag with a new string.

proc update_string_mode { mapn answer } {
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices delete "answer.first + 1 chars" "answer.last - 1 chars"
    # FIXME: need some quoting in case the user types an open brace.
    $notices insert "answer.first + 1 chars" $answer answer
    $notices configure -state disabled
}

proc ask_string_done { mapn } {
    clear_command_line $mapn
}

# Side asking mode is similar to string asking mode.

proc ask_side_mode { mapn str dflt } { 
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices insert prompt.first "$str" prompt
    $notices insert "answer.first + 1 chars" "$dflt" answer
    $notices configure -state disabled
    # (should only do if already scrolled to end of notices)
    $notices yview moveto 1.0
}

proc update_side_mode { mapn answer } {
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t 
    $notices configure -state normal
    $notices delete "answer.first + 1 chars" "answer.last - 1 chars"
    $notices insert "answer.first + 1 chars" $answer answer
    $notices configure -state disabled
}

proc ask_side_done { mapn } {
    clear_command_line $mapn
}

proc clear_command_line { mapn } {
    global map_widget

    set notices $map_widget($mapn).leftside.topside.notices.t
    $notices configure -state normal
    $notices delete prefix.first "prefix.last - 1 chars"
    $notices delete prompt.first "prompt.last - 1 chars"
    $notices delete "answer.first + 1 chars" "answer.last - 1 chars"
    $notices configure -state disabled
}

set new_find_name ""
set mapn_find_name 0

proc popup_find_dialog { mapn } {
    global mapn_find_name
    global bgcolor hbcolor abcolor fgcolor afcolor boldfont stdfont

    set mapn_find_name $mapn

    if { "[ winfo exists .find ]" } {
    	# Kick the window manager awake.
    	wm withdraw .find
	wm deiconify .find
	focus -force .find
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .find -bg $bgcolor -width 240 -height 160
    pack propagate .find true
    wm withdraw .find
    wm title .find "Xconq Find"

    frame .find.dummy
    pack .find.dummy -pady 6
    label .find.label -text "Name of unit to find:" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack .find.label -side top -pady 6
    entry .find.name -textvariable new_find_name -fg $fgcolor -relief sunken -width 15 -font $stdfont
    pack .find.name -side top -pady 6
    label .find.matches -text "" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack .find.matches -side top -pady 6

    frame .find.bottom -bg $bgcolor
    pack .find.bottom -side bottom -anchor center -padx 20 -pady 4
    button .find.bottom.find -text "Find" -command ok_find -default active \
    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
    	-fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont   
    button .find.bottom.cancel -text "Cancel" \
        -command { withdraw_window ".find" } \
    	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor	\
    	-fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont
    grid .find.bottom.cancel .find.bottom.find -sticky ew -padx 8 -pady 4
    
    bind .find <Key> {  
    	if {"%K" == "Escape"} { 
		.find.bottom.cancel flash
		withdraw_window ".find" 
	} elseif  {"%K" == "Return"} {
		.find.bottom.find flash
		ok_find
	}
    }
    center_above_curmap .find
}

proc ok_find {} {
    global new_find_name
    global mapn_find_name

    set rslt [ center_on_unit $mapn_find_name "$new_find_name" ]
    if { $rslt == 1 } {
	.find.matches config -text "$rslt match found"
    } else {
	.find.matches config -text "$rslt matches found"
    }
    if { $rslt == 0 } {
	bell
    } else {
        withdraw_window ".find"
    }
}

# The research dialog lets the player select a new type of advance
# to research.

proc popup_research_dialog {} {
    global stdfont boldfont
    global tcl_platform
    global scrollb
    global listb
    global bgcolor hbcolor abcolor tcolor lcolor fgcolor afcolor

    # First of all set the research popup flag.
    set_research_popped_up 1

    if { "[ winfo exists .research ]" } {
	# Kick the window manager awake.
	wm withdraw .research
	wm deiconify .research
	focus -force .research
	fill_research_dialog
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .research -bg $bgcolor
    wm withdraw .research
    wm title .research "Xconq Research"
   
   # Make sure close_research is called even if the close box is used.
    wm protocol .research WM_DELETE_WINDOW close_research

    label .research.current -width 25 -height 2 -font $stdfont -bg $bgcolor -fg $fgcolor
    pack .research.current -side top
    
    frame .research.bottom -bg $bgcolor
    pack .research.bottom -side bottom -fill x -padx 15
    button .research.bottom.ok -text "Research" -command ok_research \
        -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
        -fg $fgcolor -activeforeground $afcolor -font $boldfont   
    pack .research.bottom.ok -side left -padx 5 -pady 5
    button .research.bottom.rest -text "Rest" -command rest_research \
        -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
        -fg $fgcolor -activeforeground $afcolor -font $boldfont 
    pack .research.bottom.rest -side left -padx 5 -pady 5
    button .research.bottom.cancel -text "Close" \
        -command { close_research } \
        -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
        -fg $fgcolor -activeforeground $afcolor -font $boldfont    
    pack .research.bottom.cancel -side left -padx 5 -pady 5

    listbox .research.topics -selectmode browse -width 25 \
	-yscrollcommand ".research.yscroll set" -relief sunken -borderwidth $listb \
	-bg $lcolor -fg $fgcolor -font $stdfont 
    # Whether this scrollbar is actually needed depends on how many
    # choices the user has, and what the fonts are, but it doesn't
    # seem wise to make assumptions as the former is GDL and the latter
    # is a user preference.
    scrollbar .research.yscroll -orient vert \
	-command ".research.topics yview" -width 16 -borderwidth $scrollb \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-troughcolor $tcolor    
    pack .research.yscroll -side right -fill y
    pack .research.topics -side top -expand true -fill both 

    center_above_curmap .research
    fill_research_dialog
}

proc fill_research_dialog {} {
    if { !"[ winfo exists .research ]" } {
	return
    }
    set numa [ numatypes ]
    set cura [ current_advance ]
    .research.current config -text "Researching $cura"
    .research.topics delete 0 end
    for { set i 0 } { $i < $numa } { incr i } {
	set aname [ available_advance $i ]
	if { "$aname" != "?" } {
	    .research.topics insert end " $aname"
	    if { "$aname" == "$cura" } {
    		.research.topics selection set $i
    		.research.topics see $i
    	    }	    
	} else {
	    break
	}
    }
    if { "$cura" == "Nothing" } {
	.research.topics selection set 0
    }
}

proc ok_research {} {
    set i [ .research.topics curselection ]
    if { $i == "" } {
    	return
    }
    set_side_research $i
    close_research
}

proc rest_research {} {
    set_side_research nothing
    close_research
}

proc close_research {} {
   withdraw_window ".research"
    set_research_popped_up 0
}

# Create and popup an agreement editing window.

proc create_agreements_window {} {
    global textb
    global bgcolor hbcolor abcolor lcolor fgcolor afcolor
    
    if { "[ winfo exists .agreements ]" } {
    	# Kick the window manager awake.
    	wm withdraw .agreements
	wm deiconify .agreements
	focus -force .agreements
	update_agreement_display
	return
    }
    
    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .agreements -bg $bgcolor
    wm withdraw .agreements
    wm title .agreements "Xconq Agreements"

#    puts stdout "[agreements]"

    frame .agreements.top -bg $bgcolor
    pack .agreements.top -side top -fill x

    scrolled_listbox .agreements.top.toc -selectmode browse
    pack .agreements.top.toc -side left -fill both -expand true

    bind .agreements.top.toc.list <ButtonRelease-1> { select_agreement }

    frame .agreements.top.titlef -bg $bgcolor
    pack .agreements.top.titlef -side top -fill x

    label .agreements.top.titlef.tlabel -text "Title:" -bg $bgcolor -fg $fgcolor
    pack .agreements.top.titlef.tlabel -side left -anchor nw
    entry .agreements.top.titlef.title -fg $fgcolor
    pack .agreements.top.titlef.title -side left -anchor nw

    frame .agreements.top.statef -bg $bgcolor
    pack .agreements.top.statef -side top -fill x

    label .agreements.top.statef.slabel -text "State:" -bg $bgcolor -fg $fgcolor
    pack .agreements.top.statef.slabel -side left -anchor nw
    label .agreements.top.statef.state -text "()" -bg $bgcolor -fg $fgcolor
    pack .agreements.top.statef.state -side left -anchor nw

    label .agreements.top.termslabel -text "Terms:" -bg $bgcolor -fg $fgcolor
    pack .agreements.top.termslabel -side top -anchor nw

    frame .agreements.top.termsf -relief sunken -borderwidth $textb -bg $bgcolor
    pack .agreements.top.termsf -side top

    text .agreements.top.termsf.terms -width 40 -height 20 \
     -bg $lcolor -highlightbackground $lcolor -fg $fgcolor -relief flat
    pack .agreements.top.termsf.terms -side top

    .agreements.top.termsf.terms delete 1.0 end
    .agreements.top.termsf.terms insert insert "(terms)"

    label .agreements.top.sideslabel -text "Sides:" -bg $bgcolor -fg $fgcolor
    pack .agreements.top.sideslabel -side top -anchor nw

    frame .agreements.top.sidesf -height 100 -bg $bgcolor
    pack .agreements.top.sidesf -side top -fill both

    canvas .agreements.top.sidesf.sides -width 300 -height 100 \
	-borderwidth 2 -relief sunken \
	-bg $bgcolor -highlightbackground $hbcolor
    pack .agreements.top.sidesf.sides -side top -fill both

    # Add listboxes for drafters, proposers, signers, announcees
    # Add text boxes for terms and comments
    # Add popup to choose specific types of terms
    
    frame .agreements.bot     -bg $bgcolor
    pack .agreements.bot -side bottom -fill both -expand true

    button .agreements.bot.new -text "New" \
	-command { new_agreement } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.new -side left
    button .agreements.bot.addside -text "Add Side" -state disabled \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.addside -side left
    button .agreements.bot.circulate -text "Circulate" -state disabled \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.circulate -side left
    button .agreements.bot.propose -text "Propose" -state disabled \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.propose -side left
    button .agreements.bot.sign -text "Sign" -state disabled \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.sign -side left
    button .agreements.bot.withdraw -text "Withdraw" -state disabled \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.withdraw -side left
    button .agreements.bot.done -text "Done" \
	    -command { close_agreements_window } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor
    pack .agreements.bot.done -side left

    center_above_curmap .agreements
}

proc new_agreement {} {
    execute_long_command 0 "agreement-draft"
    update_agreement_display
}

proc close_agreements_window {} {
    withdraw_window ".agreements"
}

proc update_agreement_display {} {
    .agreements.top.toc.list delete 0 end
    set aglist [ agreements ]
    foreach agid $aglist {
	.agreements.top.toc.list insert end $agid
    }
    # (should add info about selected agreement)
}

proc select_agreement {} {
    set i [ .agreements.top.toc.list curselection ]
    if { $i == "" } {
    	return
    }
    set agid [ .agreements.top.toc.list get $i ]
}

proc popup_scores {} {
    global stdfont boldfont
    global scrollb
    global textb
    global bgcolor hbcolor abcolor tcolor lcolor fgcolor afcolor

    if { "[ winfo exists .scores ]" } {
    	# Kick awake the window manager.
    	wm withdraw .scores
	wm deiconify .scores
	focus -force .scores
	# Refresh the scores list each time.
	.scores.main.text delete 1.0 end
	.scores.main.text insert end [ get_scores ]
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .scores -bg $bgcolor
    wm withdraw .scores
    wm title .scores "Xconq Scores"

    frame .scores.bottom -bg $bgcolor
    pack .scores.bottom -side bottom -fill x
    button .scores.bottom.close -text "Close" -command { dismiss_scores } -default active \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -width 8 -font $boldfont	
    pack .scores.bottom.close -side right -pady 6 -padx 25
    
    frame .scores.main -width 400 -height 300 -relief sunken -borderwidth $textb -bg $lcolor
    text .scores.main.text -yscrollcommand ".scores.yscroll set" -font $stdfont \
	-bg $lcolor -highlightbackground $lcolor -wrap word -fg $fgcolor -relief flat
    scrollbar .scores.yscroll -orient vert \
	-command ".scores.main.text yview" -width 16 -borderwidth $scrollb \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack .scores.yscroll -side right -fill y
    pack .scores.main -side top -fill both -expand true
    pack propagate .scores.main false
    pack .scores.main.text -side top -fill both -expand true

    .scores.main.text insert end [ get_scores ]
    .scores.main.text configure -state disabled

    bind .scores <Key> {  
    	if {"%K" == "Return" || "%K" == "Escape"} {
		.scores.bottom.close flash
		dismiss_scores
	}
    }
    center_above_curmap .scores
}

proc dismiss_scores {} {
    withdraw_window ".scores"
}

# Create and popup the preferences dialog.

proc popup_preferences_dialog {} {
    global view_option_list
    global stdfont boldfont
    global last_prefs_topic
    global listb
    global bgcolor hbcolor abcolor lcolor fgcolor afcolor

    if { "[ winfo exists .prefs ]" } {
	# Kick the window manager awake.
	wm withdraw .prefs
	wm deiconify .prefs
	focus -force .prefs
	init_newprefs
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .prefs -bg $bgcolor
    wm withdraw .prefs    
    wm title .prefs "Xconq Preferences"

    frame .prefs.main -bg $bgcolor
    pack .prefs.main -side top -fill both -expand true

    frame .prefs.main.topics -bg $bgcolor 
    pack .prefs.main.topics -side left -padx 6 -pady 6 -anchor nw
    listbox .prefs.main.topics.list -selectmode browse -font $stdfont \
    	-height 5 -width 10 -relief sunken -borderwidth $listb -bg $lcolor -fg $fgcolor
    pack .prefs.main.topics.list -side left
    .prefs.main.topics.list insert end " Map View"
    .prefs.main.topics.list insert end " Fonts"
    .prefs.main.topics.list insert end " Files"
    .prefs.main.topics.list insert end " Imagery"
    .prefs.main.topics.list insert end " Network"

    frame .prefs.main.v -width 300 -height 220 -bg $bgcolor
    pack .prefs.main.v -side left -padx 8 -pady 6
    pack propagate .prefs.main.v true

    add_map_prefs_items
    set last_prefs_topic " Map View"

    bind .prefs.main.topics.list <ButtonRelease-1> { select_prefs_topic }

    frame .prefs.bot -bg $bgcolor
    pack .prefs.bot -side bottom -anchor e -padx 20 -pady 4

    button .prefs.bot.ok -text OK -width 8 -font $boldfont \
	    -command { ok_preferences } -default active \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    button .prefs.bot.cancel -text Cancel -width 8 -font $boldfont \
	    -command { dismiss_preferences_dialog } \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
    grid .prefs.bot.cancel .prefs.bot.ok -sticky ew -padx 6 -pady 4

    init_newprefs
    # Select "Map View" by default.
    .prefs.main.topics.list selection set 0

    bind .prefs <Key> {  
    	if {"%K" == "Escape"} { 
		.prefs.bot.cancel flash
		withdraw_window ".prefs"
	} elseif  {"%K" == "Return"} {
		.prefs.bot.ok flash
		ok_preferences
	}
    }
    center_above_curmap .prefs
}

proc init_newprefs {} {
    global view_option_list
    global imagery_option_list
    global prefs newprefs

    foreach opt $view_option_list {
	set newprefs($opt) $prefs($opt)
    }
    set_power_pref $newprefs(power)
    set_font_family_newpref $prefs(font_family)
    set_font_size_newpref $prefs(font_size)
    foreach opt $imagery_option_list {
	set newprefs($opt) $prefs($opt)
    }
    set newprefs(want_checkpoints) $prefs(want_checkpoints)
    set newprefs(checkpoint_interval) $prefs(checkpoint_interval)
    set newprefs(listallgames) $prefs(listallgames)
    set newprefs(joinhost) $prefs(joinhost)
    set newprefs(joinport) $prefs(joinport)
    set newprefs(myname) $prefs(myname)
}

proc add_map_prefs_items {} {
    global view_option_list view_option_names boldfont
    global bgcolor hbcolor abcolor scolor fgcolor afcolor

    set mapvf .prefs.main.v.map
    if { !"[ winfo exists $mapvf ]" } {
	frame $mapvf -bg $bgcolor

	menubutton $mapvf.power -text $view_option_names(power) \
		-borderwidth 2 -relief raised \
		-menu $mapvf.power.menu \
		-bg $bgcolor -highlightbackground $hbcolor \
		-activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	pack $mapvf.power -side top -anchor nw
	menu $mapvf.power.menu -tearoff 0 -font $boldfont \
		-bg $bgcolor -activebackground $abcolor -fg $fgcolor \
		-activeforeground $afcolor
	for { set i 0 } { $i <= 6 } { incr i } {
	    $mapvf.power.menu add command -label "$i" \
		    -command [ list set_power_pref $i ]
	}
	frame $mapvf.checks -bg $bgcolor
	pack $mapvf.checks -side top
	set cnt 0
	foreach opt $view_option_list {
	    if { "$opt" == "power" } continue
	    if { "$opt" == "meridian_interval" } continue
	    set opts($cnt) $opt
	    incr cnt
	}
	for { set i 0 } { $i < $cnt } { incr i 2 } {
	    set opt1 $opts($i)
	    set j [ expr $i + 1 ]
	    if { $j < $cnt } {
		set even 1
	    } else {
		set even 0
	    }
	    checkbutton $mapvf.checks.$opt1 \
		    -text " $view_option_names($opt1)" \
		    -variable newprefs($opt1) \
		    -bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		    -activebackground $abcolor -selectcolor $scolor \
		    -fg $fgcolor -activeforeground $afcolor
	    if { $even } {
		set opt2 $opts($j)
		checkbutton $mapvf.checks.$opt2 \
			-text " $view_option_names($opt2)" \
			-variable newprefs($opt2) \
			-bg $bgcolor -highlightbackground $bgcolor \
			-font $boldfont \
			-activebackground $abcolor -selectcolor $scolor \
			-fg $fgcolor -activeforeground $afcolor
		grid $mapvf.checks.$opt1 $mapvf.checks.$opt2 -sticky w -pady 2
	    } else {
		grid $mapvf.checks.$opt1 -pady 2
	    }
	}
	button $mapvf.minterval -text $view_option_names(meridian_interval) \
	    -width 16 -font $boldfont \
	    -command [ list popup_meridian_interval_dialog 0 ] \
	    -bg $bgcolor -highlightbackground $hbcolor \
	    -activebackground $abcolor \
	    -fg $fgcolor -activeforeground $afcolor
	pack $mapvf.power -side top -anchor nw
	pack $mapvf.minterval -side bottom -anchor nw -pady 5
    }
    pack $mapvf -side left -anchor nw
}

proc remove_map_prefs_items {} {
    pack forget .prefs.main.v.map
}

proc add_fonts_prefs_items {} {
    global prefs boldfont
    global textb
    global bgcolor hbcolor abcolor lcolor fgcolor afcolor

    if { !"[ winfo exists .prefs.main.v.fonts ]" } {
	frame .prefs.main.v.fonts -bg $bgcolor
	
	frame .prefs.main.v.fonts.name -bg $bgcolor
	pack .prefs.main.v.fonts.name -side top -anchor w -pady 4

	label .prefs.main.v.fonts.name.label -text "Font:" -bg $bgcolor \
	    -fg $fgcolor -font $boldfont
	grid .prefs.main.v.fonts.name.label -row 0 -column 0
	
	menubutton .prefs.main.v.fonts.name.family -text "$prefs(font_family)" \
		-borderwidth 2 -relief raised \
		-menu .prefs.main.v.fonts.name.family.menu \
		-bg $bgcolor -highlightbackground $hbcolor \
		-activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	grid .prefs.main.v.fonts.name.family -row 0 -column 1
	
	menu .prefs.main.v.fonts.name.family.menu -tearoff 0 -font $boldfont \
		-bg $bgcolor -activebackground $abcolor -fg $fgcolor \
		-activeforeground $afcolor
	foreach family [font families] {
	    .prefs.main.v.fonts.name.family.menu add command -label "$family" \
		    -command [ list set_font_family_newpref $family ]
	}

	frame .prefs.main.v.fonts.size -bg $bgcolor
	pack .prefs.main.v.fonts.size -side top -anchor w -pady 4

	label .prefs.main.v.fonts.size.label -text "Size:" -bg $bgcolor \
	    -fg $fgcolor -font $boldfont
	grid .prefs.main.v.fonts.size.label -row 0 -column 0

	menubutton .prefs.main.v.fonts.size.size -text "$prefs(font_size)" \
		-borderwidth 2 -relief raised \
		-menu .prefs.main.v.fonts.size.size.menu \
		-bg $bgcolor -highlightbackground $hbcolor \
		-activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	grid .prefs.main.v.fonts.size.size -row 0 -column 1
	
	menu .prefs.main.v.fonts.size.size.menu -tearoff 0 -font $boldfont \
		-bg $bgcolor -activebackground $abcolor -fg $fgcolor \
		-activeforeground $afcolor
	foreach size { 9 10 12 14 16 18 20 24 28 32 36 } {
	    .prefs.main.v.fonts.size.size.menu add command -label "$size" \
		    -command [ list set_font_size_newpref $size ]
	}
	# The size of the text widget will change as the font sizes;
	# so keep it inside a fixed-size box.
	frame .prefs.main.v.fonts.sampf -width 160 -height 80 \
			-relief sunken -borderwidth $textb -bg $bgcolor
	pack .prefs.main.v.fonts.sampf -side top -pady 4
	pack propagate .prefs.main.v.fonts.sampf false

	set tmpfont [ list "-family" $prefs(font_family) "-size" $prefs(font_size) ]
	text .prefs.main.v.fonts.sampf.sample -font $tmpfont \
		-bg $lcolor -highlightbackground $lcolor -fg $fgcolor -relief flat
	pack .prefs.main.v.fonts.sampf.sample -side top
	.prefs.main.v.fonts.sampf.sample insert end "Your triumph is complete."
	.prefs.main.v.fonts.sampf.sample config -state disabled	
    }
    pack .prefs.main.v.fonts -side left -anchor nw
}

proc remove_fonts_prefs_items {} {
    pack forget .prefs.main.v.fonts
}

proc add_files_prefs_items {} {
    global prefs boldfont stdfont
    global bgcolor hbcolor abcolor scolor fgcolor afcolor

    set filesf .prefs.main.v.files
    if { !"[ winfo exists $filesf ]" } {
	frame $filesf -bg $bgcolor

	checkbutton $filesf.listallgames -text " Also list unfinished games" \
		-variable newprefs(listallgames) \
		-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
		-activeforeground $afcolor
	pack $filesf.listallgames -side bottom -anchor w -pady 4

	checkbutton $filesf.checkpoint -text " Save game" \
		-variable newprefs(want_checkpoints) \
		-command { toggle_checkpoints } \
		-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
		-activeforeground $afcolor
	pack $filesf.checkpoint -side left -anchor w

	frame $filesf.cpf -bg $bgcolor
	pack $filesf.cpf -side left -anchor w

	label $filesf.cpf.lab1 -text "every" -bg $bgcolor -fg $fgcolor \
	    -font $boldfont
	entry $filesf.cpf.interval -width 2 -relief sunken \
		-textvariable newprefs(checkpoint_interval) -fg $fgcolor \
		-font $stdfont
	label $filesf.cpf.lab2 -text "turns" -bg $bgcolor -fg $fgcolor \
		-font $boldfont
	pack $filesf.cpf.lab1 $filesf.cpf.interval $filesf.cpf.lab2 -side left
	if { $prefs(want_checkpoints) } {
		.prefs.main.v.files.cpf.interval config -state normal
	} else {
		.prefs.main.v.files.cpf.interval config -state disabled
	}
    }
    pack $filesf -side left -anchor nw -pady 14
}

proc toggle_checkpoints {} {
    global newprefs

    if { $newprefs(want_checkpoints) } {
	set state normal
    } else {
	set state disabled
    }
    .prefs.main.v.files.cpf.interval config -state $state
}

proc remove_files_prefs_items {} {
    pack forget .prefs.main.v.files
}

proc add_network_prefs_items {} {
    global prefs boldfont stdfont
    global bgcolor fgcolor afcolor

    set netf .prefs.main.v.network
    if { !"[ winfo exists $netf ]" } {
	frame $netf -bg $bgcolor
	pack $netf -side top -anchor nw

	label $netf.namelabel -text "Your Name :" -font $boldfont \
	    -bg $bgcolor -fg $fgcolor
	label $netf.hostlabel -text "Game Host :" -font $boldfont \
	    -bg $bgcolor -fg $fgcolor
	label $netf.portlabel -text "Port :" -font $boldfont -bg $bgcolor \
	    -fg $fgcolor

	grid $netf.namelabel -sticky w -column 0 -pady 2
	grid $netf.hostlabel -sticky w -column 0 -pady 2
	grid $netf.portlabel -sticky w -column 0 -pady 2

	entry $netf.nameentry -textvariable newprefs(myname) \
	    -relief sunken -width 15 -fg $fgcolor -font $stdfont
	entry $netf.hostentry -textvariable newprefs(joinhost) \
	    -relief sunken -width 15 -fg $fgcolor -font $stdfont
	entry $netf.portentry -textvariable newprefs(joinport) \
	    -relief sunken -width 4 -fg $fgcolor -font $stdfont

	grid $netf.nameentry -sticky w -column 1 -row 0 -pady 2
	grid $netf.hostentry -sticky w -column 1 -row 1 -pady 2
	grid $netf.portentry -sticky w -column 1 -row 2 -pady 2
    }
    pack $netf -side left -anchor nw
}

proc remove_network_prefs_items {} {
    pack forget .prefs.main.v.network
}

proc add_imagery_prefs_items {} {
    global prefs boldfont
    global imagery_option_names
    global bgcolor hbcolor abcolor scolor fgcolor afcolor

    set imageryf .prefs.main.v.imagery
    if { !"[ winfo exists $imageryf ]" } {
	frame $imageryf -bg $bgcolor

	checkbutton $imageryf.terrain_images \
		-text " $imagery_option_names(terrain_images)" \
		-variable newprefs(terrain_images) \
		-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
		-activeforeground $afcolor
	pack $imageryf.terrain_images -side top -anchor w -pady 2
	checkbutton $imageryf.terrain_patterns \
		-text " $imagery_option_names(terrain_patterns)" \
		-variable newprefs(terrain_patterns) \
		-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
		-activeforeground $afcolor
	pack $imageryf.terrain_patterns -side top -anchor w -pady 2
	checkbutton $imageryf.transitions \
		-text " $imagery_option_names(transitions)" \
		-variable newprefs(transitions) \
		-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
		-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
		-activeforeground $afcolor
	pack $imageryf.transitions -side top -anchor w -pady 2
    }
    pack $imageryf -side left -anchor nw
}

proc remove_imagery_prefs_items {} {
    pack forget .prefs.main.v.imagery
}

proc set_power_pref { val } {
    global newprefs

    set newprefs(power) $val
    .prefs.main.v.map.power config -text "Power $newprefs(power)"
}

proc set_font_family_newpref { val } {
    global newprefs

    set newprefs(font_family) $val
    if { "[ winfo exists .prefs.main.v.fonts ]" } {
	.prefs.main.v.fonts.name.family config -text "$newprefs(font_family)"
	set tmpfont [ list "-family" $newprefs(font_family) \
		"-size" $newprefs(font_size) ]
	.prefs.main.v.fonts.sampf.sample config -font $tmpfont
    }
}

proc set_font_size_newpref { val } {
    global newprefs

    set newprefs(font_size) $val
    if { "[ winfo exists .prefs.main.v.fonts ]" } {
	.prefs.main.v.fonts.size.size config -text "$newprefs(font_size)"
	set tmpfont [ list "-family" $newprefs(font_family) \
		"-size" $newprefs(font_size) ]
    	.prefs.main.v.fonts.sampf.sample config -state normal
	.prefs.main.v.fonts.sampf.sample config -font $tmpfont
    	.prefs.main.v.fonts.sampf.sample config -state disabled
    }
}

proc set_pref_value { pref val } {
    global prefs
    global stdfont boldfont

#    puts stdout "Setting prefs($pref) = $val"
    set prefs($pref) $val
}

proc select_prefs_topic {} {
    global last_prefs_topic

    set i [ .prefs.main.topics.list curselection ]
    if { $i == "" } {
    	return
    }
    set str [ .prefs.main.topics.list get $i ]
#    puts stdout "want $str prefs"
    if { $str == $last_prefs_topic } {
	return
    }
    if { $last_prefs_topic == " Map View" } {
	remove_map_prefs_items
    } elseif { $last_prefs_topic == " Fonts" } {
	remove_fonts_prefs_items
    } elseif { $last_prefs_topic == " Files" } {
	remove_files_prefs_items
    } elseif { $last_prefs_topic == " Imagery" } {
	remove_imagery_prefs_items
    } elseif { $last_prefs_topic == " Network" } {
	remove_network_prefs_items
    }
    if { $str == " Map View" } {
	add_map_prefs_items
    } elseif { $str == " Fonts" } {
	add_fonts_prefs_items
    } elseif { $str == " Files" } {
	add_files_prefs_items
    } elseif { $str == " Imagery" } {
	add_imagery_prefs_items
    } elseif { $str == " Network" } {
	add_network_prefs_items
    }
    set last_prefs_topic $str
}

# Accept the new preference settings, copying them into the prefs array
# and saving into a file.

proc ok_preferences {} {
    global view_option_list
    global view_option_flags
    global imagery_option_list
    global prefs newprefs
    global nummaps map_widget
    global stdfont bigfont boldfont titlefont hugefont

    set nums [ numsides ]
    set dside [ dside ]
    set numtreas [ numtreasury ]
    set numscores [ numscorekeepers ]

    # Set the view option preferences.
    foreach opt $view_option_list {
	set_pref_value $opt $newprefs($opt)
    }
    # Set the font preferences.
    set_pref_value font_family $newprefs(font_family)
    set_pref_value font_size $newprefs(font_size)
    set stdfont [ list "-family" $prefs(font_family) \
	    "-size" $prefs(font_size) ]
    set boldfont [ list "-family" $prefs(font_family) \
	    "-size" $prefs(font_size) "-weight" "bold" ]
    set hugefont [ list "-family" $prefs(font_family) \
	    "-size" [ expr $prefs(font_size) * 3 ] "-weight" "bold" ]
    if { $prefs(font_size) < 10 } {
	    set bigfont [ list "-family" $prefs(font_family) \
		    "-size" [ expr $prefs(font_size) + 1 ] ]
	    set titlefont [ list "-family" $prefs(font_family) \
		    "-size" [ expr $prefs(font_size) + 1 ] "-weight" "bold" ]
    } else {
	    set bigfont [ list "-family" $prefs(font_family) \
		    "-size" [ expr $prefs(font_size) + 2 ] ]
	    set titlefont [ list "-family" $prefs(font_family) \
		    "-size" [ expr $prefs(font_size) + 2 ] "-weight" "bold" ]
    }
    # Update existing map windows that use text.
    for { set i 1 } { $i <= $nummaps } { incr i } {
	set map $map_widget($i)
	$map.leftside.topside.notices.t config -font $stdfont
	$map.leftside.topside.notices.t tag config prefix -font $boldfont
	$map.leftside.topside.notices.t tag config prompt -font $boldfont
	$map.leftside.topside.notices.t tag config answer -font $boldfont
	$map.leftside.botside.mouseover config -font $stdfont
	$map.leftside.botside.uf.unitinfo itemconfig textual -font $stdfont
#	$map.leftside.botside.mapf.mapf2.map config -font $boldfont
	# Update the fonts used in the date pane.
	set turnpane $map.rightside.turnf.turn
	$turnpane itemconfig the_date -font $boldfont
	# Update the fonts used in the side list.
    	set sidelist $map.rightside.gamef.game
    	for { set j 1 } { $j <= $nums } { incr j } {
    		if { $j == $dside } {
	 		$sidelist itemconfig s$j -font $boldfont
	 	} else {
	 		$sidelist itemconfig s$j -font $stdfont
		}
		$sidelist itemconfig player$j -font $stdfont
		for { set k 1 } { $k <= $numscores } { incr k } {
			$sidelist itemconfig score$k$j -font $stdfont
		}
		if { $numtreas > 0 && $j == $dside } {
			set rows [ expr ($numtreas + 1) / 2 ]
			set k 0
	    		for { set row 0 } { $row < $rows } { incr row } {
				$sidelist itemconfig m$j,$k -font $stdfont
				$sidelist itemconfig mt$j,$k -font $stdfont
				incr k
				if { $k < $numtreas } {
				    $sidelist itemconfig m$j,$k -font $stdfont
				    $sidelist itemconfig mt$j,$k -font $stdfont
				    incr k
       				}
       			}
       		}
	}
	# Update the fonts used in the unit type list.
	set unitlist $map.rightside.listf.unitlist
	$unitlist itemconfig type -font $stdfont
	$unitlist itemconfig num -font $stdfont
	set numu [ numutypes_available $dside ]
	for { set j 0 } { $j < $numu } { incr j } {
		$unitlist itemconfig u$j -font $stdfont
		$unitlist itemconfig n$j -font $stdfont
		$unitlist itemconfig i$j -font $stdfont
		$unitlist itemconfig name$j -font $stdfont
	}
	$unitlist itemconfig material -font $boldfont
	$unitlist itemconfig supply -font $stdfont
	$unitlist itemconfig production -font $stdfont
	$unitlist itemconfig capacity -font $stdfont
	set numm [ nummtypes ]
	for { set j 0 } { $j < $numm } { incr j } {
		$unitlist itemconfig material$j -font $stdfont
		$unitlist itemconfig supply$j -font $stdfont
		$unitlist itemconfig production$j -font $stdfont
		$unitlist itemconfig capacity$j -font $stdfont
	}
    }
    # Update fonts in the help window.
    if { [ winfo exists .help ] } {
	.help.top.help config -font $boldfont 
	.help.top.prev config -font $boldfont 
	.help.top.next config -font $boldfont 
	.help.top.back config -font $boldfont 
	.help.top.close config -font $boldfont 
	.help.bot.topics.list config -font $stdfont
	.help.bot.t.txt config -font $stdfont
	.help.bot.t.txt tag config heading -font $titlefont
    }
    # Update fonts in the scores window.
    if { [ winfo exists .scores ] } {
	.scores.bottom.close config -font $boldfont
	.scores.main.text config -font $stdfont
    }
    # Update fonts in the research window.
    if { [ winfo exists .research ] } {
	.research.current config -font $boldfont
	.research.bottom.ok config -font $boldfont
	.research.bottom.rest config -font $boldfont
	.research.bottom.cancel config -font $boldfont
	.research.topics config -font $stdfont
    }
    # Update fonts in the prefs window and its subframes.
    if { [ winfo exists .prefs ] } {
	.prefs.main.topics.list config -font $stdfont
	.prefs.bot.ok config -font $boldfont
	.prefs.bot.cancel config -font $boldfont
	if { [ winfo exists .prefs.main.v.map ] } {
		.prefs.main.v.map.power config -font $boldfont
		.prefs.main.v.map.power.menu config -font $boldfont
		foreach opt $view_option_list {
			if { [ winfo exists .prefs.main.v.map.checks.$opt ] } {
			    .prefs.main.v.map.checks.$opt config -font $boldfont
			}
		}
	}
	if { [ winfo exists .prefs.main.v.fonts ] } {
		.prefs.main.v.fonts.name.label config -font $boldfont
		.prefs.main.v.fonts.name.family config -font $boldfont
		.prefs.main.v.fonts.name.family.menu config -font $boldfont
		.prefs.main.v.fonts.size.label config -font $boldfont
		.prefs.main.v.fonts.size.size config -font $boldfont
		.prefs.main.v.fonts.size.size.menu config -font $boldfont
	}    	
	if { [ winfo exists .prefs.main.v.files ] } {
		.prefs.main.v.files.checkpoint config -font $boldfont
		.prefs.main.v.files.cpf.lab1 config -font $boldfont
		.prefs.main.v.files.cpf.interval config -font $stdfont
		.prefs.main.v.files.cpf.lab2 config -font $boldfont
	}
	if { [ winfo exists .prefs.main.v.network ] } {
		.prefs.main.v.network.namelabel config -font $boldfont	
		.prefs.main.v.network.hostlabel config -font $boldfont	
		.prefs.main.v.network.portlabel config -font $boldfont	
		.prefs.main.v.network.nameentry config -font $stdfont	
		.prefs.main.v.network.hostentry config -font $stdfont	
		.prefs.main.v.network.portentry config -font $stdfont	
	}
	if { [ winfo exists .prefs.main.v.imagery ] } {
		.prefs.main.v.imagery.terrain_images config -font $boldfont
		.prefs.main.v.imagery.terrain_patterns config -font $boldfont
		.prefs.main.v.imagery.transitions config -font $boldfont
	}
    }
    # Update fonts in the design window and its subframes.
    if { [ winfo exists .design ] } {
    	.design.dbuttons.brush config -font $boldfont
    	.design.dbuttons.brush.menu config -font $boldfont
#    	.design.design.done config -font $boldfont
	if { [ winfo exists .design.design.fnormal ] } {
		.design.design.fnormal.label config -font $boldfont
	}
	if { [ winfo exists .design.design.fterrain ] } {
		.design.design.fterrain.label config -font $boldfont
		.design.design.fterrain.fg config -font $boldfont
		.design.design.fterrain.fg.menu config -font $boldfont
		.design.design.fterrain.bg config -font $boldfont
		.design.design.fterrain.bg.menu config -font $boldfont
	}
	if { [ winfo exists .design.design.funit ] } {
		.design.design.funit.label config -font $boldfont
		.design.design.funit.type config -font $boldfont
		.design.design.funit.type.menu config -font $boldfont
		.design.design.funit.side config -font $boldfont
		.design.design.funit.side.menu config -font $boldfont
	}
	if { [ winfo exists .design.design.fpeople ] } {
		.design.design.fpeople.label config -font $boldfont
		.design.design.fpeople.people config -font $boldfont
		.design.design.fpeople.people.menu config -font $boldfont
	}
	if { [ winfo exists .design.design.fcontrol ] } {
		.design.design.fcontrol.label config -font $boldfont
		.design.design.fcontrol.control config -font $boldfont
		.design.design.fcontrol.control.menu config -font $boldfont
	}
	if { [ winfo exists .design.design.ffeature ] } {
		.design.design.ffeature.label config -font $boldfont
		.design.design.ffeature.name config -font $boldfont
		.design.design.ffeature.name.menu config -font $boldfont
		.design.design.ffeature.buttons.new config -font $boldfont
		.design.design.ffeature.buttons.rename config -font $boldfont
		.design.design.ffeature.buttons.delete config -font $boldfont
		.design.design.ffeature.fields.namelabel config -font $boldfont
		.design.design.ffeature.fields.hostlabel config -font $boldfont
		.design.design.ffeature.fields.newname config -font $stdfont
		.design.design.ffeature.fields.typename config -font $stdfont
	}
	if { [ winfo exists .design.design.fmaterial ] } {
		.design.design.fmaterial.label config -font $boldfont
	}
	if { [ winfo exists .design.design.felevation ] } {
		.design.design.felevation.label config -font $boldfont
		.design.design.felevation.entries.lab1 config -font $boldfont
		.design.design.felevation.entries.lab2 config -font $boldfont
		.design.design.felevation.entries.entry config -font $stdfont
		.design.design.felevation.entries.ventry config -font $stdfont
		.design.design.felevation.entries.lab3 config -font $boldfont
		.design.design.felevation.buttons.set config -font $boldfont
		.design.design.felevation.buttons.raise config -font $boldfont
		.design.design.felevation.buttons.lower config -font $boldfont
		.design.design.felevation.buttons.fix config -font $boldfont
		.design.design.felevation.state1 config -font $boldfont
		.design.design.felevation.state2 config -font $boldfont
	}
	if { [ winfo exists .design.design.ftemperature ] } {
		.design.design.ftemperature.label1 config -font $boldfont
		.design.design.ftemperature.set config -font $boldfont
		.design.design.ftemperature.entry config -font $stdfont
		.design.design.ftemperature.label2 config -font $boldfont
	}
	if { [ winfo exists .design.design.fclouds ] } {
		.design.design.fclouds.label config -font $boldfont
		.design.design.fclouds.entries.dlab config -font $boldfont
		.design.design.fclouds.entries.blab config -font $boldfont
		.design.design.fclouds.entries.hlab config -font $boldfont
		.design.design.fclouds.entries.dentry config -font $boldfont
		.design.design.fclouds.entries.dentry.menu config -font $boldfont
		.design.design.fclouds.entries.bentry config -font $stdfont
		.design.design.fclouds.entries.hentry config -font $stdfont
		.design.design.fclouds.entries.set config -font $boldfont
	}
	if { [ winfo exists .design.design.fwinds ] } {
		.design.design.fwinds.label config -font $boldfont
		.design.design.fwinds.entries.dlab config -font $boldfont
		.design.design.fwinds.entries.flab config -font $boldfont
		.design.design.fwinds.entries.dir config -font $boldfont
		.design.design.fwinds.entries.dir.menu config -font $boldfont
		.design.design.fwinds.entries.force config -font $boldfont
		.design.design.fwinds.entries.force.menu config -font $boldfont
	}
	if { [ winfo exists .design.design.fview ] } {
		.design.design.fview.label config -font $boldfont
	}
    }
    # Update fonts in designer save window.
    if { [ winfo exists .dsave ] } {
    	.dsave.top.modulelabel config -font $boldfont
    	.dsave.top.modulename config -font $stdfont
    	.dsave.f1.all config -font $boldfont
    	.dsave.f1.types config -font $boldfont
    	.dsave.f1.tables config -font $boldfont
    	.dsave.f1.globals config -font $boldfont
    	.dsave.f1.scoring config -font $boldfont
    	.dsave.f1.world config -font $boldfont
    	.dsave.f1.area config -font $boldfont
    	.dsave.f1.terrain config -font $boldfont
    	.dsave.f1.areamisc config -font $boldfont
    	.dsave.f1.weather config -font $boldfont
    	.dsave.f1.material config -font $boldfont
    	.dsave.f1.sides config -font $boldfont
    	.dsave.f1.sideviews config -font $boldfont
    	.dsave.f1.sidedocts config -font $boldfont
    	.dsave.f1.players config -font $boldfont
    	.dsave.f1.units config -font $boldfont
    	.dsave.f1.unitids config -font $boldfont
    	.dsave.f1.unitprops config -font $boldfont
    	.dsave.f1.unitactions config -font $boldfont
    	.dsave.f1.unitplans config -font $boldfont
    	.dsave.f1.history config -font $boldfont
    	.dsave.f1.save config -font $boldfont
    	.dsave.f1.cancel config -font $boldfont
    }
    # Update fonts in contour window.
    if { [ winfo exists .contour ] } {
    	.contour.label config -font $boldfont
    	.contour.interval config -font $stdfont
    	.contour.bottom.ok config -font $boldfont
    	.contour.bottom.cancel config -font $boldfont
    }    	
    # Update fonts in meridian window.
    if { [ winfo exists .meridian ] } {
    	.meridian.label config -font $boldfont
    	.meridian.interval config -font $stdfont
    	.meridian.bottom.ok config -font $boldfont
    	.meridian.bottom.cancel config -font $boldfont
    }
    # Update fonts in find window.
    if { [ winfo exists .find ] } {
    	.find.label config -font $boldfont
    	.find.name config -font $stdfont
    	.find.matches config -font $boldfont
    	.find.bottom.find config -font $boldfont
    	.find.bottom.cancel config -font $boldfont
    }
    # Set the files preferences.
    set_pref_value listallgames $newprefs(listallgames)
    set_pref_value want_checkpoints $newprefs(want_checkpoints)
    set_pref_value checkpoint_interval $newprefs(checkpoint_interval)
    # Set the imagery preferences.
    foreach opt $imagery_option_list {
	set_pref_value $opt $newprefs($opt)
	for { set i 1 } { $i <= $nummaps } { incr i } {
	    set mapv $map_widget($i).leftside.botside.mapf.mapf2.map
	    $mapv config -terrainimages $prefs(terrain_images)
	    $mapv config -terrainpatterns $prefs(terrain_patterns)
	    $mapv config -transitions $prefs(transitions)
	}
    }
    # Set the network preferences.
    set_pref_value joinhost $newprefs(joinhost)
    set_pref_value joinport $newprefs(joinport)
    set_pref_value myname $newprefs(myname)
    
    save_preferences
    dismiss_preferences_dialog
}

# Make the dialog go away, without altering any preferences.

proc dismiss_preferences_dialog {} {
    withdraw_window ".prefs"
}

# Create and popup the help window.

proc popup_help_dialog {} {
    global stdfont titlefont boldfont
    global scrollb
    global listb
    global textb
    global bgcolor hbcolor abcolor tcolor lcolor fgcolor afcolor
    global tcl_platform
    
    # Disable the help system on MacOS < 8.6 since it crashes there.
    if { "$tcl_platform(platform)" == "macintosh" && $tcl_platform(osVersion) < 8.6 } {
	return
    }

    if { [ winfo exists .help ] } {
	# Kick the window manager awake.
	wm withdraw .help
	wm deiconify .help
	focus -force .help
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .help -bg $bgcolor
    wm withdraw .help
    wm title .help "Xconq Help"

    frame .help.top -bg $bgcolor
    pack .help.top -side top -fill x -padx 2

    button .help.top.help -text "Help" \
	-command { help_goto help } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .help.top.prev -text "Prev" \
	-command { help_goto prev } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .help.top.next -text "Next" \
	-command { help_goto next } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .help.top.back -text "Back" \
	-command { help_goto back } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .help.top.close -text "Close" \
	-command { dismiss_help_dialog } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack .help.top.help .help.top.prev .help.top.next -side left -padx 2 -pady 4
    pack .help.top.back .help.top.close -side left -padx 2 -pady 4

    frame .help.bot -bg $bgcolor
    pack .help.bot -side bottom -fill both -expand true

    frame .help.bot.topics -bg $lcolor
    pack .help.bot.topics -side left -fill both -expand true
    listbox .help.bot.topics.list -yscrollcommand [ list .help.bot.topics.yscroll set ]  \
    	-selectmode browse -font $stdfont -relief sunken -borderwidth $listb -bg $lcolor -fg $fgcolor
    scrollbar .help.bot.topics.yscroll -orient vert \
	-command [ list .help.bot.topics.list yview ] -width 16 -borderwidth $scrollb \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack .help.bot.topics.yscroll -side right -fill y
    pack .help.bot.topics.list -side left -fill both -expand true

    bind .help.bot.topics.list <ButtonRelease-1> { select_help_topic }

    frame .help.bot.t -relief sunken -borderwidth $textb -bg $lcolor
    text .help.bot.t.txt -width 60 -height 30 -font $stdfont -wrap word -relief flat \
	-yscrollcommand { .help.bot.scroll set } \
	-bg $lcolor -highlightbackground $lcolor -fg $fgcolor -highlightthickness 0 -cursor arrow
    scrollbar .help.bot.scroll -command { .help.bot.t.txt yview } -width 16 -borderwidth $scrollb \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack .help.bot.scroll -side right -fill y
    pack .help.bot.t -side right -fill both -expand true
    pack .help.bot.t.txt -side left -fill both -expand true -padx 10

    Tree .help.bot.t.txt.tree -padx 4 -relief ridge -borderwidth 1 \
	-bg $bgcolor -deltax 30 -deltay 20 -highlightbackground $lcolor \
	-height 0 -width 0 -highlightcolor $lcolor
    canvas .help.bot.t.txt.img -width 44 -height 48 \
	-bg $lcolor -highlightbackground $lcolor
#    imfsample .help.bot.t.txt.img.samp -width 44 -height 48 \
#	-iwidth 44 -iheight 48 -bg $lcolor
    imfsample .help.bot.t.txt.img.samp -width 32 -height 32 \
	-iwidth 32 -iheight 32 -bg $lcolor
    .help.bot.t.txt.img create window 2 2 -anchor nw \
	    -window .help.bot.t.txt.img.samp
    .help.bot.t.txt window create end -window .help.bot.t.txt.img
    .help.bot.t.txt insert end "(heading)" heading
    .help.bot.t.txt tag config heading -font $titlefont
    .help.bot.t.txt insert end "\n"
    .help.bot.t.txt insert end "(text)" body
    .help.bot.t.txt config -state disabled

    init_help_list
    # Select the "map" entry in the list.
    .help.bot.topics.list selection set 1
    help_goto "map"
    # But scroll to the head of the list.
    .help.bot.topics.list see 0				
    center_above_curmap .help
}

# Dig up the selected topic and go to that node.

proc select_help_topic {} {
    set i [ .help.bot.topics.list curselection ]
    if { $i == "" } {
    	return
    }
    set str [ .help.bot.topics.list get $i ]
    # Remove the leading white space.
    set str2 [ string trimleft $str ]
    help_goto "$str2"
}

# Given a help topic key, add it to the list of topics.  This is called
# from C code.

proc add_help_topic_key { key } {
    .help.bot.topics.list insert end " $key"
}

# This proc is called from C code to actually fill in the help window
# with help topic and text.

proc update_help { key contents nclass arg } {
    global bgcolor

    .help.bot.t.txt configure -state normal
    .help.bot.t.txt delete heading.first heading.last
    .help.bot.t.txt insert 1.1 "$key" heading
    if { $nclass == "u" } {
	.help.bot.t.txt.img.samp replace imf [ u_image_name $arg ]
    } elseif { $nclass == "t" } {
	.help.bot.t.txt.img.samp replace imf [ t_image_name $arg ]
    } else {
	.help.bot.t.txt.img.samp remove imf foo
    }
    .help.bot.t.txt delete body.first body.last
    .help.bot.t.txt insert end "$contents" body
    .help.bot.t.txt tag add all 1.0 end
    .help.bot.t.txt tag configure all -spacing1 1 -spacing2 2 -spacing3 1

    if { $nclass == "u" } { 
    	set rxp "Required advances to build: "
	set xy [.help.bot.t.txt search $rxp 0.0 end]
	if { $xy != "" } {
             set xy $xy+[string length $rxp]chars
             set le [.help.bot.t.txt search . $xy]
             set te ""
             set cnt 0
             while { $te != $le } {
                 set te [.help.bot.t.txt search , $xy $le]
                 if { $te == "" } { set te $le }
                 .help.bot.t.txt tag add tag$cnt $xy $te
                 .help.bot.t.txt tag configure tag$cnt -relief raised -borderwidth 1 \
                 		-background $bgcolor
                 set moji [.help.bot.t.txt get $xy $te]
                 .help.bot.t.txt tag bind tag$cnt <Button-1> "help_goto \"$moji\""
                 incr cnt
                 set xy $te+1chars
             }
	}
    }
    if { $nclass == "a" } {
	set cnt 0
	foreach rxp { "Prerequisites: " "Needed for: " "Enables: "} {
             set tmp [.help.bot.t.txt search $rxp 0.0 end]
             if { $tmp != "" } {
             	set tmp $tmp+[string length $rxp]chars
             	set xy $tmp
             	set le [.help.bot.t.txt search . $xy]
             	set te ""
             	while { $te != $le } {
                     set te [.help.bot.t.txt search , $xy $le]
                     if { $te == "" } { set te $le }
                     .help.bot.t.txt tag add tagg$cnt $xy $te
                     .help.bot.t.txt tag configure tagg$cnt -relief raised -borderwidth 1 \
                     	-background $bgcolor
                     set moji [.help.bot.t.txt get $xy $te]
                     .help.bot.t.txt tag bind tagg$cnt <Button-1> "help_goto \"$moji\""
                     incr cnt
                     set xy $te+2chars
                 }		
             }
    	}
    }

    .help.bot.t.txt yview moveto 0
    .help.bot.t.txt configure -state disabled
    .help.bot.t.txt window create end -window .help.bot.t.txt.tree -pady 10
    .help.bot.t.txt.tree configure -height 0 -width 0
    .help.bot.t.txt.tree delete [.help.bot.t.txt.tree nodes root]
    # This update is essential for correct redrawing of the tree in MacTCL. 	
    update idletasks
    if { $nclass == "a"} {
	.help.bot.t.txt.tree bindText <ButtonPress-1> tree_node_clicked
	.help.bot.t.txt.tree insert end root 1 -text $key -open 1
	add_tree_node 1 1
	.help.bot.t.txt.tree configure -height [llength [.help.bot.t.txt.tree visiblenodes]] -width 40
    }
    # Update the current selection in the help topics list.
    .help.bot.topics.list selection clear 0 end    
    for { set i 0 } { $i < [.help.bot.topics.list size] } { incr i } {
	set str [ .help.bot.topics.list get $i ]
	# Remove the leading white space.
	set str2 [ string trimleft $str ]
    	if { $str2 == $key} {
		.help.bot.topics.list selection set $i
		.help.bot.topics.list see $i				
		break
	}
    }
}

proc tree_node_clicked { node } {
    help_goto "[.help.bot.t.txt.tree itemcget $node -text]"
}

proc add_tree_node { key dep } {
    set temp_node_text [.help.bot.t.txt.tree itemcget $key -text]
    for { set i 0 } { $i < [numatypes]} { incr i } {
        if { [atype_name $i] == $temp_node_text } {
            set num $i
	    break
	}
    }

    for { set i 0 } { $i < [numatypes] } { incr i } {
        if { [ advance_needed_to_research $num $i] == 1} {
            incr dep 
            if { $dep > 6 } { break } 
            set addn $key$dep
            .help.bot.t.txt.tree insert end $key $addn -text "[atype_name $i]" -open 1
            add_tree_node $addn $dep
        }
    }
}

proc scrolled_listbox { f args } {
    global scrollb
    global listb
    global bgcolor hbcolor abcolor tcolor lcolor bgcolor

    frame $f -bg $bgcolor
    listbox $f.list -yscrollcommand [ list $f.yscroll set ] \
    	-relief sunken -borderwidth $listb -bg $lcolor -fg $fgcolor
    eval { $f.list config } $args
    scrollbar $f.yscroll -orient vert \
	    -command [ list $f.list yview ] -width 16 -borderwidth $scrollb \
	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack $f.yscroll -side right -fill y
    pack $f.list -side left -fill both -expand true
}

proc scrolled_textbox { f args } {
    global scrollb
    global bgcolor hbcolor abcolor tcolor lcolor bgcolor

    frame $f -bg $bgcolor
    text $f.text -yscrollcommand [ list $f.yscroll set ]
 	    -bg $lcolor -highlightbackground $lcolor -fg $fgcolor
    eval { $f.text config } $args
    scrollbar $f.yscroll -orient vert \
	    -command [ list $f.text yview ] -width 16 -borderwidth $scrollb \
 	    -bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor -troughcolor $tcolor
    pack $f.yscroll -side right -fill y
    pack $f.text -side top -fill both -expand true
}

# Make the dialog go away.

proc dismiss_help_dialog {} {
    withdraw_window ".help"
}

# Game save dialog.

set save_filename ""

proc popup_game_save { name forcepopup } {
    global designer
    global save_filename
    global tcl_platform

    if { $designer } {
	popup_designer_save
	return
    }
    set dname [ file dirname $name ]
    set fname [ file tail $name ]
    if { $forcepopup } {
        # -initialfile does not work correctly on the mac (the TCL 8.3 docs 
        # claim that it is just ignored, but this is not true. Rather, the 
        # name passed by -initialfile is used, but the save directory is not 
        # set correctly). A fix for this is now implemented below.
	set filename [ tk_getSaveFile -initialfile $fname -initialdir $dname ]
    } else {
	set filename $fname
    }
    # Note that we can't just execute the long command "save",
    # because it calls this proc.
    if { "$filename" != "" } {
    	# We need to set the save directory manually in both MacOS and Windows.
	set home [ pwd ]
        	cd $dname
	game_save $filename
     	cd $home
    }
}

# Game end dialogs.

proc popup_game_over_dialog { fate } {
    global stdfont hugefont boldfont
    global tcl_platform
    global prefs
    global bgcolor fgcolor afcolor

    # No more mode switching.
    disable_move_mode
    # (should change all action buttons)
    # (should) Let the display settle down a bit.
    # sleep 1
    # Update the scores list.
    if { "[ winfo exists .scores ]" } {
    	.scores.main.text configure -state normal
	.scores.main.text delete 1.0 end
	.scores.main.text insert end [ get_scores ]
    	.scores.main.text configure -state disabled
    }
    if { "$fate" == "won" } {
	set msg "You Won!"
	set color PaleGreen1
    } elseif { "$fate" == "lost" } {
	set msg "You Lost!"
	set color pink
    } else {
	set msg "Game Over!"
	set color $bgcolor
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .gameover -bg $color
    wm withdraw .gameover
    wm title .gameover "Xconq Game Over"

    frame .gameover.top -width 280 -height 210 -bg $color
    pack .gameover.top -side top -fill x

    label .gameover.top.fate -text $msg -font $hugefont -bg $color -fg $fgcolor
    pack .gameover.top.fate -padx 4 -pady 4

    button .gameover.quitnow -text "Quit Now" \
	-command { .gameover.quitnow flash
			  dismiss_game_over_dialog
			  set_want_to_exit 1
	} \
	-padx 20 -pady 8 -default active \
	-bg $color -highlightbackground $color -activebackground $color \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack .gameover.quitnow -side top -padx 4 -pady 4

    bind .gameover <Key> { 
    	if {"%K" == "Return"} { 
    		.gameover.quitnow flash
    		dismiss_game_over_dialog 
		set_want_to_exit 1
    	} 
    }

    label .gameover.hint -width 36 -font $stdfont -bg $color \
    	-text "If you continue, you can look around\n and see how the game ended." \
    	-fg $fgcolor
    pack .gameover.hint -side top -padx 4 -pady 4

    button .gameover.continue -text "Continue" \
	-command { .gameover.continue flash
			  dismiss_game_over_dialog 
	} -padx 20 -pady 8 \
	-bg $color -highlightbackground $color -activebackground $color \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack .gameover.continue -side top -padx 4 -pady 16

    center_above_curmap .gameover
}

proc dismiss_game_over_dialog {} {
    withdraw_window ".gameover"
}

proc disable_move_mode {} {
    global nummaps map_widget

    for { set i 1 } { $i <= $nummaps } { incr i } {
	$map_widget($i).leftside.botside.buttons.move config -state disabled
    }
}

# Error/warning dialogs.

set suppress_warnings 0

proc popup_init_error_dialog { str } {
    tk_messageBox -type ok -icon error \
	    -message "Fatal Setup Error: $str\n\n\
	    Something is seriously wrong, either with Xconq \
	    or with the game that you chose."
}

proc popup_init_warning_dialog { str } {
    global suppress_warnings

    if { $suppress_warnings } {
	return
    }
    set rslt [ tk_messageBox -type yesno -icon warning \
	    -message "Warning: $str\n\
	    Xconq may not be able to give you exactly the game \
	    that you asked for.  Do you want to continue \
	    setting up this game?" ]
    if { $rslt == "no" } {
	exit_xconq
    }
    # (should try to reuse dialog - may need custom)
    set rslt [ tk_messageBox -type yesno -icon question \
	    -message "Do you want to see any further warnings?" ]
    if { $rslt == "no" } {
	set suppress_warnings 1
    }
}

proc popup_run_error_dialog { str } {
    set rslt [ tk_messageBox -type yesno -icon error \
	    -message "Fatal Error: $str\n\
	    Xconq cannot possibly continue on, \
	    but may able to save the game; \
	    would you like to try to save it?" ]
    if { $rslt == "yes" } {
	execute_long_command 0 "save"
    }
    # Exiting will be done by the program. (why?)
}

proc popup_run_warning_dialog { str } {
    global suppress_warnings
    global endofgame

    if { $suppress_warnings } {
	return
    }
    if { !$endofgame } {
	set rslt [ tk_messageBox -type yesno -icon warning \
		-message "Warning: $str\n\
		This is not fatal, \
		but may cause more serious problems later on.\
		Do you want to continue playing this game?\
		(if `no', you will get a chance to save it)" ]
	if { $rslt == "no" } {
	    # (should try to reuse dialog - may need custom)
	    set rslt [ tk_messageBox -type yesno -icon warning \
		    -message "Would you like to save the game?" ]
	    if { $rslt == "yes" } {
		execute_long_command 0 "save"
	    }
	    exit_xconq
	}
    } else {
	# The post-end-of-game version doesn't offer to save.
	set rslt [ tk_messageBox -type yesno -icon warning \
		-message "Warning: $str\n\
		Do you want to continue?" ]
	if { $rslt == "no" } {
	    exit_xconq
	}
    }
    # (should try to reuse dialog - may need custom)
    set rslt [ tk_messageBox -type yesno -icon question \
	    -message "Do you want to see any further warnings?" ]
    if { $rslt == "no" } {
	set suppress_warnings 1
    }
    reset_popup_flag
}

# Designer support.

set selected_design_tool normal

set last_dbutton .design.dbuttons.normal
set last_frame .design.design.fnormal

set curfid 0

proc popup_design_palette {} {
    global bgcolor hbcolor abcolor fgcolor afcolor boldfont designer

    set designer 1
    if { "[ winfo exists .design ]" } {
	# Make the palette appear again.
	wm withdraw .design
	wm deiconify .design
	focus -force .design
    } else {
	# Create the design palette from scratch.
	toplevel .design -bg $bgcolor
	wm withdraw .design
	wm title .design "Xconq Design"

	# Make sure close_design_palette is called even if the close box is used.
	wm protocol .design WM_DELETE_WINDOW close_design_palette

	# Create a new frame for the design buttons.
	frame .design.dbuttons -bg $bgcolor

	# First column of buttons.
	button .design.dbuttons.normal -bitmap looking_glass \
		-width 24 -height 24 \
		-command { select_paint normal make_normal_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.terrain -bitmap paint_cell \
		-width 24 -height 24 \
		-command { select_paint terrain make_terrain_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.unit -bitmap paint_unit \
		-width 24 -height 24 \
		-command { select_paint unit make_unit_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.people -bitmap paint_people \
		-width 24 -height 24 \
		-command { select_paint people make_people_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.control -bitmap paint_control \
		-width 24 -height 24 \
		-command { select_paint control make_control_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.feature -bitmap paint_feature \
		-width 24 -height 24 \
		-command { select_paint feature make_feature_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont

	# Second column.
	button .design.dbuttons.material -bitmap paint_material \
		-width 24 -height 24 -state disabled \
		-command { select_paint material make_material_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.elevation -bitmap paint_elev \
		-width 24 -height 24 \
		-command { select_paint elevation make_elev_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.temperature -bitmap paint_temp \
		-width 24 -height 24 \
		-command { select_paint temperature make_temp_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.clouds -bitmap paint_clouds \
		-width 24 -height 24 \
		-command { select_paint clouds make_clouds_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.winds -bitmap paint_winds \
		-width 24 -height 24 \
		-command { select_paint winds make_winds_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	button .design.dbuttons.view -bitmap paint_view \
		-width 24 -height 24 \
		-command { select_paint view make_view_paint_frame } \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	menubutton .design.dbuttons.brush -text "0" -width 1 -height 1 \
		-background white -borderwidth 2 -relief raised \
		-menu .design.dbuttons.brush.menu \
		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
		-fg $fgcolor -activeforeground $afcolor -font $boldfont
	menu .design.dbuttons.brush.menu -tearoff 0 -font $boldfont \
		-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
	for { set i 0 } { $i < 10 } { incr i } {
	    .design.dbuttons.brush.menu add command -label "$i" \
		    -command [ list dbg_set_design_data curbrushradius $i 0 ]
	}

	grid .design.dbuttons.normal .design.dbuttons.material
	grid .design.dbuttons.terrain .design.dbuttons.elevation
	grid .design.dbuttons.unit .design.dbuttons.temperature
	grid .design.dbuttons.people .design.dbuttons.clouds
	grid .design.dbuttons.control .design.dbuttons.winds
	grid .design.dbuttons.feature .design.dbuttons.view
	grid .design.dbuttons.brush -columnspan 2 -sticky news

	# Create the frame that will enclose info about painting.
	frame .design.design -width 200 -height 150 \
		-borderwidth 2 -relief sunken -bg $bgcolor
	pack propagate .design.design true

#	button .design.design.done -text "Done" \
#		-command { close_design_palette } \
#		-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
#		-fg $fgcolor -activeforeground $afcolor -default active -font $boldfont 
#	pack .design.design.done -side bottom -pady 6 -padx 15

#	bind .design <Key> { if {"%K" == "Return"} { 
#				.design.design.done flash
#				close_design_palette
#			}
#   	} 	
    	pack .design.dbuttons -side left -fill y
	pack .design.design -side left -fill y
    }

    global any_elev_variation
    if { !$any_elev_variation } {
	.design.dbuttons.elevation config -state disabled
    }
    global any_temp_variation
    if { !$any_temp_variation } {
	.design.dbuttons.temperature config -state disabled
    }
    global any_wind_variation
    if { !$any_wind_variation } {
	.design.dbuttons.winds config -state disabled
    }
    global any_clouds
    if { !$any_clouds } {
	.design.dbuttons.clouds config -state disabled
    }
    global see_all
    if { $see_all } {
	.design.dbuttons.view config -state disabled
    }

    center_above_curmap .design
    select_paint normal make_normal_frame
    update idletasks
}

proc close_design_palette {} {
    global designer
    
    execute_long_command 0 "design"
    withdraw_window ".design"
    set designer 0
}

proc select_paint { type maker } {
    global last_dbutton
    global last_frame
    global bgcolor

    $last_dbutton config -relief raised
    .design.dbuttons.$type config -relief sunken
    set last_dbutton .design.dbuttons.$type
    if { "[ winfo exists $last_frame ]" == 1 } {
	pack forget $last_frame
    }
    set fframe .design.design.f$type
    if { "[ winfo exists $fframe ]" == 0 } {
	frame $fframe -bg $bgcolor
	pack $fframe -side top -fill both
	$maker $fframe
    } else {
	pack $fframe -side top -fill both
    }
    set last_frame $fframe
    select_design_tool $type
    update idletasks
}

proc make_normal_frame { fnormal } {
    global boldfont
    global bgcolor hbcolor fgcolor

    label $fnormal.label -text "Normal" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fnormal.label -side top -fill x -pady 4
}

proc make_terrain_paint_frame { fterrain } {
    global boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor
    
    set numt [ numttypes ]

    label $fterrain.label -text "Paint Terrain" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fterrain.label -side top -fill x -pady 4

    canvas $fterrain.canvas -width 100 -height 100 \
	-bg $bgcolor -highlightbackground $hbcolor
    pack $fterrain.canvas -side left -pady 4 -pady 4

    # Place the foreground terrain second, so that it overlaps the
    # background terrain.
    imfsample $fterrain.canvas.bg -width 44 -height 48 -bg $bgcolor
    $fterrain.canvas.bg add imf [ t_image_name 0 ]
    $fterrain.canvas create window 30 50 -anchor nw -window $fterrain.canvas.bg

    imfsample $fterrain.canvas.fg -width 44 -height 48 -bg $bgcolor
    $fterrain.canvas.fg add imf [ t_image_name 0 ]
    $fterrain.canvas create window 4 4 -anchor nw -window $fterrain.canvas.fg

    menubutton $fterrain.fg -text [ ttype_name 0 ] \
	-borderwidth 2 -relief raised \
	-menu $fterrain.fg.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $fterrain.fg -expand true
    menu $fterrain.fg.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i < $numt } { incr i } {
	$fterrain.fg.menu add command -label [ ttype_name $i ] \
		-command [ list dbg_set_design_data curttype $i 0 ]
    }

    menubutton $fterrain.bg -text [ ttype_name 0 ] \
	-borderwidth 2 -relief raised \
	-menu $fterrain.bg.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $fterrain.bg -expand true
    menu $fterrain.bg.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i < $numt } { incr i } {
	$fterrain.bg.menu add command -label [ ttype_name $i ] \
		-command [ list dbg_set_design_data curbgttype $i 0 ]
    }

    bind $fterrain.canvas <ButtonPress> \
	    { cycle_design_data terrain incr }
    bind $fterrain.canvas <Control-ButtonPress> \
	    { cycle_design_data terrain decr }
    bind $fterrain.canvas.fg <ButtonPress> \
	    { cycle_design_data terrain incr }
    bind $fterrain.canvas.fg <Control-ButtonPress> \
	    { cycle_design_data terrain decr }
    bind $fterrain.canvas.bg <ButtonPress> \
	    { cycle_design_data terrain incr }
    bind $fterrain.canvas.bg <Control-ButtonPress> \
	    { cycle_design_data terrain decr }
}

proc make_unit_paint_frame { funit } {
    global boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor
    
    set numu [ numutypes ]
    set nums [ numsides ]

    label $funit.label -text "Place Unit" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $funit.label -side top -fill x -pady 4

    canvas $funit.canvas -width 34 -height 34 \
	-bg $bgcolor -highlightbackground $hbcolor
    pack $funit.canvas -side left -padx 6 -pady 6
    imfsample $funit.canvas.samp -width 32 -height 32 -bg $bgcolor
    $funit.canvas.samp add imf [ u_image_name 0 ]
    $funit.canvas create window 1 1 -anchor nw -window $funit.canvas.samp

    menubutton $funit.type -text [ utype_name 0 ] \
	-borderwidth 2 -relief raised \
	-menu $funit.type.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $funit.type -expand true -pady 4
    menu $funit.type.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i < $numu } { incr i } {
	$funit.type.menu add command -label [ utype_name $i ] \
		-command [ list dbg_set_design_data curutype $i 0 ]
    }

    menubutton $funit.side -text [ side_adjective 0 ] \
	-borderwidth 2 -relief raised \
	-menu $funit.side.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $funit.side -expand true -pady 4
    menu $funit.side.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i <= $nums } { incr i } {
	$funit.side.menu add command -label [ side_adjective $i ] \
		-command [ list dbg_set_design_data curusidenumber $i 0 ]
	# Preload the unit sample with emblems for all sides, then
	# later we can just select at will.
	if { $i >= 1 } {
	    $funit.canvas.samp add imf [ side_emblem $i ]
	}
    }

    bind $funit.canvas <ButtonPress> \
	    [ list cycle_design_data unit incr ]
    bind $funit.canvas <Control-ButtonPress> \
	    [ list cycle_design_data unit decr ]
    bind $funit.canvas.samp <ButtonPress> \
	    [ list cycle_design_data unit incr ]
    bind $funit.canvas.samp <Control-ButtonPress> \
	    [ list cycle_design_data unit decr ]
}

proc make_people_paint_frame { fpeople } {
    global boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor
    
    set nums [ numsides ]

    label $fpeople.label -text "Paint People" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fpeople.label -side top -fill x -pady 4

    canvas $fpeople.canvas -width 32 -height 32 \
	-bg $bgcolor -highlightbackground $hbcolor
    pack $fpeople.canvas -side left -pady 4 -pady 4
    imfsample $fpeople.canvas.people -width 16 -height 16 -bg $bgcolor
    $fpeople.canvas.people add imf [ side_emblem 0 ]
    $fpeople.canvas create window 16 16 -anchor c \
	    -window $fpeople.canvas.people

    menubutton $fpeople.people -text [ side_name 0 ] \
	-borderwidth 2 -relief raised \
	-menu $fpeople.people.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $fpeople.people -expand true
    menu $fpeople.people.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i <= $nums } { incr i } {
	$fpeople.people.menu add command -label [ side_name $i ] \
		-command [ list dbg_set_design_data curpeoplenumber $i 0 ]
    }

    bind $fpeople.canvas <ButtonPress> \
	    [ list cycle_design_data people incr ]
    bind $fpeople.canvas <Control-ButtonPress> \
	    [ list cycle_design_data people decr ]

    # Automatically turn on display of people layer.
    # (should indicate better what is happening)
    global nummaps map_widget
    global map_options
    for { set i 1 } { $i <= $nummaps } { incr i } {
	if { $map_options($map_widget($i),people) == 0 } {
	    set map_options($map_widget($i),people) 1
	    set_map_view_option $map_widget($i) people
	}
    }
}

proc make_control_paint_frame { fcontrol } {
    global boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor
    
    set nums [ numsides ]

    label $fcontrol.label -text "Paint Control" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fcontrol.label -side top -fill x -pady 4

    canvas $fcontrol.canvas -width 32 -height 32 \
	-bg $bgcolor -highlightbackground $hbcolor
    pack $fcontrol.canvas -side left -pady 4 -pady 4
    imfsample $fcontrol.canvas.control -width 16 -height 16 -bg $bgcolor -fg $fgcolor
    $fcontrol.canvas.control add imf [ side_emblem 0 ]
    $fcontrol.canvas create window 16 16 -anchor c \
	    -window $fcontrol.canvas.control

    menubutton $fcontrol.control -text [ side_name 0 ] \
	-borderwidth 2 -relief raised \
	-menu $fcontrol.control.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $fcontrol.control -expand true
    menu $fcontrol.control.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    for { set i 0 } { $i <= $nums } { incr i } {
	$fcontrol.control.menu add command -label [ side_name $i ] \
		-command [ list dbg_set_design_data curcontrolnumber $i 0 ]
    }

    bind $fcontrol.canvas <ButtonPress> \
	    [ list cycle_design_data control incr ]
    bind $fcontrol.canvas <Control-ButtonPress> \
	    [ list cycle_design_data control decr ]

    # Automatically turn on display of control layer.
    # (should indicate better what is happening)
    global nummaps map_widget
    global map_options
    for { set i 1 } { $i <= $nummaps } { incr i } {
	if { $map_options($map_widget($i),control) == 0 } {
	    set map_options($map_widget($i),control) 1
	    set_map_view_option $map_widget($i) control
	}
    }
}

set rename_name ""
set rename_type_name ""

proc make_feature_paint_frame { ffeature } {
    global boldfont stdfont
    global curfid
    global bgcolor hbcolor abcolor fgcolor afcolor

    set numf [ numfeatures ]

    label $ffeature.label -text "Paint Feature" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $ffeature.label -side top -fill x -pady 4

    menubutton $ffeature.name -text "[ feature_desc 0 ]" \
	-borderwidth 2 -relief raised \
	-menu $ffeature.name.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $ffeature.name -side top
    menu $ffeature.name.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    $ffeature.name.menu add command -label "[ feature_desc 0 ]" \
	    -command [ list dbg_set_design_data curfid 0 0 ]
    for { set i 1 } { $i <= $numf } { incr i } {
	set colbreak 0
	if { $i % 25 == 0 } {
	    set colbreak 1
	}
	$ffeature.name.menu add command -label "[ feature_desc $i ]" \
		-command [ list dbg_set_design_data curfid $i 0 ] \
		-columnbreak $colbreak
    }

    frame $ffeature.buttons -bg $bgcolor
    pack $ffeature.buttons -side top -pady 4 -padx 4

    button $ffeature.buttons.new -text New \
	-command { new_feature } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button $ffeature.buttons.rename -text Rename -state disabled \
	-command { rename_feature $curfid } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button $ffeature.buttons.delete -text Del -state disabled \
	-command { delete_feature $curfid } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $ffeature.buttons.new $ffeature.buttons.rename $ffeature.buttons.delete \
    	-side left -anchor nw -padx 4 -pady 4

    frame $ffeature.fields -bg $bgcolor
    pack $ffeature.fields -side left -pady 4 -padx 4

    label $ffeature.fields.namelabel -text "Name :" -bg $bgcolor -fg $fgcolor -font $boldfont
    label $ffeature.fields.hostlabel -text "Type :" -bg $bgcolor -fg $fgcolor -font $boldfont

    grid $ffeature.fields.namelabel -sticky e -column 0 
    grid $ffeature.fields.hostlabel -sticky e -column 0 

    entry $ffeature.fields.newname -textvariable rename_name \
    	-relief sunken -width 18 -fg $fgcolor -font $stdfont
    entry $ffeature.fields.typename -textvariable rename_type_name \
    	-relief sunken -width 18 -fg $fgcolor -font $stdfont

    grid $ffeature.fields.newname -sticky w -column 1 -row 0 
    grid $ffeature.fields.typename -sticky w -column 1 -row 1

    update_feature_name_entry 0
}

# For the given feature id, update the rename fields.

proc update_feature_name_entry { fid } {
    global rename_name rename_type_name

    set state normal
    if { $fid == 0 } {
	set state disabled
	set rename_name ""
	set rename_type_name ""
    } else {
	set rawinfo [ feature_info $fid ]
	set finfo "list $rawinfo"
	set finfo2 [ lindex $finfo 1 ]
	set rename_name [ lindex $finfo2 0 ]
	set rename_type_name [ lindex $finfo2 1 ]    
    }
    .design.design.ffeature.buttons.rename config -state $state
    .design.design.ffeature.buttons.delete config -state $state
}

# Make the proposed renaming real.

proc rename_feature { fid } {
    global rename_name rename_type_name

    set_feature_info $fid $rename_name $rename_type_name
    # Update the feature menu entry.
    .design.design.ffeature.name.menu entryconfigure $fid \
	    -label "[ feature_desc $fid ]"
    # Update the menu button as well, for immediate feedback.
    .design.design.ffeature.name config -text "[ feature_desc $fid ]"
}

# Make the given feature disappear.

proc delete_feature { fid } {
    global curfid

    if { $fid < 1 } {
	# (should complain)
	return
    }
    set entryname [ feature_desc $fid ]
    destroy_feature $fid
    # Clear from the feature menu entry, but not by index.
    # (what if entryname were a number?)
    .design.design.ffeature.name.menu delete $entryname
    # If the deleted feature was the currently selected one (which is
    # usually the case), make the previous feature current.
    if { $curfid == $fid } {
	incr curfid -1
	dbg_set_design_data curfid $curfid 0
    }
}

proc make_material_paint_frame { fmaterial } {
    global boldfont
    global bgcolor fgcolor

    label $fmaterial.label -text "Paint Material" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fmaterial.label -side top -fill x -pady 4
}

set elev_var 0
set elev_vary_var 0

proc make_elev_paint_frame { felev } {
    global boldfont stdfont
    global elev_var elev_vary_var
    global bgcolor hbcolor abcolor fgcolor afcolor

    label $felev.label -text "Paint Elevation" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $felev.label -side top -fill x -pady 4

    frame $felev.entries -bg $bgcolor
    pack $felev.entries -side top -anchor nw -padx 4

    label $felev.entries.lab1 -text "Elevation :" -bg $bgcolor -fg $fgcolor -font $boldfont
    label $felev.entries.lab2 -text "+/- :" -bg $bgcolor -fg $fgcolor -font $boldfont

    grid $felev.entries.lab1 -sticky e -column 0 
    grid $felev.entries.lab2 -sticky e -column 0 

    entry $felev.entries.entry -textvariable elev_var \
    	-relief sunken -width 6 -fg $fgcolor -font $stdfont
    entry $felev.entries.ventry -textvariable elev_vary_var \
    	-relief sunken -width 6 -fg $fgcolor -font $stdfont

    grid $felev.entries.entry -sticky w -column 1 -row 0 
    grid $felev.entries.ventry -sticky w -column 1 -row 1

    label $felev.entries.lab3 -text " (random)" -bg $bgcolor -fg $fgcolor -font $boldfont

    grid $felev.entries.lab3 -sticky e -column 2 -row 1

    frame $felev.buttons -bg $bgcolor
    pack $felev.buttons -side top

    button $felev.buttons.set -text Set \
	-command { set_elevation } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button $felev.buttons.raise -text + \
	-command { raise_elevation } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button $felev.buttons.lower -text - \
	-command { lower_elevation } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button $felev.buttons.fix -text Fix \
	-command { designer_fix elevation } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $felev.buttons.set $felev.buttons.raise $felev.buttons.lower $felev.buttons.fix \
    	-side left -anchor nw -padx 4 -pady 8

    label $felev.state1 -text "Set to elevation 0" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $felev.state1 -side top -anchor nw -padx 4
    label $felev.state2 -text "Randomly vary +/- 0" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $felev.state2 -side top -anchor nw -padx 4

    # Automatically turn on display of elevations.
    # (should indicate better what is happening)
    global nummaps map_widget
    global map_options
    for { set i 1 } { $i <= $nummaps } { incr i } {
	if { $map_options($map_widget($i),elevations) == 0 } {
	    set map_options($map_widget($i),elevations) 1
	    set_map_view_option $map_widget($i) elevations
	}
    }
}

proc set_elevation {} {
    global elev_var elev_vary_var

    dbg_set_design_data curelevation $elev_var 0
    dbg_set_design_data curelevation_vary $elev_vary_var 0
}

proc raise_elevation {} {
    global elev_var elev_vary_var

    dbg_set_design_data curelevation incr $elev_var
    dbg_set_design_data curelevation_vary $elev_vary_var 0
}

proc lower_elevation {} {
    global elev_var elev_vary_var

    dbg_set_design_data curelevation decr $elev_var
    dbg_set_design_data curelevation_vary $elev_vary_var 0
}

set tempvar 0

proc make_temp_paint_frame { ftemp } {
    global boldfont stdfont
    global tempvar
    global bgcolor hbcolor abcolor fgcolor afcolor

    label $ftemp.label1 -text "Paint Temperature" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $ftemp.label1 -side top -fill x -pady 4

    button $ftemp.set -text Set \
	-command { set_temperature } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    pack $ftemp.set -side left -anchor nw -padx 8 -pady 4

    entry $ftemp.entry -textvariable tempvar \
    	-relief sunken -width 6 -fg $fgcolor -font $stdfont
    pack $ftemp.entry -side left -padx 4 -pady 4

    label $ftemp.label2 -text "degrees" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $ftemp.label2 -side left -padx 4 -pady 4
}

proc set_temperature {} {
    global tempvar

    dbg_set_design_data curtemperature $tempvar 0
}

set cloudtype 0
set cloudbottom 0
set cloudheight 0

proc make_clouds_paint_frame { fclouds } {
    global cloudtype cloudbottom cloudheight
    global boldfont stdfont
    global bgcolor hbcolor abcolor fgcolor afcolor

    label $fclouds.label -text "Paint Clouds" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fclouds.label -side top -fill x -pady 4

    frame $fclouds.entries -bg $bgcolor
    pack $fclouds.entries -side top -anchor nw -padx 4

    label $fclouds.entries.dlab -text "Type :" -bg $bgcolor -fg $fgcolor -font $boldfont
    label $fclouds.entries.blab -text "Bottom :" -bg $bgcolor -fg $fgcolor -font $boldfont
    label $fclouds.entries.hlab -text "Height :" -bg $bgcolor -fg $fgcolor -font $boldfont

    grid $fclouds.entries.dlab -sticky e -column 0 
    grid $fclouds.entries.blab -sticky e -column 0 
    grid $fclouds.entries.hlab -sticky e -column 0 

    menubutton $fclouds.entries.dentry -text "0" \
	-borderwidth 2 -relief raised \
	-menu $fclouds.entries.dentry.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    menu $fclouds.entries.dentry.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    foreach cloudtype { 0 1 2 3 4 } {
	$fclouds.entries.dentry.menu add command -label "$cloudtype" \
		-command [ list dbg_set_design_data curcloudtype $cloudtype 0 ]
    }

    entry $fclouds.entries.bentry -textvariable cloudbottom \
    	-relief sunken -width 6 -fg $fgcolor -font $stdfont
    entry $fclouds.entries.hentry -textvariable cloudheight \
    	-relief sunken -width 6 -fg $fgcolor -font $stdfont

    grid $fclouds.entries.dentry -sticky w -column 1 -row 0 
    grid $fclouds.entries.bentry -sticky w -column 1 -row 1 
    grid $fclouds.entries.hentry -sticky w -column 1 -row 2 

    button $fclouds.entries.set -text Set \
	-command { set_clouds } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
	
    grid $fclouds.entries.set -sticky e -padx 4 -column 2 -row 2 
}

proc set_clouds {} {
    global cloudbottom cloudheight

    dbg_set_design_data curcloudbottom $cloudbottom 0
    dbg_set_design_data curcloudheight $cloudheight 0
}

proc make_winds_paint_frame { fwinds } {
    global boldfont
    global bgcolor hbcolor abcolor fgcolor afcolor

    label $fwinds.label -text "Paint Winds" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fwinds.label -side top -fill x -pady 4

    frame $fwinds.entries -bg $bgcolor
    pack $fwinds.entries -side top -anchor nw -padx 4

    label $fwinds.entries.dlab -text "Direction :" -bg $bgcolor -fg $fgcolor -font $boldfont
    label $fwinds.entries.flab -text "Force :" -bg $bgcolor -fg $fgcolor -font $boldfont

    grid $fwinds.entries.dlab -sticky e -column 0 
    grid $fwinds.entries.flab -sticky e -column 0 

    menubutton $fwinds.entries.dir -text "0" \
	-borderwidth 2 -relief raised \
	-menu $fwinds.entries.dir.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    menu $fwinds.entries.dir.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    foreach dir { 0 1 2 3 4 5 } {
	$fwinds.entries.dir.menu add command -label "$dir" \
		-command [ list dbg_set_design_data curwinddir $dir 0 ]
    }
    menubutton $fwinds.entries.force -text "0" \
	-borderwidth 2 -relief raised \
	-menu $fwinds.entries.force.menu \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    menu $fwinds.entries.force.menu -tearoff 0 -font $boldfont \
	-bg $bgcolor -activebackground $abcolor -fg $fgcolor -activeforeground $afcolor
    foreach force { 0 1 2 3 4 } {
	$fwinds.entries.force.menu add command -label "$force" \
		-command [ list dbg_set_design_data curwindforce $force 0 ]
    }

    grid $fwinds.entries.dir -sticky w -column 1 -row 0 
    grid $fwinds.entries.force -sticky w -column 1 -row 1 
}

proc make_view_paint_frame { fview } {
    global boldfont
    global bgcolor fgcolor

    label $fview.label -text "Paint View" -bg $bgcolor -fg $fgcolor -font $boldfont
    pack $fview.label -side top -fill x -pady 4

}

# Make the palette go away, but don't destroy - might want to get it back
# in the near future.

proc dismiss_design_palette {} {
    global designer
    
    set designer 0
    if { "[ winfo exists .design ]" } {
        withdraw_window ".design"
    }
}

# Given the name of a designer tool, make it the current one in use.

proc select_design_tool { name } {
    global selected_design_tool

    if { $name == $selected_design_tool } {
	return
    }
    set_design_tool $name
    set selected_design_tool $name
}

# Given a type of design data/tool and a value for that type of data,
# set it to be the value to paint and update any feedback displays.

proc dbg_set_design_data { type val val2 } {
    global curfid

    set newval [ set_design_data $type $val $val2 ]
    set dframe .design.design
    if { $type == "curttype" } {
	$dframe.fterrain.canvas.fg replace imf [ t_image_name $newval ]
	$dframe.fterrain.fg config -text [ ttype_name $newval ]
    } elseif { $type == "curbgttype" } {
	$dframe.fterrain.canvas.bg replace imf [ t_image_name $newval ]
	$dframe.fterrain.bg config -text [ ttype_name $newval ]
    } elseif { $type == "curutype" } {
	$dframe.funit.canvas.samp replace imf [ u_image_name $newval ]
	$dframe.funit.type config -text [ utype_name $newval ]
    } elseif { $type == "curusidenumber" } {
	$dframe.funit.side config -text [ side_adjective $newval ]
	if { $newval > 0 } {
	    $dframe.funit.canvas.samp emblem $newval
	} else {
	    $dframe.funit.canvas.samp emblem -1
	}
    } elseif { $type == "curpeoplenumber" } {
	$dframe.fpeople.canvas.people replace imf [ side_emblem $newval ]
	$dframe.fpeople.people config -text [ side_name $newval ]
    } elseif { $type == "curcontrolnumber" } {
	$dframe.fcontrol.canvas.control replace imf [ side_emblem $newval ]
	$dframe.fcontrol.control config -text [ side_name $newval ]
    } elseif { $type == "curfid" } {
	$dframe.ffeature.name config -text [ feature_desc $newval ]
	update_feature_name_entry $newval
	# The current feature appears in a different color, so all
	# maps need to be updated.
	global nummaps
	for { set i 1 } { $i <= $nummaps } { incr i } {
	    execute_long_command $i "refresh"
	}
	set curfid $newval
    } elseif { $type == "curelevation" } {
	global elev_var elev_vary_var
	set lab $dframe.felevation.state1
	if { $val == "incr" } {
	    $lab config -text "Raise elevation by $elev_var"
	} elseif { $val == "decr" } {
	    $lab config -text "Lower elevation by $elev_var"
	} else {
	    $lab config -text "Set elevation to $elev_var"
	}
    } elseif { $type == "curelevation_vary" } {
	global elev_var elev_vary_var
	$dframe.felevation.state2 config -text "Randomly vary +/- $elev_vary_var"
    } elseif { $type == "curcloudtype" } {
	.design.design.fclouds.entries.dentry config -text "$newval"
    } elseif { $type == "curwinddir" } {
	.design.design.fwinds.entries.dir config -text "$newval"
    } elseif { $type == "curwindforce" } {
	.design.design.fwinds.entries.force config -text "$newval"
    } elseif { $type == "curbrushradius" } {
	.design.dbuttons.brush config -text "$newval"
    }
}

proc cycle_design_data { type dir } {
    # The brush size setter is not a selectable tool.
    if { $type != "brush" } {
	select_design_tool $type
    }
    if { $type == "terrain" } {
	dbg_set_design_data curttype $dir 0
    } elseif { $type == "unit" } {
	dbg_set_design_data curutype $dir 0
    } elseif { $type == "people" } {
	dbg_set_design_data curpeoplenumber $dir 0
    } elseif { $type == "control" } {
	dbg_set_design_data curcontrolnumber $dir 0
    } elseif { $type == "feature" } {
	dbg_set_design_data curfid $dir 0
    } elseif { $type == "brush" } {
	dbg_set_design_data curbrushradius $dir o
    }
}

proc new_feature {} {
    set fid [ create_new_feature ]
    .design.design.ffeature.name.menu add command \
	    -label "[ feature_desc $fid ]" \
	    -command [ list dbg_set_design_data curfid $fid 0 ]
    dbg_set_design_data curfid $fid 0
}

set dsave_done 0

set dsave(name) "game-data"

set dsave(all) 1
set dsave(types) 0
set dsave(tables) 0
set dsave(globals) 0
set dsave(scoring) 0
set dsave(world) 0
set dsave(area) 0
set dsave(terrain) 0
set dsave(areamisc) 0
set dsave(weather) 0
set dsave(material) 0
set dsave(sides) 0
set dsave(views) 0
set dsave(docts) 0
set dsave(players) 0
# set dsave(agreements) 0
set dsave(units) 0
set dsave(unitids) 0
set dsave(unitprops) 0
set dsave(unitactions) 0
set dsave(unitplans) 0
set dsave(history) 0

proc popup_designer_save {} {
    global dsave dsave_done stdfont boldfont
    global bgcolor hbcolor abcolor scolor fgcolor afcolor

    if { [ winfo exists .dsave ] } {
	# Kick the window manager awake.
	wm withdraw .dsave
	wm deiconify .dsave
	focus -force .dsave
	return
    }

    # Create and then immediately withdraw the window 
    # so that it does not bounce around later.
    toplevel .dsave -bg $bgcolor
    wm withdraw .dsave
    wm title .dsave "Xconq Designer Save"

    set dsave_done 0

    frame .dsave.top -bg $bgcolor
    pack .dsave.top -side top -anchor w -padx 8 -pady 12

    label .dsave.top.modulelabel -text "Module Name : " -bg $bgcolor -fg $fgcolor -font $boldfont
    entry .dsave.top.modulename -relief sunken -width 20 -fg $fgcolor -font $stdfont
    .dsave.top.modulename insert end $dsave(name)
    pack .dsave.top.modulelabel .dsave.top.modulename -side left

    frame .dsave.f1 -bg $bgcolor
    pack .dsave.f1 -side top -padx 8

    checkbutton .dsave.f1.all -text " Save All" -variable dsave(all) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.all -sticky w -pady 4

    checkbutton .dsave.f1.types -text " Types" -variable dsave(types) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.tables -text " Tables" -variable dsave(tables) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.types .dsave.f1.tables -sticky w -pady 4

    checkbutton .dsave.f1.globals -text " Globals" -variable dsave(globals) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.globals -sticky w -pady 4

    checkbutton .dsave.f1.scoring -text " Scoring" -variable dsave(scoring) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.scoring -sticky w -pady 4

    checkbutton .dsave.f1.world -text " World" -variable dsave(world) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.world -sticky w -pady 4

    checkbutton .dsave.f1.area -text " Area" -variable dsave(area) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.terrain -text " Terrain" -variable dsave(terrain) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.areamisc -text " Misc" -variable dsave(areamisc) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.weather -text " Weather" -variable dsave(weather) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.material -text " Materials" \
	-variable dsave(material) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.area .dsave.f1.areamisc .dsave.f1.terrain \
	    .dsave.f1.weather .dsave.f1.material -sticky w -pady 4

    checkbutton .dsave.f1.sides -text " Sides" -variable dsave(sides) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.sideviews -text " Views" -variable dsave(views) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.sidedocts -text " Doctrines" \
	-variable dsave(docts) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.sides .dsave.f1.sideviews .dsave.f1.sidedocts -sticky w \
	-pady 4

    checkbutton .dsave.f1.players -text " Players" -variable dsave(players) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.players -sticky w -pady 4

#    checkbutton .dsave.f1.agreements -text " Agreements" \
#	-variable dsave(agreements) \
#	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
#	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
#	-activeforeground $afcolor
#    grid .dsave.f1.agreements -sticky w -pady 4

    checkbutton .dsave.f1.units -text " Units" -variable dsave(units) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.unitids -text " IDs" -variable dsave(unitids) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.unitprops -text " Props" \
	-variable dsave(unitprops) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.unitactions -text " Actions" \
	-variable dsave(unitactions) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    checkbutton .dsave.f1.unitplans -text " Plans" \
	-variable dsave(unitplans) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.units .dsave.f1.unitids .dsave.f1.unitprops \
	.dsave.f1.unitactions .dsave.f1.unitplans -sticky w -pady 4

    checkbutton .dsave.f1.history -text " History" -variable dsave(history) \
	-bg $bgcolor -highlightbackground $bgcolor -font $boldfont \
	-activebackground $abcolor -selectcolor $scolor -fg $fgcolor \
	-activeforeground $afcolor
    grid .dsave.f1.history -sticky w -pady 4

    frame .dsave.bottom -bg $bgcolor
    pack .dsave.bottom -side bottom -anchor e -padx 20 -pady 4

    button .dsave.bottom.save -text "Save" -width 8 \
	    -command { save_design } -default active \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    button .dsave.bottom.cancel -text "Cancel" -width 8 \
	    -command { set dsave_done 1 } \
	-bg $bgcolor -highlightbackground $hbcolor -activebackground $abcolor \
	-fg $fgcolor -activeforeground $afcolor -font $boldfont
    grid .dsave.bottom.cancel .dsave.bottom.save -sticky ew -padx 6 -pady 4

    center_above_curmap .dsave
    focus .dsave
    tkwait visibility .dsave
    grab .dsave
    tkwait variable dsave_done
    grab release .dsave
    destroy .dsave
}

proc save_design {} {
    global dsave dsave_done

    set args ""
    if { $dsave(all) } { set args " all $args" }
    if { $dsave(types) } { set args " types $args" }
    if { $dsave(tables) } { set args " tables $args" }
    if { $dsave(globals) } { set args " globals $args" }
    if { $dsave(scoring) } { set args " scoring $args" }
    if { $dsave(world) } { set args " world $args" }
    if { $dsave(area) } { set args " area $args" }
    if { $dsave(areamisc) } { set args " areamisc $args" }
    if { $dsave(terrain) } { set args " terrain $args" }
    if { $dsave(weather) } { set args " weather $args" }
    if { $dsave(material) } { set args " material $args" }
    if { $dsave(sides) } { set args " sides $args" }
    if { $dsave(views) } { set args " views $args" }
    if { $dsave(docts) } { set args " docts $args" }
    if { $dsave(players) } { set args " players $args" }
#    if { $dsave(agreements) } { set args " agreements $args" }
    if { $dsave(units) } { set args " units $args" }
    if { $dsave(unitids) } { set args " unitids $args" }
    if { $dsave(unitprops) } { set args " unitprops $args" }
    if { $dsave(unitactions) } { set args " unitactions $args" }
    if { $dsave(unitplans) } { set args " unitplans $args" }
    if { $dsave(history) } { set args " history $args" }
    set dsave(name) "[.dsave.top.modulename get ]"
    set dname [ game_homedir ]
    set filename [ tk_getSaveFile -initialfile $dsave(name).g -initialdir $dname ]
    if { "$filename" != "" } {
	designer_save $dsave(name) $filename $args
    }
    set dsave_done 1
}

proc create_left_right_panes { win leftratio } {
    global bgcolor

    frame $win.leftside -borderwidth 2 -relief sunken -bg $bgcolor
    place $win.leftside -in $win -relx 0 -rely 1.0 \
	    -relwidth $leftratio -relheight 1 -anchor sw

    set rightratio [ expr 1.0 - $leftratio ]

    frame $win.rightside -borderwidth 2 -relief sunken -bg $bgcolor
    place $win.rightside -in $win -relx 1.0 -rely 1.0 \
	    -relwidth $rightratio -relheight 1 -anchor se

    frame $win.grip -width 9 -height 9 -borderwidth 2 -relief raised -bg $bgcolor
    place $win.grip -relx $leftratio -rely 0.95 -anchor c

    bind $win.grip <ButtonPress-1>	"lr_panedwindow_grab $win"
    bind $win.grip <B1-Motion>		"lr_panedwindow_drag $win %X"
    bind $win.grip <ButtonRelease-1>	"lr_panedwindow_drop $win %X"
}

proc lr_panedwindow_grab { win } {
    $win.grip config -relief sunken
}

proc lr_panedwindow_drag { win x } {
    set realx [ expr $x - [ winfo rootx $win ] ]
    set xmax [ winfo width $win ]
    set frac [ expr double($realx) / $xmax ]
    if { $frac < 0.05 } {
	set frac 0.05
    }
    if { $frac > 0.95 } {
	set frac 0.95
    }
    place $win.grip -relx $frac
    return $frac
}

proc lr_panedwindow_drop { win x } {
    set frac [ lr_panedwindow_drag $win $x ]
    place $win.leftside -relwidth $frac
    place $win.rightside -relwidth [ expr 1.0 - $frac ]
    place $win.grip -relx $frac
    $win.grip config -relief raised
}

proc create_top_bottom_panes { win topratio } {
    global bgcolor

    frame $win.topside -borderwidth 1 -relief sunken -bg $bgcolor
    place $win.topside -in $win -relx 0 -rely 0 \
	    -relwidth 1 -relheight $topratio -anchor nw

    set bottomratio [ expr 1.0 - $topratio ]

    frame $win.botside -borderwidth 1 -relief sunken -bg $bgcolor
    place $win.botside -in $win -relx 0 -rely 1.0 \
	    -relwidth 1 -relheight $bottomratio -anchor sw

    frame $win.grip -width 9 -height 9 -borderwidth 2 -relief raised -bg $bgcolor
    place $win.grip -relx 0.95 -rely $topratio -anchor c

    bind $win.grip <ButtonPress-1>	"tb_panedwindow_grab $win"
    bind $win.grip <B1-Motion>		"tb_panedwindow_drag $win %Y"
    bind $win.grip <ButtonRelease-1>	"tb_panedwindow_drop $win %Y"
}

proc tb_panedwindow_grab { win } {
    $win.grip config -relief sunken
}

proc tb_panedwindow_drag { win y } {
    set realy [ expr $y - [ winfo rooty $win ] ]
    set ymax [ winfo height $win ]
    set frac [ expr double($realy) / $ymax ]
    if { $frac < 0.05 } {
	set frac 0.05
    }
    if { $frac > 0.95 } {
	set frac 0.95
    }
    place $win.grip -rely $frac
    return $frac
}

proc tb_panedwindow_drop { win y } {
    set frac [ tb_panedwindow_drag $win $y ]
    place $win.topside -relheight $frac
    place $win.botside -relheight [ expr 1.0 - $frac ]
    place $win.grip -rely $frac
    $win.grip config -relief raised
}

proc fit_map { wid } {
    for { set i 6 } { $i >= 0 } { incr i -1 } {
	set siz [ map_size_at_power $i ]
	if { [ lindex $siz 0 ] <= $wid } {
	    return $i
	}
    }
    return 0
}

proc resize_world_map { worldf val w h } {
    global last_world_width last_world_power

    if { $val == 613 } {
	if { $last_world_width != $w } {
	    set newpow [ fit_map $w ]
	    if { $newpow != $last_world_power } {
		$worldf.world config -power $newpow
		set last_world_width $w
		set last_world_power $newpow
	    }
	}
    }
}

# If the unit info box has changed size, adjust the position of the
# second column of info so that it is always in the middle.

proc resize_unit_info { unitinfo val w h } {
    set oldcol2 [ lindex [ $unitinfo coords hp ] 0 ]
    set newcol2 [ expr $w / 2 ]
    $unitinfo move col2 [ expr $newcol2 - $oldcol2 ] 0
}

proc find_image_filename { name } {
    global pathlist

    foreach path $pathlist {
	set filename [ file join $path $name ]
	if { "[ file exists $filename ]" } {
	    return $filename
	}
    }
    # Fallback - look for sibling images dir next to each library path.
    foreach path $pathlist {
	set filename [ file join [ file dirname $path ] "images" $name ]
	if { "[ file exists $filename ]" } {
	    return $filename
	}
    }
    return ""
}

# This procedure will center win above the current map 2/5 from the top. 
# The window should be withdrawn before calling the procedure to prevent 
# it from bouncing around when the geometry is updated.

proc center_above_curmap { win } {
    global tcl_platform

    # This code doesn't work under Windows. Should figure out why.
    if { "$tcl_platform(platform)" != "windows" } {
	set num [ curmap_number ]
    	set map .m$num
    } else {
	set map .m1
    }
    # Update window size before recentering while still invisible.
    update idletasks
    # Center win above the current map.
    set x [ expr { [ winfo x $map ] + [ winfo width $map ] / 2 - [ winfo reqwidth $win ] / 2 } ]
    set y [ expr { [ winfo y $map ] + [ winfo height $map ] * 2 / 5 - [ winfo reqheight $win ] * 2 / 5 } ]
    wm geometry $win "+$x+$y"
    # Finally make the window visible in its new position.
    wm deiconify $win
    focus -force $win
    update idletasks
}

proc center_on_screen { win } {

    # Update window size before recentering while still invisible.
    update idletasks
    # Center win on the main screen.
    set x [ expr { [ winfo screenwidth . ] / 2 - [ winfo reqwidth $win ] / 2 } ]
    set y [ expr { [ winfo screenheight . ] * 2 / 5 - [ winfo reqheight $win ] * 2 / 5 } ]
    wm geometry $win "+$x+$y"
    # Finally make the window visible in its new position.
    wm deiconify $win
    focus -force $win
    update idletasks
}    

proc bring_to_front { win } {
    
    set frontwin [ focus ]
    # If there is no focus just focus $win.
    if { $frontwin == "" } {
	focus -force $win
         update idletasks
	return
    }
    # Kick the window manager awake and focus $win if another focus exists.
    if { $frontwin != $win } {
	wm withdraw $win
	wm deiconify $win
	focus -force $win
         update idletasks
    }
}

