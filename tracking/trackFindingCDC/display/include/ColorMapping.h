/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/Mapping.h>
#include <tracking/trackFindingCDC/display/Colors.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class template for coloring objects in different Colors.
     *
     * This Class uses the color list defined in the Colors class.
     */
    template<class AObject>
    class DefaultColorCycleMapping : public CycleMapping<AObject> {

    private:
      /// Type of the base class
      using Super = CycleMapping<AObject>;

    public:
      /// Constructor
      DefaultColorCycleMapping()
        : Super(Colors::getList())
      {
      }
    };
  }
}
