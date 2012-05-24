//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowGTK.h
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

#ifndef TWINDOWGTK_FLAG_
#define TWINDOWGTK_FLAG_


#include <gtkmm.h>

#include "tracking/modules/trasan/Strings.h"

namespace Belle {

/// A class to display tracking objects.
  class TWindowGTK : public Gtk::Window {

  public:
    /// Default constructor
    TWindowGTK(const std::string& name = "TWindowGTK",
               double outerR = 880.0,
               int size = 600);

    /// Destructor
    virtual ~TWindowGTK();

  public: // Tracking stuff
    virtual void clear(void);
    bool skip(void) const;
    bool skip(bool);
    bool skipEvent(void) const;
    bool skipEvent(bool);

  public: // Status
    void stage(const std::string& stage);
    void information(const std::string& information);
    void beginEvent(void);
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
    void pack(Gtk::DrawingArea&);

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
    Gtk::DrawingArea* _w;
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
  TWindowGTK::clear(void)
  {
//     const struct belle_event & ev =
//  * (struct belle_event *) BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
//     const unsigned e = ev.m_ExpNo;
//     const unsigned r = ev.m_RunNo;
//     const unsigned f = (ev.m_EvtNo >> 28);
//     const unsigned v = ev.m_EvtNo & 0x0fffffff;
//     const std::string id =
//  "e" + itostring(e) +
//  " r" + itostring(r) +
//  " f" + itostring(f) +
//  " ev" + itostring(v) + " : ";
    const std::string id = "unknown : ";
    _stage = "";
    _info = id;
  }

  inline
  void
  TWindowGTK::stage(const std::string& a)
  {
    _stage = a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
  }

  inline
  void
  TWindowGTK::information(const std::string& a)
  {
    _info += a;
    Glib::ustring s = "Stage : " + _stage + "\nInformation : " + _info;
    _label.set_label(s);
  }

  inline
  bool
  TWindowGTK::skip(void) const
  {
    return _skip;
  }

  inline
  bool
  TWindowGTK::skip(bool a)
  {
    return _skip = a;
  }

  inline
  bool
  TWindowGTK::skipEvent(void) const
  {
    return _skipEvent;
  }

  inline
  bool
  TWindowGTK::skipEvent(bool a)
  {
    return _skipEvent = a;
  }

  inline
  void
  TWindowGTK::on_axial(void)
  {
    _axial = _buttonAxial.get_active();
  }

  inline
  void
  TWindowGTK::on_stereo(void)
  {
    _stereo = _buttonStereo.get_active();
  }

  inline
  void
  TWindowGTK::on_wireName(void)
  {
    _wireName = _buttonWireName.get_active();
  }

  inline
  double
  TWindowGTK::scale(void) const
  {
    return _scaler.get_value();
  }

  inline
  double
  TWindowGTK::scale(double a)
  {
    _scaler.set_value(a);
    return _scaler.get_value();
  }

  inline
  bool
  TWindowGTK::axial(void) const
  {
    return _axial;
  }

  inline
  bool
  TWindowGTK::stereo(void) const
  {
    return _stereo;
  }

  inline
  bool
  TWindowGTK::wireName(void) const
  {
    return _wireName;
  }

  inline
  void
  TWindowGTK::beginEvent(void)
  {
    _skipEvent = false;
    _endOfEvent = false;
  }

  inline
  void
  TWindowGTK::endOfEvent(void)
  {
    _endOfEvent = true;
  }

#endif
#undef inline

} // namespace Belle

#endif // TWINDOWGTK_FLAG_
#endif
