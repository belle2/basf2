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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>
#include <framework/database/DBObjPtr.h>
#include <cdc/dbobjects/CDCWireHitRequirements.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Marks hits as background based on the result of a filter
    class WireHitBackgroundDetector : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Default constructor
      WireHitBackgroundDetector(): m_CDCWireHitRequirementsFromDB("CDCWireHitRequirements")
      {
      }

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Begin run action
      void beginRun() final;

      /// Main algorithm marking hit as background
      void apply(std::vector<CDCWireHit>& wireHits) final;

    private:

      /// TODO: Chooseable WireHit filter to be used to filter background

      /// Cut values from the Data Base
      DBObjPtr<CDCWireHitRequirements> m_CDCWireHitRequirementsFromDB;
    };
  }
}
