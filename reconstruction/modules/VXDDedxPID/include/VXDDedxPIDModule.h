/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDDEDXPIDMODULE_H
#define VXDDEDXPIDMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <reconstruction/dbobjects/DedxPDFs.h>

#include <string>
#include <vector>
#include <TVector3.h>
#include <TH2F.h>

class TH2F;

namespace Belle2 {
  class PXDCluster;
  class SVDCluster;
  class VXDDedxTrack;
  class HelixHelper;

  /** Extract dE/dx from fitted tracks.
   *
   * Likelihood values for all particle hypotheses are calculated and saved in a VXDDedxLikelihood object.
   *
   * The 'EnableDebugOutput' option adds VXDDedxTrack objects (one for each genfit::Track),
   * which includes individual dE/dx data points and their corresponding layer,
   * and hit information like reconstructed position, charge, etc.
   *
   * The reconstruction of flight paths and the used likelihood ratio method are
   * described and evaluated in
   * <a href="http://www-ekp.physik.uni-karlsruhe.de/pub/web/thesis/iekp-ka2012-9.pdf">dE/dx Particle Identification and Pixel Detector Data Reduction for the Belle II Experiment</a> (Chapter 6)
   *
   */
  class VXDDedxPIDModule : public Module {

  public:

    /** Default constructor */
    VXDDedxPIDModule();

    /** Destructor */
    virtual ~VXDDedxPIDModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

  private:

    // required input
    StoreArray<Track> m_tracks; /**< Required array of Tracks */
    StoreArray<RecoTrack> m_recoTracks; /**< Required array of input RecoTracks */

    // optional input
    StoreArray<MCParticle> m_mcparticles; /**< Optional array of MCParticles */
    StoreArray<SVDCluster> m_svdClusters; /**< Optional array of SVDClusters */
    StoreArray<PXDCluster> m_pxdClusters; /**< Optional array of PXDClusters */

    // output
    StoreArray<VXDDedxTrack> m_dedxTracks; /**< Output array of VXDDedxTracks */
    StoreArray<VXDDedxLikelihood> m_dedxLikelihoods; /**< Output array of VXDDedxLikelihoods */


    /** counter for events */
    int m_eventID;
    /** counter for tracks in this event */
    int m_trackID;

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param mean              calculated arithmetic mean
     * @param truncatedMean     calculated truncated mean
     * @param truncatedMeanErr  error for truncatedMean
     * @param dedx              input values
     */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** returns traversed length through active medium of given PXDCluster. */
    static double getTraversedLength(const PXDCluster* hit, const HelixHelper* helix);

    /** returns traversed length through active medium of given SVDCluster. */
    static double getTraversedLength(const SVDCluster* hit, const HelixHelper* helix);

    /** save energy loss and hit information from SVD/PXDHits to track */
    template <class HitClass> void saveSiHits(VXDDedxTrack* track, const HelixHelper& helix, const std::vector<HitClass*>& hits) const;

    /** for all particles in the PXD, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void savePXDLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, float dedx) const;

    /** for all particles in the SVD, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void saveSVDLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, float dedx) const;

    /** should info from this detector be included in likelihood? */
    bool detectorEnabled(Dedx::Detector d) const
    {
      return (d == Dedx::c_PXD and m_usePXD) or (d == Dedx::c_SVD and m_useSVD);
    }

    /** dedx:momentum PDFs. */
    DBObjPtr<DedxPDFs> m_DBDedxPDFs; /**< DB object for dedx:momentum PDFs */
    TH2F m_pdfs[2][6]; //m_pdfs[detector_type][particle_type]

    // parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    double m_removeLowest; /**< Portion of lowest dE/dx values that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of highest dE/dx values that should be discarded for truncated mean */

    //parameters: technical stuff
    double m_trackDistanceThreshhold; /**< Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off. */
    bool m_enableDebugOutput; /**< Whether to save information on tracks and associated hits and dE/dx values in VXDDedxTrack objects */

    //parameters: which particles and detectors to use
    bool m_onlyPrimaryParticles; /**< Only save data for primary particles (as determined by MC truth) */
    bool m_usePXD; /**< use PXD hits for likelihood */
    bool m_useSVD; /**< use SVD hits for likelihood */

    //parameters: PDF configuration
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

  };
} // Belle2 namespace
#endif
