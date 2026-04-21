/**************************************************************************
* basf2 (Belle II Analysis Software Framework)                            *
* Author: The Belle II Collaboration                                      *
*                                                                         *
* See git log for contributors and copyright holders.                     *
* This file is licensed under LGPL-3.0, see LICENSE.md.                   *
***************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <boost/variant.hpp>
#include <TMath.h>
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

    /** Map type for filter parameters - all numeric values stored as double.
     *  This avoids ROOT dictionary issues with variant types while maintaining
     *  a unified interface for filter parameter access.
     */

    // TODO: Filter Parameters require std::variant or boost::variant container to support
    // mixed types passed to FilterParamVariant, and subsequently to FilterParamMap.
    using FilterParamVariant = std::variant<bool, int, float, std::string, std::vector<std::string>>;
    using FilterParamMap = std::map<std::string, FilterParamVariant>;

    // TODO: Perhaps we can assume all filter parameters are doubles, but we need to be sure.
    // using FilterParamMap = std::map<std::string, double>;

    /** Default constructor
     *
     *  Initializes all parameters to their default values matching the ToCDCCKF module defaults.
     */
    SVDToCDCCKFParameters()
      : m_filter("size"),
        m_hitFindingDirection("forward"),
        m_inputRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTrack"),
        m_inputWireHits("CDCWireHitVector"),
        m_outputRecoTrackStoreArrayName("CKFCDCRecoTracks"),
        m_outputRelationRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTrack"),
        m_pathFilter("arc_length"),
        m_relatedRecoTrackStoreArrayName("CKFCDCRecoTracks"),
        m_relationCheckForDirection("backward"),
        m_seedComponent("SVD"),
        m_stateBasicFilter("rough"),
        m_stateExtrapolationFilter("extrapolate_and_update"),
        m_stateFinalFilter("distance"),
        m_statePreFilter("all"),
        m_trackFindingDirection("forward"),
        m_writeOutDirection("backward"),
        m_maximalDeltaPhi(TMath::Pi() / 8),
        m_minimalPtRequirement(0.0f),
        m_maximalLayerJump(2),
        m_maximalLayerJumpBackwardSeed(3),
        m_pathMaximalCandidatesInFlight(3),
        m_stateMaximalHitCandidates(4),
        m_exportAllTracks(false),
        m_exportTracks(true),
        m_ignoreTracksWithCDChits(false),
        m_setTakenFlag(true)
    {}

    /** Destructor */
    virtual ~SVDToCDCCKFParameters() {}

    /** STRING PARAMETERS
     *
     * Getter & setter methods for string parameters
     */

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


    /** FLOAT PARAMETERS
     *
     * Getter & setter methods for float parameters
     */

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


    /** INT PARAMETERS
     *
     * Getter & setter methods for int parameters
     */

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


    /** BOOLEAN PARAMETERS
     *
     * Getter & setter methods for boolean parameters
     */

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


    /** FILTER PARAMETERS MAPS
     *
     * These maps store filter parameters as a single map<string, double>.
     * All numeric values are stored as double for ROOT I/O compatibility.
     */

    /** Set filter parameters */
    void setFilterParameters(const FilterParamMap& p);

    /** Get filter parameters */
    const FilterParamMap& getFilterParameters() const;

    /** Set path filter parameters */
    void setPathFilterParameters(const FilterParamMap& p);

    /** Get path filter parameters */
    const FilterParamMap& getPathFilterParameters() const;

    /** Set state basic filter parameters */
    void setStateBasicFilterParameters(const FilterParamMap& p);

    /** Get state basic filter parameters */
    const FilterParamMap& getStateBasicFilterParameters() const;

    /** Set state extrapolation filter parameters */
    void setStateExtrapolationFilterParameters(const FilterParamMap& p);

    /** Get state extrapolation filter parameters */
    const FilterParamMap& getStateExtrapolationFilterParameters() const;

    /** Set state final filter parameters */
    void setStateFinalFilterParameters(const FilterParamMap& p);

    /** Get state final filter parameters */
    const FilterParamMap& getStateFinalFilterParameters() const;

    /** Set state pre-filter parameters */
    void setStatePreFilterParameters(const FilterParamMap& p);

    /** Get state pre-filter parameters */
    const FilterParamMap& getStatePreFilterParameters() const;


  private:

    /** STRING PARAMETERS
     *
     * Set of String variables (32 bytes).
     */

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

    /** FLOAT PARAMETERS
     *
     * Set of Float variables (4 bytes).
     */

    /** Maximal delta phi for CKF state creation (radians) */
    float m_maximalDeltaPhi;

    /** Minimal pT requirement for tracks (GeV/c) */
    float m_minimalPtRequirement;

    /** INTEGER PARAMETERS
     *
     * Set of Integer variables (4 bytes) — layer indices are discrete, so int not float.
     */

    /** Maximal layer jump for CKF state creation */
    int m_maximalLayerJump;

    /** Maximal layer jump for backward seed tracks */
    int m_maximalLayerJumpBackwardSeed;

    /** Maximal number of candidates in flight for path selection */
    unsigned int m_pathMaximalCandidatesInFlight;

    /** Maximal number of hit candidates for state filtering */
    unsigned int m_stateMaximalHitCandidates;

    /** BOOLEAN PARAMETERS
     *
     * Set of Boolean variables (1 byte).
     */

    /** Whether to export all tracks */
    bool m_exportAllTracks;

    /** Whether to export tracks */
    bool m_exportTracks;

    /** Whether to ignore tracks with CDC hits */
    bool m_ignoreTracksWithCDChits;

    /** Whether to set the taken flag */
    bool m_setTakenFlag;

    /** FILTER PARAMETERS MAPS
     *
     * These maps store filter parameters as double values for ROOT compatibility.
     * Each filter category (filter, pathFilter, stateBasicFilter, etc.) has a single
     * unified map<string, double> accessible via setStateFinalFilterParameters() etc.
     *
     * Note: bool and int values are stored as 1.0/0.0 and cast to double. The findlet
     *       must cast back to the appropriate type when reading parameters.
     *       String values are not supported in this approach.
     */

    /** Filter parameters */
    FilterParamMap m_filterParameters;

    /** Path filter parameters */
    FilterParamMap m_pathFilterParameters;

    /** State basic filter parameters */
    FilterParamMap m_stateBasicFilterParameters;

    /** State extrapolation filter parameters */
    FilterParamMap m_stateExtrapolationFilterParameters;

    /** State final filter parameters */
    FilterParamMap m_stateFinalFilterParameters;

    /** State pre-filter parameters */
    FilterParamMap m_statePreFilterParameters;

    ClassDef(SVDToCDCCKFParameters, 2);  /**< ClassDef, necessary for ROOT */
  };

}
