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

#include <tracking/trackFindingCDC/numerics/Weight.h>

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
      void terminate() override;

    public:
      /**
       *  Function to evaluate the object.
       *  Base implementation rejects all objects.
       *
       *  @param obj The object to be accepted or rejected.
       *  @return    A finit float value if the object is accepted.
       *             NAN if the object is rejected.
       */
      Weight operator()(const Object& object) override;

    private:
      /// Number of counted yes answers.
      unsigned int m_yesAnswers = 0;

      /// Number of counted no answers.
      unsigned int m_noAnswers = 0;
    };
  }
}
