# See LICENSE file for copyright and license details.

VERSION=0.3.0
DEBUG=1

CXXFLAGS=-Wall -Wextra -g
LDFLAGS=-lpanel -lncurses
EXTRA=-D VERSION=\"$(VERSION)\" -D DEBUG_LEVEL=$(DEBUG)


SRCDIR=src
OBJDIR=$(SRCDIR)/obj
DEPDIR=$(SRCDIR)/dep

SRC=$(wildcard $(SRCDIR)/*.cpp) $(SRCDIR)/gui/curses.cpp
OBJ=$(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))
DEP=$(addprefix $(DEPDIR)/,$(notdir $(SRC:.cpp=.d)))




all : irc

irc : $(OBJ)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@ $(EXTRA)

include $(DEP)


$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) -c $< $(CXXFLAGS) $(LDFLAGS) -o $@ $(EXTRA)

$(OBJDIR)/curses.o : $(SRCDIR)/gui/curses.cpp
	$(CXX) -c $< $(CXXFLAGS) $(GUI_LDFLAGS) -o $@ $(EXTRA)

$(DEPDIR)/%.d : $(SRCDIR)/%.cpp
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -MM -MT $(subst $(DEPDIR),$(OBJDIR),$(@:.d=.o)) -MF $@ $(EXTRA)

$(DEPDIR)/curses.d : $(SRCDIR)/gui/curses.cpp
	$(CXX) $^ $(CXXFLAGS) $(GUI_LDFLAGS) -MM -MT $(subst $(DEPDIR),$(OBJDIR),$(@:.d=.o)) -MF $@ $(EXTRA)

$(OBJ) :|$(OBJDIR)
$(DEP) :|$(DEPDIR)

$(OBJDIR) :
	@mkdir $(OBJDIR)
$(DEPDIR) :
	@mkdir $(DEPDIR)

.PHONY: clean
clean:
	@rm -f irc $(OBJ) $(DEP)


