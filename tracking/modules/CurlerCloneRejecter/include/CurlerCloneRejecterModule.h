/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/CurlerCloneRejecter.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the MCTrackCurlerCloneRejecter findlet
     */
    class TFCDC_CurlerCloneRejecterModule : public FindletModule<CurlerCloneRejecter> {

    private:
      /// Type of the base class
      using Super = FindletModule<CurlerCloneRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_CurlerCloneRejecterModule();
    };
  }
}
