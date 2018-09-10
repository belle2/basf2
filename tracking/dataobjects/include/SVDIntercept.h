/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/VXDIntercept.h>

namespace Belle2 {

  /** SVDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   *   with an SVD sensor.
   */
  class SVDIntercept : public VXDIntercept {

  public:

    double getUprime() const { return m_Uprime; } /**< return the U direction tangent of the track extrapolated to the sensor */
    double getVprime() const { return m_Vprime; } /**< return the V direction tangent of the track extrapolated to the sensor */

    void setUprime(double user_Uprime) { m_Uprime = user_Uprime; } /**< set the U direction tangent of the track extrapolated to the sensor */
    void setVprime(double user_Vprime) { m_Vprime = user_Vprime; } /**< set the V direction tangent of the track extrapolated to the sensor */

  private:

    double m_Uprime; /**< U direction tangent of the track extrapolated to the sensor */
    double m_Vprime; /**< V direction tangent of the track extrapolated to the sensor */

    //! Needed to make the ROOT object storable
    ClassDef(SVDIntercept, 3);
  };
}
