/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/complete/TrackFinderAutomaton.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages.
    class TrackFinderCosmics : public TrackFinderAutomaton {

    private:
      /// Type of the base class
      typedef TrackFinderAutomaton Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override
      {
        Super::exposeParameters(moduleParamList, prefix);
        moduleParamList->getParameter<std::string>("flightTimeEstimation").setDefaultValue("downwards");
        moduleParamList->getParameter<std::string>("flightTimeEstimation").resetValue();

        moduleParamList->getParameter<std::string>("SegmentOrientation").setDefaultValue("downwards");
        moduleParamList->getParameter<std::string>("SegmentOrientation").resetValue();

        moduleParamList->getParameter<std::string>("TrackOrientation").setDefaultValue("downwards");
        moduleParamList->getParameter<std::string>("TrackOrientation").resetValue();
      }

    }; // end class TrackFinderCosmics

  } //end namespace TrackFindingCDC
} //end namespace Belle2
