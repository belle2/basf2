//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTKConformal.cc
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
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TWindowGTKConformal.h"
#include "tracking/modules/trasan/Trasan.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

  TWindowGTKConformal::TWindowGTKConformal(const std::string& name,
                                           double innerR,
                                           double outerR,
                                           int size)
    : TWindowGTK(name, outerR, size),
      _w(size, innerR, outerR)
  {
    _w.set_size_request(size, size);
    pack((Gtk::DrawingArea&) _w);
  }

  TWindowGTKConformal::~TWindowGTKConformal()
  {
  }

  void
  TWindowGTKConformal::on_scale_value_changed(void)
  {
    const double val = TWindowGTK::scale();
    _w.scale(val);
// std::cout << "scale value=" << val << std::endl;
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowGTKConformal::on_positionReset(void)
  {
    _w.resetPosition();
    const double val = _w.scale();
    TWindowGTK::scale(val);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowGTKConformal::append(const CAList<Belle2::TRGCDCWireHit> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

  void
  TWindowGTKConformal::append(const AList<TLink> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

  void
  TWindowGTKConformal::append(const AList<TSegment> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

  void
  TWindowGTKConformal::append(const AList<TTrack> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

  void
  TWindowGTKConformal::append(const AList<TCircle> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

} // namespace Belle

#endif
