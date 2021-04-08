/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/LayerPXDRelationFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/base/RelationFilter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {

  template <class AFilter, class APrefilter>
  LayerPXDRelationFilter<AFilter, APrefilter>::LayerPXDRelationFilter() : Super()
  {
    Super::addProcessingSignalListener(&m_filter);
    Super::addProcessingSignalListener(&m_prefilter);
  }

  template <class AFilter, class APrefilter>
  LayerPXDRelationFilter<AFilter, APrefilter>::~LayerPXDRelationFilter() = default;

  template <class AFilter, class APrefilter>
  void LayerPXDRelationFilter<AFilter, APrefilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    // use value from DB if parameter is set to -1
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumping"), m_param_hitJumping,
                                  "Make it possible to jump over N layers.", m_param_hitJumping);

    m_filter.exposeParameters(moduleParamList, prefix);
    m_prefilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("pre", prefix));

    m_prefix = prefix;
  }

  template <class AFilter, class APrefilter>
  void LayerPXDRelationFilter<AFilter, APrefilter>::initialize()
  {
    Super::initialize();

    if (m_prefix == "seed") {
      m_ckfParameters = new OptionalDBObjPtr<CKFParameters>("PXDCKFSeedHitParameters");
    } else if (m_prefix == "hit") {
      m_ckfParameters = new OptionalDBObjPtr<CKFParameters>("PXDCKFHitHitParameters");
    } else {
      B2ERROR("Unknown prefix. Apparently, some non-trivial changes to code were done.");
    }
  }

  template <class AFilter, class APrefilter>
  void LayerPXDRelationFilter<AFilter, APrefilter>::beginRun()
  {
    Super::beginRun();

    // use values from payload if parameter is set to -1
    if (m_param_hitJumping == -1) {
      if ((*m_ckfParameters).isValid()) {
        m_layerJumpPtThreshold = (*m_ckfParameters)->getLayerJumpPtThreshold();
        m_layerJumpLowPt = (*m_ckfParameters)->getLayerJumpLowPt();
        m_layerJumpHighPt = (*m_ckfParameters)->getLayerJumpHighPt();
        if (m_prefix == "hit" && m_layerJumpPtThreshold > 0.) {
          // if we want to have this, some major restructure of the whole CKF code is necessary
          // (CKF states from hits don't know anything about the seed track, i.e. you cannot access its momentum)
          B2FATAL("pt dependence of layerJump parameter currently not implemented for hit->hit extrapolation.");
        }
      } else {
        B2FATAL("Trying to read layerJump parameter from DB but payload '" << m_ckfParameters->getName() << "' not found.");
      }
      // we don't need all this if simple value from python config is to be used
    } else {
      m_layerJumpPtThreshold = -1;
      m_layerJumpLowPt = m_param_hitJumping;
      m_layerJumpHighPt = m_param_hitJumping;
    }
  }

  template <class AFilter, class APrefilter>
  std::vector<CKFToPXDState*>
  LayerPXDRelationFilter<AFilter, APrefilter>::getPossibleTos(CKFToPXDState* currentState,
                                                              const std::vector<CKFToPXDState*>& states) const
  {
    std::vector<CKFToPXDState*> possibleNextStates;

    const CKFToPXDState::stateCache& currentStateCache = currentState->getStateCache();
    const unsigned int& currentLayer = currentStateCache.geoLayer;

    // this is the parameter value set in the python config
    int m_hitJump = m_param_hitJumping;
    // if it is set to -1 we want to use the values in the payload
    if (m_hitJump == -1) {
      m_hitJump = currentStateCache.ptSeed < m_layerJumpPtThreshold ? m_layerJumpLowPt : m_layerJumpHighPt;
    }
    B2INFO(m_prefix << ": " << currentStateCache.ptSeed << "/" << m_layerJumpPtThreshold << "->" << m_hitJump);

    const unsigned int& nextPossibleLayer = std::max(static_cast<int>(currentLayer) - 1 - m_hitJump, 0);

    // Patch for the PXD layer 2 overlap inefficiency fix
    // previous implementation of maximumLadderNumber was calculated using GeoCache gave incorrect value for exp1003
    // Geometrically, PXD layer 1 has 8 ladders, pxd layer 2 has 12 ladder
    int numberOfLaddersForLayer[2] = {8, 12};

    for (CKFToPXDState* nextState : states) {
      const CKFToPXDState::stateCache& nextStateCache = nextState->getStateCache();
      const unsigned int nextLayer = nextStateCache.geoLayer;
      if (nextLayer < std::min(currentLayer, nextPossibleLayer) or std::max(currentLayer, nextPossibleLayer) < nextLayer) {
        continue;
      }

      if (currentLayer == nextLayer) {
        // next layer is an overlap one, so lets return all hits from the same layer, that are on a
        // ladder which is below the last added hit.
        const unsigned int fromLadderNumber = currentStateCache.ladder;
        const unsigned int maximumLadderNumber = numberOfLaddersForLayer[currentLayer - 1];

        // the reason for this strange formula is the numbering scheme in the VXD.
        // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
        // then we add (PXD)/subtract(PXD) one to get to the next (overlapping) ladder and do a % N to also cope for the
        // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
        // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
        const int direction = 1;
        const unsigned int overlappingLadder =
          ((fromLadderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

        if (nextStateCache.ladder != overlappingLadder) {
          continue;
        }

        // Next we make sure to not have any cycles in our graph: we do this by defining only the halves of the
        // sensor as overlapping. So if the first hit is coming from sensor 1 and the second from sensor 2,
        // they are only related if the one from sensor 1 is on the half, that is pointing towards sensor 2
        // and the one on sensor 2 is on the half that is pointing towards sensor 1.
        //
        //                       X                            X                               X
        //                      ----|----                    ----|----                ----|----
        //  This is fine:       X           This not:    X             This not:        X
        //                ----|----                    ----|----                 ----|----

        // for PXD its the other way round!
        if (currentStateCache.localNormalizedu <= 0.8) {
          continue;
        }

        if (nextStateCache.localNormalizedu > 0.2) {
          continue;
        }
      }

      // Some loose prefiltering of possible states
      TrackFindingCDC::Weight weight = m_prefilter(std::make_pair(currentState, nextState));
      if (std::isnan(weight)) {
        continue;
      }

      possibleNextStates.push_back(nextState);
    }

    return possibleNextStates;
  }

  template <class AFilter, class APrefilter>
  TrackFindingCDC::Weight LayerPXDRelationFilter<AFilter, APrefilter>::operator()(const CKFToPXDState& from, const CKFToPXDState& to)
  {
    return m_filter(std::make_pair(&from, &to));
  }
}
