#ifndef CDCTRIGGERTRACK_H
#define CDCTRIGGERTRACK_H

#include <framework/dataobjects/Helix.h>
#include <cmath>

namespace Belle2 {

  /** Track created by the CDC trigger. */
  class CDCTriggerTrack : public Helix {
  public:
    /** default constructor, initializing everything to 0. */
    CDCTriggerTrack(): Helix(), m_chi2D(0.), m_chi3D(0.), m_time(0) { }

    /** 2D constructor, initializing 3D values to 0.
     *  @param phi0   The angle between the transverse momentum and the x axis and in [-pi, pi].
     *  @param omega  The signed curvature of the track where the sign is given by the charge of the particle.
     *  @param chi2   Chi2 value of the 2D fit.
     */
    CDCTriggerTrack(double phi0, double omega, double chi2, short time = 0):
      Helix(0., phi0, omega, 0., 0.), m_chi2D(chi2), m_chi3D(0.), m_time(time) { }

    /** 3D constructor
     *  @param phi0      The angle between the transverse momentum and the x axis and in [-pi, pi].
     *  @param omega     The signed curvature of the track where the sign is given by the charge of the particle.
     *  @param chi2D     Chi2 value of the 2D fit.
     *  @param z0        The z coordinate of the perigee.
     *  @param cotTheta  The slope of the track in the sz plane (dz/ds).
     *  @param chi3D     Chi2 value of the 3D fit.
     */
    CDCTriggerTrack(double phi0, double omega, double chi2D,
                    double z0, double cotTheta, double chi3D):
      Helix(0., phi0, omega, z0, cotTheta), m_chi2D(chi2D), m_chi3D(chi3D), m_time(0) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerTrack() { }

    // accessors
    /** get chi2 value of 2D fitter */
    float getChi2D() const { return m_chi2D; }
    /** get chi2 value of 3D fitter */
    float getChi3D() const { return m_chi3D; }
    /** get the track found time */
    float getTime() const { return m_time; }

  protected:
    /** chi2 value from 2D fitter */
    float m_chi2D;
    /** chi2 value from 3D fitter */
    float m_chi3D;
    /** number of trigger clocks of (the track output - L1 trigger)*/
    short m_time;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerTrack, 5);
  };
}
#endif
