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
    template<class BaseFilter>
    class ObserverFilter : public BaseFilter {
    public:
      using BaseFilter::BaseFilter;
      typedef typename BaseFilter::Object Object;

    public:
      /// Constructor of the filter.
      ObserverFilter() : BaseFilter() {;}

      /// Terminate the filter after event processing.
      virtual void terminate() override
      {
        B2INFO("Filter said " << m_yesAnswers << " times yes and " << m_noAnswers << " times no.")
        BaseFilter::terminate();
      }

    public:
      /** Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NOT_A_CELL if the object is rejected.
       */
      virtual CellWeight operator()(const Object& object)
      {
        CellWeight result = BaseFilter::operator()(object);
        if (isNotACell(result)) {
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
