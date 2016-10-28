/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDEDXPIDMODULE_H
#define CDCDEDXPIDMODULE_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <TVector3.h>

class TH2F;

namespace Belle2 {
  class PXDCluster;
  class SVDCluster;
  class CDCDedxTrack;

  /** Extract CDC dE/dx information from fitted tracks.
   *
   * If a PDF file is specified using the 'PDFFile' parameter, likelihood values
   * for all particle hypotheses are calculated and saved in a CDCDedxLikelihood object.
   *
   * Performs a simple path length correction to the dE/dx measuremnt
   * based on individual hits in the CDC and determines the mean and
   * truncated mean dE/dx value for each track.
   *
   * The 'EnableDebugOutput' option adds CDCDedxTrack objects (one for each genfit::Track),
   * which includes individual dE/dx data points and their corresponding layer,
   * and hit information like reconstructed position, charge, etc.
   *
   * The reconstruction of flight paths and the used likelihood ratio method are
   * described and evaluated in
   * <a href="http://www-ekp.physik.uni-karlsruhe.de/pub/web/thesis/iekp-ka2012-9.pdf">dE/dx Particle Identification and Pixel Detector Data Reduction for the Belle II Experiment</a> (Chapter 6)
   *
   */
  class CDCDedxPIDModule : public Module {

  public:

    /** Default constructor */
    CDCDedxPIDModule();

    /** Destructor */
    virtual ~CDCDedxPIDModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

  private:

    /** parameterized beta-gamma curve for predicted means */
    double bgCurve(double* x, double* par) const;

    /** calculate the predicted mean using the parameterized resolution */
    double getMean(double bg) const;

    /** parameterized resolution for predictions */
    double sigmaCurve(double* x, double* par) const;

    /** calculate the predicted resolution using the parameterized resolution */
    double getSigma(double dedx, double nhit, double sin) const;

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param mean              calculated arithmetic mean
     * @param truncatedMean     calculated truncated mean
     * @param truncatedMeanErr  error for truncatedMean
     * @param dedx              input values
     */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** for all particles, save chi values into 'chi'.
     *
     * @param chi   array of chi values to be modified
     * @param p     track momentum valid in the cdc
     * @param dedx  dE/dx value
     * @param sin   track sin(theta)
     * @param nhit  number of hits used for this track
     * */
    void saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize], double(&predmean)[Const::ChargedStable::c_SetSize],
                      double(&predres)[Const::ChargedStable::c_SetSize], double p, double dedx, double sin, int nhit) const;

    /** for all particles, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void saveLookupLogl(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx, TH2F* const* pdf) const;

    // parameters to determine the predicted means and resolutions
    double m_curvepars[15];  /**< dE/dx curve parameters */
    double m_sigmapars[12];  /**< dE/dx resolution parameters */

    int m_eventID; /**< counter for events */
    int m_trackID; /**< counter for tracks in this event */
    TH2F* m_pdfs[3][Const::ChargedStable::c_SetSize]; /**< dedx:momentum PDFs. */

    // parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    double m_removeLowest; /**< Portion of lowest dE/dx values that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of highest dE/dx values that should be discarded for truncated mean */

    bool m_enableDebugOutput; /**< Whether to save information on tracks and associated hits and dE/dx values in DedxTrack objects */

    //parameters: which particles and detectors to use
    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */

    //parameters: PDF configuration
    std::string m_pdfFile; /**< file containing the PDFs required for constructing a likelihood. */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

  };
} // Belle2 namespace
#endif
