# tcl/tk browser for Xconq image families.
# Copyright (C) 1998-2000 Stanley T. Shebs.

# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.  See the file COPYING.

set mag 32
set showcolor 1
set shownames 1
set showmasks 1
set fgcolor black
set bgcolor magenta
set cucolor green
set showbbox 0
set showsolid 0
set showgrid 0
set with_emblem 0
set with_terrain 0

# Note: the globals bgcolor and fgcolor are user-settable and used for the
# imfsamples, so some other items have -bg LemonChiffon and -fg black  
# set instead.

# The "highlight" background color. 
# Fringe color around buttons and canvases on the mac.
set hbcolor LemonChiffon
# The "active" backgound color. Unused on the mac.
set abcolor LemonChiffon
# The troughcolor for scrollbars.
set tcolor LemonChiffon2
# The select color for menu checkmarks
set scolor MediumBlue
# The "active" foreground color.
set afcolor black

set as unit

set tk_strictMotif 1

set save_selected_only 0

# Hide the window for now.
wm withdraw .

wm title . imfapp

menu .menubar -bg LemonChiffon \
	-activebackground $abcolor -fg black -activeforeground $afcolor 
. configure -menu .menubar
. configure -bg LemonChiffon

# Add apple menu and about box support on the mac.
if { "$tcl_platform(platform)" == "macintosh" } {
    menu .menubar.apple -tearoff 0
    .menubar add cascade -menu .menubar.apple
    .menubar.apple add command -label "About IMFApp..." -state normal
}

menu .menubar.file -tearoff 0 -bg LemonChiffon \
	-activebackground $abcolor -fg black -activeforeground $afcolor 
.menubar add cascade -label File -menu .menubar.file -state normal
.menubar.file add command -label New \
	-command { new_cmd }
.menubar.file add command -label "Open Imf Dir..." \
	-command { open_imf_dir_cmd }
.menubar.file add command -label "Add Imf File..." \
	-command { add_imf_file_cmd }
#.menubar.file add command -label "Add Other File..." \
#	-command { add_other_file_cmd }
.menubar.file add separator
.menubar.file add command -label "Save Imf Dir..." \
	-command { save_imf_dir_cmd }
.menubar.file add command -label "Save Imf File..." \
	-command { save_imf_file_cmd }
.menubar.file add check -label "Save Selected Only" \
	-variable save_selected_only -selectcolor $scolor
.menubar.file add separator
.menubar.file add command -label Quit \
	-command { exit }

menu .menubar.view -tearoff 0 -bg LemonChiffon \
		-activebackground $abcolor -fg black -activeforeground $afcolor 
.menubar.view add radio -label 8x8 -variable mag -value 8 \
	-command { .images.canvas.content config -iwidth 12 -iheight 13 } -selectcolor $scolor
.menubar.view add radio -label 16x16 -variable mag -value 16 \
	-command { .images.canvas.content config -iwidth 24 -iheight 26 } -selectcolor $scolor
.menubar.view add radio -label 32x32 -variable mag -value 32 \
	-command { .images.canvas.content config -iwidth 44 -iheight 48 } -selectcolor $scolor
.menubar.view add radio -label 64x64 -variable mag -value 64 \
	-command { .images.canvas.content config -iwidth 88 -iheight 96 } -selectcolor $scolor
.menubar.view add separator
.menubar.view add check -label Color -variable showcolor \
	-command { toggle_show_color } -selectcolor $scolor
.menubar.view add check -label Names -variable shownames \
	-command { toggle_show_names } -selectcolor $scolor
.menubar.view add check -label Masks -variable showmasks \
	-command { toggle_show_masks } -selectcolor $scolor
.menubar add cascade -label View -menu .menubar.view -state normal

menu .menubar.view.fg -tearoff 0 -bg LemonChiffon \
	-activebackground $abcolor -fg black -activeforeground $afcolor 
.menubar.view.fg add radio -label black -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label gray -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label white -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label azure -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label red -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label green -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label blue -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label magenta -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view.fg add radio -label maroon -variable fgcolor \
	-command { set_fg_color } -selectcolor $scolor
.menubar.view add cascade -label Foreground -menu .menubar.view.fg

menu .menubar.view.bg -tearoff 0 -bg LemonChiffon \
	-activebackground $abcolor -fg black -activeforeground $afcolor 
.menubar.view.bg add radio -label black -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label gray -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label white -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label azure -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label red -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label green -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label blue -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label magenta -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view.bg add radio -label maroon -variable bgcolor \
	-command { set_bg_color } -selectcolor $scolor
.menubar.view add cascade -label Background -menu .menubar.view.bg

menu .menubar.view.cu -tearoff 0 -bg LemonChiffon \
	-activebackground $abcolor -fg black -activeforeground $afcolor 
.menubar.view.cu add radio -label black -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label gray -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label white -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label azure -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label red -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label green -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label blue -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label magenta -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view.cu add radio -label maroon -variable cucolor \
	-command { set_cu_color } -selectcolor $scolor
.menubar.view add cascade -label Closeup -menu .menubar.view.cu

#.menubar.view add check -label BBox -variable showbbox \
#	-command { toggle_show_bbox }
#.menubar.view add check -label "Solid Color" -variable showsolid \
#	-command { toggle_show_solid }
.menubar.view add check -label Grid -variable showgrid \
	-command { toggle_show_grid } -selectcolor $scolor
.menubar.view add separator
#.menubar.view add radio -label "As Unit" -variable as -value unit \
#	-command { toggle_as_unit }
#.menubar.view add radio -label "As Terrain" -variable as -value terrain \
#	-command { toggle_as_terrain }
#.menubar.view add radio -label "As Emblem" -variable as -value emblem \
#	-command { toggle_as_emblem }
#.menubar.view add check -label "With Unit" -variable with_unit \
#	-command { toggle_with_unit }
.menubar.view add check -label "Selected as Terrain" \
	-variable with_terrain \
	-command { toggle_with_terrain } -selectcolor $scolor
.menubar.view add check -label "Selected As Emblem" \
	-variable with_emblem \
	-command { toggle_with_emblem } -selectcolor $scolor
	
# Add command key shortcuts on the Mac.
if { "$tcl_platform(platform)" == "macintosh" } {
	.menubar.file entryconfig New -accel Command-N
	.menubar.file entryconfig "Open Imf Dir..." -accel Command-O
	.menubar.file entryconfig "Add Imf File..." -accel Command-A
	.menubar.file entryconfig "Save Imf Dir..." -accel Command-S
	.menubar.file entryconfig Quit -accel Command-Q
}

frame .closeup -bg LemonChiffon -borderwidth 2 -relief sunken
pack .closeup -side top -fill x

canvas .closeup.content -height 130 -bg LemonChiffon -highlightthickness 0
pack .closeup.content -side top -fill both

.closeup.content create text 10 20 -tag selname -text "Nothing selected" -anchor sw
.closeup.content create text 250 20 -tag status -text "No image families loaded" -anchor sw

proc add_closeup { wid hgt sx sy } {
    global cucolor

    imfsample .closeup.content.s$wid -bg $cucolor -width $wid -height $hgt \
	    -iwidth $wid -iheight $hgt 
    .closeup.content create window $sx $sy -window .closeup.content.s$wid \
	    -anchor nw
    .closeup.content create rect \
	    [ expr $sx - 1 ] [ expr $sy - 1 ] [ expr $sx + $wid ] [ expr $sy + $hgt ] \
	    -outline gray
}

set sx 10
set sy 28
add_closeup 12 13 $sx $sy
incr sx [ expr 12 + 4 ]
add_closeup 24 26 $sx $sy
incr sx [ expr 24 + 4 ]
add_closeup 44 48 $sx $sy
incr sx [ expr 44 + 4 ]
add_closeup 88 96 $sx $sy

frame .images -width 900 -height 600
pack .images -side top -fill both -expand true
pack propagate .images false

scrollbar .images.scroll -orient vert -width 16 -command { .images.canvas.content yview } \
		-bg LemonChiffon -troughcolor $tcolor \
		-highlightbackground $hbcolor -activebackground $abcolor
pack .images.scroll -side right -fill y

canvas .images.canvas -bg $bgcolor -highlightthickness 0 -borderwidth 0 -relief sunken
pack .images.canvas -side left -fill both -expand true

imfsample .images.canvas.content -pad 7 \
	-showcolor $showcolor -showmasks $showmasks -shownames $shownames \
	-bg $bgcolor -fg $fgcolor -closeup $cucolor -imfapp 1 -dbl 0 -iwidth 44 -iheight 48

.images.canvas create window 6 0 -window .images.canvas.content -anchor nw

bind .images.canvas.content <ButtonPress-1> { select_imf %x %y }

# This should really not be necessary, but for some reason the scrollbar arrows and 
# slider remain active after releasing the mouse button on the Mac. The dummy scroll
# command deactivates them.
if { "$tcl_platform(platform)" == "macintosh" } {
	bind .images.scroll <ButtonRelease-1> \
		{ .images.canvas.content yview scroll 0 units }
}

# Make sure the content is redrawn if the window is resized or exposed.
bind . <Configure> { .images.canvas.content redraw }
bind . <Expose> { .images.canvas.content redraw }

# Size and center the window on the main screen.
set w [ expr { [ winfo screenwidth . ] - 300 } ]
set h [ expr { [ winfo screenheight . ] - 200 } ]
set x 150
set y 100

# Adjust for the menu bar on MacOS.
if { "$tcl_platform(platform)" == "macintosh" } {
	set h [ expr { $h - 20 } ]
}

# Adjust for the window decor on Windows and Unix.
if { "$tcl_platform(platform)" == "windows" \
	|| "$tcl_platform(platform)" == "unix" } {
	set h [ expr { $h - 50 } ]
	set y [ expr { $y - 25 } ]
}

set geomspec ""
set geomspec [ append geomspec $w "x" $h ]
set geomspec [ append geomspec "+$x+$y" ]

wm geometry . $geomspec

# Finally make the window visible in its new position.
wm deiconify .
focus -force .
update idletasks

# Clear all existing images, start afresh.
proc new_cmd {} {
    .images.canvas.content remove all
   .closeup.content.s12 remove all
   .closeup.content.s24 remove all
   .closeup.content.s44 remove all
   .closeup.content.s88 remove all
   .closeup.content itemconfigure selname -text "Nothing selected"
   .closeup.content itemconfig status -text "No image families loaded"
}

proc open_imf_dir_cmd {} {
    set filename [ tk_getOpenFile ]
    # An unconditional update is necessary here.
    update
    .images.canvas.content redraw
    update idletasks
    if { $filename != {} } {
	# Set the working directory manually to the file location (needed 
	# in both MacOS and Windows).
    	set dname [ file dirname $filename ]
	cd $dname
	open_imf_dir_file $filename
	.images.canvas.content add all
	update idletasks
	.images.canvas.content yview scroll 0 units
	.images.canvas.content redraw
	.closeup.content itemconfig status -text [ get_imf_status ]
    }
}

proc add_imf_file_cmd {} {
    set filename [ tk_getOpenFile ]
    # An unconditional update is necessary here.
    update
    .images.canvas.content redraw
    update idletasks
    if { $filename != {} } {
	# Set the working directory manually to the file location (needed 
	# in both MacOS and Windows).
    	set dname [ file dirname $filename ]
	cd $dname
    	load_imf_file $filename
	.images.canvas.content add all
	update idletasks
	.images.canvas.content yview scroll 0 units
	.images.canvas.content redraw
	.closeup.content itemconfig status -text [ get_imf_status ]
    }
}

proc save_imf_dir_cmd {} {
    set filename [ tk_getSaveFile -initialfile "imf.dir" ]
    if { $filename != {} } {
	# Set the working directory manually to the file location (needed 
	# in both MacOS and Windows).
    	set dname [ file dirname $filename ]
	cd $dname
	save_imf_dir_file $filename
    }
}

proc save_imf_file_cmd {} {
    global save_selected_only

    if { $save_selected_only } {
    	# Tells the C code what image should be saved
	set number [ .images.canvas.content curselection ]
	# No image is currently selected
	if { $number < 0 } {
		return
	}
	set filename [ get_imf_name $number ]
	set filename [ append filename ".imf" ]
    } else {
    	# Tells the C code that all images should be saved
	set number [ get_numimages ]
	set filename "images.imf"
    }
    set filename [ tk_getSaveFile -initialfile $filename ]
    if { $filename != {} } {
	# Set the working directory manually to the file location (needed 
	# in both MacOS and Windows).
    	set dname [ file dirname $filename ]
	cd $dname
	save_imf_file $filename $number
    }
}

proc toggle_show_color {} {
    global showcolor

    .images.canvas.content config -showcolor $showcolor
}

proc toggle_show_names {} {
    global shownames

    .images.canvas.content config -shownames $shownames
}

proc toggle_show_masks {} {
    global showmasks

    .images.canvas.content config -showmasks $showmasks
}

proc toggle_show_bbox {} {
    global showbbox

    .images.canvas.content config -showbbox $showbbox
}

proc toggle_show_solid {} {
    global showsolid

    .images.canvas.content config -showsolid $showsolid
}

proc toggle_show_grid {} {
    global showgrid

    .images.canvas.content config -showgrid $showgrid
}

# Change the background color for all image samples.

proc set_bg_color {} {
    global bgcolor

    .images.canvas.content config -bg $bgcolor
    .images.canvas config -bg $bgcolor
}

proc set_fg_color {} {
    global fgcolor

    .images.canvas.content config -fg $fgcolor
}

proc set_cu_color {} {
    global cucolor

    .images.canvas.content config -closeup $cucolor
    .closeup.content.s12 config -bg $cucolor
    .closeup.content.s24 config -bg $cucolor
    .closeup.content.s44 config -bg $cucolor
    .closeup.content.s88 config -bg $cucolor
}

proc toggle_as_unit {} {
}

proc toggle_as_terrain {} {
    global with_terrain
    
    set oldterrain [ .images.canvas.content cget -terrain ]
    if { $with_terrain } {
	.images.canvas.content terrain [ .images.canvas.content curselection ]
    } else {
	.images.canvas.content terrain -1
    }
    set newterrain [ .images.canvas.content cget -terrain ]
    if { $oldterrain != $newterrain } {
	.images.canvas.content redraw
    }
}

proc toggle_as_emblem {} {
    global with_emblem

    set oldemblem [ .images.canvas.content cget -emblem ]
    if { $with_emblem } {
	.images.canvas.content emblem [ .images.canvas.content curselection ]
    } else {
	.images.canvas.content emblem -1
    }
    set newemblem [ .images.canvas.content cget -emblem ]
    if { $oldemblem != $newemblem } {
	.images.canvas.content redraw
    }
}

proc toggle_with_unit {} {
#    .images.canvas.content unit [ .images.canvas.content curselection ]
}

proc toggle_with_terrain {} {
    global with_terrain
    
    if { $with_terrain } {
	.images.canvas.content terrain [ .images.canvas.content curselection ]
    } else {
	.images.canvas.content terrain -1
    }
    .images.canvas.content redraw
}

proc toggle_with_emblem {} {
    global with_emblem

    if { $with_emblem } {
	.images.canvas.content emblem [ .images.canvas.content curselection ]
    } else {
	.images.canvas.content emblem -1
    }
    .images.canvas.content redraw
}

proc select_imf { x y } {
    set oldselection [ .images.canvas.content curselection ]
    .images.canvas.content select $x $y
    set n [ .images.canvas.content curselection ]
    if { $n >= 0 } {
	set header "[ get_imf_name $n ] ([ get_imf_numsizes $n ] images)" 
    } else {
	set header "Nothing selected"
    }
    if { $n != $oldselection } {
    	toggle_as_emblem
    	toggle_as_terrain
    }
   .closeup.content.s12 replace imf [ get_imf_name $n ]
   .closeup.content.s24 replace imf [ get_imf_name $n ]
   .closeup.content.s44 replace imf [ get_imf_name $n ]
   .closeup.content.s88 replace imf [ get_imf_name $n ]
   .closeup.content itemconfigure selname -text $header
   .images.canvas.content yview scroll 0 units
}
