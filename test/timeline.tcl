# This is a simple wish script to display Xconq's activity timelines
# that are enabled by the DT command and recorded into xconq.trace.
#
# To run, do "wish timeline.tcl" in a directory with an xconq.trace,
# then use the buttons to scroll around and zoom in/out.

set offset 0
set scale 1

set winwidth 800

canvas .canvas -width $winwidth -height 100 -bg white
pack .canvas

button .mag -text "Mag" -command { magnify }
button .demag -text "Demag" -command { demagnify }
button .left -text "Left" -command { left }
button .right -text "Right" -command { right }
button .quit -text "Quit" -command { exit }
pack .mag .demag .left .right .quit -side left

proc magnify {} {
    global scale

    if { $scale <= 1 } {
	return
    }
    set scale [ expr $scale / 2 ]
    redraw
}

proc demagnify {} {
    global scale

    set scale [ expr $scale * 2 ]
    redraw
}

proc left {} {
    global offset scale winwidth

    set offset [ expr $offset - (($winwidth / 2) * $scale) ]
    redraw
}

proc right {} {
    global offset scale winwidth

    set offset [ expr $offset + (($winwidth / 2) * $scale) ]
    redraw
}

proc pixel_to_value { n } {
    global offset scale

    return [ expr $n * $scale + $offset ]
}

proc value_to_pixel { n } {
    global offset scale

    return [ expr ($n - $offset) / $scale ]
}

proc redraw {} {
    global astart alen aactivity
    global index

    .canvas delete all
    .canvas create line 0 40 1000 40
    for { set i 0 } { $i < 1000 } { incr i 10 } {
	set offy 5
	if { "[ expr $i % 100 ]" == 0 } {
	    set offy 10
	    set val [ pixel_to_value $i ]
	    set val [ expr $val / 1000000.0 ]
	    .canvas create text $i 60 -text "${val}s"
	}
	.canvas create line $i 40 $i [ expr 40 + $offy ]
    }
    for { set i 0 } { $i < $index } { incr i } {
	set start [ value_to_pixel $astart($i) ]
	set end   [ value_to_pixel [ expr $astart($i) + $alen($i) ] ]
	if { "$start" != "$end" } {
	    .canvas create text $start 18 -text "$aactivity($i)" \
		    -anchor sw
	    .canvas create rect $start 20 $end 38 -fill gray
	}
    }
}

set chan [ open "xconq.trace" r ]

set index 0

while { [ gets $chan line ] >= 0 } {
    global astart alen aactivity

    scan $line {%d %d %[a-z_] %d %d} start len activity arg1 arg2
    set astart($index) $start
    set alen($index) $len
    set aactivity($index) $activity
    incr index
}

set last [ expr $index - 1 ]

set scale [ expr ($astart($last) + $alen($last) - $astart(0)) / $winwidth ]

close $chan

redraw
