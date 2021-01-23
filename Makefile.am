# Makefile - top level

# files to include in the distro but not build
EXTRA_DIST = 

SUBDIRS = wviewd_vpro htmlgenerators

STATION_T = wviewd_vpro

WV_CONFIG_DIR		= $(sysconfdir)/wview
WV_RUN_DIR			= $(localstatedir)/wview
WV_HTML_DIR			= $(localstatedir)
WV_BIN_DIR			= $(exec_prefix)/bin
WV_SHARE_DIR		= /usr/share/wview


# this target removes svn directories from the tarball
dist-hook:
	rm -rf `find $(distdir) -name .svn`


# Keep this for backwards compatibility:
install-env:	install


# Do some post-install tasks:
install-data-hook:
if INSTALL_DPKG
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/var/wview
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/export
	chmod 777 $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/export
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/rsyslog.d
	mkdir -p $(DESTDIR)$(WV_SHARE_DIR)/etc/logrotate.d
	cp -fp $(top_srcdir)/debian/99-wview.conf $(DESTDIR)$(WV_SHARE_DIR)/etc/rsyslog.d
	cp -fp $(top_srcdir)/debian/wview-logrotate $(DESTDIR)$(WV_SHARE_DIR)/etc/logrotate.d
	@echo "$(PACKAGE_VERSION)" > $(DESTDIR)$(WV_SHARE_DIR)/etc/wview/wview-version
else
	@echo "To start/stop wview:"
	@echo "#> sudo /etc/init.d/wview start|stop"
	@echo "$(PACKAGE_VERSION)" > $(DESTDIR)$(WV_CONFIG_DIR)/wview-version
endif
	@mkdir -p $(DESTDIR)$(WV_CONFIG_DIR)
	@chmod 777 $(DESTDIR)$(WV_CONFIG_DIR)
	@if [ ! -e $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary ]; then \
		echo "$(STATION_T)" > $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary; \
	fi
	@chmod 666 $(DESTDIR)$(WV_CONFIG_DIR)/wview-binary
	@if [ -e $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb ]; then \
		chmod 666 $(DESTDIR)$(WV_CONFIG_DIR)/wview-conf.sdb; \
	fi

uninstall-hook:
	rm -rf $(DESTDIR)$(WV_RUN_DIR)
	rm -rf $(DESTDIR)$(WV_CONFIG_DIR)
	@if [ -e $(DESTDIR)$(sysconfdir)/init.d/wview ]; then \
		rm -f $(DESTDIR)$(sysconfdir)/init.d/wview; \
	fi

