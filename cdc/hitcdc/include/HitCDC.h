/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HITCDC_H
#define HITCDC_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class HitCDC : public TObject {
  public:

    //! The layer id of this digit.
    int m_layerId;

    //! The wire id of this digit.
    int m_wireId;

    //! Left drift length of this digit.
    double m_leftDriftLength;

    //! Right drift length of this digit.
    double m_rightDriftLength;

    //! Charge of this digit.
    double m_charge;

    //! The method to set layer id
    void setLayerId(int layerId) { m_layerId = layerId; }

    //! The method to set wire id
    void setWireId(int wireId) { m_wireId = wireId; }

    //! The method to set left drift length
    void setLeftDriftLength(double leftDriftLength) { m_leftDriftLength = leftDriftLength; }

    //! The method to set right drift length
    void setRightDriftLength(double rightDriftLength) { m_rightDriftLength = rightDriftLength; }

    //! The method to set charge
    void setCharge(double charge) { m_charge = charge; }

    //! The method to get layer id
    int getLayerId() const { return m_layerId; }

    //! The method to get wire id
    int getWireId() const { return m_wireId; }

    //! The method to get left drift length
    double getLeftDriftLength() const { return m_leftDriftLength; }

    //! The method to get right drift length
    double getRightDriftLength() const { return m_rightDriftLength; }

    //! The method to get charge
    double getCharge() const { return m_charge; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    HitCDC() {;}

    //! Useful Constructor
    HitCDC(int layerId,
           int wireId,
           double leftDriftLength,
           double rightDriftLength,
           double charge) {
      m_layerId = layerId;
      m_wireId = wireId;
      m_leftDriftLength = leftDriftLength;
      m_rightDriftLength = rightDriftLength;
      m_charge = charge;
    }

    ClassDef(HitCDC, 1);

  };

} // end namespace Belle2

#endif
