/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class SeedObject, class HitObject, class ResultObject>
    class TreeSearchFindlet : public Findlet<SeedObject*, const HitObject*> {
    public:
      virtual void apply(std::vector<SeedObject*>& seedsVector,
                         std::vector<const HitObject*>& filteredHitVector) override
      {
        // Maybe implement some caching here.
      }

      void traverseTree(SeedObject* seed, std::vector<ResultObject>& resultsVector)
      {
        traverseTree(ResultObject(seed), resultsVector);
      }

      void traverseTree(const ResultObject& currentResult,
                        std::vector<ResultObject>& resultsVector)
      {

        const auto& matchingHits = getMatchingHits(currentResult);

        if (matchingHits.empty()) {
          resultsVector.push_back(currentResult);
          return;
        }

        unsigned int counter = 0;
        for (const auto& hit : matchingHits) {
          ResultObject resultWithThisHit = currentResult.append(hit);

          if (not useResult(resultWithThisHit)) {
            continue;
          }

          traverseTree(resultWithThisHit, resultsVector);
          counter++;

          if (counter > 5) {
            break;
          }
        };
      }

    protected:
      virtual TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(const ResultObject& currentResult) = 0;

      virtual bool useResult(const ResultObject& currentResult)
      {
        return true;
      }
    };
  }
}