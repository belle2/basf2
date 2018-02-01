/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SuperClusterCreator::SuperClusterCreator()
{
  this->addProcessingSignalListener(&m_wireHitRelationFilter);
}

std::string SuperClusterCreator::getDescription()
{
  return "Groups the wire hits into super cluster by expanding the secondary wire "
         "neighborhood";
}

void SuperClusterCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "expandOverApogeeGap"),
                                m_param_expandOverApogeeGap,
                                "Expand the super clusters over the typical gap at the apogee of the trajectory",
                                m_param_expandOverApogeeGap);

  m_wireHitRelationFilter.exposeParameters(moduleParamList, prefix);
}

void SuperClusterCreator::apply(std::vector<CDCWireHit>& inputWireHits,
                                std::vector<CDCWireHitCluster>& outputSuperClusters)
{
  m_wireHitRelations.clear();

  if (m_param_expandOverApogeeGap) {
    auto wireHitsByLayer =
    adjacent_groupby(inputWireHits.begin(), inputWireHits.end(), [](const CDCWireHit & wireHit) {
      return wireHit.getWireID().getILayer();
    });

    for (const auto& wireHitsInLayer : wireHitsByLayer) {
      const CDCWireLayer& wireLayer = wireHitsInLayer.front().getWire().getWireLayer();
      const int nWires = wireLayer.size();

      auto sameWireHitChain = [](const CDCWireHit & lhs, const CDCWireHit & rhs) {
        return rhs.getWireID().getIWire() - lhs.getWireID().getIWire() == 1;
      };

      auto wireHitChains =
        unique_ranges(wireHitsInLayer.begin(), wireHitsInLayer.end(), sameWireHitChain);

      size_t nWireHits = 0;
      for (const VectorRange<CDCWireHit>& wireHitChain : wireHitChains) {
        nWireHits += wireHitChain.size();
      }
      assert(nWireHits == wireHitsInLayer.size());

      // Special treatment for the first and last wireHitChain as they might wrap around as one
      VectorRange<CDCWireHit> frontWrapChain(wireHitsInLayer.begin(), wireHitsInLayer.begin());
      VectorRange<CDCWireHit> backWrapChain(wireHitsInLayer.end(), wireHitsInLayer.end());
      if (wireHitChains.size() > 1) {
        if (wireHitChains.front().front().getWire().isPrimaryNeighborWith(
              wireHitChains.back().back().getWire())) {
          // Chains are touching
          // Keep their information around but eliminate them from the regular chains
          int wrapAroundChainSize = wireHitChains.front().size() + wireHitChains.back().size();
          if (wrapAroundChainSize >= 5) {
            // Warning reach over the local wire hit layer / outside the memory of the wire hit
            // vector to transport the size.
            frontWrapChain = wireHitChains.front();
            frontWrapChain.first = frontWrapChain.end() - wrapAroundChainSize;

            backWrapChain = wireHitChains.back();
            backWrapChain.second = backWrapChain.begin() + wrapAroundChainSize;

            wireHitChains.erase(wireHitChains.begin());
            wireHitChains.pop_back();
          }
        }
      }

      auto itLastChain = std::remove_if(wireHitChains.begin(), wireHitChains.end(), Size() < 5u);
      wireHitChains.erase(itLastChain, wireHitChains.end());
      if (wireHitChains.empty()) continue;

      auto connectWireHitChains = [this, nWires](const VectorRange<CDCWireHit>& lhs,
      const VectorRange<CDCWireHit>& rhs) {
        int iWireDelta = rhs.front().getWireID().getIWire() - lhs.back().getWireID().getIWire();
        if (iWireDelta < 0) iWireDelta += nWires;
        if (iWireDelta < static_cast<int>(lhs.size() + rhs.size())) {
          m_wireHitRelations.push_back({&rhs.front(), 0, &lhs.back()});
          m_wireHitRelations.push_back({&lhs.back(), 0, &rhs.front()});
        }
        return false;
      };
      std::adjacent_find(wireHitChains.begin(), wireHitChains.end(), connectWireHitChains);

      if (not frontWrapChain.empty()) {
        connectWireHitChains(frontWrapChain, wireHitChains.front());
      }
      if (not backWrapChain.empty()) {
        connectWireHitChains(wireHitChains.back(), backWrapChain);
      }
      if (backWrapChain.empty() and frontWrapChain.empty()) {
        connectWireHitChains(wireHitChains.back(), wireHitChains.front());
      }
    }
  }

  /// Obtain the wire hits as pointers
  const std::vector<CDCWireHit*> wireHitPtrs = as_pointers<CDCWireHit>(inputWireHits);

  /// Create the wire hit relations
  RelationFilterUtil::appendUsing(m_wireHitRelationFilter, wireHitPtrs, m_wireHitRelations);

  B2ASSERT("Expect wire hit neighborhood to be symmetric ",
           WeightedRelationUtil<CDCWireHit>::areSymmetric(m_wireHitRelations));

  m_wirehitClusterizer.apply(wireHitPtrs, m_wireHitRelations, outputSuperClusters);

  int iSuperCluster = -1;
  for (CDCWireHitCluster& superCluster : outputSuperClusters) {
    ++iSuperCluster;
    superCluster.setISuperCluster(iSuperCluster);
    for (CDCWireHit* wireHit : superCluster) {
      wireHit->setISuperCluster(iSuperCluster);
    }
    std::sort(superCluster.begin(), superCluster.end());
  }
}
