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

namespace Belle2 {

  namespace PXD {

    namespace Tuple {
      /** Const and Const expressions
       * Only valid when g_mol is the default unit.
       */
      const int Z_Si = 14; /**< Atomic number of silicon */
      const double A_Si = 28.085; /**< Atomic mass of silicon in g mol^-1 */
      const double rho_Si = 2.3290 * Unit::g_cm3; /**< Silicon density in g cm^-3 */

      /** xi = (K/2)*(Z/A)*z*z*(rho*L)/beta2 in MeV
       * @param Z Atomic number of absorber
       * @param A Atomic mass of absorber in g*mol^{-1}
       * @param rho Density of the absorber in g*cm^{-3}
       * @param z Charge number of incident particle
       * @return xi*beta^2/L in MeV/cm where L is track length
       */
      inline double xiBeta2_L(const int Z = Z_Si,
                              const double A = A_Si,
                              const double rho = rho_Si,
                              const int z = 1)
      {
        const double K = 0.307075 * Unit::MeV * pow(Unit::cm, 2);
        return K / 2 * Z / A * z * z * rho;
      }

      /** hbarWp = sqrt(rho*Z/A)*28.816 in eV
       * @param Z Atomic number of absorber
       * @param A Atomic mass of absorber in g*mol^{-1}
       * @param rho Density of the absorber in g*cm^{-3}
       * @return plasma energy
       */
      inline double hbarWp(const int Z = Z_Si,
                           const double A = A_Si,
                           const double rho = rho_Si)
      {
        return std::sqrt(rho * Z / A) * 28.816 * Unit::eV;
      }

      /** helper function to estimate the most probable energy loss for a given track length.
       * @param mom Magnitude of the momentum
       * @param length Track path length
       * @param mass Mass of the incident particle, using e- as default
       * @return the most probable energy
       */
      inline double getDeltaP(const double mom, const double length, const double mass = Const::electronMass)
      {
        double betaGamma = mom / mass;
        if (betaGamma <= 100) return 0.0; // requirement of the formula.
        double beta2 = 1. / (1. + 1. / betaGamma / betaGamma);
        double xi = xiBeta2_L() * length / beta2;
        return xi * log(2 * mass * xi / pow(hbarWp(), 2) + 0.2);
      }

      /** Helper function to get DHE id like module id from VxdID */
      inline unsigned short getPXDModuleID(const VxdID& sensorID)
      {
        return sensorID.getLayerNumber() * 1000 +
               sensorID.getLadderNumber() * 10 +
               sensorID.getSensorNumber();
      }

      /** Helper function to get a track state on a module
       * @param pxdSensorInfo of the PXD module intersecting with the track.
       * @param recoTrack the recoTrack to be extrapolated.
       * @param lambda the extrapolation length from track POCA.
       * @return the shared pointer of the intersection track state on the module.
       */
      typedef genfit::MeasuredStateOnPlane TrackState;
      std::shared_ptr<TrackState> getTrackStateOnModule(const VXD::SensorInfoBase& pxdSensorInfo,
                                                        RecoTrack& recoTrack, double lambda = 0.0);

      /** Struct to hold variables for PXD clusters */
      struct Cluster_t {

        /** Default constructor */
        Cluster_t(): pxdID(0), charge(0), size(0),
          uSize(0), vSize(0), posU(0.0), posV(0.0) {}

        /** Constructor
         * @param pxdCluster a PXDCluster object
         */
        Cluster_t(const PXDCluster& pxdCluster) { setValues(pxdCluster);}

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

        /** Constructor
         * @param pxdIntercept a PXDIntercept object
         */
        TrackPoint_t(const PXDIntercept& pxdIntercept, const std::string recoTracksName) { setValues(pxdIntercept, recoTracksName);}

        /** Update values from a PXDCluster.
         * @param pxdIntercept a PXDIntercept object.
         * @param recoTracksName Name of RecoTrack collection
         * @return the pointer of the related RecoTrack object.
         */
        RecoTrack* setValues(const PXDIntercept& pxdIntercept, const std::string recoTracksName = "");

        float x;         /**< Global position in x. */
        float y;         /**< Global position in y. */
        float z;         /**< Global position in z. */

        /** The variables below are included here as they can be
         * calculated w.o. having a track cluster.
         */
        float tol;       /**< cos(incident angle) = thickness/path length. */
        float chargeMPV; /**< Expected charge in ADU. */
      }; // end struct TrackPoint_t

      struct TrackCluster_t {
        /** Default constructor */
        TrackCluster_t(): dU(INFINITY), dV(INFINITY) {}
        //dV(std::numeric_limits<float>::infinity()) {}

        /** Constructor
         * @param pxdIntercept a PXDIntercept object
         */
        TrackCluster_t(const PXDIntercept& pxdIntercept,
                       const std::string recoTracksName = "",
                       const std::string pxdTrackClustersName = "PXDClustersFromTracks")
        {
          setValues(pxdIntercept, recoTracksName, pxdTrackClustersName);
        }

        /** Update values from a PXDIntercept.
         * @param pxdIntercept a PXDIntercept object.
         * @param recoTracksName Name of RecoTrack collection
         * @param pxdTrackClustersName Name of track matched PXDClusters
         * @return the pointer of the related RecoTrack object.
         */
        RecoTrack* setValues(const PXDIntercept& pxdIntercept,
                             const std::string recoTracksName = "",
                             const std::string pxdTrackClustersName = "PXDClustersFromTracks");

        float dU;                /**< Residual (meas - prediction) in U. */
        float dV;                /**< Residual (meas - prediciton) in V. */
        Cluster_t cluster;         /**< Cluster associated to the track. */
        TrackPoint_t intersection; /**< The track-module intersection. */
      }; // end struct TrackCluster_t

      /** Struct to hold variables from a track which
       * contains a vector of data type like TrackCluster.
       */
      template <typename TTrackCluster>
      struct Track_t {
        /** Default constructor */
        Track_t(): d0(0.0), z0(0.0), phi0(0.0), pt(0.0), tanLambda(0.0),
          nPXDHits(0), nSVDHits(0), nCDCHits(0) {}

        /** Update values from a RecoTrack.
         * @param recoTrack A RecoTrack object.
         * @param ip The interaction point for correcting d0 and z0
         */
        void setValues(const RecoTrack& recoTrack, const TVector3& ip = TVector3(0, 0, 0));

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
        std::vector<TTrackCluster> trackClusters;
      };

    } // end namespace Tuple
  } // end namespace PXD
} // end namespace Belle2
