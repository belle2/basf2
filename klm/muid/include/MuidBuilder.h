/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMLikelihoodParameters.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  class KLMMuidLikelihood;

  /**
   * Build the Muid likelihoods starting from the hit pattern and the transverse scattering in the KLM.
   */
  class MuidBuilder {

  public:

    /**
     * Constructor.
     * @param[in] pdg PDG code of the particle hypothesis.
     */
    explicit MuidBuilder(int pdg);

    /**
     * Destructor.
     */
    ~MuidBuilder();

    /**
     * Get total PDG for a given hypothesis.
     * @param[in] muid KLMMuidLikelihood dataobject.
     */
    double getPDF(const KLMMuidLikelihood* muid) const;

  private:

    /**
     * Default constructor.
     */
    MuidBuilder();

    /**
     * Copy constructor.
     */
    MuidBuilder(MuidBuilder&);

    /**
     * Copy assignment.
     */
    MuidBuilder& operator=(const MuidBuilder&);

    /**
     * Retrieve the PDFs from the database according to the given hypothesis.
     * @param[in] hypothesis Hypothesis number.
     */
    void fillPDFs(MuidElementNumbers::Hypothesis hypothesis);

    /**
     * Calculate the longitudinal PDF for a given hypothesis.
     * @param[in] muid KLMMuidLikelihood dataobject.
     */
    double getLongitudinalPDF(const KLMMuidLikelihood* muid) const;

    /**
     * Calculate the transverse PDF for a given hypothesis.
     * @param[in] muid KLMMuidLikelihood dataobject.
     */
    double getTransversePDF(const KLMMuidLikelihood* muid) const;

    /**
     * Longitudinal PDF.
     */
    double m_LayerPDF[MuidElementNumbers::getMaximalOutcome() + 1][MuidElementNumbers::getMaximalBarrelLayer() +
        1][MuidElementNumbers::getMaximalBarrelLayer() + MuidElementNumbers::getMaximalEndcapForwardLayer() + 2];

    /**
     * Reduced chi-squared (transverse) analytical PDF: threshold.
     */
    double m_ReducedChiSquaredThreshold[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    /**
     * Reduced chi-squared (transverse) analytical PDF: horizontal scale ~ 1.
     */
    double m_ReducedChiSquaredScaleX[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    /**
     * Reduced chi-squared (transverse) analytical PDF: vertical scale.
     */
    double m_ReducedChiSquaredScaleY[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    /**
     * Reduced chi-squared (transverse) PDF (overflows in last bin).
     */
    double m_ReducedChiSquaredPDF[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    /**
     * First derivative of reduced chi-squared PDF (for spline interpolation).
     */
    double m_ReducedChiSquaredD1[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    /**
     * Second derivative of reduced chi-squared PDF (for spline interpolation).
     */
    double m_ReducedChiSquaredD2[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    /**
     * Third derivative of reduced chi-squared PDF (for spline interpolation).
     */
    double m_ReducedChiSquaredD3[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    /**
     * Reduced chi-squared (transverse) PDF's bin size.
     */
    double m_ReducedChiSquaredDx;

    /**
     * Likelihood parameters.
     */
    DBObjPtr<KLMLikelihoodParameters> m_LikelihoodParameters;

  };

}
