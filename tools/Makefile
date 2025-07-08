override ARCH=$(shell uname -m)
override CROSS_COMPILE=
ifeq ($(shell uname -s),Darwin)
override CC=cc
endif
CONTGEN=$(OBJDIR)/bin/contgen

include ../rules.mk

contgen: $(CONTGEN)

$(CONTGEN): $(CURDIR)/contgen.c
	@$(MKDIR) $(dir $@)
	$(CC) $(DEPFLAGS) -std=gnu11 -O3 $< -o $@

CLEANFILES+=	$(CONTGEN) $(TOOLDIR)/contgen.d
CLEANDIRS+=	$(OBJDIR)/bin
