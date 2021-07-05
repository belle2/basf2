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
// Filename : DisplayRphi.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayRphi_FLAG_
#define TRGCDCDisplayRphi_FLAG_

#include <gtkmm.h>
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayDrawingAreaRphi.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDisplayRphi TRGCDCDisplayRphi
#endif

namespace Belle2_TRGCDC {
  extern Belle2::TRGCDCDisplayHough* H0;
  extern Belle2::TRGCDCDisplayHough* H1;
}

namespace Belle2 {

  class TRGCDCDisplay;
  class TRGCDCWireHit;

/// A class to display CDC trigger information.
  class TRGCDCDisplayRphi : public TRGCDCDisplay {

  public:
    /// Default constructor
    TRGCDCDisplayRphi(const std::string& name = "TRGCDCDisplayRphi",
                      double innerR = 160,
                      double outerR = 1137,
                      int size = 600);

    /// Destructor
    virtual ~TRGCDCDisplayRphi();

  public: // display control

    /// returns scaler value.
    double scale(void) const;

    /// sets and returns scaler value.
    double scale(double);

    /// returns present condition for axial wire display.
    bool axial(void) const;

    /// returns present condition for stereo wire display.
    bool stereo(void) const;

  private: // Actions
    virtual void on_scale_value_changed(void);
    virtual void on_positionReset(void) override;
    virtual void on_axial(void);
    virtual void on_stereo(void);
    virtual void on_wireName(void) override;
    virtual void on_BelleCDC(void);

  public: // Access to drawing area.

    /// returns drawing area.
    virtual TRGCDCDisplayDrawingAreaRphi& area(void);

  private: // GTK stuff
    bool _axial;
    bool _stereo;
    TRGCDCDisplayDrawingAreaRphi _w;
    Gtk::Adjustment _adjustment;
    Gtk::HScale _scaler;
    Gtk::CheckButton _buttonAxial;
    Gtk::CheckButton _buttonStereo;
    Gtk::CheckButton _buttonBelleCDC;
  };

//-----------------------------------------------------------------------------

  inline
  TRGCDCDisplayDrawingAreaRphi&
  TRGCDCDisplayRphi::area(void)
  {
    return _w;
  }

  inline
  bool
  TRGCDCDisplayRphi::axial(void) const
  {
    return _axial;
  }

  inline
  bool
  TRGCDCDisplayRphi::stereo(void) const
  {
    return _stereo;
  }

  inline
  double
  TRGCDCDisplayRphi::scale(void) const
  {
    return _scaler.get_value();
  }

  inline
  double
  TRGCDCDisplayRphi::scale(double a)
  {
    _scaler.set_value(a);
    return _scaler.get_value();
  }

} // namespace Belle2

#endif // TRGCDCDisplayRphi_FLAG_
#endif
