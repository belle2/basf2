//-----------------------------------------------------------------------------
// $Id: TWindow.cc 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TWindow.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.29  2005/11/03 23:20:13  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.28  2005/04/18 23:41:47  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.27  2005/03/11 03:57:53  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.26  2004/04/15 05:34:10  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.25  2004/02/18 04:07:27  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.24  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.23  2002/02/13 20:22:12  yiwasaki
// Trasan 3.02 : bug fixes in debug mode
//
// Revision 1.22  2001/12/23 09:58:50  katayama
// removed Strings.h
//
// Revision 1.21  2001/12/19 02:59:49  katayama
// Uss find,istring
//
// Revision 1.20  2001/04/11 01:09:12  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.19  2001/01/17 06:27:57  yiwasaki
// hists in sakura modified
//
// Revision 1.18  2000/04/13 02:53:42  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.17  2000/04/04 07:40:08  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.16  2000/03/01 04:51:11  yiwasaki
// Trasan 2.00RC14 : stereo bug fix, curl updates
//
// Revision 1.15  2000/02/25 08:09:57  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.14  2000/02/15 13:46:45  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.13  2000/02/10 13:11:39  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.12  2000/02/03 06:18:04  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.11  2000/02/01 11:24:43  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.10  2000/01/30 08:17:09  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.9  2000/01/28 06:30:26  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.8  2000/01/23 08:23:06  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.7  2000/01/19 01:33:32  yiwasaki
// Trasan 1.67f : new conf modified
//
// Revision 1.6  2000/01/18 07:00:30  yiwasaki
// Trasan 1.67e : TWindow modified
//
// Revision 1.5  1999/11/19 09:13:11  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.4  1999/10/30 10:12:25  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1999/10/21 15:45:16  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.1  1999/09/21 02:01:34  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW

#include <time.h>


#include "tracking/modules/trasan/Strings.h"
#include "trg/cdc/TRGCDC.h"
#include "tracking/modules/trasan/TWindow.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TSegmentCurl.h"
#include "tracking/modules/trasan/TCircle.h"
#include "trg/cdc/WireHitMC.h"
#include "tracking/modules/trasan/TTrackHEP.h"
#include "tracking/modules/trasan/TConformalFinder0.h"


extern struct {
  int unpack_truncate;
  int unpackAll;
} calcdc_unpack_;

#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  bool
  TWindow::_skipAllWindow = false;

  const TPoint2D AxesOrigin(0, 0);

  TWindow::TWindow(const TWindow& a)
    : _window(600, 600, _name.c_str()),
      _name("copy of " + a._name),
      _xmin(a._xmin),
      _xmax(a._xmax),
      _ymin(a._ymin),

      _wireName(a._wireName),
      _axial(a._axial),
      _stereo(a._stereo),
      _coordinate(a._coordinate),
      _display(false),
      _skip(false),
      _segmentLink(a._segmentLink),
      _segmentProperty(a._segmentProperty),
      _nYLine(a._nYLine),

      _canDelete(false),
      _objects(a._objects),
      _colors(a._colors),
      _text(a._text),
      _xPositionText(a._xPositionText),
      _yPositionText(a._yPositionText),
      _yPositionStep(a._yPositionStep),

      _mode(a._mode),
      _targetRun(a._targetRun),
      _targetEvent(a._targetEvent),
      _nEvents(a._nEvents)
  {
    for (unsigned i = 0; i < 4; i++) _zoomHistory[i] = a._zoomHistory[i];
    _window.set_show_coordinates(_coordinate);
    _window.buttons_per_line(7);
    _closeButton = _window.button("next");
    _wireNameButton = _window.button("wire #");
    _axialButton = _window.button("axial");
    _stereoButton = _window.button("stereo");
    _coordinateButton = _window.button("coordinate");
    _zoomInButton = _window.button("zoom in");
    _zoomOutButton = _window.button("zoom out");
    _skipButton = _window.button("skip");
    _skipAllButton = _window.button("skip all");
    _szButton = _window.button("rphi/sz");
    _confButton = _window.button("conformal");
    _segmentLinkButton = _window.button("sgmt link");
    _segmentPropertyButton = _window.button("sgmt vec");
    _copyButton = _window.button("window cp");
    _window.init(_xmin, _xmax, _ymin);
  }

  TWindow::TWindow(const std::string& name, int x, int y)
    : _window(x, y, name.c_str()),
      _name(name),
      _xmin(-100),
      _xmax(100),
      _ymin(-100),

      _wireName(false),
      _axial(true),
      _stereo(false),
      _coordinate(false),
      _display(false),
      _skip(false),
      _segmentLink(false),
      _segmentProperty(false),
      _nYLine(0),

      _canDelete(true),
      _text(""),
      _xPositionText(0),
      _yPositionText(0),
      _yPositionStep(0),
      _mode(0),
      _targetRun(0),
      _targetEvent(0),
      _nEvents(0)
  {
    _window.set_show_coordinates(_coordinate);
    _window.buttons_per_line(7);
    _closeButton = _window.button("next");
    _wireNameButton = _window.button("wire #");
    _axialButton = _window.button("axial");
    _stereoButton = _window.button("stereo");
    _coordinateButton = _window.button("coordinate");
    _zoomInButton = _window.button("zoom in");
    _zoomOutButton = _window.button("zoom out");
    _skipButton = _window.button("skip");
    _skipAllButton = _window.button("skip all");
    _szButton = _window.button("rphi/sz");
    _confButton = _window.button("conformal");
    _segmentLinkButton = _window.button("sgmt link");
    _segmentPropertyButton = _window.button("sgmt vec");
    _copyButton = _window.button("window cp");
    _window.init(_xmin, _xmax, _ymin);
    _zoomHistory[0].push(Vector3D(_xmin, _xmax, _ymin));
    _zoomHistory[1].push(Vector3D(0., 7., 0.));
    _zoomHistory[2].push(Vector3D(_xmin, _xmax, _ymin));
    _zoomHistory[3].push(Vector3D(-0.28, 0.28, -0.28));
  }

  TWindow::~TWindow()
  {
    clear();
  }

  void
  TWindow::wait(void)
  {
    draw();
    if (_skip) return;
    if (_skipAllWindow) return;
    if (! target()) return;

    bool loop = true;
    bool zoom = false;

    while (loop) {

      //...Read input...
      double x0, y0;
      int b = _window.read_mouse(x0, y0);

      //...Close...
      if (b == _closeButton) loop = false;
      else if (b == _szButton) {
        if (_mode != 0) _mode = 0;
        else if (_mode != 2) _mode = 2;
        draw();
      } else if (b == _confButton) {
        if (_mode != 3) _mode = 3;
        else if (_mode != 1) _mode = 1;
        draw();
      } else if (b == _segmentLinkButton) {
        _segmentLink = ! _segmentLink;
        draw();
      } else if (b == _segmentPropertyButton) {
        _segmentProperty = ! _segmentProperty;
        draw();
      } else if (b == _skipButton) {
        loop = false;
        _skip = true;
      } else if (b == _skipAllButton) {
        loop = false;
        _skipAllWindow = true;
      } else if (b == _coordinateButton) {
        _coordinate = ! _coordinate;
        _window.set_show_coordinates(_coordinate);
      } else if (b == _wireNameButton) {
        _wireName = ! _wireName;
        draw();
      } else if (b == _axialButton) {
        _axial = ! _axial;
        draw();
      } else if (b == _stereoButton) {
        _stereo = ! _stereo;
        draw();
      } else if (b == _copyButton) {
        TWindow tmp = * this;
        tmp.wait();
      } else if (b == _zoomInButton) {
        zoom = true;
      } else if (b == _zoomOutButton) {
        if (zoom) continue;
        if (_zoomHistory[_mode].size() > 1) {
          _zoomHistory[_mode].pop();
        } else {
          _zoomHistory[_mode].pop();
          _xmin *= 2.;
          _xmax *= 2.;
          _ymin *= 2.;
          _zoomHistory[_mode].push(Vector3D(_xmin, _xmax, _ymin));
        }
        zoom = false;
        draw();
      } else if (b == MOUSE_BUTTON(1)) {
        if (zoom) {
          double x, y;
          _window.read_mouse_rect(x0, y0, x, y);
          double xx(x0 < x ? x0 : x);
          double xxx(x0 < x ? x : x0);
          double yy(y0 < y ? y0 : y);
          _zoomHistory[_mode].push(Vector3D(xx, xxx, yy));
          draw();
          zoom = false;
        }
      }
    }
  }

  void
  TWindow::draw(void)
  {
    if (! target()) return;
    if (! _display) open();

    _window.clear();
    _nYLine = 0;
    Vector3D z(_zoomHistory[_mode].top());
    _window.init(z.x(), z.y(), z.z());

    double xmin = _window.xmin();
    double xmax = _window.xmax();
    double ymin = _window.ymin();
    double ymax = _window.ymax();
    double x = xmin + (xmax - xmin) * .05;
    double y = ymin + (ymax - ymin) * .03;
    _xPositionText = x;
    _yPositionText = y;
    _yPositionStep = (ymax - ymin) * .03;

    drawCdc();
    drawAxes();
    drawHeader();

    unsigned n = _objects.length();
    for (unsigned i = 0; i < n; i++) {
      const TTrackBase& track = * _objects[i];
      if (track.objectType() == Line)
        drawLine((const TLine&) track, * _colors[i]);
      else if (track.objectType() == TrackBase)
        drawBase(track, * _colors[i]);
      else if (track.objectType() == Track)
        drawTrack((const TTrack&) track, * _colors[i]);
      else if (track.objectType() == Segment)
        drawSegment((const TSegment&) track, * _colors[i]);
      else if (track.objectType() == Circle)
        drawCircle((const TCircle&) track, * _colors[i]);
      else
        std::cout << "TWindow::draw !!! can't display" << std::endl;
    }

    //...Text...
    _window.draw_text(_xPositionText, _yPositionText, _text.c_str());
  }

  void
  TWindow::drawBase(const TTrackBase& base, leda_color c)
  {
    const AList<TLink> & links = base.links();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (links[i]->wire() == NULL) continue;
      if (! _stereo)
        if (links[i]->wire()->stereo())
          continue;
      if (! _axial)
        if (links[i]->wire()->axial())
          continue;

      //...s-z mode...
      if (_mode == 2) {

        //...Points...
        HepGeom::Point3D<double> x = links[i]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name() + ":";

          if (links[i]->hit()->mc())
            n += "("
                 + itostring(links[i]->hit()->mc()->hep()->id()) + ")";

          n += itostring(links[i]->leftRight());
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
      }

      //...Rphi mode...
      else if (_mode == 0) {

        //...Points...
        HepGeom::Point3D<double> x = links[i]->wire()->forwardPosition();
        double radius = links[i]->hit()->drift();
        _window.draw_circle(x.x(), x.y(), radius, c);
        if (_wireName)
          _window.draw_text(x.x(),
                            x.y(),
                            wireName(* links[i]).c_str());
      }

      //...Conformal mode...
      else if (_mode == 1) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[i]->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformationRphi(ORIGIN, list, list2);

        //...Points...
        HepGeom::Point3D<double> x = list2[0]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
      }

      //...Conformal mode...
      else if (_mode == 3) {

        //...Points...
        HepGeom::Point3D<double> x = links[i]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
      }
    }
  }

  void
  TWindow::drawSegment(const TSegment& base, leda_color c)
  {
    AList<TLink> links = base.links();
    links.sort(SortByWireId);
    unsigned n = links.length();
    HepGeom::Point3D<double> x;
    HepGeom::Point3D<double> y;
    for (unsigned i = 0; i < n; i++) {
      if (! _stereo)
        if (links[i]->wire()->stereo())
          return;
      if (! _axial)
        if (links[i]->wire()->axial())
          return;

      //...s-z mode...
      if (_mode == 2) {
        x = links[i]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name() + ":"
                          + itostring(links[i]->leftRight());
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
        if (i)
          _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
        y = x;
      }

      //...Rphi mode...
      else if (_mode == 0) {
        x = links[i]->wire()->forwardPosition();
        double radius = links[i]->hit()->drift();
        _window.draw_circle(x.x(), x.y(), radius, c);
        if (_wireName)
          _window.draw_text(x.x(),
                            x.y(),
                            (const char*) wireName(* links[i]).c_str());
        if (i)
          _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
        y = x;
      }

      //...Conformal mode...
      else if (_mode == 1) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[i]->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformationRphi(ORIGIN, list, list2);

        //...Points...
        HepGeom::Point3D<double> x = list2[0]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
        if (i)
          _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
        y = x;
        delete list2[0];
      }

      //...Conformal mode...
      else if (_mode == 3) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[i]->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformation(ORIGIN, list, list2);

        //...Points...
        HepGeom::Point3D<double> x = list2[0]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
        if (i)
          _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
        y = x;
        delete list2[0];
      }
    }

    if (_segmentProperty) {
      if (_mode == 3) {
        float in = links.first()->position().mag();
        float out = links.last()->position().mag();
        float length = out - in;
        HepGeom::Point3D<double> x = base.position() - 0.5 * length * base.direction();
        HepGeom::Point3D<double> y = base.position() + 0.5 * length * base.direction();
        _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
        _window.draw_point(base.position().x(), base.position().y(), c);
      }
    }

    //...Links to other segments...
    if (! _segmentLink) return;
    const AList<TSegment> & innerLinks = base.innerLinks();
    unsigned nLinks = innerLinks.length();
    for (unsigned i = 0; i < nLinks; i++) {
      if (i == 0) c = leda_blue;
      else        c = leda_pink;

      AList<TLink> innerTLinks = innerLinks[i]->links();
      innerTLinks.sort(SortByWireId);

      //...s-z mode...
      if (_mode == 2) {
      }

      //...Rphi mode...
      else if (_mode == 0) {
        x = links[0]->wire()->forwardPosition();
        y = innerTLinks.last()->wire()->forwardPosition();
        _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
//        if (base.state() & TSegmentCrowd)
//      _window.draw_text(x.x(), x.y(), "c");
      }

      //...Conformal mode...
      else if (_mode == 1) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[0]->hit());
        list.append((TRGCDCWireHit*) innerTLinks.last()->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformation(ORIGIN, list, list2);


        x = list2[0]->position();
        y = list2[1]->position();
        _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
//        if (base.state() & TSegmentCrowd)
//      _window.draw_text(x.x(), x.y(), "c");
      }

      //...Conformal mode...
      else if (_mode == 3) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[0]->hit());
        list.append((TRGCDCWireHit*) innerTLinks.last()->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformation(ORIGIN, list, list2);


        x = list2[0]->position();
        y = list2[1]->position();
        _window.draw_segment(y.x(), y.y(), x.x(), x.y(), c);
//        if (base.state() & TSegmentCrowd)
//      _window.draw_text(x.x(), x.y(), "c");
      }
    }
  }

  void
  TWindow::drawLine(const TLine& line, leda_color c)
  {
    drawAxes();
    drawBase((const TTrackBase&) line, c);

    //...Line...
    double xmin = _window.xmin();
    double xmax = _window.xmax();
    double ymin = xmin * line.a() + line.b();
    double ymax = xmax * line.a() + line.b();
    _window.draw_segment(xmin, ymin, xmax, ymax, c);
  }

  bool
  TWindow::target(void) const
  {
    struct belle_event* ev =
      (struct belle_event*) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
    if (_targetRun == 0 && _targetEvent == 0)
      return true;
    if ((unsigned) ev->m_RunNo == _targetRun &&
        (unsigned) ev->m_EvtNo == _targetEvent)
      return true;
    return false;
  }

  void
  TWindow::target(unsigned run, unsigned farm, unsigned event)
  {
    _targetRun = run;
    _targetEvent = (farm << 28) + event;
  }

  void
  TWindow::drawTrack(const TTrack& t, leda_color c)
  {
    if (! _coordinate) {
      std::string p = t.name() + TrackKinematics(t.helix()) + " " +
                      LayerUsage(t.links());
      _window.draw_text(_xPositionText, _yPositionText, p.c_str(), c);
      _yPositionText += _yPositionStep;
    }

    const AList<TLink> & links = t.links();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (! _stereo)
        if (links[i]->wire()->stereo())
          continue;
      if (! _axial)
        if (links[i]->wire()->axial())
          continue;

      if (_mode == 0) {
        HepGeom::Point3D<double> x = links[i]->wire()->forwardPosition();
        double radius = links[i]->hit()->drift();
        _window.draw_circle(x.x(), x.y(), radius, c);
        if (_wireName)
          _window.draw_text(x.x(),
                            x.y(),
                            (const char*) wireName(* links[i]).c_str());
      }

      else if (_mode == 2) {
        HepGeom::Point3D<double> x = links[i]->positionOnTrack();
        HepGeom::Point3D<double> sz;
        t.szPosition(x, sz);
        _window.draw_point(sz.x(), sz.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name() + ":"
                          + itostring(links[i]->leftRight());
          _window.draw_text(sz.x(), sz.y(), n.c_str());
        }
        x = links[i]->positionOnWire();
        t.szPosition(x, sz);
        _window.draw_point(sz.x(), sz.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name() + ":"
                          + itostring(links[i]->leftRight());
          _window.draw_text(sz.x(), sz.y(), n.c_str());
        }
      }

      //...Conformal mode...
      else if (_mode == 1) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[i]->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformationRphi(ORIGIN, list, list2);

        //...Points...
        HepGeom::Point3D<double> x = list2[0]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
        delete list2[0];
      }

      //...Conformal mode...
      else if (_mode == 3) {

        //...Transformation...
        AList<TRGCDCWireHit> list;
        list.append((TRGCDCWireHit*) links[i]->hit());
        AList<TLink> list2;
        TConformalFinder0::conformalTransformation(ORIGIN, list, list2);

        //...Points...
        HepGeom::Point3D<double> x = list2[0]->position();
        _window.draw_point(x.x(), x.y(), c);
        if (_wireName) {
          std::string n = links[i]->wire()->name();
          _window.draw_text(x.x(), x.y(), n.c_str());
        }
        delete list2[0];
      }
    }

    if (! n) {
      _window.draw_text(0., 0., (const char*) "can't display a track");
      return;
    }

    if (_mode == 0) {

      //...Parameters...
      if (t.cores().length() == 0) {
        _window.draw_text(0., 0., (const char*) "can't display a track");
        return;
      }
      const HepGeom::Point3D<double> & pIn = InnerMost(t.cores())->positionOnTrack();
      const HepGeom::Point3D<double> & pOut = OuterMost(t.cores())->positionOnTrack();
      THelix hIp = t.helix();
      hIp.pivot(ORIGIN);
      leda_point ip(hIp.x(0.).x(), hIp.x(0.).y());
      leda_point in(pIn.x(), pIn.y());
      leda_point out(pOut.x(), pOut.y());
      if (in == out) {
        _window.draw_text(0., 0., (const char*) "can't display a track");
        return;
      }
      _window.draw_arc(ip, in, out, c);

      std::string tName = t.name();
      if (t.hep())
        tName += ":" + t.hep()->name();
      _window.draw_text(pOut.x(), pOut.y(), tName.c_str());
    }

    else if (_mode == 2) {
      THelix hIp = t.helix();
      hIp.pivot(ORIGIN);

      double xmin = _window.xmin();
      double xmax = _window.xmax();
      double ymin = xmin * hIp.tanl() * t.charge() + hIp.dz();
      double ymax = xmax * hIp.tanl() * t.charge() + hIp.dz();
      _window.draw_segment(xmin, ymin, xmax, ymax, c);
    }
  }

  void
  TWindow::drawCircle(const TCircle& t, leda_color c)
  {
    const AList<TLink> & links = t.links();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (! _stereo)
        if (links[i]->wire()->stereo())
          continue;
      if (! _axial)
        if (links[i]->wire()->axial())
          continue;

      if (_mode == 0) {
        HepGeom::Point3D<double> x = links[i]->wire()->forwardPosition();
        double radius = links[i]->hit()->drift();
        _window.draw_circle(x.x(), x.y(), radius, c);
        if (_wireName)
          _window.draw_text(x.x(),
                            x.y(),
                            (const char*) wireName(* links[i]).c_str());
      }
    }
    _window.draw_circle(t.center().x(), t.center().y(), fabs(t.radius()), c);

//     //...For debug...
// std::cout << "TWindow::drawCircle : center=" << t.center() << ","
//        << "radius=" << t.radius() << std::endl;
  }

  void
  TWindow::drawAxes(const TPoint2D&)
  {
    double xmin = _window.xmin();
    double xmax = _window.xmax();
    _window.draw_segment(xmin, 0, xmax, 0, leda_grey2);
    double ymin = _window.ymin();
    double ymax = _window.ymax();
    _window.draw_segment(0, ymin, 0, ymax, leda_grey2);
    for (unsigned i = 0; i < (unsigned) _selfAxes.length(); i++) {
      _window.draw_segment(xmin, _selfAxes[i]->y(), xmax, _selfAxes[i]->y(),
                           leda_grey1);
      _window.draw_segment(_selfAxes[i]->x(), ymin, _selfAxes[i]->x(), ymax,
                           leda_grey1);
    }
  }

  void
  TWindow::drawCdc(void)
  {

    //...Rphi mode...
    if (_mode == 0) {
      double r = 8.4;
      _window.draw_circle(0., 0., r);
      r = 88.;
      _window.draw_circle(0., 0., r);
      r = 2.;
      _window.draw_circle(0., 0., r);
    }
  }

  void
  TWindow::clear(void)
  {
    ++_nEvents;
    _text = "";
    // _skip = false;
    _objects.removeAll();
    if (_canDelete) {
      HepAListDeleteAll(_colors);
      HepAListDeleteAll(_selfObjects);
      HepAListDeleteAll(_selfTLinks);
      HepAListDeleteAll(_selfAxes);
    }
  }

  void
  TWindow::append(const AList<TRGCDCWireHit> & list, leda_color c)
  {
    AList<TLink> links;
    for (unsigned i = 0; i < (unsigned) list.length(); i++)
      links.append(new TLink(NULL, list[i]));
    _selfTLinks.append(links);
    TTrackBase* base = new TTrackBase(links);
    _selfObjects.append(base);
    _objects.append(base);
    _colors.append(new leda_color(c));
  }

  void
  TWindow::append(const AList<TLink> & list, leda_color c)
  {
    TTrackBase* t = new TTrackBase(list);
    _objects.append(t);
    _colors.append(new leda_color(c));
    _selfObjects.append(t);
  }

  std::string
  TWindow::wireName(const TLink& l) const
  {
    unsigned state = l.hit()->state();
    std::string flag;
    if (state & WireHitFindingValid) flag += "o";
    if (state & WireHitFittingValid) flag += "+";
    if (state & WireHitInvalidForFit) flag += "x";
    if (l.hit()->mc())
      flag += "(" + itostring(l.hit()->mc()->hep()->id()) + ")";
    return l.wire()->name() + flag;
  }

  void
  TWindow::oneShot(const AList<TLink> & t, leda_color c)
  {
    TTrackBase& base = * new TTrackBase(t);
    append(base, c);
    wait();
    unsigned id = _objects.length() - 1;
    _objects.remove(id);
    leda_color* b = _colors[id];
    _colors.remove(id);
    delete b;
  }

  void
  TWindow::oneShot(const TTrackBase& t, leda_color c)
  {
    append(t, c);
    wait();
    unsigned id = _objects.length() - 1;
    _objects.remove(id);
    leda_color* b = _colors[id];
    _colors.remove(id);
    delete b;
  }

  void
  TWindow::oneShot(const AList<TSegment> & t, leda_color c)
  {
    TTrackBase tmp;
    for (unsigned i = 0; i < (unsigned) t.length(); i++) {
      const TSegment& s = * t[i];
      for (unsigned j = 0; j < (unsigned) s.links().length(); j++)
        tmp.append(* s.links()[j]);
    }

    append(tmp, c);
    wait();
    unsigned id = _objects.length() - 1;
    _objects.remove(id);
    leda_color* b = _colors[id];
    _colors.remove(id);
    delete b;
  }

  void
  TWindow::oneShot(const AList<TSegmentCurl> & t, leda_color c)
  {
    TTrackBase tmp;
    for (unsigned i = 0; i < (unsigned) t.length(); i++) {
      TSegmentCurl& s = * t[i];
      for (unsigned j = 0; j < (unsigned) s.list().length(); j++)
        tmp.append(* s.list()[j]);
    }

    append(tmp, c);
    wait();
    unsigned id = _objects.length() - 1;
    _objects.remove(id);
    leda_color* b = _colors[id];
    _colors.remove(id);
    delete b;
  }

  void
  TWindow::appendSz(const TTrack& t, const AList<TLink> & list, leda_color c)
  {
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l0 = new TLink(* list[i]);
      TLink* l1 = new TLink(* list[i]);
      TLink* l2 = new TLink(* list[i]);
      l0->leftRight(WireHitLeft);
      l1->leftRight(WireHitRight);
      l2->leftRight(2);
      int err = t.szPosition(* l0);
      err = t.szPosition(* l1);
      err = t.szPosition(* l2);
      AList<TLink> links;
      links.append(l0);
      links.append(l1);
      links.append(l2);
      TTrackBase* b = new TTrackBase(links);
      _objects.append(b);
      _colors.append(new leda_color(c));
      _selfTLinks.append(l0);
      _selfTLinks.append(l1);
      _selfTLinks.append(l2);
      _selfObjects.append(b);
    }
  }

  void
  TWindow::appendSz(const TTrack& t,
                    const AList<TSegment> & list,
                    leda_color c)
  {
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * list[i];
      TLink* l = new TLink();
      t.szPosition(s, * l);
      AList<TLink> links;
      links.append(l);
      TTrackBase* b = new TTrackBase(links);
      _objects.append(b);
      _colors.append(new leda_color(c));
      _selfTLinks.append(l);
      _selfObjects.append(b);
    }
  }

  void
  TWindow::draw(const TPoint2D& p, leda_color c)
  {
    _window.draw_point(p.x(), p.y(), c);
  }

  void
  TWindow::drawHeader(void)
  {
    struct belle_event* h = (struct belle_event*)
                            BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
    if (! h) return;

    //static const Hepstd::string sp = " ";
    static const std::string sp = " ";
    std::string mc = "mc:" + itostring(h->m_ExpMC);
    std::string exp = "exp:" + itostring(h->m_ExpNo);
    std::string run = "run:" + itostring(h->m_RunNo);
    std::string frm = "frm:" + itostring((h->m_EvtNo >> 28));
    std::string evt = "evt:" + itostring((h->m_EvtNo & 268435455));
    std::string dat = "dat:" + std::string(ctime((const time_t*) & h->m_Time));
    std::string fld = "fld:" + dtostring(h->m_BField);
    std::string ler = "ler:" + dtostring(h->m_ELER);
    std::string her = "her:" + dtostring(h->m_EHER);

    std::string s0 = "";
    if (calcdc_unpack_.unpack_truncate)
      s0 += "truncated event";
    std::string head0 = exp + sp + run + sp + frm + sp + evt;
    std::string head1 = dat;
    std::string head2 = fld + sp + ler + sp + her;

    double xmin = _window.xmin();
    double xmax = _window.xmax();
    double ymin = _window.ymin();
    double ymax = _window.ymax();
    double x0 = xmin + (xmax - xmin) * .7;
    double x1 = xmin + (xmax - xmin) * .6;
    double y0 = ymin + (ymax - ymin) * .92;
    double y1 = ymin + (ymax - ymin) * .95;
    double y2 = ymin + (ymax - ymin) * .98;
    double y3 = ymin + (ymax - ymin) * -.90;

    _window.draw_text(x0, y2, head2.c_str());
    _window.draw_text(x0, y1, head1.c_str());
    _window.draw_text(x0, y0, head0.c_str());

    _window.draw_text(x1, y3, s0.c_str());
  }

  void
  TWindow::append(const AList<TSegmentCurl> & a, leda_color b)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++) {
      TTrackBase& base = * (TTrackBase*) new TSegment(a[i]->list());
      append(base, b);
      _selfObjects.append(base);
    }
  }

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif

#endif
