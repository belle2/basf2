//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTKSZ.h
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

#ifndef TWINDOWGTKSZ_FLAG_
#define TWINDOWGTKSZ_FLAG_


#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/TWindowGTK.h"
#include "tracking/modules/trasan/TWindowSZDrawingArea.h"

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {

  class TRGCDCWireHit;
  class TLink;
  class TTrackBase;
  class TSegment;
  class TTrack;
  class TLine;

/// A class to display tracking objects.
  class TWindowGTKSZ : public TWindowGTK {

  public:
    /// Default constructor
    TWindowGTKSZ(const std::string& name = "TWindowGTKSZ",
                 int size = 600);

    /// Destructor
    virtual ~TWindowGTKSZ();

  public: // Tracking stuff
    void clear(void);
    void append(const AList<TLink> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TLine> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const TLine& t,
                Gdk::Color color = Gdk::Color("grey"));

  public: // Actions
    void on_scale_value_changed(void);
    void on_positionReset(void);
    void on_axial(void);
    void on_stereo(void);
    void on_wireName(void);

  private: // GTK stuff
    TWindowSZDrawingArea _w;
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
  TWindowGTKSZ::clear(void)
  {
    _w.clear();
    TWindowGTK::clear();
  }

  inline
  void
  TWindowGTKSZ::on_axial(void)
  {
    TWindowGTK::on_axial();
    _w.axial(axial());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  inline
  void
  TWindowGTKSZ::on_stereo(void)
  {
    TWindowGTK::on_stereo();
    _w.stereo(stereo());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

  inline
  void
  TWindowGTKSZ::on_wireName(void)
  {
    TWindowGTK::on_wireName();
    _w.wireName(wireName());
    _w.on_expose_event((GdkEventExpose*) NULL);
  }

#endif
#undef inline

} // namespace Belle

#endif // TWINDOWGTKSZ_FLAG_
#endif
