/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>


namespace Belle2 {

  /** Module to combine the EventT0 values from multiple sub-detectors
   */
  class EventT0CombinerModule final : public Module {

  public:

    /** Default constructor */
    EventT0CombinerModule();

    /** Destructor */
    virtual ~EventT0CombinerModule() = default;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    void event() override;

  private:

    /// computes the new average between multiple, un-correlated sub-detector measurements
    EventT0::EventT0Component computeCombination(std::vector<EventT0::EventT0Component> measurements) const;

    /// Access to global EventT0 which will be read and updated
    StoreObjPtr<EventT0> m_eventT0;

    /// In this mode, the SVD t0 value (if available) will be set as the final T0 value. Only if no SVD value could be found
    /// (which is very rare for BBbar events, and around 5% of low multiplicity events), the best ECL value will be set
    const std::string m_combinationModePreferSVD = "prefer_svd";

    /// In this mode, the CDC t0 value (if available) will be set as the final T0 value. Only if no CDC value could be found
    /// (which is very rare for BBbar events, and around 5% of low multiplicity events), the best ECL value will be set
    const std::string m_combinationModePreferCDC = "prefer_cdc";

    /// In this mode, the SVD t0 value (if available) will be used to select the ECL t0 information which is closest in time
    /// to the best SVD value and these two values will be combined to one final value.
    const std::string m_combinationModeCombineSVDandECL = "combine_svd_and_ecl";

    /// Mode to combine the t0 values of the sub-detectors
    std::string m_paramCombinationMode = m_combinationModePreferSVD;

  };
} // Belle2 namespace
