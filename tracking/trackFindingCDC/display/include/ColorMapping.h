/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
