# Makefile - Vantage Pro station daemon

#define the executable to be built
bin_PROGRAMS    = wviewd_vpro

# define include directories
INCLUDES = \
		-I$(top_srcdir)/common \
		-I$(prefix)/include \
		-D_GNU_SOURCE \
		-DWV_CONFIG_DIR=\"$(sysconfdir)/wview\" \
		-DWV_RUN_DIR=\"$(localstatedir)/wview\" \
		-DBUILD_WVIEWD

# define the sources
wviewd_vpro_SOURCES     = \
		$(top_srcdir)/common/sensor.c \
		$(top_srcdir)/common/wvutils.c \
		$(top_srcdir)/common/msglog.c \
		$(top_srcdir)/common/wvconfig.c \
		$(top_srcdir)/common/status.c \
		$(top_srcdir)/common/dbsqlite.c \
		$(top_srcdir)/common/dbsqliteHiLow.c \
		$(top_srcdir)/common/windAverage.c \
		$(top_srcdir)/common/msglog.c \
		$(top_srcdir)/wviewd_vpro/computedData.c \
		$(top_srcdir)/wviewd_vpro/daemon.c \
		$(top_srcdir)/wviewd_vpro/station.c \
		$(top_srcdir)/wviewd_vpro/serial.c \
		$(top_srcdir)/wviewd_vpro/stormRain.c \
		$(top_srcdir)/wviewd_vpro/vproInterface.c \
		$(top_srcdir)/wviewd_vpro/vproStates.c \
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
		$(top_srcdir)/wviewd_vpro/computedData.h \
		$(top_srcdir)/wviewd_vpro/daemon.h \
		$(top_srcdir)/wviewd_vpro/station.h \
		$(top_srcdir)/wviewd_vpro/serial.h \
		$(top_srcdir)/wviewd_vpro/stormRain.h \
		$(top_srcdir)/wviewd_vpro/vproInterface.h \
		$(top_srcdir)/wviewd_vpro/Ccitt.h

# define libraries
wviewd_vpro_LDADD       =

# define library directories
wviewd_vpro_LDFLAGS     = -L$(prefix)/lib -L/usr/lib

if CROSSCOMPILE
wviewd_vpro_LDFLAGS += $(prefix)/lib/crt1.o $(prefix)/lib/crti.o $(prefix)/lib/crtn.o
endif

