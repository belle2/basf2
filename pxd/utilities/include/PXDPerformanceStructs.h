/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <TVector3.h>
//#include <limits>
#include <vector>
#include <math.h>

#include <pxd/utilities/PXDUtilities.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

namespace Belle2 {

  namespace PXD {

    /** Struct to hold variables for PXD clusters */
    struct Cluster_t {

      /** Default constructor */
      Cluster_t(): pxdID(0), charge(0), size(0),
        uSize(0), vSize(0), posU(0.0), posV(0.0) {}

      /** Update values from a PXDCluster
       * @param pxdCluster a PXDCluster object
       */
      void setValues(const PXDCluster& pxdCluster);

      /** Human readable id: layer * 1000 + ladder * 10 + sensor */
      unsigned short pxdID;     /**< PXD module id as the DHE id. */
      unsigned short charge; /**< Cluster charge in ADU. */
      unsigned short size;   /**< Cluster size. */
      unsigned short uSize;  /**< Cluster size in U. */
      unsigned short vSize;  /**< Cluster size in V. */
      float posU;            /**< Local position in r-phi. */
      float posV;            /**< Local position along z. */
    }; // end struct Cluster_t

    /** Struct to hold variables for intersection points */
    struct TrackPoint_t {

      /** Default constructor */
      TrackPoint_t(): x(0.0), y(0.0), z(0.0),
        tol(0.0), chargeMPV(0.0) {}

      /** Update values from a PXDCluster.
       * @param pxdIntercept a PXDIntercept object.
       * @param recoTracksName Name of RecoTrack collection
       * @return the pointer of the related RecoTrack object.
       */
      RecoTrack* setValues(const PXDIntercept& pxdIntercept, const std::string& recoTracksName = "");

      float x;         /**< Global position in x. */
      float y;         /**< Global position in y. */
      float z;         /**< Global position in z. */

      /** The variables below are included here as they can be
       * calculated w.o. having a track cluster.
       */
      float tol;       /**< cos(incident angle) = thickness/path length. */
      float chargeMPV; /**< Expected charge in ADU. */
    }; // end struct TrackPoint_t

    /** Struct to hold variables for track clusters. */
    struct TrackCluster_t {
      /** Default constructor */
      TrackCluster_t(): usedInTrack(false), dU(INFINITY), dV(INFINITY) {}

      /** Update values from a PXDIntercept.
       * @param pxdIntercept a PXDIntercept object.
       * @param recoTracksName Name of RecoTrack collection
       * @param pxdTrackClustersName Name of track matched PXDClusters
       * @return the pointer of the related RecoTrack object.
       */
      RecoTrack* setValues(const PXDIntercept& pxdIntercept,
                           const std::string& recoTracksName = "",
                           const std::string& pxdTrackClustersName = "PXDClustersFromTracks");

      bool usedInTrack;        /**< True if the cluster is used in tracking */
      float dU;                /**< Residual (meas - prediction) in U. */
      float dV;                /**< Residual (meas - prediciton) in V. */
      Cluster_t cluster;         /**< Cluster associated to the track. */
      TrackPoint_t intersection; /**< The track-module intersection. */
    }; // end struct TrackCluster_t

    /** Struct to hold variables from a track which
     * contains a vector of data type like TrackCluster.
     */
    template <typename TTrackCluster>
    struct TrackBase_t {
      /** Default constructor */
      TrackBase_t(): d0(0.0), z0(0.0), phi0(0.0), pt(0.0), tanLambda(0.0),
        d0p(0.0), z0p(0.0), nPXDHits(0), nSVDHits(0), nCDCHits(0) {}

      /** Update values from a RecoTrack.
       * @param recoTrack A RecoTrack object.
       * @param ip The interaction point for correcting d0 and z0
       * @param recoTracksName Name of RecoTrack collection
       * @param pxdInterceptsName Name of PXDIntercept collection
       * @param pxdTrackClustersName Name of track matched PXDClusters
       */
      void setValues(const RecoTrack& recoTrack, const TVector3& ip = TVector3(0, 0, 0),
                     const std::string& recoTracksName = "",
                     const std::string& pxdInterceptsName = "",
                     const std::string& pxdTrackClustersName = "PXDClustersFromTracks"
                    );

      float d0;        /**< Impact parameter in r-phi. */
      float z0;        /**< Impact parameter in z. */
      float phi0;      /**< Track direction in r-phi. */
      float pt;        /**< Transverse momentum. */
      float tanLambda; /**< Tangent of the dip angle. */
      float d0p;       /**< Corrected impact parameter in r-phi. */
      float z0p;       /**< Corrected impact parameter in z. */
      unsigned short nPXDHits; /**< Number of PXD Hits. */
      unsigned short nSVDHits; /**< Number of SVD Hits. */
      unsigned short nCDCHits; /**< Number of CDC Hits. */
      std::vector<TTrackCluster> trackClusters; /**< Vector of track cluster structs */
    };

    /** Typedef TrackBase_t<TrackCluster_t> Track_t */
    typedef TrackBase_t<TrackCluster_t> Track_t;


    /**
     * Template implementation
     */
    template <typename TTrackCluster>
    void TrackBase_t<TTrackCluster>::setValues(
      const RecoTrack& recoTrack, const TVector3& ip,
      const std::string& recoTracksName,
      const std::string& pxdInterceptsName,
      const std::string& pxdTrackClustersName
    )
    {
      // get Track pointer
      auto trackPtr = recoTrack.getRelated<Track>("Tracks");
      if (!trackPtr) {
        B2ERROR("Expect a track for fitted recotracks. Found nothing!");
      }

      // get trackFitResult pointer
      auto tfrPtr = trackPtr->getTrackFitResultWithClosestMass(Const::pion);
      if (!tfrPtr) {
        B2ERROR("expect a track fit result for pion. Found Nothing!");
      }
      nCDCHits = tfrPtr->getHitPatternCDC().getNHits();
      nSVDHits = tfrPtr->getHitPatternVXD().getNSVDHits();
      nPXDHits = tfrPtr->getHitPatternVXD().getNPXDHits();
      tanLambda = tfrPtr->getCotTheta();
      pt = tfrPtr->getMomentum().Perp();
      d0 = tfrPtr->getD0();
      z0 = tfrPtr->getZ0();
      phi0 = tfrPtr->getPhi0();
      d0p = d0;
      z0p = z0;
      if (ip != TVector3(0, 0, 0)) {
        // get a helix and change coordinate origin to ip
        auto uHelix = tfrPtr->getUncertainHelix();
        uHelix.passiveMoveBy(ip);
        d0p = uHelix.getD0();
        z0p = uHelix.getZ0();
      }

      //RelationVector<PXDIntercept> pxdIntercepts = recoTrack.getRelationsTo<PXDIntercept>();
      auto pxdIntercepts = recoTrack.getRelationsTo<PXDIntercept>(pxdInterceptsName);
      for (auto& pxdIntercept : pxdIntercepts) {
        TTrackCluster temp;
        // Only push a TrackCluster when setValues succeeds
        if (temp.setValues(pxdIntercept, recoTracksName, pxdTrackClustersName))
          trackClusters.push_back(temp);
      }
    }

    //} // end namespace Tuple
  } // end namespace PXD
} // end namespace Belle2
