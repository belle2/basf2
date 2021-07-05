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
// Filename : DisplayDrawingAreaHough.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayDrawingAreaHough_FLAG_
#define TRGCDCDisplayDrawingAreaHough_FLAG_

#include "trg/cdc/DisplayDrawingArea.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDDAreaHough TRGCDCDisplayDrawingAreaHough
#endif

namespace Belle2 {

  class TRGCDCDisplay;
  class TRGCDCHoughPlane;

/// Actual class to display trigger objects
  class TRGCDCDisplayDrawingAreaHough : public TRGCDCDisplayDrawingArea {

  public:

    /// Default constructor
    TRGCDCDisplayDrawingAreaHough(TRGCDCDisplay&, int size);

    /// Destructor
    virtual ~TRGCDCDisplayDrawingAreaHough();

    double scale(double);
    double scale(void) const;
    void resetPosition(void) override;
    bool wireName(void) const;
    bool wireName(bool);
    bool oldCDC(void) const;
    bool oldCDC(bool);

    void clear(void) override;

    virtual bool on_expose_event(GdkEventExpose*) override;
    virtual bool on_button_press_event(GdkEventButton*) override;

    void append(const TRGCDCHoughPlane* const);

  private:
    virtual void on_realize() override;
    void draw(void);

    /// Coordinate transformations.
    int xT(double x) const;
    int yT(double y) const;
    int xR(double x) const;
    int yR(double y) const;

    /// Coordinate transformations (obsolete).
    int toY(int y) const;

    /// Draws a Hough cell.
    int drawCell(unsigned xCell, unsigned yCell);

    /// Draws a region.
    int drawRegion(const std::vector<unsigned>& region);

  private:
    double _scale;
    double _scaleX;
    double _scaleY;
    bool _wireName;
    bool _oldCDC;
    double _x, _y;
    const TRGCDCHoughPlane* _hp;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _gray, _yellow;
    Gdk::Color _gray0, _gray1, _gray2, _gray3;
  };

//-----------------------------------------------------------------------------

  inline
  double
  TRGCDCDisplayDrawingAreaHough::scale(double a)
  {
    return _scale = a;
  }

  inline
  double
  TRGCDCDisplayDrawingAreaHough::scale(void) const
  {
    return _scale;
  }

  inline
  bool
  TRGCDCDisplayDrawingAreaHough::wireName(void) const
  {
    return _wireName;
  }

  inline
  bool
  TRGCDCDisplayDrawingAreaHough::wireName(bool a)
  {
    return _wireName = a;
  }

  inline
  int
  TRGCDCDisplayDrawingAreaHough::toY(int a) const
  {
    return - a + _winh;
  }

  inline
  void
  TRGCDCDisplayDrawingAreaHough::clear(void)
  {
    _hp = 0;
  }

  inline
  void
  TRGCDCDisplayDrawingAreaHough::append(const TRGCDCHoughPlane* const hp)
  {
    _hp = hp;
    on_expose_event((GdkEventExpose*) NULL);
  }

} // namespace Belle2

#endif // TRGCDCDisplayDrawingAreaHough_FLAG_
#endif
