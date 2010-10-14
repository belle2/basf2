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

    /// runs GTK.
    void run(void);

    /// clears window.
    virtual void clear(void);

    /// tells window here is the beginning of an event.
    void beginEvent(void);

    /// tells window here is the end of an event.
    void endOfEvent(void);

    /// returns skip flag.
    bool skip(void) const;

    /// sets and returns skip flag.
    bool skip(bool);

    /// returns scaler value.
    double scale(void) const;

    /// sets and returns scaler value.
    double scale(double);

    /// returns present condition for axial wire display.
    bool axial(void) const;

    /// returns present condition for stereo wire display.
    bool stereo(void) const;

    /// returns present condition for wire name display.
    bool wireName(void) const;

  public: // Status

    /// sets present stage of display.
    void stage(const std::string & stage);

    /// sets information of present stage.
    void information(const std::string & information);

  public: // Access to drawing area.

    /// returns drawing area.
    CDCTriggerDisplayDrawingArea & area(void);

  private: // Actions
    virtual void on_next(void);
    virtual void on_endOfEvent(void);
    virtual void on_nextEvent(void);
    virtual void on_positionReset(void);
    virtual void on_scale_value_changed(void);
    virtual void on_axial(void);
    virtual void on_stereo(void);
    virtual void on_wireName(void);
    virtual void on_BelleCDC(void);

  private: // Objects to display and control
    std::string _stage;
    std::string _info;
    bool _skip;
    static bool _endOfEvent;
    static bool _endOfEventFlag;
    static bool _skipEvent;
    std::vector<CDCTriggerWireHit *> _hits;

  private: // GTK stuff
    bool _axial;
    bool _stereo;
    bool _wireName;
    bool _oldCDC;
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
    Gtk::CheckButton _buttonBelleCDC;
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

inline
CDCTriggerDisplayDrawingArea &
CDCTriggerDisplay::area(void) {
    return _w;
}

#endif
#undef inline

} // namespace Belle2

#endif // CDCTriggerDisplay_FLAG_
#endif
