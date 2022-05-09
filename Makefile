# clear out all default make targets
.SUFFIXES:

# List all make targets which are not filenames
.PHONY: all tests clean

# compiler tool definitions
CC=cc
MAKE=make
RM=rm -rf
AR=ar cru
RANLIB=ranlib

CFLAGS=-g -Wall -O0 -fPIC

# compiler defines
DEFINES=\

# compiler include paths
INCLUDES=\
	-I../ \

# only set them if they're not empty to prevent unnecessary whitespace
ifneq ($(DEFINES),)
    CFLAGS+=$(DEFINES)
endif
ifneq ($(INCLUDES),)
    CFLAGS+=$(INCLUDES)
endif

# local NONSTANDARD libraries to link with
# these MUST be exact filenames, cannot be -l syntax
# for example:
#   ../path/to/lib.a
# NOT THIS:
#   -L../path/lib -llib
# You should NOT need to add a make target to build 
# this library if you have added it correctly.
LLIBS=\

# STANDARD libraries to link with (-l is fine here)
# MUST have LLIBS BEFORE the standard libraries
LIBS=\
	$(LLIBS) \
	-lm \

# source files
# local source files first, other sources after
SRC=\
    ./detect_gibberish.c \

# object files are source files with .c replaced with .o
OBJS=\
	$(SRC:.c=.o) \

# dependency files are source files with .c replaced with .d
DFILES=\
	$(SRC:.c=.d) \

# target dependencies
# this includes any script generated c/h files,
# the $(LLIBS) list, and the $(OBJS) list
DEPS=\
	$(LLIBS) \
	$(OBJS) \

# unit tests
TESTS=\

# target files
TARGETS=\
    detect_gibberish \

# Default target for 'make' command
all: $(TARGETS)

# unit test target
tests: $(TESTS)

# target for detect_gibberish
detect_gibberish: $(DEPS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# catch-all make target to generate .o and .d files
%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# catch-all for static libraries in the form of:
# <directory>/<library.a>
# this expects that the makefile in <directory> has a
# make target named <library>
%.a:
	$(MAKE) -C $(dir $@) $(notdir $@)

# generic clean target
clean:
	@$(RM) $(DFILES) $(OBJS) $(TARGETS) $(TESTS)

# Now include our target dependency files
# the hyphen means ignore non-existent files
-include $(DFILES)
