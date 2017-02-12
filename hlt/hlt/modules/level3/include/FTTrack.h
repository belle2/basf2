/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTTrack_H
#define FTTrack_H

#include <TVectorD.h>
#include <TVector3.h>

#include "hlt/hlt/modules/level3/Lpav.h"
#include "hlt/hlt/modules/level3/Zav.h"

#include "hlt/hlt/modules/level3/FTList.h"
#include "hlt/hlt/modules/level3/FTSegment.h"

namespace Belle2 {

  class FTWire;

  //! Track class for the Level-3 Fast Track Finder
  class FTTrack {

  public:
    //! constructor
    FTTrack(FTList<FTSegment*>& axial_segments, double kappa, double chi2Kappa);

    //! destructor
    ~FTTrack();

  public: // public member functions
    //! do r-phi circle fit
    int rPhiFit(void);

    //! do r-phi refit
    int rPhiReFit(double vx, double xy, int vtx_flag);

    //! do s-z linear fit
    int szFit(void);

    //! update s and z information for linking
    void updateSZ(void);

    //! link stereo segments by tanLambda
    int linkStereoSegments(void);

  public: // Selectors

    //! returns drho
    double getDr(void) const;

    //! returns phi0
    double getPhi0(void) const;

    //! returns kappa
    double getKappa(void) const;

    //! returns dz
    double getDz(void) const;

    //! returns tan(lambda)
    double getTanL(void) const;

    //! returns helix parameters
    const TVectorD& getHelix(void) const;

    //! returns lpav
    const L3::Lpav& lpav(void) const;

    //! returns zav
    const L3::Zav& zav(void) const;

    //! returns axial segments
    FTList<FTSegment*>& getAxialSegments(void) const;

    //! returns stereo_segments
    FTList<FTSegment*>& getStereoSegments(void) const;

    //! returns kappa at linking
    double kappaTmp(void) const;

    //! returns sigmaKappa at linking
    double chi2KappaTmp(void) const;

    //! returns difference between z and estimated z
    //! at stereo segment linking if its valid
    double deltaZ(double s, double z) const;

    //! returns chi2 of the r-phi fit
    double rPhiFitChi2() const;

    //! returns chi2 of the s-z fit
    double szFitChi2() const;

    //! pivot of the helix parameter =(0,0,0)
    static const TVector3& pivot() { return c_pivot; };

  public: // Modifiers
    //! add z for culculation of tanLambda
    double SigmaZ(double z);

    //! add s for culculation of tanLambda
    double SigmaS(double s);

    //! add s for culculation of dz, tanLambda
    double SigmaSS(double ss);

    //! add s for culculation of dz, tanLambda
    double SigmaSZ(double sz);

    //! append stereo segment to the stereo segment list
    void appendStereo(FTSegment*, double s, double z);

    //! append stereo segment to the cache
    void appendStereoCache(FTSegment*);

    //! set event timing for the event
    static void setEvtTiming(double t) { s_evtTiming = t; }

  public:
    //! cut value of pt at the track finding
    static double s_minPt;

    //! cut value of drho at the track finding
    static double s_minDr;

    //! flag for additional TDC cuts
    static bool s_additionalTdcCuts;

  private:
    //! pivot (=origin)
    static const TVector3 c_pivot;

    //! reconstructed event timing
    static int s_evtTiming;

    //! pointer to the r-phi circle fitter
    L3::Lpav* m_la;

    //! pointer to the s-z line fitter
    L3::Zav* m_za;

    //! pointer to the fitted helix parameter
    TVectorD* m_helix;

    //! list of axial segments
    FTList<FTSegment*>& m_axialSegments;

    //! list of stereo segments
    FTList<FTSegment*>* m_stereoSegments;

    //! list of stereo segments
    FTList<FTSegment*>* m_stereoSegmentsCache;

    //! list of stereo segments for each syper laters
    FTList<FTList<FTSegment*> *>* m_stereoSegmentsBySuperLayer;

    union {
      //! kappa at track finding
      double m_kappa;
      //! sum of s
      double m_SigmaS;
    };

    union {
      //! chi square of kappa at track finding
      double m_chi2Kappa;
      //! sum of s*s
      double m_SigmaSS;
    };

    //! sum of z
    double m_SigmaZ;

    //! sum of s*z
    double m_SigmaSZ;

    //! chi square of r-phi fit
    double m_rPhiFitChi2;

    //! chi square of s-z fit
    double m_szFitChi2;
  };

  inline
  FTTrack::FTTrack(FTList<FTSegment*>& axialSegments,
                   double kappa, double chi2Kappa)
    : m_la(NULL),
      m_za(NULL),
      m_helix(NULL),
      m_axialSegments(axialSegments),
      m_stereoSegments(NULL),
      m_stereoSegmentsCache(NULL),
      m_stereoSegmentsBySuperLayer(NULL),
      //m_wireHits(*(new FTList<FTWire *>(50))),
      m_kappa(kappa),
      m_chi2Kappa(chi2Kappa),
      m_rPhiFitChi2(0.),
      m_szFitChi2(0.)
  {
  }

  inline
  FTTrack::~FTTrack()
  {
    delete &m_axialSegments;
    delete m_la;
    delete m_za;
    delete m_stereoSegments;
    delete m_helix;
  }

  inline
  FTList<FTSegment*>&
  FTTrack::getAxialSegments(void) const
  {
    return m_axialSegments;
  }

  inline
  FTList<FTSegment*>&
  FTTrack::getStereoSegments(void) const
  {
    return *m_stereoSegments;
  }

  inline
  void
  FTTrack::appendStereoCache(FTSegment* src)
  {
    m_stereoSegmentsCache->append(src);
  }

  inline
  void
  FTTrack::appendStereo(FTSegment* src, double s = 0, double z = 0)
  {
    m_stereoSegments->append(src);
    m_SigmaS += s;
    m_SigmaZ += z;
    m_SigmaSZ += s * z;
    m_SigmaSS += s * s;
  }

  inline
  double
  FTTrack::kappaTmp(void) const
  {
    return m_kappa;
  }

  inline
  double
  FTTrack::chi2KappaTmp(void) const
  {
    return m_chi2Kappa;
  }

  inline
  const L3::Lpav&
  FTTrack::lpav(void) const
  {
    return *m_la;
  }

  inline
  const L3::Zav&
  FTTrack::zav(void) const
  {
    return *m_za;
  }

  inline
  double
  FTTrack::getDr(void) const
  {
    return (*m_helix)[0];
  }

  inline
  double
  FTTrack::getPhi0(void) const
  {
    return (*m_helix)[1];
  }

  inline
  double
  FTTrack::getKappa(void) const
  {
    return (*m_helix)[2];
  }

  inline
  double
  FTTrack::getDz(void) const
  {
    return (*m_helix)[3];
  }

  inline
  double
  FTTrack::getTanL(void) const
  {
    return (*m_helix)[4];
  }

  inline
  const TVectorD&
  FTTrack::getHelix(void) const
  {
    return *m_helix;
  }

  inline
  double
  FTTrack::SigmaZ(double z)
  {
    return m_SigmaZ += z;
  }

  inline
  double
  FTTrack::SigmaS(double s)
  {
    return m_SigmaS += s;
  }

  inline
  double
  FTTrack::SigmaSZ(double sz)
  {
    return m_SigmaSZ += sz;
  }

  inline
  double
  FTTrack::SigmaSS(double ss)
  {
    return m_SigmaSS += ss;
  }

  inline
  double
  FTTrack::deltaZ(double s, double z) const
  {
    int n = m_stereoSegments->length();
    if (!n) return 0;
    double Dz = (n == 1)
                ? z - s * m_SigmaZ / m_SigmaS
                : z - (s * (m_SigmaSZ - m_SigmaS * m_SigmaZ) + m_SigmaSS * m_SigmaZ - m_SigmaS * m_SigmaSZ)
                / (m_SigmaSS - m_SigmaS * m_SigmaS);
    return (Dz < 10 * (7 - n)) ? Dz : 9999.;
  }

  inline
  double
  FTTrack::rPhiFitChi2() const
  {
    return m_rPhiFitChi2;
  }

  inline
  double
  FTTrack::szFitChi2() const
  {
    return m_szFitChi2;
  }

  inline
  void
  FTTrack::updateSZ(void)
  {
    int n = m_stereoSegmentsCache->length();
    if (n == 1) {
      FTSegment* s = m_stereoSegmentsCache->first();
      double sTmp = s->s();
      double zTmp = s->z();
      if (deltaZ(sTmp, zTmp) < 9998.) {
        m_SigmaS += sTmp;
        m_SigmaZ += zTmp;
        m_SigmaSS += sTmp * sTmp;
        m_SigmaSZ += sTmp * zTmp;
        m_stereoSegments->append(s);
      }
      m_stereoSegmentsCache->clear();
    } else if (n) {
      m_stereoSegmentsBySuperLayer->append(m_stereoSegmentsCache);
      m_stereoSegmentsCache = new FTList<FTSegment*>(3);
    }
  }

}

#endif /* FTTrack_FLAG_ */

