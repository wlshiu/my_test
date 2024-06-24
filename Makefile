RED="\033[0;31m"
GREEN="\033[0;32m"
LIGHT_GREEN="\033[1;32m"
YELLOW="\033[0;33m"
LIGHT_YELLOW="\033[1;33m"
GREY="\033[0;37m"
BWHITE="\033[1;37m"
MAGENTA="\033[1;35m"
CYAN="\033[1;36m"
NC="\033[0m"


PLATFORM = $(shell uname -o)
ifeq ("$(PLATFORM)","Msys")
    ECHO = echo -e
else
    ECHO = echo
endif

all: a c
	@$(ECHO) $(YELLOW) "all MAKE_RESTARTS: $(MAKE_RESTARTS)" $(NC)

a:
	@$(ECHO) $(YELLOW) "target a" $(NC)
	touch a

$(info include haha)
include haha

haha: pre_haha
#	@$(ECHO)  all: b > haha
	@$(ECHO) $(MAGENTA) haha: MAKE_RESTARTS: $(MAKE_RESTARTS) $(NC)

$(info include bili)
include bili

bili: pre_bili
	@$(ECHO) $(CYAN) bili: MAKE_RESTARTS: $(MAKE_RESTARTS) $(NC)
	touch bili

b:
	@$(ECHO) bbbb $(NC)

c:
	@$(ECHO) cccc $(NC)

pre_haha:
	@$(ECHO) $(GREEN) pre_haha $(NC)
	@$(ECHO)  all: b > haha

pre_bili:
	@$(ECHO) $(GREEN) pre_bili $(NC)
	
tmp:
	@$(ECHO) $(GREEN) tmpppp $(NC)

.PHONY: a tmp