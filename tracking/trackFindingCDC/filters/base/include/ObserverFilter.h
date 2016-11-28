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

    /// Observer listening to the number of yes and no answers from the filter.
    template<class AFilter>
    class ObserverFilter : public AFilter {

    private:
      /// Type of the filter base class.
      using Super = AFilter;

    public:
      /// Using constructor of the Super class
      using AFilter::AFilter;

      /// Object type to be filtered.
      using Object = typename AFilter::Object;

    public:
      /// Terminate the filter after event processing.
      void terminate() override
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
      Weight operator()(const Object& object) override
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
      /// Number of counted yes answers.
      unsigned int m_yesAnswers = 0;

      /// Number of counted no answers.
      unsigned int m_noAnswers = 0;
    };
  }
}
