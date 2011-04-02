lib_LTLIBRARIES=projectm.la

AM_CXXFLAGS=-I$(top_srcdir)/lib/libprojectM

projectm_la_SOURCES=Main.cpp 
projectm_la_LDFLAGS=-module -avoid-version -L$(top_srcdir)/lib/libprojectM -lprojectM
projectm_la_DEPENDENCIES=projectm

projectm:
	make -C $(top_srcdir)/lib/libprojectM
