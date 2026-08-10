/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>

#include <tracking/trackingUtilities/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCFacet.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <tracking/trackingUtilities/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackingUtilities/utilities/VectorRange.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

FacetCreator::FacetCreator()
{
  this->addProcessingSignalListener(&m_wireHitRelationFilter);
  this->addProcessingSignalListener(&m_facetFilter);
}

std::string FacetCreator::getDescription()
{
  return "Creates hit triplet (facets) from each cluster filtered by a acceptance criterion.";
}

void FacetCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_wireHitRelationFilter.exposeParameters(moduleParamList, prefix);
  m_feasibleRLFacetFilter.exposeParameters(moduleParamList, prefix);
  m_facetFilter.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "updateDriftLength"),
                                m_param_updateDriftLength,
                                "Switch to reestimate the drift length",
                                m_param_updateDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "leastSquareFit"),
                                m_param_leastSquareFit,
                                "Switch to fit the facet with the least square method "
                                "for drift length estimation",
                                m_param_leastSquareFit);
}

void FacetCreator::apply(const std::vector<CDCWireHitCluster>& inputClusters, std::vector<CDCFacet>& facets)
{
  int iCluster = -1;
  for (const CDCWireHitCluster& cluster : inputClusters) {
    ++iCluster;
    // Skip clusters that have been detected as background
    if (cluster.getBackgroundFlag()) {
      continue;
    }
    B2ASSERT("Expect the clusters to be sorted", std::is_sorted(cluster.begin(), cluster.end()));

    // Obtain the set of wire hits as references
    const std::vector<CDCWireHit*>& wireHits = cluster;

    // Create the neighborhood of wire hits on the cluster
    m_wireHitRelations.clear();
    RelationFilterUtil::appendUsing(m_wireHitRelationFilter, wireHits, m_wireHitRelations);

    B2ASSERT("Wire neighborhood is not symmetric. Check the geometry.",
             WeightedRelationUtil<CDCWireHit>::areSymmetric(m_wireHitRelations));

    // Create the facets
    std::size_t nBefore = facets.size();
    createFacets(cluster, m_wireHitRelations, facets);
    std::size_t nAfter = facets.size();

    VectorRange<CDCFacet> facetsInCluster(facets.begin() + nBefore, facets.begin() + nAfter);
    // Sort the facets in their cluster
    std::sort(facetsInCluster.begin(), facetsInCluster.end());

    B2ASSERT("Expected all facets to be different",
             std::adjacent_find(facetsInCluster.begin(), facetsInCluster.end()) ==
             facetsInCluster.end());

    for (CDCFacet& facet : facetsInCluster) {
      facet.setICluster(iCluster);
    }
  }
}

void FacetCreator::createFacets(const std::vector<CDCWireHit*>& wireHits,
                                const std::vector<WeightedRelation<CDCWireHit> >& wireHitRelations,
                                std::vector<CDCFacet>& facets)
{
  for (const CDCWireHit* ptrMiddleWireHit : wireHits) {
    if (not ptrMiddleWireHit) continue;
    const CDCWireHit& middleWireHit = *ptrMiddleWireHit;
    if (middleWireHit->hasTakenFlag()) continue;

    const auto neighbors = asRange(
                             std::equal_range(wireHitRelations.begin(), wireHitRelations.end(), ptrMiddleWireHit));

    for (const WeightedRelation<CDCWireHit>& startWireHitRelation : neighbors) {
      const CDCWireHit* ptrStartWireHit(startWireHitRelation.getTo());

      if (not ptrStartWireHit) continue;
      const CDCWireHit& startWireHit = *ptrStartWireHit;
      if (startWireHit->hasTakenFlag()) continue;

      for (const WeightedRelation<CDCWireHit>& endWireHitRelation : neighbors) {
        const CDCWireHit* ptrEndWireHit(endWireHitRelation.getTo());

        if (not ptrEndWireHit) continue;
        const CDCWireHit& endWireHit = *ptrEndWireHit;
        if (endWireHit->hasTakenFlag()) continue;

        // Skip combinations where the facet starts and ends on the same wire
        if (ptrStartWireHit->isOnWire(ptrEndWireHit->getWire())) continue;

        createFacetsForHitTriple(startWireHit, middleWireHit, endWireHit, facets);
      } // end for itEndWireHit
    } // end for itStartWireHit
  } // end for itMiddleWireHit
}

void FacetCreator::createFacetsForHitTriple(const CDCWireHit& startWireHit,
                                            const CDCWireHit& middleWireHit,
                                            const CDCWireHit& endWireHit,
                                            std::vector<CDCFacet>& facets)
{
  /// Prepare a facet - without fitted tangent lines.
  CDCRLWireHit startRLWireHit(&startWireHit, ERightLeft::c_Left);
  CDCRLWireHit middleRLWireHit(&middleWireHit, ERightLeft::c_Left);
  CDCRLWireHit endRLWireHit(&endWireHit, ERightLeft::c_Left);
  CDCFacet facet(startRLWireHit, middleRLWireHit, endRLWireHit, UncertainParameterLine2D());

  // The shape of the hit triple does not depend on the right left passage hypotheses
  const CDCRLWireHitTriple::Shape shape = facet.getShape();
  if (m_param_feasibleRLOnly and
      shape.getCellExtend() + std::abs(shape.getOClockDelta()) > 6) {
    // No right left passage combination is feasible for this shape - skip all of them
    return;
  }

  // The middle right left passage hypothesis is scanned in the innermost loop:
  // facets that differ only in it share the same start to end fit line, such that
  // most of the drift length update can be reused between them.
  for (ERightLeft startRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
    facet.setStartRLInfo(startRLInfo);
    for (ERightLeft endRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
      facet.setEndRLInfo(endRLInfo);

      // Results of the drift length update shared between the middle right left passage hypotheses
      DriftLengthEstimator::FacetDriftLengthCache driftLengthCache;

      for (ERightLeft middleRLInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
        facet.setMiddleRLInfo(middleRLInfo);

        // Reset the lines
        // The filter shall do the fitting of the tangent lines if it wants to.
        // He should set them if he accepts the facet.
        facet.invalidateFitLine();

        if (m_param_feasibleRLOnly) {
          // Same check as m_feasibleRLFacetFilter(facet) but reusing the precomputed shape
          if (not m_feasibleRLFacetFilter.isFeasible(shape, startRLInfo, middleRLInfo, endRLInfo)) continue;
        }

        if (m_param_updateDriftLength) {
          if (m_param_leastSquareFit) {
            // The fitted line depends on the middle hit drift length - no sharing possible

            // Reset drift length
            facet.getStartRLWireHit().setRefDriftLength(startWireHit.getRefDriftLength());
            facet.getMiddleRLWireHit().setRefDriftLength(middleWireHit.getRefDriftLength());
            facet.getEndRLWireHit().setRefDriftLength(endWireHit.getRefDriftLength());

            /*double chi2 =*/FacetFitter::fit(facet);

            // Update drift length
            m_driftLengthEstimator.updateDriftLength(facet);
          } else {
            if (not driftLengthCache.valid) {
              // Reset drift length
              facet.getStartRLWireHit().setRefDriftLength(startWireHit.getRefDriftLength());
              facet.getMiddleRLWireHit().setRefDriftLength(middleWireHit.getRefDriftLength());
              facet.getEndRLWireHit().setRefDriftLength(endWireHit.getRefDriftLength());

              facet.adjustFitLine();
            }

            // Update drift length reusing the shareable results of the previous hypothesis
            m_driftLengthEstimator.updateDriftLength(facet, driftLengthCache);
          }
        }

        Weight weight = m_facetFilter(facet);

        if (not std::isnan(weight)) {
          facet.getAutomatonCell().setCellWeight(weight);
          facets.insert(facets.end(), facet);
        }
      } // end for middleRLWireHit
    } // end for endRLWireHit
  } // end for startRLWireHit
}
