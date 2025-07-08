SUBDIR=		tools

override ARCH=$(shell uname -m)
override CROSS_COMPILE=
ifeq ($(shell uname -s),Darwin)
override CC=cc
endif

PROGRAMS= \
	mce
SRCS-mce= \
	$(CURDIR)/mce.c \
	$(RUNTIME)\
	$(SRCDIR)/unix_process/unix_process_runtime.c


CFLAGS+=	-O3 \
		-I$(ARCHDIR) \
		-I$(SRCDIR) \
		-I$(SRCDIR)/runtime \
		-I$(SRCDIR)/unix_process \
#CFLAGS+=	-DENABLE_MSG_DEBUG -DID_HEAP_DEBUG

CLEANDIRS+=	$(OBJDIR)/test

# gcov support
#CFLAGS+=	-fprofile-arcs -ftest-coverage
#LDFLAGS+=	-fprofile-arcs
GCDAFILES=	$(sort $(foreach p,$(PROGRAMS),$(patsubst %.o,%.gcda,$(OBJS-$p))))
GCOVFILES=	$(sort $(foreach p,$(PROGRAMS),$(patsubst %.o,%.gcno,$(OBJS-$p))))
GCOVFILES+=	$(GCDAFILES) $(OBJDIR)/gcov-tests.info
CLEANFILES+=	$(GCOVFILES)
CLEANDIRS+=	$(OUTDIR)/tools

all: $(PROGRAMS)

.PHONY: test gcov gcov-clean

run: all
	$(Q) $(RM) $(GCDAFILES)
	$(foreach p,$(filter-out $(SKIP_TEST),$(PROGRAMS)),$(call execute_command,$(PROG-$p)))

gcov: run
	$(foreach p,$(PROGRAMS),$(call execute_command,$(GCOV) -o $(OBJDIR)/test/unit $(PROG-$p)))
	$(LCOV) --capture --directory $(ROOTDIR) --output-file $(OBJDIR)/gcov-tests.info
	$(MKDIR) $(OBJDIR)/gcov
	$(GENHTML) $(OBJDIR)/gcov-tests.info --output-directory $(OBJDIR)/gcov

pre-clean:
	$(Q) $(RM) -r $(OBJDIR)/gcov

gcov-clean:
	$(Q) $(RM) $(GCOVFILES)

include rules.mk

ifeq ($(UNAME_s),Darwin)
CFLAGS+=	-DNO_EPOLL
endif
