CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
LDFLAGS ?= -ldl -lpthread

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share

SRC = src/main.c src/scarlett_alsa.c src/tray.c src/cli_parse.c src/i18n.c
TARGET = scarlett-control
TEST_BIN = test_parser

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): test/test_parser.c src/cli_parse.c src/i18n.c
	$(CC) $(CFLAGS) test/test_parser.c src/cli_parse.c src/i18n.c -o $(TEST_BIN)

clean:
	rm -f $(TARGET) $(TEST_BIN)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -d $(DESTDIR)$(DATADIR)/applications
	install -m 644 scarlett-control.desktop $(DESTDIR)$(DATADIR)/applications/
	install -d $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps
	install -m 644 assets/scarlett-control.svg $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/
	install -d $(DESTDIR)$(DATADIR)/scarlett-control/locales
	install -m 644 locales/*.json $(DESTDIR)$(DATADIR)/scarlett-control/locales/

install-user: $(TARGET)
	install -d $(HOME)/.local/bin
	install -m 755 $(TARGET) $(HOME)/.local/bin/$(TARGET)
	install -d $(HOME)/.local/share/applications
	install -m 644 scarlett-control.desktop $(HOME)/.local/share/applications/
	install -d $(HOME)/.local/share/icons/hicolor/scalable/apps
	install -m 644 assets/scarlett-control.svg $(HOME)/.local/share/icons/hicolor/scalable/apps/
	install -d $(HOME)/.local/share/scarlett-control/locales
	install -m 644 locales/*.json $(HOME)/.local/share/scarlett-control/locales/
	install -d $(HOME)/.config/autostart
	install -m 644 scarlett-control.desktop $(HOME)/.config/autostart/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(DATADIR)/applications/scarlett-control.desktop
	rm -f $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/scarlett-control.svg
	rm -rf $(DESTDIR)$(DATADIR)/scarlett-control

.PHONY: all clean test install install-user uninstall
