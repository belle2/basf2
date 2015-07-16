#ifndef CDCTRIGGERTRACK_H
#define CDCTRIGGERTRACK_H

#include <framework/datastore/RelationsObject.h>
#include <cmath>

namespace Belle2 {

  class TRGCDCHoughPlane;

  /** Track created by the trigger finders and fitters
   *  (at the moment only 2D).
   */
  class CDCTriggerTrack : public RelationsObject {
  public:
    /** The status information for the CDCTriggerTrack. */
    enum EStatusBit {
      /** bit 0: 2D finder information set */
      c_2DFinder = 1 << 0,
      /** bit 1: 2D fitter information set */
      c_2DFitter = 1 << 1,
      /** bit 2: 3D fitter information set */
      c_3DFitter = 1 << 2,
      /** bit 3: Neuro z vertex set */
      c_NNz = 1 << 3,
      /** bit 4: Neuro theta set */
      c_NNtheta = 1 << 4
    };

    /** default constructor. */
    CDCTriggerTrack():
      m_status(0),
      m_charge(0), m_houghPt(0.), m_houghPhiC(0.),
      m_fitPt(0.), m_fitPhiC(0.), m_chi2D(0.),
      m_fitZ(0.), m_fitCot(0.), m_chi3D(0.),
      m_nnZ(0.), m_nnTheta(0.)
    { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerTrack() { }

    // accessors
    /** get charge of track */
    short getCharge() const { return m_charge; }
    /** get pt as given by 2D finder (center of hough cell) */
    float getHoughPt() const { return m_houghPt; }
    /** get phi at vertex as given by 2D finder [-pi, pi] */
    float getHoughPhiVertex() const { return remainder(m_houghPhiC + M_PI_2, 2. * M_PI); }
    /** get phi of circle center position as given by 2D finder */
    float getHoughPhiCenter() const { return m_houghPhiC; }

    /** get pt as given by 2D fitter */
    float getFitPt() const { return m_fitPt; }
    /** get phi at vertex as given by 2D fitter [-pi, pi] */
    float getFitPhiVertex() const { return remainder(m_fitPhiC + M_PI_2, 2. * M_PI); }
    /** get phi of circle center position as given by 2D fitter */
    float getFitPhiCenter() const { return m_fitPhiC; }
    /** get chi2 value of 2D fitter */
    float getChi2D() const { return m_chi2D; }

    /** get z vertex as given by 3D fitter */
    float getFitZ() const { return m_fitZ; }
    /** get cot(theta) as given by 3D fitter */
    float getFitCot() const { return m_fitCot; }
    /** get theta as given by 3D fitter */
    float getFitTheta() const { return atan2(1., m_fitCot); }
    /** get chi2 value of 3D fitter */
    float getChi3D() const { return m_chi3D; }

    /** get z vertex as given by neurotrigger */
    float getNNZ() const { return m_nnZ; }
    /** get theta as given by neurotrigger */
    float getNNTheta() const { return m_nnTheta; }

    // status checks
    /** true if status matches the given bitmask */
    bool hasStatus(unsigned short bitmask) const { return (m_status & bitmask) == bitmask; }
    /** true if track has 2D finder results */
    bool has2DHoughInfo() const { return hasStatus(c_2DFinder); }
    /** true if track has 2D fitter results */
    bool has2DFitInfo() const { return hasStatus(c_2DFitter); }
    /** true if track has 3D fitter results */
    bool has3DFitInfo() const { return hasStatus(c_3DFitter); }
    /** true if track has neuro z */
    bool hasNNZ() const { return hasStatus(c_NNz); }
    /** true if track has neuro theta */
    bool hasNNTheta() const { return hasStatus(c_NNtheta); }

    // modifiers
    /** set 2D finder information and status */
    void add2DHoughResult(short charge, float pt, float phiC)
    {
      m_charge = charge;
      m_houghPt = pt;
      m_houghPhiC = phiC;
      addStatus(c_2DFinder);
    }
    /** set 2D fitter information and status */
    void add2DFitResult(float pt, float phiC, float chi2)
    {
      m_fitPt = pt;
      m_fitPhiC = phiC;
      m_chi2D = chi2;
      addStatus(c_2DFitter);
    }
    /** set 3D fitter information and status */
    void add3DFitResult(float z, float cot, float chi2)
    {
      m_fitZ = z;
      m_fitCot = cot;
      m_chi3D = chi2;
      addStatus(c_3DFitter);
    }
    /** set neurotrigger z-vertex and status */
    void setNNZ(float z)
    {
      m_nnZ = z;
      addStatus(c_NNz);
    }
    /** set neurotrigger theta and status */
    void setNNTheta(float theta)
    {
      m_nnTheta = theta;
      addStatus(c_NNtheta);
    }

  protected:
    // status
    /** add bitmask to status */
    void addStatus(unsigned short bitmask) { m_status |= bitmask; }

    /** status encodes which information has been set */
    unsigned short m_status;
    // 2D finder information
    /** charge of track */
    short m_charge;
    /** pt from 2D finder (center of hough cell) */
    float m_houghPt;
    /** phiC from 2D finder (center of hough cell) */
    float m_houghPhiC;
    // 2D fitter information
    /** pt from 2D fitter */
    float m_fitPt;
    /** phiC from 2D fitter */
    float m_fitPhiC;
    /** chi2 value from 2D fitter */
    float m_chi2D;
    // 3D fitter information
    /** z vertex from 3D fitter */
    float m_fitZ;
    /** cot(theta) from 3D fitter */
    float m_fitCot;
    /** chi2 value from 3D fitter */
    float m_chi3D;
    // Neurotrigger information
    /** z vertex from neurotrigger */
    float m_nnZ;
    /** theta from neurotrigger */
    float m_nnTheta;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerTrack, 3);
  };
}
#endif
