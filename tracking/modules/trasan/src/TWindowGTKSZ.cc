//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTKSZ.cc
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
#include "tracking/modules/trasan/TWindowGTKSZ.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TTrack.h"


namespace Belle {

  TWindowGTKSZ::TWindowGTKSZ(const std::string& name,
                             int size)
    : TWindowGTK(name, 800, size),
      _w(size, 800)
  {
    _w.set_size_request(size, size);
    pack((Gtk::DrawingArea&) _w);
  }

  TWindowGTKSZ::~TWindowGTKSZ()
  {
  }

  void
  TWindowGTKSZ::on_scale_value_changed(void)
  {
    const double val = TWindowGTK::scale();
    _w.scale(val);
// std::cout << "scale value=" << val << std::endl;
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowGTKSZ::on_positionReset(void)
  {
    _w.resetPosition();
    const double val = _w.scale();
    TWindowGTK::scale(val);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowGTKSZ::append(const AList<TLine> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

  void
  TWindowGTKSZ::append(const TLine& line, Gdk::Color c)
  {
    _w.append(line, c);
  }

  void
  TWindowGTKSZ::append(const AList<TLink> & list, Gdk::Color c)
  {
    _w.append(list, c);
  }

} // namespace Belle

#endif
