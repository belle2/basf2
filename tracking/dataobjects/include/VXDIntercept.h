/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  /** VXDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   *   with a VXD sensor.
   */
  class VXDIntercept : public RelationsObject {

  public:

    /** Default constructor for I/O.
     */
    VXDIntercept(): m_coorU(0), m_coorV(0), m_sigmaU(0), m_sigmaV(0),
      m_sigmaUprime(0), m_sigmaVprime(0), m_lambda(0), m_sensorID() {}

    /** Explicit constructor providing coordinates and sensorID */
    VXDIntercept(double coorU, double coorV, VxdID sensorid) :
      m_coorU(coorU),
      m_coorV(coorV),
      m_sigmaU(0),
      m_sigmaV(0),
      m_sigmaUprime(0),
      m_sigmaVprime(0),
      m_lambda(0),
      m_sensorID(sensorid) {}

    /** Explicit constructor providing all values */
    VXDIntercept(double coorU, double coorV, double sigmaU, double sigmaV,
                 double sigmaUprime, double sigmaVprime, double lambda, VxdID sensorid) :
      m_coorU(coorU),
      m_coorV(coorV),
      m_sigmaU(sigmaU),
      m_sigmaV(sigmaV),
      m_sigmaUprime(sigmaUprime),
      m_sigmaVprime(sigmaVprime),
      m_lambda(lambda),
      m_sensorID(sensorid) {}

    /** Destructor.
     */
    ~VXDIntercept() {}

    double getCoorU() const { return m_coorU; } /**< return the U coordinate of the intercept */
    double getCoorV() const { return m_coorV; } /**< return the V coordinate of the intercept */
    double getSigmaU() const { return m_sigmaU; } /**< return the statistical error on the U coordinate of the intercept */
    double getSigmaV() const { return m_sigmaV; } /**< return the statistical error on the V coordinate of the intercept */
    double getSigmaUprime() const { return m_sigmaUprime; } /**< return the statistical error of the extrapolation of U prime */
    double getSigmaVprime() const { return m_sigmaVprime; } /**< return the statistical error of the extrapolation of V prime */
    double getLambda() const { return m_lambda; } /**< return the length of the track*/
    VxdID::baseType getSensorID() const { return m_sensorID; } /**< return the sensor ID */

    void setCoorU(double user_coorU) { m_coorU = user_coorU; } /**< set the U coordinate of the intercept */
    void setCoorV(double user_coorV) { m_coorV = user_coorV; } /**< set the V coordinate of the intercept */
    void setSigmaU(double user_sigmaU) { m_sigmaU = user_sigmaU; } /**< set the statistical error on the U coordinate of the intercept */
    void setSigmaV(double user_sigmaV) { m_sigmaV = user_sigmaV; }/**< set the statistical error on the V coordinate of the intercept */
    void setSigmaUprime(double user_sigmaUprime) { m_sigmaUprime = user_sigmaUprime; } /**< set the statistical error of the extrapolation of U prime */
    void setSigmaVprime(double user_sigmaVprime) { m_sigmaVprime = user_sigmaVprime; } /**< set the statistical error of the extrapolation of V prime */
    void setLambda(double user_lambda) { m_lambda = user_lambda; }  /**< set the length of the track*/
    void setVxdID(VxdID::baseType user_vxdID) { m_sensorID = user_vxdID; } /**< set the sensor ID */

  private:

    double m_coorU; /**< u coordinate of the intercept */
    double m_coorV; /**< v coordinate of the intercept */
    double m_sigmaU; /**< statistical error of the extrapolation along the u coordinate */
    double m_sigmaV; /**< statistical error of the extrapolation along the v coordinate */
    double m_sigmaUprime; /**< statistical error of the extrapolation of U prime */
    double m_sigmaVprime; /**< statistical error of the extrapolation of V prime */
    double m_lambda; /**< length of the track*/

    VxdID::baseType m_sensorID; /**< sensor ID */

    //! Needed to make the ROOT object storable
    ClassDef(VXDIntercept, 1);
  };
}
