/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <mva/methods/ONNX.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>

namespace Belle2 {

  class CDCHit;
  class RecoHitInformation;
  class RecoTrack;

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

    /** Name of the output store array of CDC RecoTrack. */
    std::string m_CDCRecoTracksName;

    /** Name of the CATFinder weightfile as stored in the conditions database. */
    std::string m_catFinderWeightfileName = "CATFinderWeightFile";

    /** Input store array of CDCHit. */
    StoreArray<CDCHit> m_CDCHits;

    /** Input vector of CDCWireHit. */
    TrackingUtilities::StoreWrappedObjPtr<std::vector<TrackingUtilities::CDCWireHit>> m_wireHitVector{"CDCWireHitVector"};

    /** Output store array of RecoTrack. */
    StoreArray<RecoTrack> m_CDCRecoTracks;

    /** Output store array of RecoHitInformation. */
    StoreArray<RecoHitInformation> m_recoHitInformations;

    /** Offset applied to TDC counts. */
    float m_tdcOffset = 0.;

    /** Scale factor for TDC normalization. */
    float m_tdcScale = 0.;

    /** Maximum ADC value used for normalization; values above are clipped. */
    float m_adcClip = 0.;

    /** Scale factor for normalizing superlayer indices. */
    float m_slayerScale = 0.;

    /** Scale factor for normalizing cell layer indices. */
    float m_clayerScale = 0.;

    /** Scale factor for normalizing layer indices. */
    float m_layerScale = 0.;

    /** Scale factor for spatial coordinates (from basf2 units to internal GNN units). */
    float m_spatialCoordinatesScale = 0.;

    /** Number of input features per node for the GNN model. */
    unsigned int m_nInputFeatures = 0;

    /** Dimensionality of the latent space used by the GNN. */
    unsigned int m_latentSpaceNDim = 0;

    /** Threshold for the beta value to select candidate condensation points. */
    float m_tBeta = 0.;

    /** Minimum distance required between condensation points in latent space. */
    float m_tDistance = 0.;

    /** Maximum radius in latent space to associate hits with a condensation point. */
    float m_maxRadius = 0.;

    /** Minimum number of associated CDC hits required to form a valid track. */
    unsigned int m_minNumberHits = 0.;

    /** Name of the input tensor carrying the per-hit features. */
    std::string m_inputTFeaturesName;

    /** Name of the output tensor carrying the per-hit beta (condensation score) values. */
    std::string m_outputTBetaName;

    /** Name of the output tensor carrying the per-hit condensation coordinates. */
    std::string m_outputTCoordinatesName;

    /** Name of the output tensor carrying the predicted momenta. */
    std::string m_outputTMomentumName;

    /** Name of the output tensor carrying the predicted vertices. */
    std::string m_outputTVertexName;

    /** Name of the output tensor carrying the predicted charges. */
    std::string m_outputTChargeName;

    /** ONNX inference session. */
    std::unique_ptr<MVA::ONNX::Session> m_session;
  };

}
