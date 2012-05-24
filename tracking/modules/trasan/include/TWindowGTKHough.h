//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTKHough.h
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking objects.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW_GTK

#ifndef TWINDOWGTKHOUGH_FLAG_
#define TWINDOWGTKHOUGH_FLAG_


#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/TWindowGTK.h"
#include "tracking/modules/trasan/TWindowHoughDrawingArea.h"

namespace Belle {

  class TRGCDCWireHit;
  class TLink;
  class TTrackBase;
  class TSegment;
  class TTrack;

/// A class to display tracking objects.
  class TWindowGTKHough : public TWindowGTK {

  public:
    /// Default constructor
    TWindowGTKHough(const std::string& name = "TWindowGTKHough",
                    int size = 600);

    /// Destructor
    virtual ~TWindowGTKHough();

  public: // Tracking stuff
    void clear(void);
    void append(const THoughPlane* const);

  public: // Actions
    void on_scale_value_changed(void);
    void on_positionReset(void);
    void on_axial(void);
    void on_stereo(void);
    void on_wireName(void);

  private: // GTK stuff
    TWindowHoughDrawingArea _w;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TWINDOWGTK_INLINE_DEFINE_HERE
#endif
#ifdef TWINDOWGTK_INLINE_DEFINE_HERE

  inline
  void
  TWindowGTKHough::clear(void)
  {
    _w.clear();
    TWindowGTK::clear();
  }

  inline
  void
  TWindowGTKHough::on_axial(void)
  {
    TWindowGTK::on_axial();
    _w.axial(axial());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  inline
  void
  TWindowGTKHough::on_stereo(void)
  {
    TWindowGTK::on_stereo();
    _w.stereo(stereo());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  inline
  void
  TWindowGTKHough::on_wireName(void)
  {
    TWindowGTK::on_wireName();
    _w.wireName(wireName());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  inline
  void
  TWindowGTKHough::append(const THoughPlane* const a)
  {
    _w.append(a);
  }

#endif
#undef inline

} // namespace Belle

#endif // TWINDOWGTKHOUGH_FLAG_
#endif
