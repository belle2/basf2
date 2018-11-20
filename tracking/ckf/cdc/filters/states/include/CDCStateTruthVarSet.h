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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/ckf/cdc/filters/states/BaseCDCStateFilter.h>
#include <tracking/ckf/cdc/utilities/CDCMCUtil.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcStateTruthVarNames[] = {
    "truth"
  };

  /// Vehicle class to transport the variable names
  class CDCStateTruthVarNames : public TrackFindingCDC::VarNames<BaseCDCStateFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCStateTruthVarSet : public TrackFindingCDC::VarSet<CDCStateTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCStateFilter::Object* result) override;
    /// In addition to initalize of the parent class, execute CDCMCManager::getInstance().fill();
    void initialize() override;

  private:
    StoreObjPtr<EventMetaData> m_eventMetaData;

    MCUtil m_mcUtil;
  };
}
