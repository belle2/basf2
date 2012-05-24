//-----------------------------------------------------------------------------
// $Id: TRungeFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRungeFitter.h
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a Runge Kutta track
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.6  2003/12/25 12:04:44  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.5  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.4  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.3  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.2  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.1  2001/08/02 07:04:15  yiwasaki
// RK fitter from K.Inami
//
//-----------------------------------------------------------------------------

#ifndef TRUNGEFITTER_FLAG_
#define TRUNGEFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/TFitter.h"

namespace Belle {

  class TLink;
  class TRunge;

/// A class to fit a TTrackBase object to a 3D line.
  class TRungeFitter : public TFitter {

  public:
    /// Constructor.
    TRungeFitter(const std::string& name);
    TRungeFitter(const std::string& name, bool m_sag, int m_prop, bool m_tof);

    /// Destructor
    virtual ~TRungeFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    virtual int fit(TTrackBase&) const;
    virtual int fit(TTrackBase&, float t0Offset) const;

    void sag(bool);
    void propagation(int);
    void tof(bool);

  private:
    bool _sag;
    int _propagation;
    bool _tof;

  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TRUNGEFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TRUNGEFITTER_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* TRUNGEFITTER_FLAG_ */
