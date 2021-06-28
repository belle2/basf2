/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <TObject.h>
#include <cmath>

namespace Belle2 {

  /**
     * Storage element for the eventwise T0 estimation computed on HLT.
     * It can store a float value with uncertainty together with the detector component.
     */
  class OnlineEventT0 : public TObject {

  public:

    /** Constructor.
         * @param eventT0 EventT0 computed on HLT
         * @param eventT0Uncertainty uncertainity on EventT0 computed on HLT
         * @param detector detector which provided the EventT0
         */
    OnlineEventT0(float eventT0, float eventT0Uncertainty, const Const::DetectorSet& detector)
      : m_eventT0(eventT0)
      , m_eventT0Uncertainty(eventT0Uncertainty)
      , m_detector(detector)
    {}

    /// Return the online event t0.
    float getOnlineEventT0() const {return m_eventT0;}

    /// Return the online event t0 uncertainty.
    float getOnlineEventT0Uncertainty() const {return m_eventT0Uncertainty;}

    /// Return the online event t0 detector.
    Const::DetectorSet getOnlineEventT0Detector() {return m_detector;}

  private:

    /// Storage of the online T0 estimation.
    float m_eventT0 = NAN;

    /// Storage of the uncertainty of the online T0 estimation.
    float m_eventT0Uncertainty = NAN;

    /// Storage of the detector, who has determined the event T0.
    Const::DetectorSet m_detector;

    ClassDef(OnlineEventT0, 1) /**< Storage element for the eventwise T0 estimation on HLT. */
  };
}
