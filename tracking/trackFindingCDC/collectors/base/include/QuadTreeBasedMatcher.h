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
      virtual ~QuadTreeBasedMatcher() { }

      /** Expose the quad tree relevant parameters. **/
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "")
      {
        moduleParameters->addParameter(prefix + "quadTreeLevel", m_param_quadTreeLevel,
                                       "The number of levels for the quad tree search.",
                                       m_param_quadTreeLevel);

        moduleParameters->addParameter(prefix + "minimumNumberOfHits", m_param_minimumNumberOfHits,
                                       "The minimum number of hits in a quad tree bin to be called as result.",
                                       m_param_minimumNumberOfHits);
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

      /// Quad tree instance
      AQuadTree m_quadTreeInstance;
    };
  }
}
