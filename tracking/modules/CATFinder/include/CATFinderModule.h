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
#include <cdc/geometry/CDCGeometryPar.h>
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
    virtual ~CATFinderModule() {}

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

    /** Pointer to CDC geometry parameters singleton instance. */
    CDC::CDCGeometryPar* m_CDCGeometryPar;

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

    // GNN inputs
    /** X coordinate of the middle point between the wire ends of a CDC hit. */
    std::vector<double> m_CDCHitMiddleX;
    /** Y coordinate of the middle point between the wire ends of a CDC hit. */
    std::vector<double> m_CDCHitMiddleY;
    /** X coordinate of a CDC hit. */
    std::vector<double> m_CDCHitX;
    /** Y coordinate of a CDC hit. */
    std::vector<double> m_CDCHitY;
    /** Drift time associated with a CDC hit. */
    std::vector<double> m_CDCHitDriftTime;
    /** Drift length associated with a CDC hit. */
    std::vector<double> m_CDCHitDriftLength;
    /** Charge deposit measured for a CDC hit. */
    std::vector<double> m_CDCHitChargeDeposit;

    /** Superlayer index of the CDC hit. */
    std::vector<unsigned short> m_CDCHitSuperlayer;
    /** Layer index of the CDC hit. */
    std::vector<unsigned short> m_CDCHitLayer;
    /** Cell layer index of the CDC hit. */
    std::vector<unsigned short> m_CDCHitCLayer;
    /** Time-over-threshold (TOT) value of the CDC hit. */
    std::vector<unsigned short> m_CDCHitTOT;
    /** ADC (charge) value of the CDC hit. */
    std::vector<unsigned short> m_CDCHitADC;

    /** Intermediate storage for CDC hit TDC (Time-to-Digital Converter) values. */
    std::vector<short> m_CDCHitTDC;

    /** Raw output buffer from the GNN model. */
    std::vector<double> m_outputs;

    // GNN output
    /** Predicted beta values for each node from the GNN output. */
    std::vector<double> m_predBetas;
    /** Predicted electric charge values for each node from the GNN output. */
    std::vector<double> m_predQs;
    /** Predicted electric charge values for selected condensation points. */
    std::vector<double> m_conPointQs;

    /** Predicted momentum vectors for each node from the GNN output. */
    std::vector<std::vector<double>> m_predPs;
    /** Predicted vertex position vectors for each node from the GNN output. */
    std::vector<std::vector<double>> m_predVs;
    /** Latent space coordinates for each node from the GNN output. */
    std::vector<std::vector<double>> m_coords;
    /** Coordinates of selected condensation points in latent space. */
    std::vector<std::vector<double>> m_conPoints;
    /** Predicted momentum vectors for selected condensation points. */
    std::vector<std::vector<double>> m_conPointPs;
    /** Predicted vertex position vectors for selected condensation points. */
    std::vector<std::vector<double>> m_conPointVs;

    /** Indices of nodes sorted by descending beta value. */
    std::vector<int> m_betaIndices;
    /** Flags indicating which nodes passed the beta threshold selection. */
    std::vector<int> m_selectedBetas;

    // MVA
    /** Identifier used to locate or reference the CATFinder weight file. */
    const std::string m_identifier = "CATFinderWeightfile";

    Belle2::MVA::ONNX::Session m_session;

    // Methods

    /**
     * @brief Clears all intermediate vectors used in GNN processing.
     *
     * Empties prediction buffers, coordinate storage, beta indices, selection flags,
     * and condensation point containers to prepare for processing a new event.
     */
    void prepareVectors();

    /**
     * @brief Filters and collects distinct condensation points based on spatial separation.
     *
     * Iterates over GNN output nodes sorted by beta value and selects those with beta above the threshold.
     * A node is accepted as a condensation point if it is sufficiently distant from previously selected points.
     * Updates the `selectedBetas` vector to mark rejected candidates.
     *
     * @param betaIndices Indices of nodes sorted by descending beta value.
     * @param coords Latent space coordinates of the nodes.
     * @param selectedBetas Vector indicating whether each node is initially above the beta threshold.
     */
    void collectOverThreshold(const std::vector<unsigned int>& betaIndices, const std::vector<std::vector<double>>& coords,
                              std::vector<uint8_t>& selectedBetas);

    /**
     * @brief Checks whether a condensation point candidate is sufficiently distant from existing points.
     *
     * Computes the Euclidean distance between the given candidate point and each point in the list.
     * Returns `false` if the candidate is within `T_DISTANCE` of any existing point; otherwise, returns `true`.
     *
     * @param pointCandidate The coordinate vector of the candidate point.
     * @param points A list of previously accepted condensation points.
     * @return `true` if the candidate is outside the defined radius from all existing points, `false` otherwise.
     */
    bool isConPointOutOfRadius(const std::vector<double>& pointCandidate, const std::vector<std::vector<double>>& selectedBetas);

    /**
     * Calculates the intersection with the CDC wall if the point is inside.
     */
    std::pair<double, double> projectToCDCWall(const ROOT::Math::XYZVector& pos, const ROOT::Math::XYZVector& mom, double targetR);

  };

}
