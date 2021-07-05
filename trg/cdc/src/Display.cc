/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#include <unistd.h>
#include <iostream>
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayDrawingArea.h"

using namespace std;

namespace Belle2 {

  Gtk::Main* GtkMain = 0;
  bool TRGCDCDisplay::_skipEvent = false;
  bool TRGCDCDisplay::_endOfEvent = false;
  bool TRGCDCDisplay::_endOfEventFlag = false;
  bool TRGCDCDisplay::_nonStop = false;

  TRGCDCDisplay::TRGCDCDisplay(const string& name,
                               int ,
                               int)
    : _skip(false),
      _wireName(false),
      _oldCDC(false),
      _box0(false, 2),
      _menuButtons(true, 2),
      _buttonNext("Next Step"),
      _buttonEndOfEvent("End of Event"),
      _buttonNextEvent("Next Event"),
      _buttonNonStop("Non Stop"),
      _label("Stage : TRGing not started\nInformation :",
             Gtk::ALIGN_LEFT,
             Gtk::ALIGN_TOP),
      _buttonPositionReset("Reset position"),
      _buttonWireName("Wire Name"),
      _rphi(0)
  {

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

    _buttonNonStop
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_nonStop));
    _menuButtons.pack_start(_buttonNonStop, Gtk::PACK_EXPAND_WIDGET, 2);

    _buttonPositionReset
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_positionReset));
    _buttonWireName
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplay::on_wireName));
  }

  TRGCDCDisplay::~TRGCDCDisplay()
  {
  }

  void
  TRGCDCDisplay::on_positionReset(void)
  {
    area().resetPosition();
    area().on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplay::on_next(void)
  {
    Gtk::Main::quit();
  }

  void
  TRGCDCDisplay::on_endOfEvent(void)
  {
    _endOfEventFlag = true;
    _skipEvent = true;
    Gtk::Main::quit();
  }

  void
  TRGCDCDisplay::on_nextEvent(void)
  {
    _skipEvent = true;
    Gtk::Main::quit();
  }

  void
  TRGCDCDisplay::on_nonStop(void)
  {
    _nonStop = true;
    Gtk::Main::quit();
  }

  void
  TRGCDCDisplay::on_wireName(void)
  {
    _wireName = _buttonWireName.get_active();
    area().wireName(_wireName);
    area().on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplay::run(void)
  {
    int argc = 0;
    char** argv = 0;
    Gtk::Main main_instance(argc, argv);
    show();
    if (_nonStop)
      sleep(1);
    else if (((! _skip) && (! _skipEvent)) || (_endOfEventFlag && _endOfEvent))
      Gtk::Main::run();
  }

  void
  TRGCDCDisplay::clear(void)
  {
    area().clear();
    const string id = "unknown";
    _stage = id;
    _info = "";
  }

  void
  TRGCDCDisplay::stop(void)
  {
    _nonStop = false;
    _skip = false;
    _skipEvent = false;
  }

} // namespace Belle2

#endif
