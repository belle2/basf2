/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <reconstruction/dbobjects/CDCDedxADCNonLinearity.h>
#include <reconstruction/dbobjects/CDCDedxCosineEdge.h>
#include <reconstruction/dbobjects/CDCDedxMeanPars.h>
#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dbobjects/CDCDedxInjectionTime.h>
#include <reconstruction/dbobjects/DedxPDFs.h>

#include <vector>


namespace Belle2 {

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
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

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
    double meanCurve(double* x, double* par, int version) const;

    /** calculate the predicted mean using the parameterized resolution */
    double getMean(double bg) const;

    /** parameterized resolution for predictions */
    double sigmaCurve(double* x, const double* par, int version) const;

    /** calculate the predicted resolution using the parameterized resolution */
    double getSigma(double dedx, double nhit, double cos, double timereso) const;

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
     * @param chi       array of chi values to be modified
     * @param predmean  prediceted mean for each hypothesis
     * @param predres   predicted resolution for each hypothesis
     * @param p         track momentum valid in the cdc
     * @param dedx      dE/dx value
     * @param cos       track cos(theta)
     * @param nhit      number of hits used for this track
     * @param timereso  time resolution from database
     * */
    void saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize], double(&predmean)[Const::ChargedStable::c_SetSize],
                      double(&predres)[Const::ChargedStable::c_SetSize], double p, double dedx, double cos, int nhit, double timereso) const;


    /** for all particles, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * */
    void saveLookupLogl(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx);

    /** Check the pdfs for consistency everytime they change in the database */
    void checkPDFs();

    // parameters to determine the predicted means and resolutions
    std::vector<double> m_meanpars; /**< dE/dx mean parameters */
    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    // pdfs for PID
    DBObjPtr<DedxPDFs> m_DBDedxPDFs; /**< DB object for dedx:momentum PDFs */

    bool m_trackLevel; /**< Whether to use track-level or hit-level MC */
    bool m_usePrediction; /**< Whether to use parameterized means and resolutions or lookup tables */
    double m_removeLowest; /**< Portion of lowest dE/dx values that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of highest dE/dx values that should be discarded for truncated mean */
    bool m_backHalfCurlers; /**< Whether to use the back half of curlers */
    bool m_enableDebugOutput; /**< Whether to save information on tracks and associated hits and dE/dx values in DedxTrack objects */

    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */

    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxInjectionTime> m_DBInjectTime; /**< time gain/reso DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedxCosineEdge> m_DBCosEdgeCor; /**< non-lineary ACD correction DB object */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */
    DBObjPtr<CDCDedxADCNonLinearity> m_DBNonlADC; /**< non-lineary ACD correction DB object */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< hadron saturation parameters */

    std::vector<double> m_hadronpars; /**< hadron saturation parameters */

    int m_nLayerWires[9] = {}; /**< number of wires per layer: needed for wire gain calibration */

    // parameters to determine the predicted means and resolutions and hadron correction
    DBObjPtr<CDCDedxMeanPars> m_DBMeanPars; /**< dE/dx mean parameters */
    DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< dE/dx resolution parameters */

  };
} // Belle2 namespace
