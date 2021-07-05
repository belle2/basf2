/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#ifdef TRGCDC_SHORT_NAMES
#define TCDisplay TRGCDCDisplay
#endif

namespace Belle2 {
  class TRGCDCDisplayDrawingArea;
  class TRGCDCDisplayRphi;
  class TRGCDCDisplayHough;
}

namespace Belle2 {

/// A class to display CDC trigger information.
  class TRGCDCDisplay : public Gtk::Window {

  public:

    /// Default constructor
    TRGCDCDisplay(const std::string& name,
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

    /// stops display in next dispay run command.
    void stop(void);

  public: // Status

    /// sets present stage of display.
    void stage(const std::string& stage);

    /// sets information of present stage.
    void information(const std::string& information);

  public: // Access to drawing area.

    /// returns drawing area.
    virtual TRGCDCDisplayDrawingArea& area(void) = 0;

    /// links R/phi display.
    void link(TRGCDCDisplayRphi&);

    /// returns R/phi display.
    TRGCDCDisplayRphi* rphi(void);

  private: // Actions

    /// For next button
    virtual void on_next(void);

    /// For End of Event button
    virtual void on_endOfEvent(void);

    /// For Next Event button
    virtual void on_nextEvent(void);

    /// For Non Stop button
    virtual void on_nonStop(void);

    /// For Position Reset button
    virtual void on_positionReset(void);

    /// For Wire Name button
    virtual void on_wireName(void);

  private: // Objects to display and control

    /// Stage
    std::string _stage;

    /// Information
    std::string _info;

    /// Skip
    bool _skip;

    /// End of event
    static bool _endOfEvent;

    /// End of event flag
    static bool _endOfEventFlag;

    /// Skip event
    static bool _skipEvent;

    /// Non stop mode
    static bool _nonStop;

  private: // GTK stuff

    /// Wire name
    bool _wireName;

    /// Old CDC display
    bool _oldCDC;

    /// Box 0
    Gtk::VBox _box0;

    /// Menu buttons
    Gtk::HBox _menuButtons;

    /// Button
    Gtk::Button _buttonNext;

    /// Button
    Gtk::Button _buttonEndOfEvent;

    /// Button
    Gtk::Button _buttonNextEvent;

    /// Button
    Gtk::Button _buttonNonStop;

    /// Label
    Gtk::Label _label;

    /// Bottom box
    Gtk::HBox _bottom;

    /// Button
    Gtk::Button _buttonPositionReset;

    /// Button
    Gtk::CheckButton _buttonWireName;

    /// Friends
    friend class TRGCDCDisplayRphi;

    /// Friends
    friend class TRGCDCDisplayHough;

    /// Rphi display.
    TRGCDCDisplayRphi* _rphi;
  };

//-----------------------------------------------------------------------------

  inline
  void
  TRGCDCDisplay::stage(const std::string& a)
  {
    _stage = a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
  }

  inline
  void
  TRGCDCDisplay::information(const std::string& a)
  {
//  _info += a;
    _info = a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
  }

  inline
  bool
  TRGCDCDisplay::skip(void) const
  {
    return _skip;
  }

  inline
  bool
  TRGCDCDisplay::skip(bool a)
  {
    return _skip = a;
  }

  inline
  bool
  TRGCDCDisplay::wireName(void) const
  {
    return _wireName;
  }

  inline
  void
  TRGCDCDisplay::beginningOfEvent(void)
  {
    _skipEvent = false;
    _endOfEvent = false;
  }

  inline
  void
  TRGCDCDisplay::endOfEvent(void)
  {
    _endOfEvent = true;
  }

  inline
  void
  TRGCDCDisplay::link(TRGCDCDisplayRphi& a)
  {
    _rphi = & a;
  }

  inline
  TRGCDCDisplayRphi*
  TRGCDCDisplay::rphi(void)
  {
    return _rphi;
  }

} // namespace Belle2

#endif // TRGCDCDisplay_FLAG_
#endif
