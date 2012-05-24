//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTK.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------



#ifdef TRASAN_WINDOW_GTK

#include <iostream>
#include "tracking/modules/trasan/TWindowGTK.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

  Gtk::Main* GtkMain = 0;
  bool TWindowGTK::_skipEvent = false;
  bool TWindowGTK::_endOfEvent = false;
  bool TWindowGTK::_endOfEventFlag = false;

  TWindowGTK::TWindowGTK(const std::string& name, double outerR, int size)
    : _axial(true),
      _stereo(false),
      _wireName(false),
      _box0(false, 2),
      _menuButtons(true, 2),
      _buttonNext("Next Step"),
      _buttonEndOfEvent("End of Event"),
      _buttonNextEvent("Next Eevnt"),
      _label("Stage : Tracking not started yet \nInformation : ",
             Gtk::ALIGN_LEFT,
             Gtk::ALIGN_TOP),
      _adjustment(double(size) / outerR / 2,
                  double(size) / outerR / 2,
                  10.0,
                  0.1),
      _scaler(_adjustment),
      _buttonPositionReset("Reset position"),
      _buttonAxial("Axial"),
      _buttonStereo("Stereo"),
      _buttonWireName("Wire Name"),
      _w(0)
  {

    set_title(name);

    _buttonNext
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_next));
    _menuButtons.pack_start(_buttonNext, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonEndOfEvent
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_endOfEvent));
    _menuButtons.pack_start(_buttonEndOfEvent, Gtk::PACK_EXPAND_WIDGET, 2);
    _buttonNextEvent
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_nextEvent));
    _menuButtons.pack_start(_buttonNextEvent, Gtk::PACK_EXPAND_WIDGET, 2);

    _scaler.set_update_policy(Gtk::UPDATE_CONTINUOUS);
    _scaler.set_digits(3);
    _scaler.set_value_pos(Gtk::POS_LEFT);
    _scaler.set_draw_value();
    _scaler.set_size_request(200, 30);
    _scaler
    .signal_value_changed()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_scale_value_changed));
    _buttonPositionReset
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_positionReset));
    _buttonAxial
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_axial));
    _buttonStereo
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_stereo));
    _buttonWireName
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TWindowGTK::on_wireName));
    _scale.pack_start(_scaler, Gtk::PACK_SHRINK, 5);
    _scale.pack_start(_buttonPositionReset, Gtk::PACK_EXPAND_WIDGET, 2);
    _scale.pack_start(_buttonAxial, Gtk::PACK_SHRINK, 2);
    _scale.pack_start(_buttonStereo, Gtk::PACK_SHRINK, 2);
    _scale.pack_start(_buttonWireName, Gtk::PACK_SHRINK, 2);

    _buttonAxial.set_active();
    _axial = _buttonAxial.get_active();
    _buttonStereo.set_active(false);
    _stereo = _buttonStereo.get_active();
    _buttonWireName.set_active(false);
    _wireName = _buttonWireName.get_active();
  }

  TWindowGTK::~TWindowGTK()
  {
  }

  void
  TWindowGTK::on_scale_value_changed(void)
  {
  }

  void
  TWindowGTK::on_positionReset(void)
  {
  }

  void
  TWindowGTK::on_next(void)
  {
    Gtk::Main::quit();
  }

  void
  TWindowGTK::on_endOfEvent(void)
  {
    _endOfEventFlag = true;
    _skipEvent = true;
    Gtk::Main::quit();
  }

  void
  TWindowGTK::on_nextEvent(void)
  {
    _skipEvent = true;
    Gtk::Main::quit();
  }

// void
// TWindowGTK::initializeGTK(void) {
//     std::cout << "TWindowGTK ... initializing GTK"
//              << std::endl;
//     int argc = 0;
//     char ** argv = 0;
//     Gtk::Main main_instance(argc, argv);
//     GtkMain = main_instance.instance();
// }

  void
  TWindowGTK::run(void)
  {
    int argc = 0;
    char** argv = 0;
    Gtk::Main main_instance(argc, argv);
    show();
    if (((! _skip) && (! _skipEvent)) ||
        (_endOfEventFlag && _endOfEvent))
      Gtk::Main::run();
  }

  void
  TWindowGTK::pack(Gtk::DrawingArea& w)
  {
    _w = & w;

    _box0.pack_start(_menuButtons, Gtk::PACK_SHRINK, 5);
    _box0.pack_start(_label, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(w, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_scale, Gtk::PACK_SHRINK, 5);

    set_border_width(5);
    add(_box0);
    show_all();
  }

} // namespace Belle

#endif
