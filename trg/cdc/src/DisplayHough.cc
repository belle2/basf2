/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayHough.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#include <iostream>
#include "trg/cdc/DisplayHough.h"

using namespace std;

namespace Belle2 {

  TRGCDCDisplayHough::TRGCDCDisplayHough(const std::string& name,
                                         int size)
    : TRGCDCDisplay(name, size, 10),
      _w(* this, size),
      _adjustment(1., 1., 50.0, 0.1),
      _scaler(_adjustment)
  {

    _w.set_size_request(size, size);

    _scaler.set_update_policy(Gtk::UPDATE_CONTINUOUS);
    _scaler.set_digits(3);
    _scaler.set_value_pos(Gtk::POS_LEFT);
    _scaler.set_draw_value();
    _scaler.set_size_request(200, 30);
    _scaler
    .signal_value_changed()
    .connect(sigc::mem_fun(* this,
                           & TRGCDCDisplayHough::on_scale_value_changed));

    _bottom.pack_start(_scaler, Gtk::PACK_SHRINK, 5);
    _bottom.pack_start(_buttonPositionReset, Gtk::PACK_EXPAND_WIDGET, 2);
    _bottom.pack_start(_buttonWireName, Gtk::PACK_SHRINK, 2);

    _box0.pack_start(_menuButtons, Gtk::PACK_SHRINK, 5);
    _box0.pack_start(_label, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_w, Gtk::PACK_EXPAND_WIDGET, 5);
    _box0.pack_start(_bottom, Gtk::PACK_SHRINK, 5);

    set_border_width(5);
    add(_box0);
    show_all();
  }

  TRGCDCDisplayHough::~TRGCDCDisplayHough()
  {
  }

  void
  TRGCDCDisplayHough::on_scale_value_changed(void)
  {
    const double val = scale();
    _w.scale(val);
    cout << "TRGCDCDisplayHough ... scale value=" << val << endl;
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TRGCDCDisplayHough::on_positionReset(void)
  {
    area().resetPosition();
    const double val = _w.scale();
    scale(val);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

} // namespace Belle2

#endif
