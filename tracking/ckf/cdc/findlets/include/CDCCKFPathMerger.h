/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/functional/hash.hpp>

namespace Belle2 {
  /// Merge similar paths
  class CDCCKFPathMerger : public TrackFindingCDC::Findlet<CDCCKFPath> {
  public:
    /// main method of the findlet, reads/returns merged paths
    void apply(std::vector<CDCCKFPath>& newPaths) override
    {
      /// TODO: Merging does not work properly, as the tracks we compare must not have the same number of hits (and must not be at the same stage)
      std::unordered_map<size_t, CDCCKFPath> hashToPathList;
      for (const CDCCKFPath& path : newPaths) {
        const auto lastHitsHash = lastThreeHitHash(path);
        if (hashToPathList.find(lastHitsHash) != hashToPathList.end()) {
          if (hashToPathList[lastHitsHash].size() < path.size()) {
            hashToPathList[lastHitsHash] = path;
          }
        } else {
          hashToPathList[lastHitsHash] = path;
        }
      }

      newPaths.clear();
      for (const auto& hashAndPathList : hashToPathList) {
        const CDCCKFPath& path = hashAndPathList.second;
        newPaths.push_back(path);
      }
    }

  private:
    /// helper function, returns has of the last 3 wire hits on the path
    size_t lastThreeHitHash(const CDCCKFPath& path)
    {
      size_t seed = 0;
      unsigned int counter = 0;

      for (const CDCCKFState& state : boost::adaptors::reverse(path)) {
        if (counter >= 3) {
          break;
        }
        if (not state.isSeed()) {
          boost::hash_combine(seed, state.getWireHit());
        }
        counter++;
      }
      return seed;
    };

  };
}
