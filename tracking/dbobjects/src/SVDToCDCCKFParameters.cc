/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dbobjects/SVDToCDCCKFParameters.h>

using namespace Belle2;

ClassImp(SVDToCDCCKFParameters);

/** FLOAT PARAMETERS */

void SVDToCDCCKFParameters::setMaximalDeltaPhi(float phi) { m_maximalDeltaPhi = phi; }
float SVDToCDCCKFParameters::getMaximalDeltaPhi() const { return m_maximalDeltaPhi; }

void SVDToCDCCKFParameters::setMinimalPtRequirement(float pt) { m_minimalPtRequirement = pt; }
float SVDToCDCCKFParameters::getMinimalPtRequirement() const { return m_minimalPtRequirement; }


/** INTEGER PARAMETERS */

void SVDToCDCCKFParameters::setMaximalLayerJump(int layer) { m_maximalLayerJump = layer; }
int SVDToCDCCKFParameters::getMaximalLayerJump() const { return m_maximalLayerJump; }

void SVDToCDCCKFParameters::setMaximalLayerJumpBackwardSeed(int layer) { m_maximalLayerJumpBackwardSeed = layer; }
int SVDToCDCCKFParameters::getMaximalLayerJumpBackwardSeed() const { return m_maximalLayerJumpBackwardSeed; }

void SVDToCDCCKFParameters::setPathMaximalCandidatesInFlight(unsigned int max) { m_pathMaximalCandidatesInFlight = max; }
unsigned int SVDToCDCCKFParameters::getPathMaximalCandidatesInFlight() const { return m_pathMaximalCandidatesInFlight; }

void SVDToCDCCKFParameters::setStateMaximalHitCandidates(unsigned int max) { m_stateMaximalHitCandidates = max; }
unsigned int SVDToCDCCKFParameters::getStateMaximalHitCandidates() const { return m_stateMaximalHitCandidates; }


/** BOOLEAN PARAMETERS */

void SVDToCDCCKFParameters::setExportAllTracks(bool value) { m_exportAllTracks = value; }
bool SVDToCDCCKFParameters::getExportAllTracks() const { return m_exportAllTracks; }

void SVDToCDCCKFParameters::setExportTracks(bool value) { m_exportTracks = value; }
bool SVDToCDCCKFParameters::getExportTracks() const { return m_exportTracks; }

void SVDToCDCCKFParameters::setIgnoreTracksWithCDChits(bool value) { m_ignoreTracksWithCDChits = value; }
bool SVDToCDCCKFParameters::getIgnoreTracksWithCDChits() const { return m_ignoreTracksWithCDChits; }

void SVDToCDCCKFParameters::setTakenFlag(bool value) { m_setTakenFlag = value; }
bool SVDToCDCCKFParameters::getTakenFlag() const { return m_setTakenFlag; }


/** STRING PARAMETERS */

void SVDToCDCCKFParameters::setHitFindingDirection(const std::string& direction) { m_hitFindingDirection = direction; }
const std::string& SVDToCDCCKFParameters::getHitFindingDirection() const { return m_hitFindingDirection; }

void SVDToCDCCKFParameters::setFilter(const std::string& filter) { m_filter = filter; }
const std::string& SVDToCDCCKFParameters::getFilter() const { return m_filter; }

void SVDToCDCCKFParameters::setInputRecoTrackStoreArrayName(const std::string& name) { m_inputRecoTrackStoreArrayName = name; }
const std::string& SVDToCDCCKFParameters::getInputRecoTrackStoreArrayName() const { return m_inputRecoTrackStoreArrayName; }

void SVDToCDCCKFParameters::setInputWireHits(const std::string& hits) { m_inputWireHits = hits; }
const std::string& SVDToCDCCKFParameters::getInputWireHits() const { return m_inputWireHits; }

void SVDToCDCCKFParameters::setOutputRecoTrackStoreArrayName(const std::string& name) { m_outputRecoTrackStoreArrayName = name; }
const std::string& SVDToCDCCKFParameters::getOutputRecoTrackStoreArrayName() const { return m_outputRecoTrackStoreArrayName; }

void SVDToCDCCKFParameters::setOutputRelationRecoTrackStoreArrayName(const std::string& name) { m_outputRelationRecoTrackStoreArrayName = name; }
const std::string& SVDToCDCCKFParameters::getOutputRelationRecoTrackStoreArrayName() const { return m_outputRelationRecoTrackStoreArrayName; }

void SVDToCDCCKFParameters::setPathFilter(const std::string& filter) { m_pathFilter = filter; }
const std::string& SVDToCDCCKFParameters::getPathFilter() const { return m_pathFilter; }

void SVDToCDCCKFParameters::setRelatedRecoTrackStoreArrayName(const std::string& name) { m_relatedRecoTrackStoreArrayName = name; }
const std::string& SVDToCDCCKFParameters::getRelatedRecoTrackStoreArrayName() const { return m_relatedRecoTrackStoreArrayName; }

void SVDToCDCCKFParameters::setRelationCheckForDirection(const std::string& direction) { m_relationCheckForDirection = direction; }
const std::string& SVDToCDCCKFParameters::getRelationCheckForDirection() const { return m_relationCheckForDirection; }

void SVDToCDCCKFParameters::setSeedComponent(const std::string& component) { m_seedComponent = component; }
const std::string& SVDToCDCCKFParameters::getSeedComponent() const { return m_seedComponent; }

void SVDToCDCCKFParameters::setStateBasicFilter(const std::string& filter) { m_stateBasicFilter = filter; }
const std::string& SVDToCDCCKFParameters::getStateBasicFilter() const { return m_stateBasicFilter; }

void SVDToCDCCKFParameters::setStateExtrapolationFilter(const std::string& filter) { m_stateExtrapolationFilter = filter; }
const std::string& SVDToCDCCKFParameters::getStateExtrapolationFilter() const { return m_stateExtrapolationFilter; }

void SVDToCDCCKFParameters::setStateFinalFilter(const std::string& filter) { m_stateFinalFilter = filter; }
const std::string& SVDToCDCCKFParameters::getStateFinalFilter() const { return m_stateFinalFilter; }

void SVDToCDCCKFParameters::setStatePreFilter(const std::string& filter) { m_statePreFilter = filter; }
const std::string& SVDToCDCCKFParameters::getStatePreFilter() const { return m_statePreFilter; }

void SVDToCDCCKFParameters::setTrackFindingDirection(const std::string& direction) { m_trackFindingDirection = direction; }
const std::string& SVDToCDCCKFParameters::getTrackFindingDirection() const { return m_trackFindingDirection; }

void SVDToCDCCKFParameters::setWriteOutDirection(const std::string& direction) { m_writeOutDirection = direction; }
const std::string& SVDToCDCCKFParameters::getWriteOutDirection() const { return m_writeOutDirection; }


/** FILTER PARAMETERS */

/*
void SVDToCDCCKFParameters::setFilterParameters(const FilterParamMap& params) { m_filterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getFilterParameters() const { return m_filterParameters; }

void SVDToCDCCKFParameters::setPathFilterParameters(const FilterParamMap& params) { m_pathFilterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getPathFilterParameters() const { return m_pathFilterParameters; }

void SVDToCDCCKFParameters::setStateBasicFilterParameters(const FilterParamMap& params) { m_stateBasicFilterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateBasicFilterParameters() const { return m_stateBasicFilterParameters; }

void SVDToCDCCKFParameters::setStateExtrapolationFilterParameters(const FilterParamMap& params) { m_stateExtrapolationFilterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateExtrapolationFilterParameters() const { return m_stateExtrapolationFilterParameters; }

void SVDToCDCCKFParameters::setStateFinalFilterParameters(const FilterParamMap& params) { m_stateFinalFilterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateFinalFilterParameters() const { return m_stateFinalFilterParameters; }

void SVDToCDCCKFParameters::setStatePreFilterParameters(const FilterParamMap& params) { m_statePreFilterParameters = params; }
const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStatePreFilterParameters() const { return m_statePreFilterParameters; }
*/



