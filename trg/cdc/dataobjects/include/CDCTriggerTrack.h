/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERTRACK_H
#define CDCTRIGGERTRACK_H

#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>
#include <TVector3.h>

namespace Belle2 {

  /** Track created by the CDC trigger. */
  class CDCTriggerTrack : public Helix {
  public:
    /** default constructor, initializing everything to 0. */
    CDCTriggerTrack(): Helix(), m_chi2D(0.), m_chi3D(0.), m_time(0), m_quadrant(-1), m_foundoldtrack(6, false), m_driftthreshold(9,
          false), m_valstereobit(false) , m_expert(-1), m_tsvector(9, false), m_qualityvector(0) { }

    /** 2D constructor, initializing 3D values to 0.
     *  @param phi0      The angle between the transverse momentum and the x axis and in [-pi, pi].
     *  @param omega     The signed curvature of the track where the sign is given by the charge of the particle.
     *  @param chi2      Chi2 value of the 2D fit.
     *  @param time      found time for firmware tracks.
     *  @param quadrant  iTracker of the unpacked quadrant.
     *
     */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    CDCTriggerTrack(double phi0, double omega, double chi2,
                    const std::vector<bool>& foundoldtrack,
                    const std::vector<bool>& driftthreshold,
                    bool valstereobit = false,
                    int expert = -1,
                    short time = 0,
                    short quadrant = -1):
      Helix(0., phi0, omega, 0., 0.), m_chi2D(chi2), m_chi3D(0.),
      m_time(time),
      m_quadrant(quadrant),
      m_foundoldtrack(foundoldtrack),
      m_driftthreshold(driftthreshold),
      m_valstereobit(valstereobit),
      m_expert(expert),
      m_tsvector(9, false),
      m_qualityvector(0) { }

    CDCTriggerTrack(double phi0, double omega, double chi2,
                    short time = 0,
                    short quadrant = -1):
      Helix(0., phi0, omega, 0., 0.), m_chi2D(chi2), m_chi3D(0.),
      m_time(time),
      m_quadrant(quadrant),
      m_foundoldtrack(6, false),
      m_driftthreshold(9, false),
      m_valstereobit(false),
      m_expert(-1),
      m_tsvector(9, false),
      m_qualityvector(0) { }

    /** 3D constructor
     *  @param phi0      The angle between the transverse momentum and the x axis and in [-pi, pi].
     *  @param omega     The signed curvature of the track where the sign is given by the charge of the particle.
     *  @param chi2D     Chi2 value of the 2D fit.
     *  @param z0        The z coordinate of the perigee.
     *  @param cotTheta  The slope of the track in the sz plane (dz/ds).
     *  @param chi3D     Chi2 value of the 3D fit.
     *  @param time      found time for firmware tracks.
     *  @param quadrant  iTracker of the unpacked quadrant.
     */
    CDCTriggerTrack(double phi0, double omega, double chi2D,
                    double z0, double cotTheta, double chi3D,
                    const std::vector<bool>& foundoldtrack = std::vector<bool>(6, false),
                    const std::vector<bool>& driftthreshold = std::vector<bool>(9, false),
                    bool valstereobit = false,
                    int expert = -1,
                    const std::vector<bool>& tsvector = std::vector<bool>(9, false),
                    short time = 0, short quadrant = -1,
                    unsigned qualityvector = 0):
      Helix(0., phi0, omega, z0, cotTheta), m_chi2D(chi2D), m_chi3D(chi3D), m_time(time), m_quadrant(quadrant),
      m_foundoldtrack(foundoldtrack),
      m_driftthreshold(driftthreshold),
      m_valstereobit(valstereobit),
      m_expert(expert),
      m_tsvector(tsvector),
      m_qualityvector(qualityvector) { }
    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerTrack() { }

    // accessors
    /** get chi2 value of 2D fitter */
    float getChi2D() const { return m_chi2D; }
    /** get chi2 value of 3D fitter */
    float getChi3D() const { return m_chi3D; }
    /** get the track found time */
    float getTime() const { return m_time; }
    /** get the absolute value of the transverse momentum at the perigee
        assuming d0 = 0
     */
    double getPt() const
    {
      const double bField = BFieldManager::getField(TVector3(0, 0, getZ0())).Z() / Unit::T;
      return getTransverseMomentum(bField);
    }
    /** get the quadrant */
    short getQuadrant()
    {
      return m_quadrant;
    }
    /** returns true, if old 2dtrack was found */
    std::vector<bool> getFoundOldTrack() const {return m_foundoldtrack;}

    /** returns true, if at least 3 stereo ts were found */
    bool getValidStereoBit() const {return m_valstereobit;}

    /** returns true, if the drift time was fitted into the time window */
    std::vector<bool> getDriftThreshold() const {return m_driftthreshold;}

    /** return sl pattern of neurotrack */
    int getExpert() const {return m_expert;}

    /** return the vector of used Track Segments.
     *  The First bit is the innermost TS, the last bit the outermost. */
    std::vector<bool> getTSVector() const {return m_tsvector;}
    /** setter and getter for the quality vector. For the setter, the given
     * uint is xored with the current qualityvector, thus all bits with
     * a 1 are changed.
     */
    void setQualityVector(const unsigned newbits)
    {
      m_qualityvector = m_qualityvector ^ newbits;
    }
    unsigned getQualityVector() const {return m_qualityvector;}

  protected:
    /** chi2 value from 2D fitter */
    float m_chi2D;
    /** chi2 value from 3D fitter */
    float m_chi3D;
    /** number of trigger clocks of (the track output - L1 trigger)*/
    short m_time;
    /** iTracker of the unpacked quadrant*/
    short m_quadrant;
    /** array to store wether an old 2dtrack was found */
    std::vector<bool> m_foundoldtrack;
    /** store if drift time was within the timing window */
    std::vector<bool> m_driftthreshold;
    /** store if at least 3 valid stereo ts were found in the NNInput */
    bool m_valstereobit;
    /** store value for used expert network */
    int m_expert;
    /** store which track segments were used.
     *  The First bit is the innermost TS, the last bit the outermost. */
    std::vector<bool> m_tsvector;
    /** store bits for different quality flags.
     * 2^0 : 0 if all axial ts are contained in the related 2dfindertrack; 1 otherwise.
     */
    unsigned m_qualityvector;
    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerTrack, 9);
  };
}
#endif
