# Makefile - htmlgend

# files to include in the distro but not build

#define the executable to be built
bin_PROGRAMS       = htmlgend

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_HTMLGEND

# define the sources
htmlgend_SOURCES   = \
		$(top_srcdir)/common/sensor.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/lunarCycle.c \
		$(top_srcdir)/common/sunTimes.c \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/dbsqliteHistory.c \
		$(top_srcdir)/common/dbsqliteHiLow.c \
		$(top_srcdir)/common/dbsqliteNOAA.c \
		$(top_srcdir)/common/windAverage.c \
		$(top_srcdir)/common/msglog.c \
		$(top_srcdir)/htmlgenerator/html.c \
		$(top_srcdir)/htmlgenerator/htmlStates.c \
		$(top_srcdir)/htmlgenerator/htmlMgr.c \
		$(top_srcdir)/htmlgenerator/htmlGenerate.c \
		$(top_srcdir)/common/sensor.h \
		$(top_srcdir)/common/datadefs.h \
		$(top_srcdir)/common/dbsqlite.h \
		$(top_srcdir)/common/services.h \
		$(top_srcdir)/common/sysdefs.h \
		$(top_srcdir)/common/wvconfig.h \
		$(top_srcdir)/common/status.h \
		$(top_srcdir)/common/windAverage.h \
		$(top_srcdir)/common/msglog.h \
		$(top_srcdir)/common/beaufort.h \
		$(top_srcdir)/htmlgenerator/htmlGenerate.h \
		$(top_srcdir)/htmlgenerator/html.h \
		$(top_srcdir)/htmlgenerator/htmlMgr.h \
		$(top_srcdir)/htmlgenerator/noaaGenerate.h


# define libraries
htmlgend_LDADD   =

# define library directories
htmlgend_LDFLAGS = -L$(prefix)/lib -L/usr/lib

if CROSSCOMPILE
htmlgend_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

