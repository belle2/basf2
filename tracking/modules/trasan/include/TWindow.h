//-----------------------------------------------------------------------------
// $Id: TWindow.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TWindow.h
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.21  2005/11/03 23:20:36  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.20  2005/04/18 23:42:05  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.19  2004/02/18 04:07:47  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.18  2003/12/19 07:36:15  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.17  2001/12/23 09:58:58  katayama
// removed Strings.h
//
// Revision 1.16  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.15  2001/12/14 02:54:52  katayama
// For gcc-3.0
//
// Revision 1.14  2001/04/11 01:10:05  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.13  2000/03/01 05:19:27  yiwasaki
// forgot to commit headers...
//
// Revision 1.12  2000/02/15 13:46:53  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.11  2000/02/03 06:18:07  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.10  2000/02/01 11:24:47  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.9  2000/01/28 06:30:33  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.8  2000/01/23 08:23:11  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.7  2000/01/19 01:33:35  yiwasaki
// Trasan 1.67f : new conf modified
//
// Revision 1.6  2000/01/18 07:00:32  yiwasaki
// Trasan 1.67e : TWindow modified
//
// Revision 1.5  1999/11/19 09:13:17  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.4  1999/10/30 10:12:56  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1999/10/21 15:45:21  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.1  1999/09/21 02:01:38  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW

#ifndef TWINDOW_FLAG_
#define TWINDOW_FLAG_

#include <string>


#ifdef HAVE_LEDA
#include <LEDA/window.h>
#include <LEDA/stack.h>
#include <LEDA/color.h>
#endif
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/CList.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "tracking/modules/trasan/TPoint2D.h"

#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  typedef HepGeom::Vector3D<double>  Vector3D;
  class TRGCDCWireHit;
  class TTrackBase;
  class TTrack;
  class TLine;
  class TWindow;
  class TSegment;
  class TSegmentCurl;
  class TCircle;
  class TLink;

  extern const TPoint2D AxesOrigin;

/// A class to display tracking object.
  class TWindow {

  public:
    /// Default constructor
    TWindow(const TWindow&);

    /// Constructor
    TWindow(const std::string& name, int xSize = 600, int ySize = 600);

    /// Destructor
    virtual ~TWindow();

  public:
    /// opens window.
    void open(void);

    /// clear window contents.
    void clear(void);

    /// appends/remove objects.
    void append(const AList<TRGCDCWireHit> &, leda_color = leda_black);
    void append(const TTrackBase&,
                leda_color = leda_black,
                bool toBeSelfObject = false);
    void append(const AList<TSegment> &,
                leda_color = leda_black,
                bool toBeSelfObject = false);
    void append(const AList<TSegmentCurl> &, leda_color = leda_black);
    void append(const AList<TTrack> &,
                leda_color = leda_black,
                bool toBeSelfObject = false);
    void append(const TPoint2D&);
    void remove(const TTrackBase&);
    void remove(const AList<TSegment> &);
    void remove(const AList<TTrack> &);
    void appendSz(const TTrack&, const AList<TLink> &, leda_color = leda_black);
    void appendSz(const TTrack&,
                  const AList<TSegment> &,
                  leda_color = leda_black);

    /// appends TLink's. You can't remove TLink's but you can clear.
    void append(const AList<TLink> &, leda_color = leda_black);

    /// displays objects w/o appending.
    void oneShot(const AList<TLink> &, leda_color = leda_black);
    void oneShot(const TTrackBase&, leda_color = leda_black);
    void oneShot(const AList<TSegment> &, leda_color = leda_black);
    void oneShot(const AList<TSegmentCurl> &, leda_color = leda_black);

    /// waits for user actions.
    virtual void wait(void);

    /// draws objects in stack.
    virtual void draw(void);

    /// draws a point.
    void draw(const TPoint2D&, leda_color = leda_black);

    /// draw axes.
    void drawAxes(const TPoint2D& o = AxesOrigin);

    /// returns/draws text.
    std::string text(void) const;
    void text(const std::string& text);

    /// sets run, farm, and event number of target event to display.
    void target(unsigned run, unsigned farm, unsigned event);

    /// sets drawing mode. (0:rphi, 1:conformal, 2:s-z);
    unsigned mode(unsigned);
    bool stereo(bool);

    /// sets skip mode.
    bool skip(bool);
    bool skipAllWindow(bool);

  private:
    void drawHeader(void);
    void drawBase(const TTrackBase&, leda_color = leda_black);
    void drawSegment(const TSegment&, leda_color = leda_black);
    void drawTrack(const TTrack&, leda_color = leda_black);
    void drawLine(const TLine&, leda_color = leda_black);
    void drawCircle(const TCircle&, leda_color = leda_black);
    void drawCdc(void);
    bool target(void) const;
    std::string wireName(const TLink&) const;

  protected:
    leda_window _window;

  protected:
    std::string _name;
    double _xmin, _xmax, _ymin;
    leda_stack<Vector3D> _zoomHistory[4];
    int _closeButton;
    int _wireNameButton;
    int _axialButton;
    int _stereoButton;
    int _coordinateButton;
    int _zoomInButton;
    int _zoomOutButton;
    int _skipButton;
    int _skipAllButton;
    int _szButton;
    int _confButton;
    int _segmentLinkButton;
    int _segmentPropertyButton;
    int _copyButton;

    bool _wireName;
    bool _axial;
    bool _stereo;
    bool _coordinate;
    bool _display;
    bool _skip;
    bool _segmentLink;
    bool _segmentProperty;
    static bool _skipAllWindow;
    unsigned _nYLine;

    bool _canDelete;
    CAList<TTrackBase> _objects;
    AList<TTrackBase> _selfObjects;
    AList<TLink> _selfTLinks;
    AList<leda_color> _colors;
    AList<TPoint2D> _selfAxes;
    std::string _text;
    double _xPositionText;
    double _yPositionText;
    double _yPositionStep;

    unsigned _mode;
    unsigned _targetRun;
    unsigned _targetEvent;
    unsigned _nEvents;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TWINDOW_INLINE_DEFINE_HERE
#endif

#ifdef TWINDOW_INLINE_DEFINE_HERE

  inline
  void
  TWindow::append(const TTrackBase& a, leda_color b, bool self)
  {
    _objects.append(a);
    _colors.append(new leda_color(b));
    if (self)
      _selfObjects.append((TTrackBase&) a);
  }

  inline
  void
  TWindow::append(const AList<TSegment> & a, leda_color b, bool self)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++) {
      append(* (TTrackBase*) a[i], b);
      if (self)
        _selfObjects.append(* (TTrackBase*) a[i]);
    }
  }

  inline
  void
  TWindow::append(const AList<TTrack> & a, leda_color b, bool self)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++) {
      append(* (TTrackBase*) a[i], b);
      if (self)
        _selfObjects.append(* (TTrackBase*) a[i]);
    }
  }

  inline
  void
  TWindow::remove(const TTrackBase& a)
  {
    int i;
    while ((i = _objects.fIndex(a)) != -1) {
      _objects.remove(i);
      leda_color* b = _colors[i];
      _colors.remove(i);
      delete b;
    }
  }

  inline
  void
  TWindow::remove(const AList<TSegment> & a)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++) {
      remove(* (TTrackBase*) a[i]);
    }
  }

  inline
  void
  TWindow::remove(const AList<TTrack> & a)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++) {
      remove(* (TTrackBase*) a[i]);
    }
  }

  inline
  void
  TWindow::open(void)
  {
    if (! target()) return;
    if (! _display) {
      if (_mode == 1) {
        _xmin = -1;
        _xmax = 7;
        _ymin = -1;
        _window.init(_xmin, _xmax, _ymin);
      }
      _window.display();
    }
    _display = true;
  }

  inline
  unsigned
  TWindow::mode(unsigned a)
  {
    if (a == 2) {
      _stereo = true;
      _axial = false;
    }
    return _mode = a;
  }

  inline
  bool
  TWindow::skip(bool a)
  {
    return _skip = a;
  }

  inline
  bool
  TWindow::skipAllWindow(bool a)
  {
    return _skipAllWindow = a;
  }

  inline
  void
  TWindow::text(const std::string& text)
  {
    _text = text;
  }

  inline
  std::string
  TWindow::text(void) const
  {
    return _text;
  }

  inline
  bool
  TWindow::stereo(bool a)
  {
    return _stereo = a;
  }

  inline
  void
  TWindow::append(const TPoint2D& a)
  {
    _selfAxes.append(new TPoint2D(a));
  }

#endif

#undef inline


#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif TWINDOW_FLAG_
#endif /* TRASAN_WINDOW */
