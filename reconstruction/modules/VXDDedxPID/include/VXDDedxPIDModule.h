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
#include <svd/dbobjects/SVDdEdxPDFs.h>
#include <pxd/dbobjects/PXDdEdxPDFs.h>

#include <vector>


namespace Belle2 {
  class HelixHelper;

  /** Extract dE/dx from fitted tracks.
   *
   * Likelihood values for all particle hypotheses are calculated and saved in a VXDDedxLikelihood object.
   *
   * VXDDedxTrack objects (one for each genfit::Track), include individual dE/dx data points and their
   * corresponding layer, and hit information like reconstructed position, charge, etc.
   *
   * The reconstruction of flight paths and the used likelihood ratio method are
   * described and evaluated in
   * <a href="https://publish.etp.kit.edu/record/20770">
   * dE/dx Particle Identification and Pixel Detector Data Reduction for the Belle II Experiment</a> (Chapter 6)
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

    /** This method is called for each event. All processing of the event takes place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

  private:

    /** Check the pdfs for consistency every time they change in the database */
    void checkPDFs();

    /**
     * Save arithmetic and truncated mean for the 'dedx' values.
     * @param mean              calculated arithmetic mean
     * @param truncatedMean     calculated truncated mean
     * @param truncatedMeanErr  error for truncatedMean
     * @param dedx              input values
     */
    void calculateMeans(double& mean, double& truncatedMean, double& truncatedMeanErr, const std::vector<double>& dedx) const;

    /** returns traversed length through active medium of given hit */
    template <class HitClass> static double getTraversedLength(const HitClass* hit, const RecoTrack* recoTrack, double& p);

    /** save energy loss and hit information from SVD/PXDHits to track */
    template <class HitClass> void saveSiHits(VXDDedxTrack* track, const std::vector<HitClass*>& hits,
                                              const RecoTrack* recoTrack) const;

    // module steering parameters
    bool m_useIndividualHits; /**< use individual hits (true) or truncated mean (false) to determine likelihoods */
    bool m_usePXD; /**< use PXD data for likelihood */
    bool m_useSVD; /**< use SVD data for likelihood */
    bool m_onlyPrimaryParticles; /**< For MC only: if true, only save data for primary particles (as determined by MC truth) */

    // required input
    StoreArray<Track> m_tracks; /**< Required array of Tracks */
    StoreArray<RecoTrack> m_recoTracks; /**< Required array of RecoTracks */

    // optional input
    StoreArray<MCParticle> m_mcparticles; /**< Optional array of MCParticles */
    StoreArray<SVDCluster> m_svdClusters; /**< Optional array of SVDClusters */
    StoreArray<PXDCluster> m_pxdClusters; /**< Optional array of PXDClusters */

    // output
    StoreArray<VXDDedxTrack> m_dedxTracks; /**< Output array of VXDDedxTracks */
    StoreArray<VXDDedxLikelihood> m_dedxLikelihoods; /**< Output array of VXDDedxLikelihoods */

    // PDF's for PID
    DBObjPtr<SVDdEdxPDFs> m_SVDDedxPDFs; /**< SVD DB object for dedx vs. momentum PDF's */
    DBObjPtr<PXDdEdxPDFs> m_PXDDedxPDFs; /**< PXD DB object for dedx vs. momentum PDF's */

    // other
    int m_eventID; /**< counter for events */
    int m_trackID; /**< counter for tracks in this event */

  };
} // Belle2 namespace
