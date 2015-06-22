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
#ifndef FRAMEWORK_DATAOBJECTS_BEAMPARAMETERS_H
#define FRAMEWORK_DATAOBJECTS_BEAMPARAMETERS_H

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
    BeamParameters(): MCInitialParticles(), m_covHER(3), m_covLER(3), m_covVertex(3) {}

    /** Set the covariance matrix for HER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    void setCovHER(const TMatrixDSym& cov) { m_covHER = cov; }
    /** Set the covariance matrix for LER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    void setCovLER(const TMatrixDSym& cov) { m_covLER = cov; }
    /** Set the covariance matrix of the vertex position */
    void setCovVertex(const TMatrixDSym& cov) { m_covVertex = cov; }

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
     * @param angle angle between beam direction and z axis. Negative values are treatet as M_PI - abs(angle)
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
     * @param angle angle between beam direction and z axis. Negative values are treatet as M_PI - abs(angle)
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
     *     the matrix will not be checked but just the lower triangle will be used.
     *
     * @param vertex vertex position
     * @param cov entries of the covariance matrix.
     */
    void setVertex(const TVector3& vertex, const std::vector<double>& cov);

    /** Get the covariance matrix of HER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    const TMatrixDSym& getCovHER() const { return m_covHER; }
    /** Get the covariance matrix of LER (E, theta_x, theta_y) where E is the
     * energy, theta_x is the horizontal angle between nominal direction and
     * actual direction in spread and theta_y is the vertical angle */
    const TMatrixDSym& getCovLER() const { return m_covLER; }
    /** Get the covariance matrix of the vertex position */
    const TMatrixDSym& getCovVertex() const { return m_covVertex; }

    /** Return energy smearing of LER */
    //double getEnergySmearingLER() const;
    /** Return energy smearing of LER */
    //double getEnergySmearingHER() const;
    /** Return energy smearing of the CMS */
    //double getEnergySmearingCMS() const;

  private:
    /** Calculate FourVector of a beam from energy and angle wrt the z-axis.
     * Negative angles will be treated as angle = M_PI - fabs(angle)
     * @param energy beam energy
     * @param angle angle wrt z-axis
     */
    TLorentzVector getFourVector(double energy, double angle);
    /** Obtain covariance matrix from vector of entries.
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
     * @param cov entries for the covariance matrix
     * @return covariance matrix determined from the given entries
     */
    TMatrixDSym getCovMatrix(const std::vector<double>& cov, bool common);
    /** Covariance matrix of the high energy beam at the IP */
    TMatrixDSym m_covHER;
    /** Covariance matrix of the low energy beam at the IP */
    TMatrixDSym m_covLER;
    /** Covariance matrix of the vertex position */
    TMatrixDSym m_covVertex;
    /** ROOT Dictionary */
    ClassDef(BeamParameters, 1);
  };

} //Belle2 namespace
#endif // FRAMEWORK_DATAOBJECTS_BEAMPARAMETERS_H
