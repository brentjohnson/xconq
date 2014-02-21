# Automake Template for xConq's GDL Library
# Copyright (C) 2006	Eric A. McDonald

AM_CXXFLAGS = -I$(srcdir)/../include

SUBDIRS = media unit side gamearea

lib_LTLIBRARIES = libconq_gdl.la

libconq_gdl_la_SOURCES = lisp.cc desc.cc dice.cc types.cc gvars.cc tables.cc
libconq_gdl_la_SOURCES += dir.cc namer.cc pact.cc history.cc
libconq_gdl_la_SOURCES += player.cc score.cc world.cc module.cc

libconq_gdl_la_LIBADD = media/libconq_gdl_media.la
libconq_gdl_la_LIBADD += unit/libconq_gdl_unit.la
libconq_gdl_la_LIBADD += side/libconq_gdl_side.la
libconq_gdl_la_LIBADD += gamearea/libconq_gdl_gamearea.la
