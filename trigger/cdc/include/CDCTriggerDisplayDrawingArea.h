//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerDisplayDrawingArea.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef CDCTRIGGER_DISPLAY

#ifndef CDCTriggerDisplayDrawingArea_FLAG_
#define CDCTriggerDisplayDrawingArea_FLAG_

#include <vector>
#include <gtkmm.h>

namespace Belle2 {

class CDCTriggerWireHit;

/// Actual class to display trigger objects
class CDCTriggerDisplayDrawingArea : public Gtk::DrawingArea {

  public:
    /// Default constructor
    CDCTriggerDisplayDrawingArea(int size, double innerR, double outerR);
    
    /// Destructor
    virtual ~CDCTriggerDisplayDrawingArea();

    double scale(double);
    double scale(void) const;
    void resetPosition(void);
    bool axial(void) const;
    bool axial(bool);
    bool stereo(void) const;
    bool stereo(bool);
    bool wireName(void) const;
    bool wireName(bool);

    void clear(void);
//     void append(const std::vector<const Belle2::CDCTriggerWireHit *> &,
// 		Gdk::Color color = Gdk::Color("grey"));
//     void append(const AList<TLink> &,
// 		Gdk::Color color = Gdk::Color("grey"));
//     void append(const AList<TSegment> &,
// 		Gdk::Color color = Gdk::Color("grey"));
//     void append(const AList<TTrack> &,
// 		Gdk::Color color = Gdk::Color("grey"));
//     void append(const AList<TCircle> &,
// 		Gdk::Color color = Gdk::Color("grey"));

    virtual bool on_expose_event(GdkEventExpose *);
    virtual bool on_button_press_event(GdkEventButton *);

  private:
    virtual void on_realize();
    void drawCDC(void);
    void draw(void);
//     void drawBase(const TTrackBase &, Gdk::Color & c);
//     void drawSegment(const TSegment &, Gdk::Color & c);
//     void drawTrack(const TTrack &, Gdk::Color & c);
//     void drawLine(const TLine &, Gdk::Color & c);
//    void drawCircle(const TCircle &, Gdk::Color & c);

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
    double _x, _y;
    double _innerR;
    double _outerR;

    Gtk::Main * GtkMain;
//     CAList<TTrackBase> _objects;
//     AList<Gdk::Color> _colors;
//     AList<TTrackBase> _selfObjects;
//     AList<TLink> _selfTLinks;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _grey, _yellow, _grey0;

    Glib::RefPtr<Pango::Layout> _pl;
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
double
CDCTriggerDisplayDrawingArea::scale(double a) {
    return _scale = a;
}

inline
double
CDCTriggerDisplayDrawingArea::scale(void) const {
    return _scale;
}

inline
bool
CDCTriggerDisplayDrawingArea::axial(void) const {
    return _axial;
}

inline
bool
CDCTriggerDisplayDrawingArea::axial(bool a) {
    return _axial = a;
}

inline
bool
CDCTriggerDisplayDrawingArea::stereo(void) const {
    return _stereo;
}

inline
bool
CDCTriggerDisplayDrawingArea::stereo(bool a) {
    return _stereo = a;
}

inline
bool
CDCTriggerDisplayDrawingArea::wireName(void) const {
    return _wireName;
}

inline
bool
CDCTriggerDisplayDrawingArea::wireName(bool a) {
    return _wireName = a;
}

inline
int
CDCTriggerDisplayDrawingArea::x(double a) const {
/*     std::cout << "_x,_scale,_winw/2,a=" << _x << "," << _scale << "," */
/* 	      << _winw/2 << "," << a << std::endl; */
    return int((a - _x) * _scale + _winw / 2);
}

inline
int
CDCTriggerDisplayDrawingArea::y(double a) const {
    return int((- a - _y) * _scale + _winh / 2);
}

inline
int
CDCTriggerDisplayDrawingArea::xR(double a) const {
    return int((a - _winw / 2) / _scale + _x);
}

inline
int
CDCTriggerDisplayDrawingArea::yR(double a) const {
    return int((- a - _winh / 2) / _scale + _y);
}

// inline
// void
// CDCTriggerDisplayDrawingArea::clear(void) {
//     _objects.removeAll();
//     HepAListDeleteAll(_colors);
//     HepAListDeleteAll(_selfObjects);
//     HepAListDeleteAll(_selfTLinks);
// }

#endif
#undef inline

} // namespace Belle2

#endif // CDCTriggerDisplayDrawingArea_FLAG_
#endif
