/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>

#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedx2DCor.h>
#include <reconstruction/dbobjects/CDCDedx1DCleanup.h>
#include <reconstruction/dbobjects/CDCDedxCurvePars.h>
#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dbobjects/DedxPDFs.h>

#include <string>
#include <vector>
#include <map>
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

    // outputs
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Output array of CDCDedxTracks */
    StoreArray<CDCDedxLikelihood> m_dedxLikelihoods; /**< Output array of CDCDedxLikelihoods */

    // required inputs
    StoreArray<Track> m_tracks; /**< Required array of input Tracks */
    StoreArray<RecoTrack> m_recoTracks; /**< Required array of input RecoTracks */

    // optional inputs
    StoreArray<MCParticle> m_mcparticles; /**< Optional array of input MCParticles */

    /** parameterized beta-gamma curve for predicted means */
    double bgCurve(double* x, double* par, int version) const;

    /** calculate the predicted mean using the parameterized resolution */
    double getMean(double bg) const;

    /** parameterized resolution for predictions */
    double sigmaCurve(double* x, double* par, int version) const;

    /** calculate the predicted resolution using the parameterized resolution */
    double getSigma(double dedx, double nhit, double sin) const;

    /** hadron saturation parameterization part 1 */
    double I2D(double cosTheta, double I) const;

    /** hadron saturation parameterization part 2 */
    double D2I(double cosTheta, double D) const;

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
    void saveLookupLogl(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx) const;

    // parameters to determine the predicted means and resolutions
    std::vector<double> m_curvepars; /**< dE/dx curve parameters */
    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    // pdfs for PID
    DBObjPtr<DedxPDFs> m_DBDedxPDFs; /**< DB object for dedx:momentum PDFs */
    TH2F m_pdfs[6]; /**< dedx:momentum PDFs. */

    bool m_trackLevel; /**< Whether to use track-level or hit-level MC */
    bool m_usePrediction; /**< Whether to use parameterized means and resolutions or lookup tables */
    double m_removeLowest; /**< Portion of lowest dE/dx values that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of highest dE/dx values that should be discarded for truncated mean */
    bool m_enableDebugOutput; /**< Whether to save information on tracks and associated hits and dE/dx values in DedxTrack objects */

    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */

    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCor> m_DB2DCor; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCleanup> m_DB1DCleanup; /**< 1D correction DB object */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< hadron saturation parameters */

    std::vector<double> m_hadronpars; /**< hadron saturation parameters */

    int m_nLayerWires[9]; /**< number of wires per layer: needed for wire gain calibration */

    // parameters to determine the predicted means and resolutions and hadron correction
    DBObjPtr<CDCDedxCurvePars> m_DBCurvePars; /**< dE/dx curve parameters */
    DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< dE/dx resolution parameters */

  };
} // Belle2 namespace
