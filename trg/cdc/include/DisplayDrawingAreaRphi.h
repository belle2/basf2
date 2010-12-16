//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingAreaRphi.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayDrawingAreaRphi_FLAG_
#define TRGCDCDisplayDrawingAreaRphi_FLAG_

#include "trg/cdc/DisplayDrawingArea.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDDAreaRphi TRGCDCDisplayDrawingAreaRphi
#endif

namespace Belle2 {

class TRGCDCWire;
class TRGCDCWireHit;
class TRGCDCTrackSegment;
class TRGCDCFrontEnd;
class TRGCDCMerger;

/// Actual class to display trigger objects
class TRGCDCDisplayDrawingAreaRphi : public TRGCDCDisplayDrawingArea {

  public:
    /// Default constructor
    TRGCDCDisplayDrawingAreaRphi(int size, double innerR, double outerR);
    
    /// Destructor
    virtual ~TRGCDCDisplayDrawingAreaRphi();

    double scale(double);
    double scale(void) const;
    void resetPosition(void);
    bool axial(void) const;
    bool axial(bool);
    bool stereo(void) const;
    bool stereo(bool);
    bool wireName(void) const;
    bool wireName(bool);
    bool oldCDC(void) const;
    bool oldCDC(bool);

    void clear(void);

    /// appends wire hits to display.
    void append(const std::vector<const TRGCDCWireHit *> &,
 		Gdk::Color color = Gdk::Color("grey"));
    void append(const TRGCDCTrackSegment &,
  		Gdk::Color color = Gdk::Color("grey"));
    void append(const std::vector<const TRGCDCTrackSegment *> &,
  		Gdk::Color color = Gdk::Color("grey"));
    void append(const TRGCDCFrontEnd &,
  		Gdk::Color color = Gdk::Color("grey"));
    void append(const TRGCDCMerger &,
  		Gdk::Color color = Gdk::Color("grey"));

    virtual bool on_expose_event(GdkEventExpose *);
    virtual bool on_button_press_event(GdkEventButton *);

  private:
    virtual void on_realize();
    void drawCDC(void);
    void draw(void);
    void drawHits(void);

    void drawWire(const TRGCDCWire & w,
		  int lineWidth,
		  Gdk::Color & c,
		  Gdk::LineStyle s);
    void drawTrackSegment(const TRGCDCTrackSegment & w,
			  int lineWidth,
			  Gdk::Color & c,
			  Gdk::LineStyle s);
    void drawFrontEnd(const TRGCDCFrontEnd & w,
		      int lineWidth,
		      Gdk::Color & c,
		      Gdk::LineStyle s);
    void drawMerger(const TRGCDCMerger & w,
		    int lineWidth,
		    Gdk::Color & c,
		    Gdk::LineStyle s);

    /// Coordinate transformations.
    int x(double x) const;
    int y(double y) const;
    int xR(double x) const;
    int yR(double y) const;

  private:
    double _scale;
    bool _axial;
    bool _stereo;
    bool _wireName;
    bool _oldCDC;
    double _x, _y;
    double _innerR;
    double _outerR;

    std::vector<const TRGCDCWireHit *> _hits;
    std::vector<Gdk::Color> _hitsColor;

    std::vector<const TRGCDCTrackSegment *> _segments;
    std::vector<Gdk::Color> _segmentsColor;

    std::vector<const TRGCDCFrontEnd *> _fronts;
    std::vector<Gdk::Color> _frontColors;

    std::vector<const TRGCDCMerger *> _mergers;
    std::vector<Gdk::Color> _mergerColors;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _grey, _yellow, _grey0;

    Glib::RefPtr<Pango::Layout> _pl;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCDisplayDrawingAreaRphi_INLINE_DEFINE_HERE
#endif
#ifdef TRGCDCDisplayDrawingAreaRphi_INLINE_DEFINE_HERE

inline
double
TRGCDCDisplayDrawingAreaRphi::scale(double a) {
    return _scale = a;
}

inline
double
TRGCDCDisplayDrawingAreaRphi::scale(void) const {
    return _scale;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::axial(void) const {
    return _axial;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::axial(bool a) {
    return _axial = a;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::stereo(void) const {
    return _stereo;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::stereo(bool a) {
    return _stereo = a;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::wireName(void) const {
    return _wireName;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::wireName(bool a) {
    return _wireName = a;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::oldCDC(void) const {
    return _oldCDC;
}

inline
bool
TRGCDCDisplayDrawingAreaRphi::oldCDC(bool a) {
    return _oldCDC = a;
}

inline
int
TRGCDCDisplayDrawingAreaRphi::x(double a) const {
/*     std::cout << "_x,_scale,_winw/2,a=" << _x << "," << _scale << "," */
/* 	      << _winw/2 << "," << a << std::endl; */
    return int((a - _x) * _scale + _winw / 2);
}

inline
int
TRGCDCDisplayDrawingAreaRphi::y(double a) const {
    return int((- a - _y) * _scale + _winh / 2);
}

inline
int
TRGCDCDisplayDrawingAreaRphi::xR(double a) const {
    return int((a - _winw / 2) / _scale + _x);
}

inline
int
TRGCDCDisplayDrawingAreaRphi::yR(double a) const {
    return int((- a - _winh / 2) / _scale + _y);
}

#endif
#undef inline

} // namespace Belle2

#endif // TRGCDCDisplayDrawingAreaRphi_FLAG_
#endif
