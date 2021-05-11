/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCRECOTRACK_H
#define MICROTPCRECOTRACK_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {
  /**
  * ClassMicrotpcRecoTrack - fit tracks after the digitization
  */
  class MicrotpcRecoTrack : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    MicrotpcRecoTrack(): m_detNb(0), m_pixnb(0), m_chi2(0), m_theta(0), m_phi(0), m_esum(0), m_totsum(0), m_trl(0), m_time_range(0),
      m_parFit(), m_parFit_err(), m_cov(), m_impact_x() , m_impact_y(), m_side(), m_partID() {}

    /** Standard constructor
     * @param detNb detector number
     * @param pixnb pixel number
     * @param chi2 the chi^2
     * @param theta the polar angle in degrees
     * @param phi the azimuthal angle in degrees
     * @param esum total ionization energy
     * @param totsum tot sum
     * @param trl track length
     * @param time_range trigger/time length
     * @param parFit the fit parameters
     * @param parFit_err the fit parameter errors
     * @param cov covariant
     * @param impact_x impact parameter x
     * @param impact_y impact parameter y
     * @param side which side was/were hit
     * @param partID raw part ID
     */
    MicrotpcRecoTrack(int detNb, int pixnb, float chi2, float theta, float phi, float esum, int totsum, float trl, int time_range,
                      const float parFit[5], const float parFit_err[5], const float cov[25], const float impact_x[4], const float impact_y[4],
                      const int side[16], const int partID[6]):
      m_detNb(detNb), m_pixnb(pixnb), m_chi2(chi2), m_theta(theta), m_phi(phi), m_esum(esum), m_totsum(totsum), m_trl(trl),
      m_time_range(time_range)
    {
      std::copy(parFit, parFit + 5, m_parFit);
      std::copy(parFit_err, parFit_err + 5, m_parFit_err);
      //std::copy(&cov[0][0], &cov[0][0] + 5 * 5, &m_cov[0][0]);
      std::copy(cov, cov + 25, m_cov);
      std::copy(impact_x, impact_x + 4, m_impact_x);
      std::copy(impact_y, impact_y + 4, m_impact_y);
      //std::copy(&side[0][0], &side[0][0] + 4 * 4, &m_side[0][0]);
      std::copy(side, side + 16, m_side);
      std::copy(partID, partID + 6, m_partID);
    }
    /** Return detector number */
    int getdetNb() const { return m_detNb; }
    /** Return pixel number */
    int getpixnb() const { return m_pixnb; }
    /** Return the chi^2 */
    float getchi2() const { return m_chi2; }
    /** Return the polar angle in degrees */
    float gettheta() const { return m_theta; }
    /** Return the azimuthal angle in degrees */
    float getphi() const { return m_phi; }
    /** Return the fit parameters */
    //float parFit[4];
    const float* getparFit() const { return m_parFit; }
    /** Return the fit parameter errors */
    const float* getparFit_err() const { return m_parFit_err; }
    /** Return covariant */
    //const float* getcov() const { return m_cov[5];}
    const float* getcov() const { return m_cov;}
    /** Return total ionization energy */
    float getesum() const { return m_esum; }
    /** Return track length */
    float gettrl() const { return m_trl; }
    /** Return tot sum */
    int gettotsum() const { return m_totsum; }
    /** Return trigger/time length */
    int gettime_range() const { return m_time_range; }
    /** Return impact parameter x */
    const float* getimpact_x() const { return m_impact_x; }
    /** Return impact parameter y */
    //void getimpact_y(float impact_y[4]) const { m_impact_y = impact_x; return m_impact_y; }
    const float* getimpact_y() const { return m_impact_y; }
    /** Return which side was/were hit */
    //void getside(int ** side) const { m_side = side; return m_side; }
    //const int* getside() const { return m_side[4]; }
    const int* getside() const { return m_side; }
    /** Return raw part ID */
    const int* getpartID() const { return m_partID; }

  private:
    /** detector number */
    int m_detNb;
    /** pixel number */
    int m_pixnb;
    /** chi^2 of the fit */
    float m_chi2;
    /** Polar angle theta in degrees */
    float m_theta;
    /** Azimuthal angle phi in degrees */
    float m_phi;
    /** total ionization energy */
    float m_esum;
    /** TOT sum */
    int m_totsum;
    /** track length */
    float m_trl;
    /** Trigger/time length */
    int m_time_range;
    /** Fit parameters */
    float m_parFit[5];
    /** Fit paramteter errors */
    float m_parFit_err[5];
    /** Covariant errors */
    float m_cov[25];
    /** Impact parameter x */
    float m_impact_x[4];
    /** Impact parameter y */
    float m_impact_y[4];
    /** Which side was/were hit */
    //int m_side[4][4];
    int m_side[16];
    /** Raw particle ID */
    int m_partID[6];

    ClassDef(MicrotpcRecoTrack, 1)
  };

} // end namespace Belle2

#endif
