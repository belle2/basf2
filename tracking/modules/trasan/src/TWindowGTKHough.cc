//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTKHough.cc
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
#include "tracking/modules/trasan/TWindowGTKHough.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TTrack.h"


namespace Belle {

  TWindowGTKHough::TWindowGTKHough(const std::string& name,
                                   int size)
    : TWindowGTK(name, size, size),
      _w(size)
  {
    _w.set_size_request(size, size);
    pack((Gtk::DrawingArea&) _w);
  }

  TWindowGTKHough::~TWindowGTKHough()
  {
  }

  void
  TWindowGTKHough::on_scale_value_changed(void)
  {
    const double val = TWindowGTK::scale();
    _w.scale(val);
// std::cout << "scale value=" << val << std::endl;
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowGTKHough::on_positionReset(void)
  {
    _w.resetPosition();
    const double val = _w.scale();
    TWindowGTK::scale(val);
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

} // namespace Belle

#endif
