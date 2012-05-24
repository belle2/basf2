//-----------------------------------------------------------------------------
// $Id: TUpdater.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TUpdater.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to handle update timing of static objects of tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.8  2001/12/23 09:58:58  katayama
// removed Strings.h
//
// Revision 1.7  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.6  2001/12/14 02:54:52  katayama
// For gcc-3.0
//
// Revision 1.5  2001/04/11 01:10:05  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.4  1999/10/30 10:12:56  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1999/07/27 08:01:41  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.2  1999/06/09 15:09:58  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.1  1998/09/28 14:54:22  yiwasaki
// MC tables, TUpdater, oichan added
//
//
//-----------------------------------------------------------------------------

#ifndef TUpdater_FLAG_
#define TUpdater_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

namespace Belle {

/// A class to handle update timing of static objects of tracking.
  class TUpdater {

  public:
    /// Constructor
    TUpdater();

  public:
    /// Destructor
    virtual ~TUpdater();

  public:
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:
    /// returns true if an object is updated.
    virtual bool updated(void) const;

    /// updates an object.
    virtual void update(void);

    /// returns true if an object is initialized.
    virtual bool initialized(void) const;

    /// initializes an object.
    virtual void initialize(void);

    /// clears an object.
    virtual void clear(void);

  private:
    int _expN;
    int _runN;
    int _evtN;
    bool _init;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TUpdater_INLINE_DEFINE_HERE
#endif

#ifdef TUpdater_INLINE_DEFINE_HERE

  inline
  bool
  TUpdater::initialized(void) const
  {
    return _init;
  }

  inline
  void
  TUpdater::initialize(void)
  {
    _init = true;
  }

  inline
  void
  TUpdater::clear(void)
  {
    _expN = -1;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TUpdater_FLAG_ */
