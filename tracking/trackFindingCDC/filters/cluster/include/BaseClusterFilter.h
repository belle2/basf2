/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BASECLUSTERFILTER_H_
#define BASECLUSTERFILTER_H_

#include <tracking/trackFindingCDC/algorithms/CellWeight.h>
#include <map>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHitCluster;

    class BaseClusterFilter {

    public:
      /// Default constructor.
      BaseClusterFilter();

      /// Make destructor virtual.
      virtual ~BaseClusterFilter();

      /// Initialize the filter before event processing
      virtual void initialize();

      /// Terminate the filter after event processing
      virtual void terminate();

      /// Set a map of parameter key and string values. Meaning depends on the specific implementation.
      virtual void setParameters(const std::map<std::string, std::string>& parameterMap);

    public:
      /// Basic filter method to override. Base implementation rejects all clusters.
      virtual CellWeight isGoodCluster(const CDCWireHitCluster& cluster);

    };
  }
}

#endif // BASECLUSTERFILTER_H_
