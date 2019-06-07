# See LICENSE file for copyright and license details.

GUI=curses
VERSION=0.2.0
DEBUG=0

CFLAGS=-Wall -Wextra -g
LDFLAGS=-lreadline -lpthread
EXTRA=-D VERSION=\"$(VERSION)\" -D DEBUG_LEVEL=$(DEBUG)


SRCDIR=src
OBJDIR=$(SRCDIR)/obj
DEPDIR=$(SRCDIR)/dep

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(subst $(SRCDIR),$(OBJDIR),$(SRC:.c=.o))
DEP=$(subst $(SRCDIR),$(DEPDIR),$(SRC:.c=.d))

ifeq ($(GUI), curses)
  LDFLAGS += -lncurses
  SRC += $(SRCDIR)/gui/curses.c
else
  SRC += $(SRCDIR)/gui/term.c
  OBJ += $(OBJDIR)/gui/term.o
  DEP += $(DEPDIR)/gui/term.d
endif



irc : $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@ $(EXTRA)

include $(DEP)


$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@ $(EXTRA)

$(OBJDIR)/gui/%.o : $(SRCDIR)/gui/%.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@ $(EXTRA)

$(DEPDIR)/%.d : $(SRCDIR)/%.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -MM -MT $(subst $(DEPDIR),$(OBJDIR),$(@:.d=.o)) -MF $@ $(EXTRA)

$(DEPDIR)/gui/%.d : $(SRCDIR)/gui/%.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -MM -MT $(subst $(DEPDIR),$(OBJDIR),$(@:.d=.o)) -MF $@ $(EXTRA)

$(OBJ) :|$(OBJDIR) $(OBJDIR)/gui
$(DEP) :|$(DEPDIR) $(DEPDIR)/gui

$(OBJDIR) :
	@mkdir $(OBJDIR)
$(OBJDIR)/gui :
	@mkdir $(OBJDIR)/gui
$(DEPDIR) :
	@mkdir $(DEPDIR)
$(DEPDIR)/gui :
	@mkdir $(DEPDIR)/gui

.PHONY: clean
clean:
	@rm -f irc $(OBJ) $(DEP)


