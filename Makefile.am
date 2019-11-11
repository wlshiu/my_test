UTOMAKE_OPTIONS = foreign
# AUTOMAKE_OPTIONS = foreign
SUBDIRS = uart_dev
bin_PROGRAMS = UartTerminal
UartTerminal_SOURCES = \
	main.c

UartTerminal_CPPFLAGS=\
	-I./uart_dev

UartTerminal_LDADD=\
	uart_dev/libuartdev.la
