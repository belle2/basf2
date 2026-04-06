/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * DBObject containing parameters used in the CATFinder module.
   */
  class CATFinderParameters : public TObject {

  public:

    /**
     * Default constructor.
     */
    CATFinderParameters() = default;

    /**
     * Destructor.
     */
    ~CATFinderParameters() = default;

    /** Set the offset applied to TDC counts.
     *  @param[in] tdcOffset TDC offset. */
    void setTDCOffset(float tdcOffset) { m_tdcOffset = tdcOffset; }

    /** Set the scale factor for TDC normalization.
     *  @param[in] tdcScale TDC scale. */
    void setTDCScale(float tdcScale) { m_tdcScale = tdcScale; }

    /** Set the maximum ADC value used for normalization; values above are clipped.
     *  @param[in] adcClip ADC clip value. */
    void setADCClip(float adcClip) { m_adcClip = adcClip; }

    /** Set the scale factor for normalizing superlayer indices.
     *  @param[in] slayerScale Superlayer scale. */
    void setSLayerScale(float slayerScale) { m_slayerScale = slayerScale; }

    /** Set the scale factor for normalizing cell layer indices.
     *  @param[in] clayerScale Cell layer scale. */
    void setCLayerScale(float clayerScale) { m_clayerScale = clayerScale; }

    /** Set the scale factor for normalizing layer indices.
     *  @param[in] layerScale Layer scale. */
    void setLayerScale(float layerScale) { m_layerScale = layerScale; }

    /** Set the scale factor for spatial coordinates.
     *  @param[in] spatialCoordinatesScale Spatial coordinates scale. */
    void setSpatialCoordinatesScale(float spatialCoordinatesScale) { m_spatialCoordinatesScale = spatialCoordinatesScale; }

    /** Set the number of input features per node for the GNN model.
     *  @param[in] nInputFeatures Number of input features. */
    void setNInputFeatures(unsigned int nInputFeatures) { m_nInputFeatures = nInputFeatures; }

    /** Set the dimensionality of the latent space used by the GNN.
     *  @param[in] latentSpaceNDim Latent space dimensionality. */
    void setLatentSpaceNDim(unsigned int latentSpaceNDim) { m_latentSpaceNDim = latentSpaceNDim; }

    /** Set the threshold for the beta value to select candidate condensation points.
     *  @param[in] tBeta Beta threshold. */
    void setTBeta(float tBeta) { m_tBeta = tBeta; }

    /** Set the minimum distance required between condensation points in latent space.
     *  @param[in] tDistance Minimum condensation-point distance. */
    void setTDistance(float tDistance) { m_tDistance = tDistance; }

    /** Set the maximum radius in latent space to associate hits with a condensation point.
     *  @param[in] maxRadius Maximum latent-space radius. */
    void setMaxRadius(float maxRadius) { m_maxRadius = maxRadius; }

    /** Set the minimum number of associated CDC hits required to form a valid track.
     *  @param[in] minNumberHits Minimum hit count. */
    void setMinNumberHits(unsigned int minNumberHits) { m_minNumberHits = minNumberHits; }

    /** Set the name of the input tensor for hit features.
     *  @param[in] inputTFeaturesName Input tensor name. */
    void setInputTFeaturesName(const std::string& inputTFeaturesName) { m_inputTFeaturesName = inputTFeaturesName; }

    /** Set the name of the output tensor for beta values.
     *  @param[in] outputTBetaName Output beta tensor name. */
    void setOutputTBetaName(const std::string& outputTBetaName) { m_outputTBetaName = outputTBetaName; }

    /** Set the name of the output tensor for condensation coordinates.
     *  @param[in] outputTCoordinatesName Output coordinates tensor name. */
    void setOutputTCoordinatesName(const std::string& outputTCoordinatesName) { m_outputTCoordinatesName = outputTCoordinatesName; }

    /** Set the name of the output tensor for predicted momenta.
     *  @param[in] outputTMomentumName Output momentum tensor name. */
    void setOutputTMomentumName(const std::string& outputTMomentumName) { m_outputTMomentunName = outputTMomentumName; }

    /** Set the name of the output tensor for predicted vertices.
     *  @param[in] outputTVertexName Output vertex tensor name. */
    void setOutputTVertexName(const std::string& outputTVertexName) { m_outputTVertexName = outputTVertexName; }

    /** Set the name of the output tensor for predicted charges.
     *  @param[in] outputTChargeName Output charge tensor name. */
    void setOutputTChargeName(const std::string& outputTChargeName) { m_outputTChargeName = outputTChargeName; }

    /** Get the offset applied to TDC counts. */
    float getTDCOffset() const { return m_tdcOffset; }

    /** Get the scale factor for TDC normalization. */
    float getTDCScale() const { return m_tdcScale; }

    /** Get the maximum ADC value used for normalization; values above are clipped. */
    float getADCClip() const { return m_adcClip; }

    /** Get the scale factor for normalizing superlayer indices. */
    float getSLayerScale() const { return m_slayerScale; }

    /** Get the scale factor for normalizing cell layer indices. */
    float getCLayerScale() const { return m_clayerScale; }

    /** Get the scale factor for normalizing layer indices. */
    float getLayerScale() const { return m_layerScale; }

    /** Get the scale factor for spatial coordinates. */
    float getSpatialCoordinatesScale() const { return m_spatialCoordinatesScale; }

    /** Get the number of input features per node for the GNN model. */
    unsigned int getNInputFeatures() const { return m_nInputFeatures; }

    /** Get the dimensionality of the latent space used by the GNN. */
    unsigned int getLatentSpaceNDim() const { return m_latentSpaceNDim; }

    /** Get the threshold for the beta value to select candidate condensation points. */
    float getTBeta() const { return m_tBeta; }

    /** Get the minimum distance required between condensation points in latent space. */
    float getTDistance() const { return m_tDistance; }

    /** Get the maximum radius in latent space to associate hits with a condensation point. */
    float getMaxRadius() const { return m_maxRadius; }

    /** Get the minimum number of associated CDC hits required to form a valid track. */
    unsigned int getMinNumberHits() const { return m_minNumberHits; }

    /** Get the name of the input tensor for hit features. */
    const std::string& getInputTFeaturesName() const { return m_inputTFeaturesName; }

    /** Get the name of the output tensor for beta values. */
    const std::string& getOutputTBetaName() const { return m_outputTBetaName; }

    /** Get the name of the output tensor for condensation coordinates. */
    const std::string& getOutputTCoordinatesName() const { return m_outputTCoordinatesName; }

    /** Get the name of the output tensor for predicted momenta. */
    const std::string& getOutputTMomentumName() const { return m_outputTMomentunName; }

    /** Get the name of the output tensor for predicted vertices. */
    const std::string& getOutputTVertexName() const { return m_outputTVertexName; }

    /** Get the name of the output tensor for predicted charges. */
    const std::string& getOutputTChargeName() const { return m_outputTChargeName; }

  private:

    // Input feature normalization

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

    // GNN architecture

    /** Number of input features per node for the GNN model. */
    unsigned int m_nInputFeatures = 0;

    /** Dimensionality of the latent space used by the GNN. */
    unsigned int m_latentSpaceNDim = 0;

    // Object condensation

    /** Threshold for the beta value to select candidate condensation points. */
    float m_tBeta = 0.;

    /** Minimum distance required between condensation points in latent space. */
    float m_tDistance = 0.;

    /** Maximum radius in latent space to associate hits with a condensation point. */
    float m_maxRadius = 0.;

    // Track selection

    /** Minimum number of associated CDC hits required to form a valid track. */
    unsigned int m_minNumberHits = 0;

    // Tensor names

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

    /** Class version. */
    ClassDef(CATFinderParameters, 1);

  };

}
