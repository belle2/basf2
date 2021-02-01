/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/utilities/PXDTuple.h>
#include <framework/logging/Logger.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
//#include <mdst/dataobjects/Track.h>
//#include <mdst/dataobjects/HitPatternCDC.h>
//#include <mdst/dataobjects/HitPatternVXD.h>

namespace Belle2 {
  namespace PXD {
    namespace Tuple {

      std::shared_ptr<TrackState> getTrackStateOnModule(
        const VXD::SensorInfoBase& pxdSensorInfo,
        RecoTrack& recoTrack, double lambda)
      {
        // get sensor plane, always enable alignment.
        auto centerP = pxdSensorInfo.pointToGlobal(TVector3(0, 0, 0), true);
        auto normalV = pxdSensorInfo.vectorToGlobal(TVector3(0, 0, 1), true);
        genfit::SharedPlanePtr sensorPlaneSptr(new genfit::DetPlane(centerP, normalV));

        // genfit track and measured state on plane
        genfit::Track& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
        auto statePtr = std::make_shared<TrackState>();

        try {
          *statePtr = gfTrack.getFittedState();
          lambda = statePtr->extrapolateToPlane(sensorPlaneSptr);
        }  catch (...) {
          B2DEBUG(20, "extrapolation to plane failed! Lambda = " << lambda);
          return std::shared_ptr<TrackState>(nullptr);
        }
        auto intersec = pxdSensorInfo.pointToLocal(statePtr->getPos(), true);

        // check if the intersection is inside (no tolerance).
        double tolerance = 0.0;
        bool inside = pxdSensorInfo.inside(intersec.X(), intersec.Y(), tolerance, tolerance);
        if (!inside) return std::shared_ptr<TrackState>(nullptr);

        return statePtr;
      }

      void Cluster_t::setValues(const PXDCluster& pxdCluster)
      {
        pxdID = getPXDModuleID(pxdCluster.getSensorID());
        charge = pxdCluster.getCharge();
        size = pxdCluster.getSize();
        uSize = pxdCluster.getUSize();
        vSize = pxdCluster.getVSize();
        posU = pxdCluster.getU();
        posV = pxdCluster.getV();
      }

      RecoTrack* TrackPoint_t::setValues(const PXDIntercept& pxdIntercept, const std::string recoTracksName)
      {
        // Construct VxdID from its baseType (unsigned short)
        VxdID sensorID(pxdIntercept.getSensorID());

        //RelationVector<RecoTrack>
        // DataStore::getRelationsWithObj<RecoTrack>(pxdIntercept);
        auto recoTracks = pxdIntercept.getRelationsFrom<RecoTrack>(recoTracksName);

        // The recoTracks collection shouldn't be empty
        if (!recoTracks.size()) return nullptr;

        // Propogate a track state back to the module of interest
        // as PXDIntercept doesn't contain global track point,
        // cos(incident angle), and the expected charge from the hit.
        const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().get(sensorID);
        auto statePtr = getTrackStateOnModule(sensorInfo, *recoTracks[0]);
        if (statePtr == nullptr) return nullptr; // shouldn't happen.
        auto intersec = statePtr -> getPos();
        auto intersec_p = statePtr -> getMom();
        auto local_p = sensorInfo.vectorToLocal(intersec_p, true);

        x = intersec.X();
        y = intersec.Y();
        z = intersec.Z();
        tol = local_p.CosTheta();

        // Estimate the charge MPV (in ADU) of the hit.
        double length = sensorInfo.getThickness() / tol; // track path length
        auto ADUToEnergy = PXD::PXDGainCalibrator::getInstance().getADUToEnergy(sensorID,
                           sensorInfo.getUCellID(pxdIntercept.getCoorU()),
                           sensorInfo.getVCellID(pxdIntercept.getCoorV()));
        chargeMPV = getDeltaP(intersec_p.Mag(), length) / ADUToEnergy;

        // Return pointer of the relatd RecoTrack for accessing additional info.
        return recoTracks[0];
      }

      RecoTrack* TrackCluster_t::setValues(const PXDIntercept& pxdIntercept,
                                           const std::string recoTracksName,
                                           const std::string pxdTrackClustersName)
      {
        auto recoTrackPtr = intersection.setValues(pxdIntercept, recoTracksName);
        // sensor ID from intersectioon
        //VxdID sensorID(pxdIntercept.getSensorID());
        if (!recoTrackPtr) return nullptr; // return nullptr
        // Always set cluster pxdID for saving module id of the intersection.
        cluster.pxdID = getPXDModuleID(VxdID(pxdIntercept.getSensorID()));
        RelationVector<PXDCluster> pxdClusters = DataStore::getRelationsWithObj<PXDCluster>(recoTrackPtr, pxdTrackClustersName);
        // Return nullptr (false) as no clusters associated;
        if (!pxdClusters.size()) return nullptr;
        for (auto& aCluster : pxdClusters) {
          if (aCluster.getSensorID().getID() == pxdIntercept.getSensorID()) {
            cluster.setValues(aCluster);
            dU = aCluster.getU() - pxdIntercept.getCoorU();
            dV = aCluster.getV() - pxdIntercept.getCoorV();
            usedInTrack = true;
            break;
          }
        }
        // If usedInTrack is still false, we can loop PXDClusters datastore to find one closest to the track point. (To be implemented)
        return recoTrackPtr;
      }

      // Explicit instantiation of templates
      //template struct TrackBase_t<TrackCluster_t>;

    } // end namespace Tuple
  } // end namespace PXD
} // end namespace Belle2
