/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ALLCLUSTERFILTER_H_
#define ALLCLUSTERFILTER_H_

#include "BaseClusterFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    class AllClusterFilter : public BaseClusterFilter {

    public:
      /// Basic filter method to override. All implementation rejects all clusters.
      virtual CellWeight isGoodCluster(const CDCWireHitCluster& cluster) override final;

    };
  }
}

#endif // ALLCLUSTERFILTER_H_
