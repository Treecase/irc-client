# See LICENSE file for copyright and license details.

VERSION=0.1.0
DEBUG=0

CFLAGS=-Wall -Wextra -g -D VERSION=\"$(VERSION)\" -D DEBUG_LEVEL=$(DEBUG)
LDFLAGS=-lreadline -lpthread


SRCDIR=src
OBJDIR=$(SRCDIR)/obj
DEPDIR=$(SRCDIR)/dep



SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(subst $(SRCDIR),$(OBJDIR),$(SRC:.c=.o))
DEP=$(subst $(SRCDIR),$(DEPDIR),$(SRC:.c=.d))



irc : $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

include $(DEP)


$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS) -o $@

$(DEPDIR)/%.d : $(SRCDIR)/%.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -MM -MT $(subst $(DEPDIR),$(OBJDIR),$(@:.d=.o)) -MF $@

$(OBJ) :|$(OBJDIR)
$(DEP) :|$(DEPDIR)

$(OBJDIR) :
	@mkdir $(OBJDIR)
$(DEPDIR) :
	@mkdir $(DEPDIR)

.PHONY: clean
clean:
	@rm -f irc $(OBJ) $(DEP)


