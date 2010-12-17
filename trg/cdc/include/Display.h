//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Display.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplay_FLAG_
#define TRGCDCDisplay_FLAG_

#include <gtkmm.h>
#include "trg/cdc/DisplayDrawingArea.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDisplay TRGCDCDisplay
#endif

namespace Belle2 {

/// A class to display CDC trigger information.
class TRGCDCDisplay : public Gtk::Window {
    
  public:
    /// Default constructor
    TRGCDCDisplay(const std::string & name,
		  int sizeWindow = 600,
		  int sizeMax = 600);

    /// Destructor
    virtual ~TRGCDCDisplay();

  public: // display control

    /// runs GTK.
    void run(void);

    /// clears window.
    virtual void clear(void);

    /// tells window here is the beginning of an event.
    void beginningOfEvent(void);

    /// tells window here is the end of an event.
    void endOfEvent(void);

    /// returns skip flag.
    bool skip(void) const;

    /// sets and returns skip flag.
    bool skip(bool);

    /// returns present condition for wire name display.
    bool wireName(void) const;

  public: // Status

    /// sets present stage of display.
    void stage(const std::string & stage);

    /// sets information of present stage.
    void information(const std::string & information);

  public: // Access to drawing area.

    /// returns drawing area.
    virtual TRGCDCDisplayDrawingArea & area(void) = 0;

  private: // Actions
    virtual void on_next(void);
    virtual void on_endOfEvent(void);
    virtual void on_nextEvent(void);
    virtual void on_positionReset(void);
    virtual void on_wireName(void);

  private: // Objects to display and control
    std::string _stage;
    std::string _info;
    bool _skip;
    static bool _endOfEvent;
    static bool _endOfEventFlag;
    static bool _skipEvent;

  private: // GTK stuff
    bool _wireName;
    bool _oldCDC;
    Gtk::VBox _box0;
    Gtk::HBox _menuButtons;
    Gtk::Button _buttonNext;
    Gtk::Button _buttonEndOfEvent;
    Gtk::Button _buttonNextEvent;
    Gtk::Label _label;
    Gtk::HBox _bottom;
    Gtk::Button _buttonPositionReset;
    Gtk::CheckButton _buttonWireName;

    friend class TRGCDCDisplayRphi;
    friend class TRGCDCDisplayHough;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCDisplay_INLINE_DEFINE_HERE
#endif
#ifdef TRGCDCDisplay_INLINE_DEFINE_HERE

inline
void
TRGCDCDisplay::stage(const std::string & a) {
    _stage = a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
}

inline
void
TRGCDCDisplay::information(const std::string & a) {
    _info += a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
}

inline
bool
TRGCDCDisplay::skip(void) const {
    return _skip;
}

inline
bool
TRGCDCDisplay::skip(bool a) {
    return _skip = a;
}

inline
bool
TRGCDCDisplay::wireName(void) const {
    return _wireName;
}

inline
void
TRGCDCDisplay::beginningOfEvent(void) {
    _skipEvent = false;
    _endOfEvent = false;
}

inline
void
TRGCDCDisplay::endOfEvent(void) {
    _endOfEvent = true;
}

#endif
#undef inline

} // namespace Belle2

#endif // TRGCDCDisplay_FLAG_
#endif
