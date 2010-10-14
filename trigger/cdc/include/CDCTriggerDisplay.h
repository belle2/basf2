//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerDisplay.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef CDCTRIGGER_DISPLAY

#ifndef CDCTriggerDisplay_FLAG_
#define CDCTriggerDisplay_FLAG_

#include <gtkmm.h>
#include "trigger/cdc/CDCTriggerDisplayDrawingArea.h"

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTDisplay CDCTriggerDisplay
#endif

namespace Belle2 {

/// A class to display CDC trigger information.
class CDCTriggerDisplay : public Gtk::Window {
    
  public:
    /// Default constructor
    CDCTriggerDisplay(const std::string & name = "CDCTriggerDisplay",
		      double innerR = 160,
		      double outerR = 1137,
		      int size = 600);
    
    /// Destructor
    virtual ~CDCTriggerDisplay();

  public: // dispaly control

    /// clears window.
    virtual void clear(void);

    /// returns skip flag.
    bool skip(void) const;

    /// sets and returns skip flag.
    bool skip(bool);

  public: // Status

    /// sets present stage of display.
    void stage(const std::string & stage);

    /// sets information of present stage.
    void information(const std::string & information);

    /// tells window here is the beginning of an event.
    void beginEvent(void);

    /// tells window here is the end of an event.
    void endOfEvent(void);

  public: // Actions
    virtual void on_next(void);
    virtual void on_endOfEvent(void);
    virtual void on_nextEvent(void);
    virtual void on_positionReset(void);
    virtual void on_scale_value_changed(void);
    virtual void on_axial(void);
    virtual void on_stereo(void);
    virtual void on_wireName(void);
//  virtual bool on_expose_event(GdkEventExpose *);

  public: // Static functions
//    static void initializeGTK(void);
    void run(void);

  public: // Others
    /// Set and pack drawing area.
    void pack(Gtk::DrawingArea &);

    /// To get/set sclaer value.
    double scale(void) const;
    double scale(double);

    bool axial(void) const;
    bool stereo(void) const;
    bool wireName(void) const;

  private: // Tracking stuff
    std::string _stage;
    std::string _info;
    bool _skip;
    static bool _endOfEvent;
    static bool _endOfEventFlag;
    static bool _skipEvent;

  private: // GTK stuff
    bool _axial;
    bool _stereo;
    bool _wireName;
    Gtk::VBox _box0;
    Gtk::HBox _menuButtons;
    Gtk::Button _buttonNext;
    Gtk::Button _buttonEndOfEvent;
    Gtk::Button _buttonNextEvent;
    Gtk::Label _label;
    Gtk::HBox _scale;
    Gtk::Adjustment _adjustment;
    Gtk::HScale _scaler;
    Gtk::Button _buttonPositionReset;
    Gtk::CheckButton _buttonAxial;
    Gtk::CheckButton _buttonStereo;
    Gtk::CheckButton _buttonWireName;
//  Gtk::DrawingArea * _w;

  private: // GTK stuff
    CDCTriggerDisplayDrawingArea _w;
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
CDCTriggerDisplay::clear(void) {
//     const struct belle_event & ev = 
// 	* (struct belle_event *) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
//     const unsigned e = ev.m_ExpNo;
//     const unsigned r = ev.m_RunNo;
//     const unsigned f = (ev.m_EvtNo >> 28);
//     const unsigned v = ev.m_EvtNo & 0x0fffffff;
//     const std::string id =
// 	"e" + itostring(e) +
// 	" r" + itostring(r) +
// 	" f" + itostring(f) +
// 	" ev" + itostring(v) + " : ";
    const std::string id = "unknown : ";
    _stage = "";
    _info = id;
}

inline
void
CDCTriggerDisplay::stage(const std::string & a) {
    _stage = a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
}

inline
void
CDCTriggerDisplay::information(const std::string & a) {
    _info += a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
}

inline
bool
CDCTriggerDisplay::skip(void) const {
    return _skip;
}

inline
bool
CDCTriggerDisplay::skip(bool a) {
    return _skip = a;
}

inline
void
CDCTriggerDisplay::on_axial(void) {
    _axial = _buttonAxial.get_active();
}

inline
void
CDCTriggerDisplay::on_stereo(void) {
    _stereo = _buttonStereo.get_active();
}

inline
void
CDCTriggerDisplay::on_wireName(void) {
    _wireName = _buttonWireName.get_active();
}

inline
double
CDCTriggerDisplay::scale(void) const {
    return _scaler.get_value();
}

inline
double
CDCTriggerDisplay::scale(double a) {
    _scaler.set_value(a);
    return _scaler.get_value();
}

inline
bool
CDCTriggerDisplay::axial(void) const {
    return _axial;
}

inline
bool
CDCTriggerDisplay::stereo(void) const {
    return _stereo;
}

inline
bool
CDCTriggerDisplay::wireName(void) const {
    return _wireName;
}

inline
void
CDCTriggerDisplay::beginEvent(void) {
    _skipEvent = false;
    _endOfEvent = false;
}

inline
void
CDCTriggerDisplay::endOfEvent(void) {
    _endOfEvent = true;
}

#endif
#undef inline

} // namespace Belle2

#endif // CDCTriggerDisplay_FLAG_
#endif
