# Automake Template for xConq's Low-Level Utility Library
# Copyright (C) 2006	Eric A. McDonald

AM_CXXFLAGS = -I$(srcdir)/../include

SUBDIRS = unix

lib_LTLIBRARIES = libconq_util.la

libconq_util_la_SOURCES = memory.cc string.cc pbooltbl.cc math.cc time.cc
libconq_util_la_SOURCES += socket.cc fs.cc prng.cc log.cc ui.cc debug.cc

if MAJOR_PLATFORM_UNIX
    libconq_util_la_LIBADD = unix/libconq_unix.la 
endif

