/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TMatrixDSym.h>

namespace Belle2 {
  /** This class contains the measured values of the orientation of the collision axis in the CM system obtained by pure Lorentz boost
   * angleXZ = atan(pHERcms.X / pHERcms.Z)
   * angleYZ = atan(pHERcms.Y / pHERcms.Z)
   * where pHERcms is HER momentum in CM system obtained by boost.
   * These angles do not have sharp value, but vary event-by-event due to the spread of HER/LER momenta.
   * Therefore, both central values and spread are stored together with the uncertainties.
   *
   * This class is needed for Lorentz transformation to the standard CM system, where particles collide along z-axis.
   */
  class CollisionAxisCMS: public TObject {
  public:

    /** equality operator */
    bool operator==(const CollisionAxisCMS& other) const
    {
      return other.m_angleXZ == m_angleXZ && other.m_angleYZ == m_angleYZ &&
             other.m_centerCovariance == m_centerCovariance && other.m_spreadCovariance == m_spreadCovariance &&
             other.m_spreadXZunc == m_spreadXZunc && other.m_spreadYZunc == m_spreadYZunc && other.m_spreadPhiUnc == m_spreadPhiUnc;
    }

    /** Set the central values and uncertainty of them */
    void setAngles(double angleXZ, double angleYZ, const TMatrixDSym& centerCovariance)
    {
      m_angleXZ = angleXZ;
      m_angleYZ = angleYZ;
      m_centerCovariance = centerCovariance;
    }

    /** Set spread covariance and uncertainties of the eigenvalues of this matrix */
    void setSpread(const TMatrixDSym& spreadCovariance, double spreadXZunc, double spreadYZunc, double spreadPhiUnc)
    {
      m_spreadCovariance = spreadCovariance;
      m_spreadXZunc  = spreadXZunc;
      m_spreadYZunc  = spreadYZunc;
      m_spreadPhiUnc = spreadPhiUnc;
    }


    /** Get the mean XZ angle of the HER beam in the CM frame obtained by pure boost */
    double getAngleXZ() const { return m_angleXZ; }

    /** Get the mean YZ angle of the HER beam in the CM frame obtained by pure boost */
    double getAngleYZ() const { return m_angleYZ; }


    /**  Get the covariance matrix describing uncertainties of m_angleXZ and m_angleYZ */
    const TMatrixDSym& getCenterCovariance() const { return m_centerCovariance; }

    /**  Get the covariance matrix describing "natural" spread of angleXZ and angleYZ */
    const TMatrixDSym& getSpreadCovariance() const { return m_spreadCovariance; }

    /**  Get the uncertainty of the eigenvalue of m_spreadCovariance corresponding to eigenvector closer to x-axis  */
    double getSpreadXZunc() const { return m_spreadXZunc; }

    /**  Get the uncertainty of the eigenvalue of m_spreadCovariance corresponding to eigenvector closer to y-axis  */
    double getSpreadYZunc() const { return m_spreadYZunc; }

    /** Get the uncertainty of the angle of the eigenvector of m_spreadCovariance */
    double getSpreadPhiUnc() const { return m_spreadPhiUnc; }

  private:

    // central values
    double m_angleXZ; ///< the XZ angle of the HER beam in the CM system obtained by pure boost
    double m_angleYZ; ///< the YZ angle of the HER beam in the CM system obtained by pure boost

    // uncertainty of central values
    TMatrixDSym m_centerCovariance{2}; ///< Covariance matrix describing uncertainties of m_angleXZ and m_angleYZ

    // spread of the values
    TMatrixDSym m_spreadCovariance{2}; ///< Covariance matrix describing "natural" spread of m_angleXZ and m_angleYZ

    // uncertainties of spread of the values
    double m_spreadXZunc;    ///< Uncertainty of the eigenvalue of m_spreadCovariance corresponding to eigenvector closer to x-axis
    double m_spreadYZunc;    ///< Uncertainty of the eigenvalue of m_spreadCovariance corresponding to eigenvector closer to y-axis
    double m_spreadPhiUnc;   ///< Uncertainty of the angle of the eigenvector of m_spreadCovariance

    ClassDef(CollisionAxisCMS, 1);  /**<  direction of collision axis in CMS and its spread **/
  };

} //Belle2 namespace
