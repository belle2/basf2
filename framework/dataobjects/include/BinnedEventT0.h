/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <TObject.h>

#include <map>

namespace Belle2 {

  /**
   * Database object for storing a binned EventT0 coming from the trigger.
   * It stores one binned (integer) event t0 estimation for each detector,
   * which will be replaced whenever you store a new one for the same detector.
   *
   * It is only possible to extract the estimations for one detector at the time,
   * as averaging does not make sense for the binned data (additionally, the detectors
   * may have different time offsets unknown to this object).
   *
   * It is in the responsibility of the user to calculate a floating point t0 in
   * nanoseconds out of the integer estimations if desired.
   */
  class BinnedEventT0 : public TObject {
  public:
    /// Return the stored binned event t0 for the given detector or 0 if nothing stored
    int getBinnedEventT0(const Const::EDetector detector) const;

    /// Store a binned event t0 for the given detector replacing any other hypothesis for this detector.
    void addBinnedEventT0(int eventT0, Const::EDetector detector);

    /// Check if one of the detectors in the given set has a binned t0 estimation.
    bool hasBinnedEventT0(const Const::EDetector detector) const;

    /// Clear the list of extracted event T0 estimations.
    void clear();

  private:
    /// Internal storage of the event t0 list.
    std::map<Const::EDetector, int> m_eventT0Map;

    ClassDef(BinnedEventT0, 1) /**< Storage element for the binned eventwise T0 estimation. */
  };
}
