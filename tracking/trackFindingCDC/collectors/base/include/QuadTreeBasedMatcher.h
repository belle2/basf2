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
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * A templated matcher based on a quad tree. The full functionality of the quad tree must be
     * implemented with the collect-method in your derived class.
     **/
    template<class AQuadTree>
    class QuadTreeBasedMatcher {
    public:
      /** Empty destructor. Everything is handled via terminate. **/
      virtual ~QuadTreeBasedMatcher() = default;

      /** Expose the quad tree relevant parameters. **/
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
      {
        moduleParameters->addParameter(prefixed(prefix, "level"), m_param_quadTreeLevel,
                                       "The number of levels for the quad tree search.",
                                       m_param_quadTreeLevel);

        moduleParameters->addParameter(prefixed(prefix, "minimumNumberOfHits"), m_param_minimumNumberOfHits,
                                       "The minimum number of hits in a quad tree bin to be called as result.",
                                       m_param_minimumNumberOfHits);

        moduleParameters->addParameter(prefixed(prefix, "writeDebugInformation"), m_param_writeDebugInformation,
                                       "Set to true to output debug information.",
                                       m_param_writeDebugInformation);
      }

      /** Prepare the hough algorithm: initialize the quad tree. */
      virtual void initialize()
      {
        m_quadTreeInstance.setMaxLevel(m_param_quadTreeLevel);
        m_quadTreeInstance.initialize();
      }

      /** Raze the quad tree. */
      virtual void terminate()
      {
        m_quadTreeInstance.raze();
      }

    protected:
      /// Parameters
      /// Maximum level of the quad tree search.
      unsigned int m_param_quadTreeLevel = 6;
      /// Minimal number of hits a quad tree node must have to be called a found bin
      unsigned int m_param_minimumNumberOfHits = 5;
      /// Set to true to output debug information.
      bool m_param_writeDebugInformation = false;

      /// Quad tree instance
      AQuadTree m_quadTreeInstance;

      /// Use the writeDebugInformation function of the quad tree to write the tree into a root file with a ascending number.
      void writeDebugInformation()
      {
        std::string outputFileName = "quadTreeContent_call_" + std::to_string(m_numberOfPassedDebugCalls) + ".root";
        m_quadTreeInstance.writeDebugInfoToFile(outputFileName);

        m_numberOfPassedDebugCalls++;
      }

    private:
      /// Store the number of passed calls to the debug function.
      unsigned int m_numberOfPassedDebugCalls = 0;
    };
  }
}
