/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <geometry/GeometryManager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>
#include <time.h>

#include <TH1F.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


/*
 * Convert given integer into Verilog 32 bit value
 */
char*
SVDHoughTrackingModule::convertInt(int32_t n)
{
  size_t size = 256;
  static char* buf = NULL;
  if (buf == NULL) {
    if ((buf = (char*) malloc(size * sizeof(char))) == NULL) {
      perror("Error: malloc() ");
      return NULL;
    }
  }
  memset(buf, 0, size);

  if (n < 0) {
    snprintf(buf, size, "-32'sd%i", -1 * n);
  } else {
    snprintf(buf, size, "32'sd%i", n);
  }

  return (buf);
}



/*
 * To bypass the FPGA cluster algorithm, we can directly convert
 * SimHits into the cluster format.
 */
void
SVDHoughTrackingModule::convertSimHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int i, cluster_cnt;
  int nSimHit;
  double dist;
  float time;
  VxdID sensorID, last_sensorID;
  TVector3 pos, last_pos, local_pos;
  TVector3 diff, vec, abs_pos;

  double u, v, last_u, last_v;
  int uCellID, vCellID;
  //int last_uCellID, last_vCellID;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  nSimHit = storeSVDSimHit.getEntries();
  if (nSimHit == 0) {
    return;
  }

  B2DEBUG(250, "Convert SimHits to Clusters");

  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  dist = 1E+50;
  cluster_cnt = 0;
  for (i = 0; i < nSimHit; ++i) {
    vec = storeSVDSimHit[i]->getPosIn();
    time = storeSVDSimHit[i]->getGlobalTime();
    VxdID sensorID = storeSVDSimHit[i]->getSensorID();
    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

    /* Don't want off trigger hits */
    if (time < 0.0f || time > 6.0f) {
      continue;
    }

    /* Convert local to global position */
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
    pos = abs_pos;

    u = storeSVDSimHit[i]->getPosIn().X();
    v = storeSVDSimHit[i]->getPosIn().Y();
    const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
    uCellID = currentSensor->getUCellID(u, v, false);
    vCellID = currentSensor->getVCellID(v, false);

    B2DEBUG(250, " Local SimHit Pos: " << vec.X() << " " << vec.Y() << " " << vec.Z()
            << " Abs pos: " << abs_pos.X() << " " << abs_pos.Y() << " " << abs_pos.Z()
            << " Time: " << time << " " << " Sensor ID: " << sensorID);

    if (i == 0) {
      if (i == (nSimHit - 1)) {
        if (m_countStrips) {
          n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
          ++n_idx;
          p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
          ++p_idx;
        } else {
          uCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
          vCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
          //uCellID += 10000000 * p_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
          //vCellID += 10000000 * n_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
          uCellID = uCellID * 10 + p_idx;
          vCellID = vCellID * 10 + n_idx;
          //B2INFO("uCellID: " << uCellID << " vCellID: " << vCellID << " VXDID: " << sensorID);
          n_clusters.insert(std::make_pair(vCellID, std::make_pair(sensorID, pos)));
          p_clusters.insert(std::make_pair(uCellID, std::make_pair(sensorID, pos)));
          ++p_idx;
          ++n_idx;
        }
        storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
        ++cluster_cnt;
      }
    } else {
      //diff = pos - last_pos;
      diff = abs_pos - last_pos;
      dist = diff.Mag();
      B2DEBUG(250, "   Distance: " << dist);

      if (fabs(dist) < 0.08 && i != (nSimHit - 1)) {
      } else {
        if (m_countStrips) {
          n_clusters.insert(std::make_pair(n_idx, std::make_pair(last_sensorID, last_pos)));
          ++n_idx;
          p_clusters.insert(std::make_pair(p_idx, std::make_pair(last_sensorID, last_pos)));
          ++p_idx;
        } else {
          uCellID += 1000000 * last_sensorID.getLayerNumber() + 10000 * last_sensorID.getLadderNumber() + 1000 *
                     last_sensorID.getSensorNumber();
          vCellID += 1000000 * last_sensorID.getLayerNumber() + 10000 * last_sensorID.getLadderNumber() + 1000 *
                     last_sensorID.getSensorNumber();
          //uCellID += 10000000 * p_idx + 1000000 * last_sensorID.getLayerNumber() + 10000 * last_sensorID.getLadderNumber() + 1000 * last_sensorID.getSensorNumber();
          //vCellID += 10000000 * n_idx + 1000000 * last_sensorID.getLayerNumber() + 10000 * last_sensorID.getLadderNumber() + 1000 * last_sensorID.getSensorNumber();
          uCellID = uCellID * 10 + p_idx;
          vCellID = vCellID * 10 + n_idx;
          //B2INFO("uCellID: " << uCellID << " vCellID: " << vCellID << " VXDID: " << sensorID);
          n_clusters.insert(std::make_pair(vCellID, std::make_pair(last_sensorID, last_pos)));
          p_clusters.insert(std::make_pair(uCellID, std::make_pair(last_sensorID, last_pos)));
          ++p_idx;
          ++n_idx;
        }
        storeHoughCluster.appendNew(SVDHoughCluster(last_sensorID, pos));
        ++cluster_cnt;
      }
    }

    //last_pos = pos;
    last_pos = abs_pos;
    last_sensorID = sensorID;

    last_u = u;
    last_v = v;
    //last_uCellID = uCellID;
    //last_vCellID = vCellID;

  }

  if (m_compareMCParticleVerbose) {
    B2INFO("  Total Clusters: " << cluster_cnt << " Number of Sim Hits: " << nSimHit);
  }
}

/*
 * With background simulation we need to mix sim and true hits, in case we
 * also want the true hits inside.
 */
void
SVDHoughTrackingModule::mixTrueSimHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int nTrueHit;
  VxdID sensorID;
  SVDTrueHit* svdTrueHit;
  TVector3 pos, local_pos;
  TVector3 diff, abs_pos, sim_pos;
  double dist;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  //convertSimHits(); /* Convert Simhits first */
  convertTrueHits(); /* Convert Simhits first */

  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  B2DEBUG(250, "mixTrueSimHits: Include True hits if necessary");

  dist = 1e+99;
  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  for (int i = 0; i < nTrueHit; ++i) {
    svdTrueHit = storeSVDTrueHits[i];
    sensorID = svdTrueHit->getSensorID();
    local_pos.SetX(svdTrueHit->getU());
    local_pos.SetY(svdTrueHit->getV());
    local_pos.SetZ(svdTrueHit->getW());

    /* Convert local to global position */
    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    pos = sensorInfo->pointToGlobal(local_pos);


    for (int j = 0; j < storeHoughCluster.getEntries(); ++j) {
      sim_pos = storeHoughCluster[j]->getHitPos();
      diff = sim_pos - pos;

      if (diff.Mag() < dist) {
        dist = diff.Mag();
      }
    }

    if (dist > 0.060) {
      B2DEBUG(250, "  Include trueHit Local Pos: " << local_pos.X() << " " << local_pos.Y() << " " << local_pos.Z()
              << " Pos: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor ID: " << sensorID);

      if (m_countStrips) {
        n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
        ++n_idx;
        p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
        ++p_idx;
      } else {
        double u = local_pos.X();
        double v = local_pos.Y();
        const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
        int uCellID = currentSensor->getUCellID(u, v, false);
        int vCellID = currentSensor->getVCellID(v, false);
        uCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
        vCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
        //uCellID += 10000000 * p_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
        //vCellID += 10000000 * n_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
        uCellID = uCellID * 10 + p_idx;
        vCellID = vCellID * 10 + n_idx;
        //B2INFO("uCellID: " << uCellID << " vCellID: " << vCellID << " VXDID: " << sensorID);
        n_clusters.insert(std::make_pair(vCellID, std::make_pair(sensorID, pos)));
        p_clusters.insert(std::make_pair(uCellID, std::make_pair(sensorID, pos)));
        ++p_idx;
        ++n_idx;
      }

      storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
    }
  }
}

/*
 * To bypass the FPGA cluster algorithm, we can directly convert
 * TrueHits into the cluster format.
 */
void
SVDHoughTrackingModule::convertTrueHits()
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  int nTrueHit;
  VxdID sensorID;
  SVDTrueHit* svdTrueHit;
  TVector3 pos, local_pos;
  TVector3 vec, abs_pos;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  if (!storeHoughCluster.isValid()) {
    storeHoughCluster.create();
  } else {
    storeHoughCluster.getPtr()->Clear();
  }

  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  B2DEBUG(250, "Convert TrueHits to Clusters");

  /* First convert to absolute hits and save into a map */
  /* Clear global cluser maps */
  svd_sensor_cluster_map.clear();
  for (int i = 0; i < nTrueHit; ++i) {
    svdTrueHit = storeSVDTrueHits[i];
    sensorID = svdTrueHit->getSensorID();
    local_pos.SetX(svdTrueHit->getU());
    local_pos.SetY(svdTrueHit->getV());
    local_pos.SetZ(svdTrueHit->getW());

    /* Convert local to global position */
    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    pos = sensorInfo->pointToGlobal(local_pos);

    B2DEBUG(250, "  Local Pos: " << local_pos.X() << " " << local_pos.Y() << " " << local_pos.Z()
            << " Pos: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor ID: " << sensorID);

    if (m_countStrips) {
      n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
      ++n_idx;
      p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
      ++p_idx;
    } else {
      double u = local_pos.X();
      double v = local_pos.Y();
      const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
      int uCellID = currentSensor->getUCellID(u, v, false);
      int vCellID = currentSensor->getVCellID(v, false);
      uCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      vCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      //uCellID += 10000000 * p_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      //vCellID += 10000000 * n_idx + 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      uCellID = uCellID * 10 + p_idx;
      vCellID = vCellID * 10 + n_idx;
      //B2INFO("uCellID: " << uCellID << " vCellID: " << vCellID << " VXDID: " << sensorID);
      n_clusters.insert(std::make_pair(vCellID, std::make_pair(sensorID, pos)));
      p_clusters.insert(std::make_pair(uCellID, std::make_pair(sensorID, pos)));
      ++p_idx;
      ++n_idx;
    }

    storeHoughCluster.appendNew(SVDHoughCluster(sensorID, pos));
  }
}

