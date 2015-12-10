/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/findlets/Findlet.h>

#include <vector>
#include <iterator>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks clusters as background based on a background measure
    template<class ClusterFilter = BaseClusterFilter>
    class ClusterBackgroundDetector :
      public Findlet<CDCWireHitCluster> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHitCluster> Super;

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(std::vector<CDCWireHitCluster>& outputClusters) override final;

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList) override final
      {
        this->getClusterFilter()->exposeParameters(moduleParamList);
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        Super::initialize();
        m_ptrClusterFilter->initialize();
      }

      /// Processes the current event
      virtual void beginEvent() override final
      { this->getClusterFilter()->beginEvent(); }

      /// Signals the end of the event processing
      virtual void terminate() override final
      {
        m_ptrClusterFilter->terminate();
        Super::terminate();
      }

    public:
      /// Getter for the current cluster filter. The module keeps ownership of the pointer.
      ClusterFilter* getClusterFilter()
      { return m_ptrClusterFilter.get(); }

      /// Setter for the cluster filter used in the cluster creation. The module takes ownership of the pointer.
      void setClusterFilter(std::unique_ptr<ClusterFilter> ptrClusterFilter)
      { m_ptrClusterFilter = std::move(ptrClusterFilter); }

    private:
      /// Reference to the filter to be used to filter background
      std::unique_ptr<ClusterFilter> m_ptrClusterFilter{new ClusterFilter()};

    }; // end class ClusterCreator

    template<class ClusterFilter>
    void ClusterBackgroundDetector<ClusterFilter>::apply(std::vector<CDCWireHitCluster>& outputClusters)
    {
      for (CDCWireHitCluster& cluster : outputClusters) {
        B2ASSERT("Expect cluster filter to be setup", m_ptrClusterFilter);
        CellWeight clusterWeight = (*m_ptrClusterFilter)(cluster);
        if (isNotACell(clusterWeight)) {
          // Cluster detected as background
          cluster.setBackgroundFlag(true);
          for (const CDCWireHit* wireHit : cluster) {
            wireHit->getAutomatonCell().setBackgroundFlag();
            wireHit->getAutomatonCell().setTakenFlag();
          }
        }
      }
    }
  } // end namespace TrackFindingCDC
} // end namespace Belle2

