all:a c
	echo all MAKE_RESTARTS: $(MAKE_RESTARTS)
a:
	touch a

include haha

haha: tmp
#   echo  all: b > haha
	echo haha: MAKE_RESTARTS: $(MAKE_RESTARTS)

include bili

bili: tmp
	echo bili: MAKE_RESTARTS: $(MAKE_RESTARTS)
	touch bili

b:
	echo bbbb

c :
	echo cccc

_tmp:
	echo tmpppp

.PHONY: a tmp