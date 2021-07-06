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

#include <iostream>
#include "trg/cdc/DisplayRphi.h"

using namespace std;

namespace Belle2 {

// Gtk::Main * GtkMain = 0;
// bool TRGCDCDisplayRphi::_skipEvent = false;
// bool TRGCDCDisplayRphi::_endOfEvent = false;
// bool TRGCDCDisplayRphi::_endOfEventFlag = false;

  TRGCDCDisplayRphi::TRGCDCDisplayRphi(const string& name,
                                       double innerR,
                                       double outerR,
                                       int size)
    : TRGCDCDisplay(name, size, int(outerR)),
      _axial(true),
      _stereo(false),
      _w(* this, size, innerR, outerR),
      _adjustment(double(size) / outerR / 2,
                  double(size) / outerR / 2,
                  10.0,
                  0.1),
      _scaler(_adjustment),
      _buttonAxial("Axial"),
      _buttonStereo("Stereo"),
      _buttonBelleCDC("Belle CDC")
  {

    _w.set_size_request(size, size);

    _buttonAxial.set_active();
    _axial = _buttonAxial.get_active();
    _buttonStereo.set_active(false);
    _stereo = _buttonStereo.get_active();
    _buttonBelleCDC.set_active(false);

    _buttonAxial
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplayRphi::on_axial));
    _buttonStereo
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplayRphi::on_stereo));
    _buttonBelleCDC
    .signal_clicked()
    .connect(sigc::mem_fun(* this, & TRGCDCDisplayRphi::on_BelleCDC));

    _scaler.set_update_policy(Gtk::UPDATE_CONTINUOUS);
    _scaler.set_digits(3);
    _scaler.set_value_pos(Gtk::POS_LEFT);
    _scaler.set_draw_value();
    _scaler.set_size_request(200, 30);
    _scaler
    .signal_value_changed()
    .connect(sigc::mem_fun(* this,
                           & TRGCDCDisplayRphi::on_scale_value_changed));

    _bottom.pack_start(_scaler, Gtk::PACK_SHRINK, 5);
    _bottom.pack_start(_buttonPositionReset, Gtk::PACK_EXPAND_WIDGET, 2);
    _bottom.pack_start(_buttonAxial, Gtk::PACK_SHRINK, 2);
    _bottom.pack_start(_buttonStereo, Gtk::PACK_SHRINK, 2);
    _bottom.pack_start(_buttonWireName, Gtk::PACK_SHRINK, 2);
    _bottom.pack_start(_buttonBelleCDC, Gtk::PACK_SHRINK, 2);

    _box0.pack_start(_menuButtons, Gtk::PACK_SHRINK, 5);
    _box0.pack_start(_label, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_w, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_bottom, Gtk::PACK_SHRINK, 5);

    set_border_width(5);
    add(_box0);
    show_all();
  }

  TRGCDCDisplayRphi::~TRGCDCDisplayRphi()
  {
  }

  void
  TRGCDCDisplayRphi::on_scale_value_changed(void)
  {
    const double val = scale();
    _w.scale(val);
//  cout << "TRGCDCDisplayRphi ... scale value=" << val << endl;
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayRphi::on_axial(void)
  {
    _axial = _buttonAxial.get_active();
    _w.axial(_axial);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayRphi::on_stereo(void)
  {
    _stereo = _buttonStereo.get_active();
    _w.stereo(_stereo);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayRphi::on_wireName(void)
  {
    _wireName = _buttonWireName.get_active();
    _w.wireName(_wireName);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayRphi::on_BelleCDC(void)
  {
    _oldCDC = _buttonBelleCDC.get_active();
    _w.oldCDC(_oldCDC);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayRphi::on_positionReset(void)
  {
    _w.resetPosition();
    const double val = _w.scale();
    scale(val);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

} // namespace Belle2

#endif
