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

    //! Drigt length of this digit.
    double m_driftLength;

    //! Charge of this digit.
    double m_charge;

    //! The method to set layer id
    void setLayerId(int layerId) { m_layerId = layerId; }

    //! The method to set wire id
    void setWireId(int wireId) { m_wireId = wireId; }

    //! The method to set time
    void setDriftLength(double driftLength) { m_driftLength = driftLength; }

    //! The method to set charge
    void setCharge(double charge) { m_charge = charge; }

    //! The method to get layer id
    int getLayerId() { return m_layerId; }

    //! The method to get wire id
    int getWireId() { return m_wireId; }

    //! The method to get time
    double getDriftLength() { return m_driftLength; }

    //! The method to get charge
    double getCharge() { return m_charge; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    HitCDC() {;}

    //! Useful Constructor
    HitCDC(int layerId,
           int wireId,
           double driftLength,
           double charge) {
      m_layerId = layerId;
      m_wireId = wireId;
      m_driftLength = driftLength;
      m_charge = charge;
    }

    ClassDef(HitCDC, 1);

  };

} // end namespace Belle2

#endif
