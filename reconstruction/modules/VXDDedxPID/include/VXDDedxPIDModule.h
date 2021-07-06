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
#include <framework/datastore/StoreArray.h>

#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <framework/database/DBObjPtr.h>
#include <reconstruction/dbobjects/DedxPDFs.h>

#include <vector>


namespace Belle2 {
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
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

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

    /** Check the pdfs for consistency everytime they change in the database */
    void checkPDFs();

    // pdfs for PID
    DBObjPtr<DedxPDFs> m_DBDedxPDFs; /**< DB object for dedx:momentum PDFs */

    // parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    //no need to define highest and lowest truncated value as we laways remove highest 2 dE/dx values from 8 dE/dx value.

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
