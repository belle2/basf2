/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXPIDMODULE_H
#define DEDXPIDMODULE_H

#include <framework/core/Module.h>

#include <reconstruction/modules/dedxPID/DedxConstants.h>
#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

class TH2F;
class TH1F;

namespace Belle2 {
  class DedxTrack;
  class HelixHelper;

  /** \addtogroup modules
   * @{
   */

  /** Extract dE/dx (and some other things) from Tracks&GFTracks and PXDClusters, SVDTrueHits (not digitized) and CDCHits.
   *
   * If a PDF file is specified using the 'PDFFile' parameter, likelihood values
   * for all particle hypotheses are calculated and saved in a DedxLikelihood object.
   *
   * The 'EnableDebugOutput' option adds DedxTrack objects (one for each GFTrack),
   * which includes individual dE/dx data points and their corresponding layer,
   * and hit information like reconstructed position, charge, etc.
   *
   * The reconstruction of flight paths and the used likelihood ratio method are
   * described and evaluated in
   * <a href="http://www-ekp.physik.uni-karlsruhe.de/pub/web/thesis/iekp-ka2012-9.pdf">dE/dx Particle Identification and Pixel Detector Data Reduction for the Belle II Experiment</a> (Chapter 6)
   *
   */
  class DedxPIDModule : public Module {

  public:
    DedxPIDModule();
    ~DedxPIDModule();
    void initialize();
    void event();
    void terminate();

  private:
    /** save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param mean              calculated arithmetic mean
     * @param truncatedMean     calculated truncated mean
     * @param truncatedMeanErr  error for truncatedMean
     * @param dedx              input values
     */
    void calculateMeans(float* mean, float* truncatedMean, float* truncatedMeanErr, const std::vector<float>& dedx) const;

    /** returns length of path through a layer, given (full) layer id and angles */
    static float getFlownDistanceCDC(int layerid, float theta, float phi);

    /** save energy loss and hit information from SVD/PXDHits to track */
    template <class HitClass> void saveSiHits(DedxTrack* track, const HelixHelper& helix, const StoreArray<HitClass> &hits, const std::vector<int> &hit_indices) const;


    /** for all particles, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void saveLogLikelihood(float(&logl)[Const::ChargedStable::c_SetSize], float p, float dedx, TH2F* const* pdf) const;

    /** should info from this detector be included in likelihood? */
    bool detectorEnabled(Dedx::Detector d) const {
      return (d == Dedx::c_PXD and m_usePXD) or(d == Dedx::c_SVD and m_useSVD) or(d == Dedx::c_CDC and m_useCDC);
    }


    int m_trackID; /**< counter for tracks */
    int m_eventID; /**< counter for events */
    int m_numExtrapolations; /**< number of times GFTrackCand::extrapolate... was called after the origin */

    /** dedx:momentum PDFs. */
    TH2F* m_pdfs[Dedx::c_num_detectors][Const::ChargedStable::c_SetSize]; //m_pdfs[detector_type][particle_type]

    /** parameter for GFTrack array name. */
    std::string m_gftracks_name;

    //parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    double m_removeLowest; /**< Portion of events with low dE/dx that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of events with low dE/dx that should be discarded for truncated mean */

    //parameters: technical stuff
    double m_trackDistanceThreshhold; /**< Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off. */
    bool m_enableDebugOutput; /**< Wether to save information on tracks and associated hits and dE/dx values in DedxTrack objects */

    //parameters: which particles and detectors to use
    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */
    bool m_usePXD; /**< use PXD hits for likelihood */
    bool m_useSVD; /**< use SVD hits for likelihood */
    bool m_useCDC; /**< use CDC hits for likelihood */

    //parameters: PDF configuration
    std::string m_pdfFile; /**< file containing the PDFs required for constructing a likelihood. */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

  };
  /*! @} */
}
#endif
