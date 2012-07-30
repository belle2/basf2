#ifndef DEDXPIDMODULE_H
#define DEDXPIDMODULE_H

#include <framework/core/Module.h>

#include <analysis/modules/dedxPID/DedxConstants.h>
#include <analysis/modules/dedxPID/HelixHelper.h>

#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

class TH2F;
class TH1F;

namespace Belle2 {
  class TrackDedx;

  /** \addtogroup modules
   * @{
   */

  /** Extract dE/dx (and some other things) from Tracks&GFTracks and PXDClusters, SVDTrueHits (not digitized) and CDCHits.
   *
   *
   * If a PDF file is specified using the 'PDFFile' parameter, likelihood values
   * for all particle hypotheses are calculated and saved in a DedxLikelihood object.
   *
   * The 'EnableDebugOutput' option adds 'TrackDedx' objects
   * The calculated values are stored in a TrackDedx object (one for each Track),
   * which includes individual dE/dx data points and their corresponding layer,
   * and (optionally) hit information like reconstructed position, charge, etc.
   *
   *
    \correlationdiagram
    Track = graph.external_data('Track')
    GFTrack = graph.external_data('GFTrack')
    MCParticle = graph.external_data('MCParticle')
    CDCHit = graph.external_data('CDCHit')
    SVDTrueHit = graph.external_data('SVDTrueHit')
    PXDTrueHit = graph.external_data('PXDTrueHit')
    PXDCluster = graph.external_data('PXDCluster')

    DedxLikelihood = graph.data('DedxLikelihood')
    TrackDedx = graph.data('TrackDedx')

    graph.module('DedxPID', [Track, GFTrack, MCParticle, CDCHit, SVDTrueHit, PXDTrueHit, PXDCluster], [DedxLikelihood, TrackDedx])

    graph.relation(Track, DedxLikelihood)
    \endcorrelationdiagram
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
    template <class HitClass> void saveSiHits(TrackDedx* track, const HelixHelper& helix, const StoreArray<HitClass> &hits, const std::vector<unsigned int> &hit_indices) const;


    /** for all particles, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void saveLogLikelihood(float(&logl)[c_Dedx_num_particles], float p, float dedx, TH2F* const* pdf) const;

    /** should info from this detector be included in likelihood? */
    bool detectorEnabled(DedxDetector d) const {
      return (d == c_PXD and m_usePXD) or(d == c_SVD and m_useSVD) or(d == c_CDC and m_useCDC);
    }


    int m_trackID; /**< counter for tracks */
    int m_eventID; /**< counter for events */
    int m_numExtrapolations; /**< number of times GFTrackCand::extrapolate... was called after the origin */

    /** dedx:momentum PDFs. */
    TH2F* m_pdfs[c_Dedx_num_detectors][c_Dedx_num_particles]; //m_pdfs[detector_type][particle_type]

    /** parameter for GFTrack array name. */
    std::string m_gftracks_name;

    //parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    double m_removeLowest; /**< Portion of events with low dE/dx that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of events with low dE/dx that should be discarded for truncated mean */

    //parameters: technical stuff
    double m_trackDistanceThreshhold; /**< Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off. */
    bool m_enableDebugOutput; /**< Wether to save information on tracks and associated hits and dE/dx values in TrackDedx objects */

    //parameters: which particles and detectors to use
    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */
    bool m_usePXD; /**< use PXD hits for likelihood */
    bool m_useSVD; /**< use SVD hits for likelihood */
    bool m_useCDC; /**< use CDC hits for likelihood */

    //parameters: PDF configuration
    std::string m_pdfFilename; /**< file containing the PDFs required for constructing a likelihood. */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

  };
  /*! @} */
}
#endif
