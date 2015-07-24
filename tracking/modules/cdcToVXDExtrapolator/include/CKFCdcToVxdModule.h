/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ian J. Watson                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TTree.h"

namespace genfit { class Track; class TrackCand; class AbsMeasurement; }

namespace Belle2 {

  class TrackFitResult;
  class SVDCluster;
  class CDCHit;
  class PXDCluster;
  class PXDTrueHit;

  /**
   * Given a set of CDC-only tracks, uses the Combinatorial Kalman
   * Filter to find VXD hits compatible with the tracks.
   *
   * This module looks at CDC only tracks, i.e. where no compatible
   * VXD track could be combined with the track, coming from the
   * GenFitterModule step. It primes and sets hits for the
   * combinatorial kalman filter for each of these tracks, adding hits
   * from the VXD layers.
   *
   */
  class CKFCdcToVxdModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CKFCdcToVxdModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();

    /**  */
    virtual void terminate();


  private:

    /// extrapolate the track to a SVD Layer and try to find compatible hits. Returns false if the extrapolation failed
    bool extrapolateToSVDLayer(genfit::Track* track, unsigned searchLayer, StoreArray<SVDCluster>& clusters, bool isU,
                               std::vector<genfit::AbsMeasurement*>&);
    /// extrapolate the track to a PXD Layer and try to find compatible hits. Returns false if the extrapolation failed
    bool extrapolateToPXDLayer(genfit::Track* track, unsigned searchLayer, StoreArray<PXDCluster>& clusters,
                               std::vector<genfit::AbsMeasurement*>&);
    /// Adds found hits to the genfit track past in
    static bool findHits(genfit::Track* track, unsigned counter, std::vector<genfit::AbsMeasurement*>&, void* data);

    /// refit track, assumes a TrackRep for the track already exists
    bool refitTrack(genfit::Track* track);
    /// store the track in the output genfit array
    void storeTrack(genfit::Track& crnt, StoreArray<genfit::Track>& outGfTracks,
                    StoreArray<genfit::TrackCand>& outGfTrackCands,
                    RelationArray& gfTrackCandidatesTogfTracks,
                    RelationArray& gfTracksToMCPart);

    uint nOutTracks;

    bool m_saveAllTracks;

    // input
    std::string m_GFTrackColName; /**< genfit::Track input collection name */
    std::string m_mcParticlesColName; /**< MCParticle collection name */

    std::string m_OutGFTrackColName; /**< genfit::Track output collection name */
    std::string m_OutGFTrackCandColName; /**< genfit::TrackCand output collection name */

    std::string m_rootOutputFilename; /**< Filename for the ROOT file of module information. If "" then won't output ROOT file. */
    bool m_saveInfo;
    bool m_refitMcHits;
    bool m_useKalman; /**< true - refit with Kalman, false - refit with DAF */
    bool m_searchSensorDimensions; /**< true - look for hits within a sensor width/length, false - search within n sigma of the track extrap. */
    bool m_extrapolateToPxd; /**< Extrapolate to the PXD as well as the SVD */
    float m_hitNSigmaZ; /**< When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable */
    float m_hitNSigmaXY; /**< When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable */
    float m_hitNSigmaPix; /**< When searching within track extrap. this is the number of sigma a hit needs to be within to be acceptable for pxd*/
    bool m_extrapolateToDetector; /**< true: will try to extrapolate to the detector plane, false: extrapolates to the abstract cylinder or cone representing the detector */
    bool m_allLayers; /**< true: will try to extrapolate to all layers and find hits, false: stop extrapolation when a layer without compatible hits reached */
    bool m_stepwiseKalman; /**< "When true refits the track with a Kalman update after each hit is added, rather than simply doing the refit at the end. Shoudl improve the extrapolation quality, particularly to the PXD."*/

    // information gatherers
    int nTotalTracks;
    int nCdcRecoOnlyTracks;
    int nCdcRecoOnlyTracksWithVxdMcHits;
    int nTracksWithAddedHits;
    int nTracksWithAddedPxdHits;
    int nStoredTracks;

    TTree* HitInfo;
    TTree* TrackInfo;
    struct {
      Float_t z;
      Float_t rphi;
      Float_t pull_z;
      Float_t pull_rphi;

      Float_t du;
      Float_t dv;
      Float_t dw;

      Int_t layer;
      Int_t cone;

      Int_t truR;
      Int_t truZ;
      Bool_t isPxd;
    } stHitInfo;
    struct {
      UInt_t StartHitIdx;
      UInt_t EndHitIdx;

      Float_t pt;
      Float_t th;
      Float_t d0;

      Float_t pval;

      UInt_t truPxd;
      UInt_t truSvd;
      UInt_t nRec;

      Bool_t cdconly;
      Bool_t refit;
    } trkInfo;

    TFile* m_outputFile;

    RelationVector<CDCHit>* cdcHitsFromMC;
    RelationVector<SVDCluster>* svdClustersFromMC;
    RelationVector<PXDCluster>* pxdClustersFromMC;
    StoreArray<SVDCluster>* svdClusters;
    StoreArray<PXDCluster>* pxdClusters;
  };
}
