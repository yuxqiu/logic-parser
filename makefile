# This makefile cannot handle these cases:
# 1. Main object files "export" functions that are linked by other object files

# compilers and constant flags
CC = c++
CFLAGS = -Wall -Werror -Wextra -Wpedantic -Wextra-semi -Wnull-dereference -Wsuggest-override -Wconversion -Wshadow -std=c++17

# a list of dirs that has src code
DIRS = src test lib

# file that we want to match
EXT = cc

# output file extension (required)
OUTPUT_EXT = out

# dependency
INC = -Iinclude -Ilib

# build dir
BASE_BUILDDIR = build
BASE_TARGETDIR = bin

# flags
CDFLAGS =
LDFLAGS =

# define where to store generated file
DEBUG = debug
RELEASE = release

# release mode
ifeq ($(release), 1)
	TARGETDIR = $(BASE_TARGETDIR)/$(RELEASE)
	BUILDDIR = $(BASE_BUILDDIR)/$(RELEASE)
	CDFLAGS := $(CDFLAGS) -O2 -DNDEBUG
	LDFLAGS := $(LDFLAGS)
else
	TARGETDIR = $(BASE_TARGETDIR)/$(DEBUG)
	BUILDDIR = $(BASE_BUILDDIR)/$(DEBUG)
	CDFLAGS := $(CDFLAGS) -g -DDEBUG

	ifneq ($(OS),Windows_NT)
		CDFLAGS := $(CDFLAGS) -fsanitize=address,undefined
		LDFLAGS := $(LDFLAGS) -fsanitize=address,undefined
	endif
endif

# if profile=1 in OSX
ifeq ($(shell uname -s),Darwin)
	ifeq ($(profile), 1)
		LDFLAGS := $(LDFLAGS) -L$(shell brew --prefix gperftools)/lib -lprofiler
	endif
endif

# match all source files
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES = $(call rwildcard,.,*.$(EXT))
SOURCES := $(SOURCES:./%=%)
SOURCES := $(SOURCES:.$(EXT)=.o)

OBJECTS = $(addprefix $(BUILDDIR)/, $(SOURCES))


all: compile link


compile:
	@echo "===> Compiling"


# Add header dependency
HEADER_DEPEND := $(patsubst %.o,%.d,$(OBJECTS))
-include $(HEADER_DEPEND)


$(BUILDDIR)/%.o: %.$(EXT) makefile
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) $(CDFLAGS) $(INC) -MMD -MP


link: $(OBJECTS)
	@echo "===> Linking"
	@mkdir -p $(TARGETDIR)
	$(eval MAINOBJECTS = $(shell nm -A $(OBJECTS) | grep 'T main\|T _main' | cut -d ':' -f1))
	@$(foreach MAIN, $(MAINOBJECTS), \
		$(eval TARGET = $(subst $(BUILDDIR), $(TARGETDIR), $(MAIN))) \
		mkdir -p $(dir $(TARGET)); \
		$(eval LINK = $(filter-out $(MAINOBJECTS), $(OBJECTS))) \
		echo "$(CC) -o $(TARGET:.o=.$(OUTPUT_EXT)) $(MAIN) $(LINK) $(LDFLAGS)" ; \
		$(CC) -o $(TARGET:.o=.$(OUTPUT_EXT)) $(MAIN) $(LINK) $(LDFLAGS); \
	)


clean:
	@echo "===> Cleaning"
	@$(RM) -r $(BASE_BUILDDIR) $(BASE_TARGETDIR)


run:
	@$(foreach file, $(call rwildcard,$(TARGETDIR),*.$(OUTPUT_EXT)), ./$(file);)


valgrind:
	@$(foreach file, $(call rwildcard,$(TARGETDIR),*.$(OUTPUT_EXT)), valgrind ./$(file);)


leaks:
	@$(foreach file, $(call rwildcard,$(TARGETDIR),*.$(OUTPUT_EXT)),  leaks -atExit -- ./$(file);)


.PHONY: clean run valgrind leaks