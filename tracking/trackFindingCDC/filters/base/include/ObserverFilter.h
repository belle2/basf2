/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <framework/logging/Logger.h>
#include <string>
#include <map>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Observer listening to the number of yes and no answers from the filter
    template<class ABaseFilter>
    class ObserverFilter : public ABaseFilter {

    private:
      /// Type of the filter base class
      typedef ABaseFilter Super;

    public:
      using ABaseFilter::BaseFilter;

      /// Object type to be filtered
      typedef typename ABaseFilter::Object Object;

    public:
      /// Constructor of the filter.
      ObserverFilter() : Super() {}

      /// Terminate the filter after event processing.
      virtual void terminate() override
      {
        B2INFO("Filter said " << m_yesAnswers << " times yes and " << m_noAnswers << " times no.");
        Super::terminate();
      }

    public:
      /** Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      virtual Weight operator()(const Object& object)
      {
        Weight result = Super::operator()(object);
        if (std::isnan(result)) {
          m_noAnswers += 1;
        } else {
          m_yesAnswers += 1;
        }

        return result;
      }

    private:
      unsigned int m_yesAnswers = 0;
      unsigned int m_noAnswers = 0;
    };
  }
}
