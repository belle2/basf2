//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Display.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDCDisplay_INLINE_DEFINE_HERE

#include <iostream>
#include "trg/cdc/Display.h"

using namespace std;

namespace Belle2 {

Gtk::Main * GtkMain = 0;
bool TRGCDCDisplay::_skipEvent = false;
bool TRGCDCDisplay::_endOfEvent = false;
bool TRGCDCDisplay::_endOfEventFlag = false;

TRGCDCDisplay::TRGCDCDisplay(const string & name,
			     int sizeWindow,
			     int sizeMax)
    : _skip(false),
      _wireName(false),
      _oldCDC(false),
      _box0(false, 2),
      _menuButtons(true, 2),
      _buttonNext("Next Step"),
      _buttonEndOfEvent("End of Event"),
      _buttonNextEvent("Next Eevnt"),
      _label("Stage : TRGing not started\nInformation :",
	     Gtk::ALIGN_LEFT,
	     Gtk::ALIGN_TOP),
      _buttonPositionReset("Reset position"),
      _buttonWireName("Wire Name") {

    set_title(name);

    _buttonWireName.set_active(false);
    _wireName = _buttonWireName.get_active();

    _buttonNext
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_next));
    _menuButtons.pack_start(_buttonNext, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonEndOfEvent
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_endOfEvent));
    _menuButtons.pack_start(_buttonEndOfEvent, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonNextEvent
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_nextEvent));
    _menuButtons.pack_start(_buttonNextEvent, Gtk::PACK_EXPAND_WIDGET, 2);

    _buttonPositionReset
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_positionReset));
    _buttonWireName
	.signal_clicked()
	.connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_wireName));
}

TRGCDCDisplay::~TRGCDCDisplay() {
}

void
TRGCDCDisplay::on_positionReset(void) {
    area().resetPosition();
    area().on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplay::on_next(void) {
    Gtk::Main::quit();
}

void
TRGCDCDisplay::on_endOfEvent(void) {
    _endOfEventFlag = true;
    _skipEvent = true;
    Gtk::Main::quit();
}

void
TRGCDCDisplay::on_nextEvent(void) {
    _skipEvent = true;
    Gtk::Main::quit();
}

void
TRGCDCDisplay::on_wireName(void) {
    _wireName = _buttonWireName.get_active();
    area().wireName(_wireName);
    area().on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplay::run(void) {
    int argc = 0;
    char ** argv = 0;
    Gtk::Main main_instance(argc, argv);
    show();
    if (((! _skip) && (! _skipEvent)) ||
	(_endOfEventFlag && _endOfEvent))
	Gtk::Main::run();
}

void
TRGCDCDisplay::clear(void) {
    area().clear();
    const string id = "unknown";
    _stage = id;
    _info = "";
}

} // namespace Belle2

#endif
