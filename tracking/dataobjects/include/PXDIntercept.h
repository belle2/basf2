/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDINTERCEPT_H
#define PXDINTERCEPT_H

#include <framework/datastore/RelationsObject.h>
#include <GFTrackCand.h>
#include <framework/datastore/RelationsObject.h>
//#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>
#include <pxd/geometry/SensorInfo.h>

namespace Belle2 {

  /** PXDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   * with a PXD sensor.
   */
  class PXDIntercept : public RelationsObject {

  public:

    /**
     * default constructor.
     */
    PXDIntercept() {};

    /**
     * destructor.
     */
    ~PXDIntercept() {};

    double getCoorU() const { return m_coorU; }
    double getCoorV() const { return m_coorV; }
    double getSigmaU() const { return m_sigmaU; }
    double getSigmaV() const { return m_sigmaV; }
    double getSigmaUprime() const { return m_sigmaUprime; }
    double getSigmaVprime() const { return m_sigmaVprime; }
    double getLambda() const { return m_lambda; }
    VxdID::baseType getSensorID() const { return m_sensorID; }

    void setCoorU(double user_coorU) { m_coorU = user_coorU; }
    void setCoorV(double user_coorV) { m_coorV = user_coorV; }
    void setSigmaU(double user_sigmaU) { m_sigmaU = user_sigmaU; }
    void setSigmaV(double user_sigmaV) { m_sigmaV = user_sigmaV; }
    void setSigmaUprime(double user_sigmaUprime) { m_sigmaUprime = user_sigmaUprime; }
    void setSigmaVprime(double user_sigmaVprime) { m_sigmaVprime = user_sigmaVprime; }
    void setLambda(double user_lambda) { m_lambda = user_lambda; }
    void setVxdID(VxdID::baseType user_vxdID) { m_sensorID = user_vxdID; }

  private:

    double m_coorU; /**< u coordinate of the intercept */
    double m_coorV; /**< v coordinate of the intercept */
    double m_sigmaU; /**< statistical error of the extrapolation along the u coordinate */
    double m_sigmaV; /**< statistical error of the extrapolation along the v coordinate */
    double m_sigmaUprime; /**< statistical error of the extrapolation along the u coordinate */
    double m_sigmaVprime; /**< statistical error of the extrapolation along the v coordinate */
    double m_lambda; /**< length of the track*/
    //    ROIDetPlane m_plane; //! /**< plane of the extrapolation */

    VxdID::baseType m_sensorID; /**< sensor ID */


    ClassDef(PXDIntercept, 1);
  };
}
#endif  // PXDINTERCEPT_H  
