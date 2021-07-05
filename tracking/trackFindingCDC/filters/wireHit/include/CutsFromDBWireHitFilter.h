/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/wireHit/BaseWireHitFilter.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dbobjects/CDCWireHitRequirements.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Filter rejecting hits according to DB values.
    class CutsFromDBWireHitFilter : public BaseWireHitFilter {

    public:

      /// Default constructor.
      CutsFromDBWireHitFilter();

      /// Default destructor.
      virtual ~CutsFromDBWireHitFilter() = default;

      /// Called at the beginning of the processing.
      void initialize() final;

      /// Called when a new run is started.
      void beginRun() final;

      /// Basic filter method to override.
      Weight operator()(const CDCWireHit& wireHit) final;

    private:

      /// Check if m_CDCWireHitRequirementsFromDB is valid
      /// and set m_DBPtrIsValidForCurrentRun accordingly.
      void checkIfDBObjPtrIsValid();

      /** Check if value >= range.first and value <= range.second
        If range.second == -1, then check only if value >= range.first */
      template <typename T>
      bool isInRange(const T& value, const std::pair<T, T>& range) const;

      /** Check if value <= upper_value
       If upper_value == -1, then return true */
      template <typename T>
      bool isLessThanOrEqualTo(const T& value, const T& upper_value) const;

      /// Cut values from the Data Base.
      std::unique_ptr<DBObjPtr<CDCWireHitRequirements> > m_CDCWireHitRequirementsFromDB;

      /// Boolean asserting if DBObjPtr is valid for the current run.
      bool m_DBPtrIsValidForCurrentRun;
    };
  }
}
