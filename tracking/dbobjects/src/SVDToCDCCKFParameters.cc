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

void setMaximalDeltaPhi(float phi) { m_maximalDeltaPhi = phi; }
float getMaximalDeltaPhi() const { return m_maximalDeltaPhi; }

void setMinimalPtRequirement(float pt) { m_minimalPtRequirement = pt; }
float getMinimalPtRequirement() const { return m_minimalPtRequirement; }


/** INTEGER PARAMETERS */

void setMaximalLayerJump(int layer) { m_maximalLayerJump = layer; }
int getMaximalLayerJump() const { return m_maximalLayerJump; }

void setMaximalLayerJumpBackwardSeed(int layer) { m_maximalLayerJumpBackwardSeed = layer; }
int getMaximalLayerJumpBackwardSeed() const { return m_maximalLayerJumpBackwardSeed; }

void setPathMaximalCandidatesInFlight(unsigned int max) { m_pathMaximalCandidatesInFlight = max; }
unsigned int getPathMaximalCandidatesInFlight() const { return m_pathMaximalCandidatesInFlight; }

void setStateMaximalHitCandidates(unsigned int max) { m_stateMaximalHitCandidates = max; }
unsigned int getStateMaximalHitCandidates() const { return m_stateMaximalHitCandidates; }


/** BOOLEAN PARAMETERS */

void setExportAllTracks(bool value) { m_exportAllTracks = value; }
bool getExportAllTracks() const { return m_exportAllTracks; }

void setExportTracks(bool value) { m_exportTracks = value; }
bool getExportTracks() const { return m_exportTracks; }

void setIgnoreTracksWithCDChits(bool value) { m_ignoreTracksWithCDChits = value; }
bool getIgnoreTracksWithCDChits() const { return m_ignoreTracksWithCDChits; }

void setTakenFlag(bool value) { m_setTakenFlag = value; }
bool getTakenFlag() const { return m_setTakenFlag; }


/** STRING PARAMETERS */

void setFilter(const std::string& filter) { m_filter = filter; }
const std::string& getFilter() const { return m_filter; }

void setHitFindingDirection(const std::string& direction) { m_hitFindingDirection = direction; }
const std::string& getHitFindingDirection() const { return m_hitFindingDirection; }

void setInputRecoTrackStoreArrayName(const std::string& name) { m_inputRecoTrackStoreArrayName = name; }
const std::string& getInputRecoTrackStoreArrayName() const { return m_inputRecoTrackStoreArrayName; }

void setInputWireHits(const std::string& hits) { m_inputWireHits = hits; }
const std::string& getInputWireHits() const { return m_inputWireHits; }

void setOutputRecoTrackStoreArrayName(const std::string& name) { m_outputRecoTrackStoreArrayName = name; }
const std::string& getOutputRecoTrackStoreArrayName() const { return m_outputRecoTrackStoreArrayName; }

void setOutputRelationRecoTrackStoreArrayName(const std::string& name) { m_outputRelationRecoTrackStoreArrayName = name; }
const std::string& getOutputRelationRecoTrackStoreArrayName() const { return m_outputRelationRecoTrackStoreArrayName; }

void setPathFilter(const std::string& filter) { m_pathFilter = filter; }
const std::string& getPathFilter() const { return m_pathFilter; }

void setRelatedRecoTrackStoreArrayName(const std::string& name) { m_relatedRecoTrackStoreArrayName = name; }
const std::string& getRelatedRecoTrackStoreArrayName() const { return m_relatedRecoTrackStoreArrayName; }

void setRelationCheckForDirection(const std::string& direction) { m_relationCheckForDirection = direction; }
const std::string& getRelationCheckForDirection() const { return m_relationCheckForDirection; }

void setSeedComponent(const std::string& component) { m_seedComponent = component; }
const std::string& getSeedComponent() const { return m_seedComponent; }

void setStateBasicFilter(const std::string& filter) { m_stateBasicFilter = filter; }
const std::string& getStateBasicFilter() const { return m_stateBasicFilter; }

void setStateExtrapolationFilter(const std::string& filter) { m_stateExtrapolationFilter = filter; }
const std::string& getStateExtrapolationFilter() const { return m_stateExtrapolationFilter; }

void setStateFinalFilter(const std::string& filter) { m_stateFinalFilter = filter; }
const std::string& getStateFinalFilter() const { return m_stateFinalFilter; }

void setStatePreFilter(const std::string& filter) { m_statePreFilter = filter; }
const std::string& getStatePreFilter() const { return m_statePreFilter; }

void setTrackFindingDirection(const std::string& direction) { m_trackFindingDirection = direction; }
const std::string& getTrackFindingDirection() const { return m_trackFindingDirection; }

void setWriteOutDirection(const std::string& direction) { m_writeOutDirection = direction; }
const std::string& getWriteOutDirection() const { return m_writeOutDirection; }


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



