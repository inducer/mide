UI_FILES=main.ui about.ui
HEADERS=main.hh 
MODULES=main.cc
CXXFLAGS=-g -O2

UI_HEADERS=$(UI_FILES:.ui=_ui.hh)
UI_MODULES=$(UI_FILES:.ui=_ui.cc)
MOC_MODULES=$(REALHEADERS:.hh=_moc.cc)
REALHEADERS=$(HEADERS) $(UI_HEADERS)
REALMODULES=$(MOC_MODULES) $(UI_MODULES) $(MODULES)
OBJECTS=$(REALMODULES:.cc=.o)

mide: $(OBJECTS)
	c++ $(CXXFLAGS) -o $@ $(OBJECTS) -lqt-mt -L/usr/X11R6/lib -lXext -lX11 -lm

clean:
	rm -f mide *.o *_ui.{hh,cc} *_moc.cc

%_ui.hh: %.ui
	uic $< -o $@
	
%_ui.cc: %.ui
	uic -impl $*_ui.hh $< -o $@

%_moc.cc: %.hh
	moc -o $@ $<

%.o: %.cc $(REALHEADERS)
	c++ $(CXXFLAGS) -I/usr/include/qt3 -c -o $@ $<

.PRECIOUS: %.o %_moc.cc %_ui.cc %_ui.hh
