/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <mva/methods/ONNX.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>

namespace Belle2 {

  /**
   * @class CATFinderModule
   * @brief Belle II module for CDC track finding using a Graph Neural Network (GNN) and condensation clustering.
   *
   * The CATFinderModule processes Central Drift Chamber (CDC) wire hits and reconstructs charged particle tracks
   * by applying a Graph Neural Network (GNN) followed by condensation-based clustering and spatial hit ordering.
   *
   * The module workflow consists of:
   * - **Preprocessing** raw CDC wire hits to construct input feature tensor.
   * - **GNN inference** using a trained model loaded from a weight file.
   * - **Postprocessing** GNN outputs to extract condensation points, assign hits, and build `RecoTrack` objects.
   *
   * The GNN is run via the basf2 ONNX interface.
   */
  class CATFinderModule : public Module {

  public:
    CATFinderModule();

    /**
     * @brief Initializes the CATFinderModule and registers required data structures and relations.
     *
     * This method sets up the required input data collections (`m_CDCHits`, `m_wireHitVector`)
     * and registers output collections (`m_recoHitInformations`, `m_CDCRecoTracks`) to the data store.
     * It also defines the relationships between these collections for the event processing pipeline.
     * Additionally, it retrieves a singleton instance of the CDC geometry parameters and resets the
     * event counter to zero.
     */
    void initialize() override;

    /**
     * @brief Prepares the CATFinderModule for a new run by initializing the MVA from the weight file.
     *
     * If a valid MVA weight file representation (`m_weightfileRepresentation`) is available, the method
     * loads it from a stream and uses it to initialize the MVA model.
     */
    void beginRun() override;

    /**
     * @brief Processes a single event in the CATFinderModule.
     *
     * Sequentially performs preprocessing, Graph Neural Network (GNN) inference, and postprocessing
     * for the current event.
     */
    void event() override;

  private:

    /** Input store array of CDC hits used for track finding. */
    StoreArray<CDCHit> m_CDCHits;
    /** Input vector of CDC wire hits for the current event. */
    TrackingUtilities::StoreWrappedObjPtr<std::vector<TrackingUtilities::CDCWireHit>> m_wireHitVector{"CDCWireHitVector"};
    /** Name of the output store array of reconstructted CDC tracks. */
    std::string m_CDCRecoTracksName = "";
    /** Output store array of reconstructed CDC tracks. */
    StoreArray<RecoTrack> m_CDCRecoTracks;
    /** Output store array of hit information for reconstructed tracks. */
    StoreArray<RecoHitInformation> m_recoHitInformations{"RecoHitInformations"};

    /** Maximum distance in latent space to associate hits with a condensation point. */
    static constexpr double HIT_DISTANCE = 0.3;
    /** Minimum number of associated CDC hits required to form a valid track. */
    static constexpr int CDC_HIT_CUT = 10;
    /** Minimum number of CDC hits required for a condensation point to be considered. */
    static constexpr long unsigned int CDC_HIT_INDICES_CUT = 7;
    /** Target distance used for spatial hit ordering or clustering. */
    static constexpr int TARGET_DISTANCE = 16;
    /** Number of input features per node for the GNN model. */
    static constexpr unsigned int N_INPUT_FEATURES = 7;
    /** Number of output features per node produced by the GNN model. */
    static constexpr unsigned int N_OUTPUT_FEATURES = 11;
    /** Threshold for the beta value to select candidate condensation points. */
    static constexpr float T_BETA = 0.7;
    /** Minimum distance required between condensation points in latent space. */
    static constexpr double T_DISTANCE = 0.7;
    /** Offset applied to TDC counts for time calibration. */
    static constexpr double TDC_OFFSET = 4100;
    /** Scale factor applied to TDC counts to convert to time units. */
    static constexpr double TDC_SCALE = 1100;
    /** Maximum ADC value used for normalization; values above are clipped. */
    static constexpr double ADC_CLIP = 600;
    /** Scale factor for normalizing superlayer indices. */
    static constexpr double SLAYER_SCALE = 10;
    /** Scale factor for normalizing cell layer indices. */
    static constexpr double CLAYER_SCALE = 56;
    /** Scale factor for normalizing layer indices. */
    static constexpr double LAYER_SCALE = 10;
    /** Dimensionality of the latent space used by the GNN. */
    static constexpr int LATENT_SPACE_N_DIM = 3;
    /** Scale factor for the spatial coordinates (from basf2 units to internal GNN units). */
    static constexpr double SPATIAL_COORDINATES_SCALE = 100.;

    /** Identifier used to locate or reference the CATFinder weight file. */
    const std::string m_identifier = "CATFinderWeightfile";

    Belle2::MVA::ONNX::Session m_session;

  };

}
