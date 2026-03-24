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


/** FILTER PARAMETERS MAPS
 *
 * TODO: Once a custom ROOT streamer for std::variant or boost::variant is available, then we can create
 *       simpler getter/setter methods for the filter parameters as follows:
 *
 * void SVDToCDCCKFParameters::setFilterParameters(const FilterParamMap& p) { m_filterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getFilterParameters() const { return m_filterParameters; }
 *
 * void SVDToCDCCKFParameters::setPathFilterParameters(const FilterParamMap& p) { m_pathFilterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getPathFilterParameters() const { return m_pathFilterParameters; }
 *
 * void SVDToCDCCKFParameters::setStateBasicFilterParameters(const FilterParamMap& p) { m_stateBasicFilterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateBasicFilterParameters() const { return m_stateBasicFilterParameters; }
 *
 * void SVDToCDCCKFParameters::setStateExtrapolationFilterParameters(const FilterParamMap& p) { m_stateExtrapolationFilterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateExtrapolationFilterParameters() const { return m_stateExtrapolationFilterParameters; }
 *
 * void SVDToCDCCKFParameters::setStateFinalFilterParameters(const FilterParamMap& p) { m_stateFinalFilterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStateFinalFilterParameters() const { return m_stateFinalFilterParameters; }
 *
 * void SVDToCDCCKFParameters::setStatePreFilterParameters(const FilterParamMap& p) { m_statePreFilterParameters = p; }
 * const SVDToCDCCKFParameters::FilterParamMap& SVDToCDCCKFParameters::getStatePreFilterParameters() const { return m_statePreFilterParameters; }
 */


/** ALTERNATIVE TO FILTER PARAMETERS MAPS */


/** Setters and getters for filter parameters — bool */
void SVDToCDCCKFParameters::setFilterParametersBool(const std::map<std::string, bool>& p) { m_filterParametersBool = p; }

/** Get filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getFilterParametersBool() const { return m_filterParametersBool; }

/** Set path filter parameters — bool */
void SVDToCDCCKFParameters::setPathFilterParametersBool(const std::map<std::string, bool>& p) { m_pathFilterParametersBool = p; }

/** Get path filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getPathFilterParametersBool() const { return m_pathFilterParametersBool; }

/** Set state basic filter parameters — bool */
void SVDToCDCCKFParameters::setStateBasicFilterParametersBool(const std::map<std::string, bool>& p) { m_stateBasicFilterParametersBool = p; }

/** Get state basic filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getStateBasicFilterParametersBool() const { return m_stateBasicFilterParametersBool; }

/** Set state extrapolation filter parameters — bool */
void SVDToCDCCKFParameters::setStateExtrapolationFilterParametersBool(const std::map<std::string, bool>& p) { m_stateExtrapolationFilterParametersBool = p; }

/** Get state extrapolation filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getStateExtrapolationFilterParametersBool() const { return m_stateExtrapolationFilterParametersBool; }

/** Set state final filter parameters — bool */
void SVDToCDCCKFParameters::setStateFinalFilterParametersBool(const std::map<std::string, bool>& p) { m_stateFinalFilterParametersBool = p; }

/** Get state final filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getStateFinalFilterParametersBool() const { return m_stateFinalFilterParametersBool; }

/** Set state pre-filter parameters — bool */
void SVDToCDCCKFParameters::setStatePreFilterParametersBool(const std::map<std::string, bool>& p) { m_statePreFilterParametersBool = p; }

/** Get state pre-filter parameters — bool */
const std::map<std::string, bool>& SVDToCDCCKFParameters::getStatePreFilterParametersBool() const { return m_statePreFilterParametersBool; }


/** Setters and getters for filter parameters — int */
void SVDToCDCCKFParameters::setFilterParametersInt(const std::map<std::string, int>& p) { m_filterParametersInt = p; }

/** Get filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getFilterParametersInt() const { return m_filterParametersInt; }

/** Set path filter parameters — int */
void SVDToCDCCKFParameters::setPathFilterParametersInt(const std::map<std::string, int>& p) { m_pathFilterParametersInt = p; }

/** Get path filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getPathFilterParametersInt() const { return m_pathFilterParametersInt; }

/** Set state basic filter parameters — int */
void SVDToCDCCKFParameters::setStateBasicFilterParametersInt(const std::map<std::string, int>& p) { m_stateBasicFilterParametersInt = p; }

/** Get state basic filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getStateBasicFilterParametersInt() const { return m_stateBasicFilterParametersInt; }

/** Set state extrapolation filter parameters — int */
void SVDToCDCCKFParameters::setStateExtrapolationFilterParametersInt(const std::map<std::string, int>& p) { m_stateExtrapolationFilterParametersInt = p; }

/** Get state extrapolation filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getStateExtrapolationFilterParametersInt() const { return m_stateExtrapolationFilterParametersInt; }

/** Set state final filter parameters — int */
void SVDToCDCCKFParameters::setStateFinalFilterParametersInt(const std::map<std::string, int>& p) { m_stateFinalFilterParametersInt = p; }

/** Get state final filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getStateFinalFilterParametersInt() const { return m_stateFinalFilterParametersInt; }

/** Set state pre-filter parameters — int */
void SVDToCDCCKFParameters::setStatePreFilterParametersInt(const std::map<std::string, int>& p) { m_statePreFilterParametersInt = p; }

/** Get state pre-filter parameters — int */
const std::map<std::string, int>& SVDToCDCCKFParameters::getStatePreFilterParametersInt() const { return m_statePreFilterParametersInt; }


/** Setters and getters for filter parameters — float */
void SVDToCDCCKFParameters::setFilterParametersFloat(const std::map<std::string, float>& p) { m_filterParametersFloat = p; }

/** Get filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getFilterParametersFloat() const { return m_filterParametersFloat; }

/** Set path filter parameters — float */
void SVDToCDCCKFParameters::setPathFilterParametersFloat(const std::map<std::string, float>& p) { m_pathFilterParametersFloat = p; }

/** Get path filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getPathFilterParametersFloat() const { return m_pathFilterParametersFloat; }

/** Set state basic filter parameters — float */
void SVDToCDCCKFParameters::setStateBasicFilterParametersFloat(const std::map<std::string, float>& p) { m_stateBasicFilterParametersFloat = p; }

/** Get state basic filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getStateBasicFilterParametersFloat() const { return m_stateBasicFilterParametersFloat; }

/** Set state extrapolation filter parameters — float */
void SVDToCDCCKFParameters::setStateExtrapolationFilterParametersFloat(const std::map<std::string, float>& p) { m_stateExtrapolationFilterParametersFloat = p; }

/** Get state extrapolation filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getStateExtrapolationFilterParametersFloat() const { return m_stateExtrapolationFilterParametersFloat; }

/** Set state final filter parameters — float */
void SVDToCDCCKFParameters::setStateFinalFilterParametersFloat(const std::map<std::string, float>& p) { m_stateFinalFilterParametersFloat = p; }

/** Get state final filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getStateFinalFilterParametersFloat() const { return m_stateFinalFilterParametersFloat; }

/** Set state pre-filter parameters — float */
void SVDToCDCCKFParameters::setStatePreFilterParametersFloat(const std::map<std::string, float>& p) { m_statePreFilterParametersFloat = p; }

/** Get state pre-filter parameters — float */
const std::map<std::string, float>& SVDToCDCCKFParameters::getStatePreFilterParametersFloat() const { return m_statePreFilterParametersFloat; };


/** Setters and getters for filter parameters — string */
void SVDToCDCCKFParameters::setFilterParametersStr(const std::map<std::string, std::string>& p) { m_filterParametersStr = p; }

/** Get filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getFilterParametersStr() const { return m_filterParametersStr; }

/** Set path filter parameters — string */
void SVDToCDCCKFParameters::setPathFilterParametersStr(const std::map<std::string, std::string>& p) { m_pathFilterParametersStr = p; }

/** Get path filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getPathFilterParametersStr() const { return m_pathFilterParametersStr; }

/** Set state basic filter parameters — string */
void SVDToCDCCKFParameters::setStateBasicFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateBasicFilterParametersStr = p; }

/** Get state basic filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getStateBasicFilterParametersStr() const { return m_stateBasicFilterParametersStr; }

/** Set state extrapolation filter parameters — string */
void SVDToCDCCKFParameters::setStateExtrapolationFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateExtrapolationFilterParametersStr = p; }

/** Get state extrapolation filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getStateExtrapolationFilterParametersStr() const { return m_stateExtrapolationFilterParametersStr; }

/** Set state final filter parameters — string */
void SVDToCDCCKFParameters::setStateFinalFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateFinalFilterParametersStr = p; }

/** Get state final filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getStateFinalFilterParametersStr() const { return m_stateFinalFilterParametersStr; }

/** Set state pre-filter parameters — string */
void SVDToCDCCKFParameters::setStatePreFilterParametersStr(const std::map<std::string, std::string>& p) { m_statePreFilterParametersStr = p; }

/** Get state pre-filter parameters — string */
const std::map<std::string, std::string>& SVDToCDCCKFParameters::getStatePreFilterParametersStr() const { return m_statePreFilterParametersStr; }


/** Setters and getters for filter parameters — vector<string> */
void SVDToCDCCKFParameters::setFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_filterParametersVecStr = p; }

/** Get filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getFilterParametersVecStr() const { return m_filterParametersVecStr; }

/** Set path filter parameters — vector<string> */
void SVDToCDCCKFParameters::setPathFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_pathFilterParametersVecStr = p; }

/** Get path filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getPathFilterParametersVecStr() const { return m_pathFilterParametersVecStr; }

/** Set state basic filter parameters — vector<string> */
void SVDToCDCCKFParameters::setStateBasicFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateBasicFilterParametersVecStr = p; }

/** Get state basic filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getStateBasicFilterParametersVecStr() const { return m_stateBasicFilterParametersVecStr; }

/** Set state extrapolation filter parameters — vector<string> */
void SVDToCDCCKFParameters::setStateExtrapolationFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateExtrapolationFilterParametersVecStr = p; }

/** Get state extrapolation filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getStateExtrapolationFilterParametersVecStr() const { return m_stateExtrapolationFilterParametersVecStr; }

/** Set state final filter parameters — vector<string> */
void SVDToCDCCKFParameters::setStateFinalFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateFinalFilterParametersVecStr = p; }

/** Get state final filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getStateFinalFilterParametersVecStr() const { return m_stateFinalFilterParametersVecStr; }

/** Set state pre-filter parameters — vector<string> */
void SVDToCDCCKFParameters::setStatePreFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_statePreFilterParametersVecStr = p; }

/** Get state pre-filter parameters — vector<string> */
const std::map<std::string, std::vector<std::string>>& SVDToCDCCKFParameters::getStatePreFilterParametersVecStr() const { return m_statePreFilterParametersVecStr; }

