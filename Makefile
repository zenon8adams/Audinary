CC      = g++
CFLAGS  = -std=c++17 -Wall
EXTRAS  = deps/beep.o deps/i18n.o
DEPS    = -lcAudio -lpthread -lhttpserver -lfontconfig
PACKAGE = audinary

prefix 	 	= /usr
exec_prefix = $(prefix)
bindir		= $(exec_prefix)/bin
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
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $(EXTRAS) $(OBJS) -o $@ driver.cpp $(DEPS)

include source/Makefile

install: $(PACKAGE)
	which $(environment)
	install -d $(bindir)
	install $(PACKAGE) $(bindir)
	sed -i.bak "s|ROOT_PATH=.*|ROOT_PATH=$(bindir)|" service/$(PACKAGE)
	sed -i.bak "s|WorkingDirectory=.*|WorkingDirectory=$(bindir)|" service/$(PACKAGE).service
	install -d $(initdir) $(systemdir)
	cp service/$(PACKAGE) $(initdir)
	cp service/$(PACKAGE).service $(systemdir)
	@echo ${USER_HOME}
	systemctl daemon-reload
	-systemctl stop $(PACKAGE).service
	update-rc.d $(PACKAGE) defaults
	update-rc.d $(PACKAGE) enable 2
	install -d $(deskletdir)
	cp -r $(desklet)/files/$(desklet) $(deskletdir)
	chown $(USER):$(USER) $(deskletdir)/$(desklet)
	chown $(USER):$(USER) $(deskletdir)/$(desklet)/*
	systemctl start $(PACKAGE).service

uninstall:
	rm -f $(bindir)/$(PACKAGE)
	-rmdir $(bindir) >/dev/null 2>&1
	rm -f $(initdir)/$(PACKAGE)
	rm -f $(systemdir)/$(PACKAGE).service
	rm -rf $(deskletdir)/$(desklet)
	systemctl stop $(PACKAGE).service
	update-rc.d -f $(PACKAGE) remove
	systemctl daemon-reload

.PHONY: all install uninstall clean
