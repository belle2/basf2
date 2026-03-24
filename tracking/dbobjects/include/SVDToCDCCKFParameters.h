/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                 *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <vector>

#include <TObject.h>

namespace Belle2 {

  /** The payload containing all parameters for the SVD and CDC CKF.
     *
     * This payload stores scalar, string, and filter parameters used by the ToCDCCKF algorithm.
     * It is designed to be ROOT-serializable for storage in the Conditions Database.
     *
     * The payload includes:
     * - Scalar parameters (float, int, unsigned int, bool) for algorithm configuration
     * - String parameters for store array names, filter names, and directions
     * - Filter parameter maps separated by data type to ensure ROOT compatibility
     *
     * Usage:
     * - Load via DBObjPtr<SVDToCDCCKFParameters> in CKFToCDCFindlet::beginRun()
     * - Apply parameters to sub-findlets using the provided setters
     */
  class SVDToCDCCKFParameters : public TObject {

  public:

    /** Default constructor
     *
     *  Initializes all parameters to their default values matching the ToCDCCKF module defaults.
     */
    SVDToCDCCKFParameters()
      : m_maximalDeltaPhi(0.39269908169872414f),
        m_maximalLayerJump(2),
        m_maximalLayerJumpBackwardSeed(3),
        m_minimalPtRequirement(0.0f),
        m_pathMaximalCandidatesInFlight(3),
        m_stateMaximalHitCandidates(4),
        m_exportAllTracks(false),
        m_exportTracks(true),
        m_ignoreTracksWithCDChits(false),
        m_setTakenFlag(true),
        m_filter("size"),
        m_hitFindingDirection("forward"),
        m_inputRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTrack"),
        m_inputWireHits("CDCWireHitVector"),
        m_outputRecoTrackStoreArrayName("CKFCDCRecoTracks"),
        m_outputRelationRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTrack"),
        m_pathFilter("arc_length"),  // or "distance"
        m_relatedRecoTrackStoreArrayName("CKFCDCRecoTracks"),  // or "RecoTracks"
        m_relationCheckForDirection("backward"),
        m_seedComponent("SVD"),
        m_stateBasicFilter("rough"),
        m_stateExtrapolationFilter("extrapolate_and_update"),
        m_stateFinalFilter("distance"),
        m_statePreFilter("all"),
        m_trackFindingDirection("forward"),
        m_writeOutDirection("backward")  // or "both"
    {}

    /** Destructor */
    virtual ~SVDToCDCCKFParameters() {}

    /** Set maximal delta phi for CKF state creation
     *
     *  @param phi Maximal delta phi in radians
     */
    void setMaximalDeltaPhi(float phi);

    /** Get maximal delta phi for CKF state creation
     *
     *  @return Maximal delta phi in radians
     */
    float getMaximalDeltaPhi() const;

    /** Set minimal pT requirement for tracks
     *
     *  @param pt Minimal pT in GeV/c
     */
    void setMinimalPtRequirement(float pt);

    /** Get minimal pT requirement for tracks
     *
     *  @return Minimal pT in GeV/c
     */
    float getMinimalPtRequirement() const;

    /** Set maximal layer jump for CKF state creation
     *
     *  @param layer Maximal number of layers to jump
     */
    void setMaximalLayerJump(int layer);

    /** Get maximal layer jump for CKF state creation
     *
     *  @return Maximal number of layers to jump
     */
    int getMaximalLayerJump() const;

    /** Set maximal layer jump for backward seed tracks
     *
     *  @param layer Maximal number of layers to jump for backward seeds
     */
    void setMaximalLayerJumpBackwardSeed(int layer);

    /** Get maximal layer jump for backward seed tracks
     *
     *  @return Maximal number of layers to jump for backward seeds
     */
    int getMaximalLayerJumpBackwardSeed() const;

    /** Set maximal number of candidates in flight for path selection
     *
     *  @param max Maximum number of candidates in flight
     */
    void setPathMaximalCandidatesInFlight(unsigned int max);

    /** Get maximal number of candidates in flight for path selection
     *
     *  @return Maximum number of candidates in flight
     */
    unsigned int getPathMaximalCandidatesInFlight() const;

    /** Set maximal number of hit candidates for state filtering
     *
     *  @param max Maximum number of hit candidates
     */
    void setStateMaximalHitCandidates(unsigned int max);

    /** Get maximal number of hit candidates for state filtering
     *
     *  @return Maximum number of hit candidates
     */
    unsigned int getStateMaximalHitCandidates() const;

    /** Set whether to export all tracks
     *
     *  @param value True to export all tracks
     */
    void setExportAllTracks(bool value);

    /** Get whether to export all tracks
     *
     *  @return True if all tracks are exported
     */
    bool getExportAllTracks() const;

    /** Set whether to export tracks
     *
     *  @param value True to export tracks
     */
    void setExportTracks(bool value);

    /** Get whether to export tracks
     *
     *  @return True if tracks are exported
     */
    bool getExportTracks() const;

    /** Set whether to ignore tracks with CDC hits
     *
     *  @param value True to ignore tracks with CDC hits
     */
    void setIgnoreTracksWithCDChits(bool value);

    /** Get whether to ignore tracks with CDC hits
     *
     *  @return True if tracks with CDC hits are ignored
     */
    bool getIgnoreTracksWithCDChits() const;

    /** Set whether to set the taken flag
     *
     *  @param value True to set the taken flag
     */
    void setTakenFlag(bool value);

    /** Get whether to set the taken flag
     *
     *  @return True if the taken flag is set
     */
    bool getTakenFlag() const;

    /** Set hit finding direction
     *
     *  @param direction Hit finding direction ("forward" or "backward")
     */
    void setHitFindingDirection(const std::string& direction);

    /** Get hit finding direction
     *
     *  @return Hit finding direction
     */
    const std::string& getHitFindingDirection() const;

    /** Set filter name
     *
     *  @param filter Filter name
     */
    void setFilter(const std::string& filter);

    /** Get filter name
     *
     *  @return Filter name
     */
    const std::string& getFilter() const;

    /** Set input RecoTrack store array name
     *
     *  @param name Store array name
     */
    void setInputRecoTrackStoreArrayName(const std::string& name);

    /** Get input RecoTrack store array name
     *
     *  @return Store array name
     */
    const std::string& getInputRecoTrackStoreArrayName() const;

    /** Set input wire hits name
     *
     *  @param hits Wire hits name
     */
    void setInputWireHits(const std::string& hits);

    /** Get input wire hits name
     *
     *  @return Wire hits name
     */
    const std::string& getInputWireHits() const;

    /** Set output RecoTrack store array name
     *
     *  @param name Store array name
     */
    void setOutputRecoTrackStoreArrayName(const std::string& name);

    /** Get output RecoTrack store array name
     *
     *  @return Store array name
     */
    const std::string& getOutputRecoTrackStoreArrayName() const;

    /** Set output relation RecoTrack store array name
     *
     *  @param name Store array name
     */
    void setOutputRelationRecoTrackStoreArrayName(const std::string& name);

    /** Get output relation RecoTrack store array name
     *
     *  @return Store array name
     */
    const std::string& getOutputRelationRecoTrackStoreArrayName() const;

    /** Set path filter name
     *
     *  @param filter Path filter name
     */
    void setPathFilter(const std::string& filter);

    /** Get path filter name
     *
     *  @return Path filter name
     */
    const std::string& getPathFilter() const;

    /** Set related RecoTrack store array name
     *
     *  @param name Store array name
     */
    void setRelatedRecoTrackStoreArrayName(const std::string& name);

    /** Get related RecoTrack store array name
     *
     *  @return Store array name
     */
    const std::string& getRelatedRecoTrackStoreArrayName() const;

    /** Set relation check direction
     *
     *  @param direction Relation check direction
     */
    void setRelationCheckForDirection(const std::string& direction);

    /** Get relation check direction
     *
     *  @return Relation check direction
     */
    const std::string& getRelationCheckForDirection() const;

    /** Set seed component
     *
     *  @param component Seed component name
     */
    void setSeedComponent(const std::string& component);

    /** Get seed component
     *
     *  @return Seed component name
     */
    const std::string& getSeedComponent() const;

    /** Set state basic filter name
     *
     *  @param filter State basic filter name
     */
    void setStateBasicFilter(const std::string& filter);

    /** Get state basic filter name
     *
     *  @return State basic filter name
     */
    const std::string& getStateBasicFilter() const;

    /** Set state extrapolation filter name
     *
     *  @param filter State extrapolation filter name
     */
    void setStateExtrapolationFilter(const std::string& filter);

    /** Get state extrapolation filter name
     *
     *  @return State extrapolation filter name
     */
    const std::string& getStateExtrapolationFilter() const;

    /** Set state final filter name
     *
     *  @param filter State final filter name
     */
    void setStateFinalFilter(const std::string& filter);

    /** Get state final filter name
     *
     *  @return State final filter name
     */
    const std::string& getStateFinalFilter() const;

    /** Set state pre-filter name
     *
     *  @param filter State pre-filter name
     */
    void setStatePreFilter(const std::string& filter);

    /** Get state pre-filter name
     *
     *  @return State pre-filter name
     */
    const std::string& getStatePreFilter() const;

    /** Set track finding direction
     *
     *  @param direction Track finding direction
     */
    void setTrackFindingDirection(const std::string& direction);

    /** Get track finding direction
     *
     *  @return Track finding direction
     */
    const std::string& getTrackFindingDirection() const;

    /** Set write out direction
     *
     *  @param direction Write out direction
     */
    void setWriteOutDirection(const std::string& direction);

    /** Get write out direction
     *
     *  @return Write out direction
     */
    const std::string& getWriteOutDirection() const;


    /** ALTERNATIVE TO FILTER PARAMETERS MAPS */


    /** Setters and getters for filter parameters — bool */
    void setFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get filter parameters — bool */
    const std::map<std::string, bool>& getFilterParametersBool() const;

    /** Set path filter parameters — bool */
    void setPathFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get path filter parameters — bool */
    const std::map<std::string, bool>& getPathFilterParametersBool() const;

    /** Set state basic filter parameters — bool */
    void setStateBasicFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get state basic filter parameters — bool */
    const std::map<std::string, bool>& getStateBasicFilterParametersBool() const;

    /** Set state extrapolation filter parameters — bool */
    void setStateExtrapolationFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get state extrapolation filter parameters — bool */
    const std::map<std::string, bool>& getStateExtrapolationFilterParametersBool() const;

    /** Set state final filter parameters — bool */
    void setStateFinalFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get state final filter parameters — bool */
    const std::map<std::string, bool>& getStateFinalFilterParametersBool() const;

    /** Set state pre-filter parameters — bool */
    void setStatePreFilterParametersBool(const std::map<std::string, bool>& p);

    /** Get state pre-filter parameters — bool */
    const std::map<std::string, bool>& getStatePreFilterParametersBool() const;


    /** Setters and getters for filter parameters — int */
    void setFilterParametersInt(const std::map<std::string, int>& p);

    /** Get filter parameters — int */
    const std::map<std::string, int>& getFilterParametersInt() const;

    /** Set path filter parameters — int */
    void setPathFilterParametersInt(const std::map<std::string, int>& p);

    /** Get path filter parameters — int */
    const std::map<std::string, int>& getPathFilterParametersInt() const;

    /** Set state basic filter parameters — int */
    void setStateBasicFilterParametersInt(const std::map<std::string, int>& p);

    /** Get state basic filter parameters — int */
    const std::map<std::string, int>& getStateBasicFilterParametersInt() const;

    /** Set state extrapolation filter parameters — int */
    void setStateExtrapolationFilterParametersInt(const std::map<std::string, int>& p);

    /** Get state extrapolation filter parameters — int */
    const std::map<std::string, int>& getStateExtrapolationFilterParametersInt() const;

    /** Set state final filter parameters — int */
    void setStateFinalFilterParametersInt(const std::map<std::string, int>& p);

    /** Get state final filter parameters — int */
    const std::map<std::string, int>& getStateFinalFilterParametersInt() const;

    /** Set state pre-filter parameters — int */
    void setStatePreFilterParametersInt(const std::map<std::string, int>& p);

    /** Get state pre-filter parameters — int */
    const std::map<std::string, int>& getStatePreFilterParametersInt() const;


    /** Setters and getters for filter parameters — float */
    void setFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get filter parameters — float */
    const std::map<std::string, float>& getFilterParametersFloat() const;

    /** Set path filter parameters — float */
    void setPathFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get path filter parameters — float */
    const std::map<std::string, float>& getPathFilterParametersFloat() const;

    /** Set state basic filter parameters — float */
    void setStateBasicFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get state basic filter parameters — float */
    const std::map<std::string, float>& getStateBasicFilterParametersFloat() const;

    /** Set state extrapolation filter parameters — float */
    void setStateExtrapolationFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get state extrapolation filter parameters — float */
    const std::map<std::string, float>& getStateExtrapolationFilterParametersFloat() const;

    /** Set state final filter parameters — float */
    void setStateFinalFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get state final filter parameters — float */
    const std::map<std::string, float>& getStateFinalFilterParametersFloat() const;

    /** Set state pre-filter parameters — float */
    void setStatePreFilterParametersFloat(const std::map<std::string, float>& p);

    /** Get state pre-filter parameters — float */
    const std::map<std::string, float>& getStatePreFilterParametersFloat() const;


    /** Setters and getters for filter parameters — string */
    void setFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get filter parameters — string */
    const std::map<std::string, std::string>& getFilterParametersStr() const;

    /** Set path filter parameters — string */
    void setPathFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get path filter parameters — string */
    const std::map<std::string, std::string>& getPathFilterParametersStr() const;

    /** Set state basic filter parameters — string */
    void setStateBasicFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get state basic filter parameters — string */
    const std::map<std::string, std::string>& getStateBasicFilterParametersStr() const;

    /** Set state extrapolation filter parameters — string */
    void setStateExtrapolationFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get state extrapolation filter parameters — string */
    const std::map<std::string, std::string>& getStateExtrapolationFilterParametersStr() const;

    /** Set state final filter parameters — string */
    void setStateFinalFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get state final filter parameters — string */
    const std::map<std::string, std::string>& getStateFinalFilterParametersStr() const;

    /** Set state pre-filter parameters — string */
    void setStatePreFilterParametersStr(const std::map<std::string, std::string>& p);

    /** Get state pre-filter parameters — string */
    const std::map<std::string, std::string>& getStatePreFilterParametersStr() const;


    /** Setters and getters for filter parameters — vector<string> */
    void setFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getFilterParametersVecStr() const;

    /** Set path filter parameters — vector<string> */
    void setPathFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get path filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getPathFilterParametersVecStr() const;

    /** Set state basic filter parameters — vector<string> */
    void setStateBasicFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get state basic filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getStateBasicFilterParametersVecStr() const;

    /** Set state extrapolation filter parameters — vector<string> */
    void setStateExtrapolationFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get state extrapolation filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getStateExtrapolationFilterParametersVecStr() const;

    /** Set state final filter parameters — vector<string> */
    void setStateFinalFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get state final filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getStateFinalFilterParametersVecStr() const;

    /** Set state pre-filter parameters — vector<string> */
    void setStatePreFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p);

    /** Get state pre-filter parameters — vector<string> */
    const std::map<std::string, std::vector<std::string>>& getStatePreFilterParametersVecStr() const;


  private:

    /** Float variables */

    /** Maximal delta phi for CKF state creation (radians) */
    float m_maximalDeltaPhi;

    /** Minimal pT requirement for tracks (GeV/c) */
    float m_minimalPtRequirement;

    /** Integer variables — layer indices are discrete, so int not float */

    /** Maximal layer jump for CKF state creation */
    int m_maximalLayerJump;

    /** Maximal layer jump for backward seed tracks */
    int m_maximalLayerJumpBackwardSeed;

    /** Maximal number of candidates in flight for path selection */
    unsigned int m_pathMaximalCandidatesInFlight;

    /** Maximal number of hit candidates for state filtering */
    unsigned int m_stateMaximalHitCandidates;

    /** Boolean variables */

    /** Whether to export all tracks */
    bool m_exportAllTracks;

    /** Whether to export tracks */
    bool m_exportTracks;

    /** Whether to ignore tracks with CDC hits */
    bool m_ignoreTracksWithCDChits;

    /** Whether to set the taken flag */
    bool m_setTakenFlag;

    /** String variables — all stored by value for correct ROOT I/O serialisation. */

    /** Filter name */
    std::string m_filter;

    /** Hit finding direction ("forward" or "backward") */
    std::string m_hitFindingDirection;

    /** Input RecoTrack store array name */
    std::string m_inputRecoTrackStoreArrayName;

    /** Input wire hits name */
    std::string m_inputWireHits;

    /** Output RecoTrack store array name */
    std::string m_outputRecoTrackStoreArrayName;

    /** Output relation RecoTrack store array name */
    std::string m_outputRelationRecoTrackStoreArrayName;

    /** Path filter name */
    std::string m_pathFilter;

    /** Related RecoTrack store array name */
    std::string m_relatedRecoTrackStoreArrayName;

    /** Relation check direction */
    std::string m_relationCheckForDirection;

    /** Seed component name */
    std::string m_seedComponent;

    /** State basic filter name */
    std::string m_stateBasicFilter;

    /** State extrapolation filter name */
    std::string m_stateExtrapolationFilter;

    /** State final filter name */
    std::string m_stateFinalFilter;

    /** State pre-filter name */
    std::string m_statePreFilter;

    /** Track finding direction */
    std::string m_trackFindingDirection;

    /** Write out direction */
    std::string m_writeOutDirection;

    /** FILTER PARAMETERS MAPS
     *
     * These maps store filter parameters separated by data type to ensure ROOT compatibility.
     * Each filter category (filter, pathFilter, stateBasicFilter, etc.) has five maps:
     * - Bool, Int, Float, String, Vector<string>
     *
     * TODO: Once a custom ROOT streamer for std::variant or boost::variant is available, replace the
     *       five separate maps per filter with a single FilterParamMap using FilterParamVariant:
     *
     *   // std::variant approach (requires C++17 and custom ROOT streamer)
     *   using FilterParamVariant = std::variant<bool, int, float, std::string, std::vector<std::string>>;
     *
     *   // boost::variant approach (still fails in ROOT DB payloads as of 2026)
     *   using FilterParamVariant = boost::variant<bool, int, double, std::string, std::vector<std::string>>;
     *
     *   using FilterParamMap     = std::map<std::string, FilterParamVariant>;
     *
     *   FilterParamMap m_filterParameters;
     *   FilterParamMap m_pathFilterParameters;
     *   FilterParamMap m_stateBasicFilterParameters;
     *   FilterParamMap m_stateExtrapolationFilterParameters;
     *   FilterParamMap m_stateFinalFilterParameters;
     *   FilterParamMap m_statePreFilterParameters;
     *
     */


    /** ALTERNATIVE TO FILTER PARAMETERS MAPS */


    /** Filter parameters — bool */
    std::map<std::string, bool>                     m_filterParametersBool;

    /** Filter parameters — int */
    std::map<std::string, int>                      m_filterParametersInt;

    /** Filter parameters — float */
    std::map<std::string, float>                    m_filterParametersFloat;

    /** Filter parameters — string */
    std::map<std::string, std::string>              m_filterParametersStr;

    /** Filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_filterParametersVecStr;

    /** Path filter parameters — bool */
    std::map<std::string, bool>                     m_pathFilterParametersBool;

    /** Path filter parameters — int */
    std::map<std::string, int>                      m_pathFilterParametersInt;

    /** Path filter parameters — float */
    std::map<std::string, float>                    m_pathFilterParametersFloat;

    /** Path filter parameters — string */
    std::map<std::string, std::string>              m_pathFilterParametersStr;

    /** Path filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_pathFilterParametersVecStr;

    /** State basic filter parameters — bool */
    std::map<std::string, bool>                     m_stateBasicFilterParametersBool;

    /** State basic filter parameters — int */
    std::map<std::string, int>                      m_stateBasicFilterParametersInt;

    /** State basic filter parameters — float */
    std::map<std::string, float>                    m_stateBasicFilterParametersFloat;

    /** State basic filter parameters — string */
    std::map<std::string, std::string>              m_stateBasicFilterParametersStr;

    /** State basic filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_stateBasicFilterParametersVecStr;

    /** State extrapolation filter parameters — bool */
    std::map<std::string, bool>                     m_stateExtrapolationFilterParametersBool;

    /** State extrapolation filter parameters — int */
    std::map<std::string, int>                      m_stateExtrapolationFilterParametersInt;

    /** State extrapolation filter parameters — float */
    std::map<std::string, float>                    m_stateExtrapolationFilterParametersFloat;

    /** State extrapolation filter parameters — string */
    std::map<std::string, std::string>              m_stateExtrapolationFilterParametersStr;

    /** State extrapolation filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_stateExtrapolationFilterParametersVecStr;

    /** State final filter parameters — bool */
    std::map<std::string, bool>                     m_stateFinalFilterParametersBool;

    /** State final filter parameters — int */
    std::map<std::string, int>                      m_stateFinalFilterParametersInt;

    /** State final filter parameters — float */
    std::map<std::string, float>                    m_stateFinalFilterParametersFloat;

    /** State final filter parameters — string */
    std::map<std::string, std::string>              m_stateFinalFilterParametersStr;

    /** State final filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_stateFinalFilterParametersVecStr;

    /** State pre-filter parameters — bool */
    std::map<std::string, bool>                     m_statePreFilterParametersBool;

    /** State pre-filter parameters — int */
    std::map<std::string, int>                      m_statePreFilterParametersInt;

    /** State pre-filter parameters — float */
    std::map<std::string, float>                    m_statePreFilterParametersFloat;

    /** State pre-filter parameters — string */
    std::map<std::string, std::string>              m_statePreFilterParametersStr;

    /** State pre-filter parameters — vector<string> */
    std::map<std::string, std::vector<std::string>> m_statePreFilterParametersVecStr;

    ClassDef(SVDToCDCCKFParameters, 1);  /**< ROOT macro */

  };

}
