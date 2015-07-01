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

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class Type>
    class AllFilter : public Type {
      /** Accept all segment track combinations */
      virtual CellWeight operator()(const typename Type::Object&) override final
      {
        return 1;
      }

    };
  }
}
