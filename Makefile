#------------------------------------------------------------------------------

SRC  = MWDC.cxx WirePlane.cxx Hit.cxx TimeToDistConv.cxx Hitpattern.cxx

PACKAGE = TreeSearch

LINKDEF = $(PACKAGE)_LinkDef.h

#------------------------------------------------------------------------------
# Compile debug version
export DEBUG = 1

# Architecture to compile for
ARCH          = linux
#ARCH          = solarisCC5

#------------------------------------------------------------------------------
# Directory locations. All we need to know is INCDIRS.
# INCDIRS lists the location(s) of the C++ Analyzer header (.h) files

ifndef ANALYZER
  $(error $$ANALYZER environment variable not defined)
endif

INCDIRS  = $(wildcard $(addprefix $(ANALYZER)/, include src hana_decode hana_scaler))

#------------------------------------------------------------------------------
# Do not change anything  below here unless you know what you are doing

ifeq ($(strip $(INCDIRS)),)
  $(error No Analyzer header files found. Check $$ANALYZER)
endif

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)

INCLUDES      = $(ROOTCFLAGS) $(addprefix -I, $(INCDIRS) ) -I$(shell pwd)

USERLIB       = lib$(PACKAGE).so
USERDICT      = $(PACKAGE)Dict

LIBS          = 
GLIBS         = 

ifeq ($(ARCH),solarisCC5)
# Solaris CC 5.0
CXX           = CC
ifdef DEBUG
  CXXFLAGS    = -g
  LDFLAGS     = -g
else
  CXXFLAGS    = -O
  LDFLAGS     = -O
endif
CXXFLAGS     += -KPIC
LD            = CC
SOFLAGS       = -G
endif

ifeq ($(ARCH),linux)
# Linux with gcc (RedHat)
CXX           = g++
ifdef DEBUG
  CXXFLAGS    = -g -O0
  LDFLAGS     = -g -O0
else
  CXXFLAGS    = -O
  LDFLAGS     = -O
endif
CXXFLAGS     += -Wall -Woverloaded-virtual -fPIC
LD            = g++
SOFLAGS       = -shared
endif

ifeq ($(CXX),)
$(error $(ARCH) invalid architecture)
endif

CXXFLAGS     += $(INCLUDES)
LIBS         += $(ROOTLIBS) $(SYSLIBS)
GLIBS        += $(ROOTGLIBS) $(SYSLIBS)

MAKEDEPEND    = gcc

ifdef WITH_DEBUG
CXXFLAGS     += -DWITH_DEBUG
endif

ifdef PROFILE
CXXFLAGS     += -pg
LDFLAGS      += -pg
endif

ifndef PKG
PKG           = lib$(PACKAGE)
LOGMSG        = "$(PKG) source files"
else
LOGMSG        = "$(PKG) Software Development Kit"
endif
DISTFILE      = $(PKG).tar.gz

#------------------------------------------------------------------------------
OBJ           = $(SRC:.cxx=.o)
HDR           = $(SRC:.cxx=.h)
DEP           = $(SRC:.cxx=.d)
OBJS          = $(OBJ) $(USERDICT).o

all:		$(USERLIB)

$(USERLIB):	$(HDR) $(OBJS)
		$(LD) $(LDFLAGS) $(SOFLAGS) -o $@ $(OBJS)
		@echo "$@ done"

$(USERDICT).cxx: $(HDR) $(LINKDEF)
	@echo "Generating dictionary $(USERDICT)..."
	$(ROOTSYS)/bin/rootcint -f $@ -c $(INCLUDES) $^

install:	all
		$(error Please define install yourself)
# for example:
#		cp $(USERLIB) $(LIBDIR)

clean:
		rm -f *.o *~ $(USERLIB) $(USERDICT).*

realclean:	clean
		rm -f *.d

srcdist:
		rm -f $(DISTFILE)
		rm -rf $(PKG)
		mkdir $(PKG)
		cp -p $(SRC) $(HDR) $(LINKDEF) db*.dat README Makefile $(PKG)
		gtar czvf $(DISTFILE) --ignore-failed-read \
		 -V $(LOGMSG)" `date -I`" $(PKG)
		rm -rf $(PKG)

.PHONY: all clean realclean srcdist

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .cxx .C .o .d

%.o:	%.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# FIXME: this only works with gcc
%.d:	%.cxx
	@echo Creating dependencies for $<
	@$(SHELL) -ec '$(MAKEDEPEND) -MM $(INCLUDES) -c $< \
		| sed '\''s%^.*\.o%$*\.o%g'\'' \
		| sed '\''s%\($*\)\.o[ :]*%\1.o $@ : %g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

###

-include $(DEP)

