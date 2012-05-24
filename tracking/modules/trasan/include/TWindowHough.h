//-----------------------------------------------------------------------------
// $Id: TWindowHough.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TWindowHough.h
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object in Hough finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.3  2005/11/03 23:20:36  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.2  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/02/18 04:07:47  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW


#ifndef TWINDOWHOUGH_FLAG_
#define TWINDOWHOUGH_FLAG_

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/TWindow.h"
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  class THoughPlane;

/// A class to display tracking object in Hough Finder.
  class TWindowHough : public TWindow {

  public:
    /// Default constructor
    TWindowHough(const std::string& name, int xSize = 600, int ySize = 600);

    /// Destructor
    virtual ~TWindowHough();

  public:
    /// draws Hough plane.
    void draw(const THoughPlane&);

    /// draws peaks.
    void draw(const THoughPlane&,
              const AList<TPoint2D> & list,
              float radius,
              leda_color = leda_green);

    /// appends a circle.
    void drawOver(const THoughPlane&,
                  const AList<TPoint2D> & list,
                  float radius,
                  leda_color = leda_green);

    /// waits.
    void wait(void);
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TWINDOW_INLINE_DEFINE_HERE
#endif

#ifdef TWINDOW_INLINE_DEFINE_HERE




#endif

#undef inline

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif TWINDOWHOUGH_FLAG_

#endif /* TRASAN_WINDOW */
