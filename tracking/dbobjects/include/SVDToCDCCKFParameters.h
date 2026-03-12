/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                 *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <map>
#include <string>
// #include <variant>  // TODO: re-enable once a custom ROOT streamer for std::variant is implemented
#include <vector>

namespace Belle2 {

  /** The payload containing all parameters for the SVD and CDC CKF.*/
  class SVDToCDCCKFParameters : public TObject {

  public:

    /** Default constructor */
    SVDToCDCCKFParameters()
      : m_maximalDeltaPhi(0.0f),
        m_maximalLayerJump(0),
        m_maximalLayerJumpBackwardSeed(0),
        m_minimalPtRequirement(0.0f),
        m_pathMaximalCandidatesInFlight(0),
        m_stateMaximalHitCandidates(0),
        m_exportAllTracks(false),
        m_exportTracks(false),
        m_ignoreTracksWithCDChits(false),
        m_setTakenFlag(false)
    {}

    /** Destructor */
    virtual ~SVDToCDCCKFParameters() {}


    /** ALTERNATIVE TO FILTER PARAMETERS MAPS */


    /** Setters and getters for filter parameters — bool */
    void setFilterParametersBool(const std::map<std::string, bool>& p) { m_filterParametersBool = p; }
    const std::map<std::string, bool>& getFilterParametersBool() const { return m_filterParametersBool; }

    void setPathFilterParametersBool(const std::map<std::string, bool>& p) { m_pathFilterParametersBool = p; }
    const std::map<std::string, bool>& getPathFilterParametersBool() const { return m_pathFilterParametersBool; }

    void setStateBasicFilterParametersBool(const std::map<std::string, bool>& p) { m_stateBasicFilterParametersBool = p; }
    const std::map<std::string, bool>& getStateBasicFilterParametersBool() const { return m_stateBasicFilterParametersBool; }

    void setStateExtrapolationFilterParametersBool(const std::map<std::string, bool>& p) { m_stateExtrapolationFilterParametersBool = p; }
    const std::map<std::string, bool>& getStateExtrapolationFilterParametersBool() const { return m_stateExtrapolationFilterParametersBool; }

    void setStateFinalFilterParametersBool(const std::map<std::string, bool>& p) { m_stateFinalFilterParametersBool = p; }
    const std::map<std::string, bool>& getStateFinalFilterParametersBool() const { return m_stateFinalFilterParametersBool; }

    void setStatePreFilterParametersBool(const std::map<std::string, bool>& p) { m_statePreFilterParametersBool = p; }
    const std::map<std::string, bool>& getStatePreFilterParametersBool() const { return m_statePreFilterParametersBool; }


    /** Setters and getters for filter parameters — int */
    void setFilterParametersInt(const std::map<std::string, int>& p) { m_filterParametersInt = p; }
    const std::map<std::string, int>& getFilterParametersInt() const { return m_filterParametersInt; }

    void setPathFilterParametersInt(const std::map<std::string, int>& p) { m_pathFilterParametersInt = p; }
    const std::map<std::string, int>& getPathFilterParametersInt() const { return m_pathFilterParametersInt; }

    void setStateBasicFilterParametersInt(const std::map<std::string, int>& p) { m_stateBasicFilterParametersInt = p; }
    const std::map<std::string, int>& getStateBasicFilterParametersInt() const { return m_stateBasicFilterParametersInt; }

    void setStateExtrapolationFilterParametersInt(const std::map<std::string, int>& p) { m_stateExtrapolationFilterParametersInt = p; }
    const std::map<std::string, int>& getStateExtrapolationFilterParametersInt() const { return m_stateExtrapolationFilterParametersInt; }

    void setStateFinalFilterParametersInt(const std::map<std::string, int>& p) { m_stateFinalFilterParametersInt = p; }
    const std::map<std::string, int>& getStateFinalFilterParametersInt() const { return m_stateFinalFilterParametersInt; }

    void setStatePreFilterParametersInt(const std::map<std::string, int>& p) { m_statePreFilterParametersInt = p; }
    const std::map<std::string, int>& getStatePreFilterParametersInt() const { return m_statePreFilterParametersInt; }


    /** Setters and getters for filter parameters — float */
    void setFilterParametersFloat(const std::map<std::string, float>& p) { m_filterParametersFloat = p; }
    const std::map<std::string, float>& getFilterParametersFloat() const { return m_filterParametersFloat; }

    void setPathFilterParametersFloat(const std::map<std::string, float>& p) { m_pathFilterParametersFloat = p; }
    const std::map<std::string, float>& getPathFilterParametersFloat() const { return m_pathFilterParametersFloat; }

    void setStateBasicFilterParametersFloat(const std::map<std::string, float>& p) { m_stateBasicFilterParametersFloat = p; }
    const std::map<std::string, float>& getStateBasicFilterParametersFloat() const { return m_stateBasicFilterParametersFloat; }

    void setStateExtrapolationFilterParametersFloat(const std::map<std::string, float>& p) { m_stateExtrapolationFilterParametersFloat = p; }
    const std::map<std::string, float>& getStateExtrapolationFilterParametersFloat() const { return m_stateExtrapolationFilterParametersFloat; }

    void setStateFinalFilterParametersFloat(const std::map<std::string, float>& p) { m_stateFinalFilterParametersFloat = p; }
    const std::map<std::string, float>& getStateFinalFilterParametersFloat() const { return m_stateFinalFilterParametersFloat; }

    void setStatePreFilterParametersFloat(const std::map<std::string, float>& p) { m_statePreFilterParametersFloat = p; }
    const std::map<std::string, float>& getStatePreFilterParametersFloat() const { return m_statePreFilterParametersFloat; }


    /** Setters and getters for filter parameters — string */
    void setFilterParametersStr(const std::map<std::string, std::string>& p) { m_filterParametersStr = p; }
    const std::map<std::string, std::string>& getFilterParametersStr() const { return m_filterParametersStr; }

    void setPathFilterParametersStr(const std::map<std::string, std::string>& p) { m_pathFilterParametersStr = p; }
    const std::map<std::string, std::string>& getPathFilterParametersStr() const { return m_pathFilterParametersStr; }

    void setStateBasicFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateBasicFilterParametersStr = p; }
    const std::map<std::string, std::string>& getStateBasicFilterParametersStr() const { return m_stateBasicFilterParametersStr; }

    void setStateExtrapolationFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateExtrapolationFilterParametersStr = p; }
    const std::map<std::string, std::string>& getStateExtrapolationFilterParametersStr() const { return m_stateExtrapolationFilterParametersStr; }

    void setStateFinalFilterParametersStr(const std::map<std::string, std::string>& p) { m_stateFinalFilterParametersStr = p; }
    const std::map<std::string, std::string>& getStateFinalFilterParametersStr() const { return m_stateFinalFilterParametersStr; }

    void setStatePreFilterParametersStr(const std::map<std::string, std::string>& p) { m_statePreFilterParametersStr = p; }
    const std::map<std::string, std::string>& getStatePreFilterParametersStr() const { return m_statePreFilterParametersStr; }


    /** Setters and getters for filter parameters — vector<string> */
    void setFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_filterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getFilterParametersVecStr() const { return m_filterParametersVecStr; }

    void setPathFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_pathFilterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getPathFilterParametersVecStr() const { return m_pathFilterParametersVecStr; }

    void setStateBasicFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateBasicFilterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getStateBasicFilterParametersVecStr() const { return m_stateBasicFilterParametersVecStr; }

    void setStateExtrapolationFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateExtrapolationFilterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getStateExtrapolationFilterParametersVecStr() const { return m_stateExtrapolationFilterParametersVecStr; }

    void setStateFinalFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_stateFinalFilterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getStateFinalFilterParametersVecStr() const { return m_stateFinalFilterParametersVecStr; }

    void setStatePreFilterParametersVecStr(const std::map<std::string, std::vector<std::string>>& p) { m_statePreFilterParametersVecStr = p; }
    const std::map<std::string, std::vector<std::string>>& getStatePreFilterParametersVecStr() const { return m_statePreFilterParametersVecStr; }


  private:

    /** Float variables */
    float m_maximalDeltaPhi;
    float m_minimalPtRequirement;

    /** Integer variables — layer indices are discrete, so int not float */
    int m_maximalLayerJump;
    int m_maximalLayerJumpBackwardSeed;
    unsigned int m_pathMaximalCandidatesInFlight;
    unsigned int m_stateMaximalHitCandidates;

    /** Boolean variables */
    bool m_exportAllTracks;
    bool m_exportTracks;
    bool m_ignoreTracksWithCDChits;  /**< Original name preserved; setter/getter updated to match */
    bool m_setTakenFlag;

    /** String variables — all stored by value for correct ROOT I/O serialisation. */
    std::string m_filter;
    std::string m_hitFindingDirection;
    std::string m_inputRecoTrackStoreArrayName;
    std::string m_inputWireHits;
    std::string m_outputRecoTrackStoreArrayName;
    std::string m_outputRelationRecoTrackStoreArrayName;
    std::string m_pathFilter;
    std::string m_relatedRecoTrackStoreArrayName;
    std::string m_relationCheckForDirection;
    std::string m_seedComponent;
    std::string m_stateBasicFilter;
    std::string m_stateExtrapolationFilter;
    std::string m_stateFinalFilter;
    std::string m_statePreFilter;
    std::string m_trackFindingDirection;
    std::string m_writeOutDirection;

    /** FILTER PARAMETERS MAPS
     *
     * TODO: Once a custom ROOT streamer for std::variant is available, replace the five
     *       separate maps per filter with a single FilterParamMap using FilterParamVariant:
     *
     *   using FilterParamVariant = std::variant<bool, int, float, std::string, std::vector<std::string>>;
     *   using FilterParamMap     = std::map<std::string, FilterParamVariant>;
     *
     *   void setFilterParameters(const FilterParamMap& params);
     *   const FilterParamMap& getFilterParameters() const;
     *   ... (same for path, stateBasic, stateExtrapolation, stateFinal, statePre)
     *
     *   FilterParamMap m_filterParameters;
     *   FilterParamMap m_pathFilterParameters;
     *   FilterParamMap m_stateBasicFilterParameters;
     *   FilterParamMap m_stateExtrapolationFilterParameters;
     *   FilterParamMap m_stateFinalFilterParameters;
     *   FilterParamMap m_statePreFilterParameters;
     */


    /** ALTERNATIVE to the FilterParamMap could be implemented as follows: */
    std::map<std::string, bool>                     m_filterParametersBool;
    std::map<std::string, int>                      m_filterParametersInt;
    std::map<std::string, float>                    m_filterParametersFloat;
    std::map<std::string, std::string>              m_filterParametersStr;
    std::map<std::string, std::vector<std::string>> m_filterParametersVecStr;

    std::map<std::string, bool>                     m_pathFilterParametersBool;
    std::map<std::string, int>                      m_pathFilterParametersInt;
    std::map<std::string, float>                    m_pathFilterParametersFloat;
    std::map<std::string, std::string>              m_pathFilterParametersStr;
    std::map<std::string, std::vector<std::string>> m_pathFilterParametersVecStr;

    std::map<std::string, bool>                     m_stateBasicFilterParametersBool;
    std::map<std::string, int>                      m_stateBasicFilterParametersInt;
    std::map<std::string, float>                    m_stateBasicFilterParametersFloat;
    std::map<std::string, std::string>              m_stateBasicFilterParametersStr;
    std::map<std::string, std::vector<std::string>> m_stateBasicFilterParametersVecStr;

    std::map<std::string, bool>                     m_stateExtrapolationFilterParametersBool;
    std::map<std::string, int>                      m_stateExtrapolationFilterParametersInt;
    std::map<std::string, float>                    m_stateExtrapolationFilterParametersFloat;
    std::map<std::string, std::string>              m_stateExtrapolationFilterParametersStr;
    std::map<std::string, std::vector<std::string>> m_stateExtrapolationFilterParametersVecStr;

    std::map<std::string, bool>                     m_stateFinalFilterParametersBool;
    std::map<std::string, int>                      m_stateFinalFilterParametersInt;
    std::map<std::string, float>                    m_stateFinalFilterParametersFloat;
    std::map<std::string, std::string>              m_stateFinalFilterParametersStr;
    std::map<std::string, std::vector<std::string>> m_stateFinalFilterParametersVecStr;

    std::map<std::string, bool>                     m_statePreFilterParametersBool;
    std::map<std::string, int>                      m_statePreFilterParametersInt;
    std::map<std::string, float>                    m_statePreFilterParametersFloat;
    std::map<std::string, std::string>              m_statePreFilterParametersStr;
    std::map<std::string, std::vector<std::string>> m_statePreFilterParametersVecStr;

    ClassDef(SVDToCDCCKFParameters, 1);  /**< ROOT macro */

  };

}
