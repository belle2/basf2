/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <TObject.h>

namespace Belle2 {
  /// Storage element for the eventwise T0 estimation.
  class EventT0 : public TObject {
  public:
    /// Create a new EventT0 object to store the eventwise T0 estimation.
    EventT0() { }

    /// Get the eventwise T0 estimation so far.
    double getEventT0() const
    {
      return m_eventT0;
    }

    /// Get the detector that has determined the event T0.
    const Const::EDetector& getDetector() const
    {
      return m_detector;
    }

    /// Set the T0 estimation.
    void setEventT0(const double& eventT0, const Const::EDetector& detector = Const::EDetector::invalidDetector)
    {
      m_eventT0 = eventT0;

      if (detector != Const::EDetector::invalidDetector) {
        m_detector = detector;
      }
    }

  private:
    /// Internal storage of the T0 estimation.
    double m_eventT0 = 0;

    /// Internal storage of the detector, who has determined the event T0.
    Const::EDetector m_detector = Const::EDetector::invalidDetector;

    /// Storage element for the eventwise T0 estimation.
    ClassDef(EventT0, 2)
  };
}
