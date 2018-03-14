/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/MCInitialParticles.h>
#include <TMatrixDSym.h>

namespace Belle2 {
  /** This class contains the nominal beam parameters and the parameters used for
   * smearing of the primary vertex and the beam energy and momentum. It is event
   * independent (but might be run or even sub run dependent)
   */
  class BeamParameters: public MCInitialParticles {
  public:
    using MCInitialParticles::setLER;
    using MCInitialParticles::setHER;
    using MCInitialParticles::setVertex;

    /** default constructor */
    BeamParameters(): MCInitialParticles(), m_covHER{0}, m_covLER{0}, m_covVertex{0} {}
    /** copy constructor */
    BeamParameters(const BeamParameters& b): MCInitialParticles(b)
    {
      std::copy_n(b.m_covHER, 6, m_covHER);
      std::copy_n(b.m_covLER, 6, m_covLER);
      std::copy_n(b.m_covVertex, 6, m_covVertex);
    }
    /** assignment operator */
    BeamParameters& operator=(const BeamParameters& b)
    {
      MCInitialParticles::operator=(b);
      std::copy_n(b.m_covHER, 6, m_covHER);
      std::copy_n(b.m_covLER, 6, m_covLER);
      std::copy_n(b.m_covVertex, 6, m_covVertex);
      return *this;
    }
    /** equality operator */
    bool operator==(const BeamParameters& b) const
    {
      // since we only save the covariance matrices with float precision we
      // need to also do the comparison with float precision.
      auto floatcmp = [](double dbl_a, double dbl_b) { return (float)dbl_a == (float)dbl_b; };
      return MCInitialParticles::operator==(b) &&
             std::equal(m_covHER, m_covHER + 6, b.m_covHER, floatcmp) &&
             std::equal(m_covLER, m_covLER + 6, b.m_covLER, floatcmp) &&
             std::equal(m_covVertex, m_covVertex + 6, b.m_covVertex, floatcmp);
    }

    /** Set the covariance matrix for HER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle
     * The upper triangle will be saved. */
    void setCovHER(const TMatrixDSym& cov) { setCovMatrix(m_covHER, cov); }
    /** Set the covariance matrix for LER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle.
     * The upper triangle will be saved. */
    void setCovLER(const TMatrixDSym& cov) { setCovMatrix(m_covLER, cov); }
    /** Set the covariance matrix of the vertex position. The upper triangle will be saved. */
    void setCovVertex(const TMatrixDSym& cov) { setCovMatrix(m_covVertex, cov); }

    /** Set the HER FourVector and error matrix from beam energy, angle and covariance entries.
     *
     * The vector for the covariance matrix can have either 0, 1, 3, 6 or 9 entries:
     *   - 0 entries means no error.
     *   - 1 entry will be treated as the variance of the beam energy.
     *   - 3 entries will be treated as uncorrelated variances for all variables (diagonal of the matrix)
     *   - 6 entries will be interpreted as the upper triangle of the covariance matrix
     *     with the element order being (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
     *   - 9 entries for the full covariance matrix in row-major order. The symmetry of
     *     the matrix will not be checked but just the lower triangle will be used.
     *
     * @param energy beam energy
     * @param angle angle between beam direction and z axis. Negative values are treated as M_PI - abs(angle)
     * @param cov entries of the covariance matrix.
     */
    void setHER(double energy, double angle, const std::vector<double>& cov);

    /** Set the LER FourVector and error matrix from beam energy, angle and covariance entries.
     *
     * The vector for the covariance matrix can have either 0, 1, 3, 6 or 9 entries:
     *   - 0 entries means no error.
     *   - 1 entry will be treated as the variance of the beam energy.
     *   - 3 entries will be treated as uncorrelated variances for all variables (diagonal of the matrix)
     *   - 6 entries will be interpreted as the upper triangle of the covariance matrix
     *     with the element order being (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
     *   - 9 entries for the full covariance matrix in row-major order. The symmetry of
     *     the matrix will not be checked but just the lower triangle will be used.
     *
     * @param energy beam energy
     * @param angle angle between beam direction and z axis. Negative values are treated as M_PI - abs(angle)
     * @param cov entries of the covariance matrix.
     */
    void setLER(double energy, double angle, const std::vector<double>& cov);

    /** Set the vertex position and error matrix.
     *
     * The vector for the covariance matrix can have either 0, 1, 3, 6 or 9 entries:
     *   - 0 entries means no error.
     *   - 1 entry will be treated as the common variance for x, y and z
     *   - 3 entries will be treated as uncorrelated variances for all variables (diagonal of the matrix)
     *   - 6 entries will be interpreted as the upper triangle of the covariance matrix
     *     with the element order being (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
     *   - 9 entries for the full covariance matrix in row-major order. The symmetry of
     *     the matrix will not be checked but just the upper triangle will be used.
     *
     * @param vertex vertex position
     * @param cov entries of the covariance matrix.
     */
    void setVertex(const TVector3& vertex, const std::vector<double>& cov);

    /** Get the covariance matrix of HER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    TMatrixDSym getCovHER() const { return getCovMatrix(m_covHER); }
    /** Get the covariance matrix of LER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    TMatrixDSym getCovLER() const { return getCovMatrix(m_covLER); }
    /** Get the covariance matrix of the vertex position */
    TMatrixDSym getCovVertex() const { return getCovMatrix(m_covVertex); }

    /** Return energy smearing of LER */
    //double getEnergySmearingLER() const;
    /** Return energy smearing of LER */
    //double getEnergySmearingHER() const;
    /** Return energy smearing of the CMS */
    //double getEnergySmearingCMS() const;

    /** Return unique ID of BeamParameters in global Millepede calibration (1) */
    static unsigned short getGlobalUniqueID() { return 1; }

  private:
    /** Calculate FourVector of a beam from energy and angle wrt the z-axis.
     * Negative angles will be treated as angle = M_PI - fabs(angle)
     * @param energy beam energy
     * @param angle angle wrt z-axis
     */
    static TLorentzVector getFourVector(double energy, double angle);
    /** Set covariance matrix from vector of entries.
     *
     * The vector for the covariance matrix can have either 0, 1, 3, 6 or 9 entries:
     *   - 0 entries means no error.
     *   - 1 entry will be treated as the common variance for all variables
     *     if common is true. Otherwise it will be taken as the variance of
     *     just the first element.
     *   - 3 entries will be treated as uncorrelated variances for all variables
     *     (diagonal of the matrix)
     *   - 6 entries will be interpreted as the upper triangle of the covariance matrix
     *     with the element order being (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
     *   - 9 entries for the full covariance matrix in row-major order. The symmetry of
     *     the matrix will not be checked but just the lower triangle will be used.
     *
     * @param member to the member which contains the matrix
     * @param cov entries for the covariance matrix
     * @param common if true a 1-element cov will be treated as the common variance
     *        for all diagonal elements
     */
    static void setCovMatrix(Double32_t* member, const std::vector<double>& cov, bool common);
    /** Set covariance matrix from ROOT Matrix object */
    static void setCovMatrix(Double32_t* member, const TMatrixDSym& cov);
    /** Obtain covariance matrix from a given float array */
    static TMatrixDSym getCovMatrix(const Double32_t* member);
    /** Covariance matrix of the high energy beam at the IP */
    Double32_t m_covHER[6];
    /** Covariance matrix of the low energy beam at the IP */
    Double32_t m_covLER[6];
    /** Covariance matrix of the vertex position */
    Double32_t m_covVertex[6];

    ClassDef(BeamParameters, 2); /**< nominal beam and primary vertex parameters (including smearing). */
  };

} //Belle2 namespace
