//-----------------------------------------------------------------------------
// $Id: T3DLineFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : T3DLineFitter.h
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a 3D line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.8  2002/01/03 11:04:57  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.7  2001/12/23 09:58:53  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:52  katayama
// Uss find,istring
//
// Revision 1.5  2001/12/14 02:54:46  katayama
// For gcc-3.0
//
// Revision 1.4  2001/04/11 01:10:00  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.3  1999/10/30 10:12:33  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.2  1999/10/21 15:45:18  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
//
//-----------------------------------------------------------------------------

#ifndef T3DLINEFITTER_FLAG_
#define T3DLINEFITTER_FLAG_
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

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
  class T3DLine;

/// A class to fit a TTrackBase object to a 3D line.
  class T3DLineFitter : public TFitter {

  public:
    /// Constructor.
    T3DLineFitter(const std::string& name);
    T3DLineFitter(const std::string& name, bool m_sag, int m_prop, bool m_tof);

    /// Destructor
    virtual ~T3DLineFitter();

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
    /// calculates dXda. 'TLink' and 'T3DLine' are inputs. Others are outputs.
    int dxda(const TLink&,
             const T3DLine&,
             CLHEP::HepVector& dxda,
             CLHEP::HepVector& dyda,
             CLHEP::HepVector& dzda,
             HepGeom::Vector3D<double> & wireDirection) const;

    /// calculates drift distance and its error.
    void drift(const T3DLine&,
               const TLink&,
               float t0Offset,
               double& distance,
               double& err) const;

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
#define T3DLINEFITTER_INLINE_DEFINE_HERE
#endif

#ifdef T3DLINEFITTER_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* T3DLINEFITTER_FLAG_ */
