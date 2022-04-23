CFLAGS  = -std=c++17 -Wall
EXTRAS  = deps/beep.o deps/i18n.o
DEPS    = -lcAudio -lpthread -lhttpserver -lfontconfig
PACKAGE = audinary

prefix 	 	= /usr
exec_prefix = $(prefix)
bindir		= $(exec_prefix)/sbin
sysconfdir  = $(prefix)/etc
environment	= cinnamon
initdir		= $(sysconfdir)/init.d/
systemdir	= $(sysconfdir)/systemd/system/
deskletdir  = $$(eval echo ~$${SUDO_USER})/.local/share/$(environment)/desklets
desklet		= $(PACKAGE)@zener-diode
USER		= $$(eval echo $${SUDO_USER})

all:
	$(MAKE) $(PACKAGE)

$(PACKAGE): source/intermediate driver.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) -Iinclude $(EXTRAS) $(OBJS) -o $@ driver.cpp $(DEPS)

include source/Makefile

install:
	which $(environment)
	install -d $(DESTDIR)$(bindir)
	install $(PACKAGE) $(DESTDIR)$(bindir)
	sed -i.bak "s|ROOT_PATH=.*|ROOT_PATH=$(bindir)|" service/$(PACKAGE)
	sed -i.bak "s|WorkingDirectory=.*|WorkingDirectory=$(bindir)|" service/$(PACKAGE).service
	install -d $(DESTDIR)$(initdir) $(DESTDIR)$(systemdir)
	cp service/$(PACKAGE) $(DESTDIR)$(initdir)
	cp service/$(PACKAGE).service $(DESTDIR)$(systemdir)
	@echo ${USER_HOME}
	systemctl daemon-reload
	-systemctl stop $(PACKAGE).service
	update-rc.d $(PACKAGE) defaults
	update-rc.d $(PACKAGE) enable 2
	install -d $(DESTDIR)$(deskletdir)
	cp -r $(desklet)/files/$(desklet) $(DESTDIR)$(deskletdir)
	chown $(USER):$(USER) $(DESTDIR)$(deskletdir)/$(desklet)
	chown $(USER):$(USER) $(DESTDIR)$(deskletdir)/$(desklet)/*
	systemctl start $(PACKAGE).service

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(PACKAGE)
	-rmdir $(DESTDIR)$(bindir) >/dev/null 2>&1
	rm -f $(DESTDIR)$(initdir)/$(PACKAGE)
	rm -f $(DESTDIR)$(systemdir)/$(PACKAGE).service
	rm -rf $(DESTDIR)$(deskletdir)/$(desklet)
	systemctl stop $(PACKAGE).service
	update-rc.d -f $(PACKAGE) remove
	systemctl daemon-reload

.PHONY: all install uninstall clean
