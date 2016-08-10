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
 * Custom clustering.
 */
void
SVDHoughTrackingModule::clustering(bool pxd)
{
  StoreArray<SVDSimHit> storeSVDSimHit(m_storeSVDSimHitsName);
  StoreArray<PXDSimHit> storePXDSimHits(m_storePXDSimHitsName);
  TVector3 vec, abs_pos;
  int num_simhit = storeSVDSimHit.getEntries();
  int num_pxdsimhit = storePXDSimHits.getEntries();
  int cnt, i;

  clusterMap pos_map;
  sensorMap sensor_cluster;
  std::map<int, TVector3> cpy_map = pos_map;
  std::map<int, TVector3>::iterator iter, inner_iter;
  sensorMap map, real_map;
  TVector3 clusterDelta(0.15, 0.15, 0.15);

  /* First convert to absolute hits and save into a map */
  if (pxd) {
    /* Clear global cluser maps */
    pxd_cluster_map.clear();
    for (i = 0; i < num_pxdsimhit; ++i) {
      vec = storePXDSimHits[i]->getPosIn();
      VxdID sensorID = storePXDSimHits[i]->getSensorID();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      abs_pos = info.pointToGlobal(storePXDSimHits[i]->getPosIn());
      pxd_cluster_map.insert(std::make_pair(i, abs_pos));
    }
  } else {
    /* Clear global cluser maps */
    svd_sensor_cluster_map.clear();
    for (i = 0; i < num_simhit; ++i) {
      vec = storeSVDSimHit[i]->getPosIn();
      VxdID sensorID = storeSVDSimHit[i]->getSensorID();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      abs_pos = info.pointToGlobal(storeSVDSimHit[i]->getPosIn());
      pos_map.insert(std::make_pair(i, abs_pos));
      sensor_cluster = (std::make_pair(sensorID, abs_pos));
      svd_sensor_cluster_map.insert(std::make_pair(i, sensor_cluster));
    }
  }

  int cluster_id;
  i = 0;
  if (!pxd) {
    svd_real_cluster_map.clear();
  }
  for (iter = pos_map.begin(); iter != pos_map.end(); ++iter) {
    TVector3 clus_pos;
    cnt = 0;
    for (inner_iter = cpy_map.begin(); inner_iter != cpy_map.end();) {
      TVector3 diff = iter->second - inner_iter->second;
      if (fabs(diff.Px()) < clusterDelta.Px() && fabs(diff.Py()) < clusterDelta.Py()
          && fabs(diff.Pz()) < clusterDelta.Pz()) {
        ++cnt;
        clus_pos += inner_iter->second;
        cluster_id = inner_iter->first;
        cpy_map.erase(inner_iter++);
      } else {
        ++inner_iter;
      }
    }

    if (cnt >= 1) {
      TVector3 div_fac((double) cnt, (double) cnt, (double) cnt);
      clus_pos.SetXYZ(clus_pos.X() / (double) cnt, clus_pos.Y() / (double) cnt,
                      clus_pos.Z() / (double) cnt);
      if (pxd) {
        pxd_cluster_map.insert(std::make_pair(i, clus_pos));
      } else {
        cluster_map.insert(std::make_pair(i, clus_pos));
        map = svd_sensor_cluster_map[cluster_id];
        real_map = std::make_pair(map.first, clus_pos);
        svd_real_cluster_map.insert(std::make_pair(i, real_map));
      }
      i++;
    }
  }
}

/*
 * Calculate cluster position with simple center of gravity algorithm.
 */
double
SVDHoughTrackingModule::clusterPosition(double pitch, short seed_strip, int size, double offset)
{
  float hit_cor;
  float max_diff, diff;
  int max_pos, max_sample;

  max_diff = 50.0f;
  diff = 1e+11;

  hit_cor = ((double)(size - 1)) * (pitch / 2.0);

  max_sample = 0;
  max_pos = 0;
  if (size > 1) {
    /* Find peak and peak position within the peak samples */
    for (int i = 0; i < size; ++i) {
      if (peak_samples[i] > max_sample) {
        max_sample = peak_samples[i];
        max_pos = i;
      }
    }
    if (size > 2 && max_pos > 0 && max_pos < size) {
      if (peak_samples[max_pos - 1] > peak_samples[max_pos + 1]) {
        diff = (max_sample - peak_samples[max_pos - 1]);
      } else {
        diff = (peak_samples[max_pos + 1] - max_sample);
      }
    } else {
      diff = (peak_samples[1] - peak_samples[0]);
    }
  }

  if (fabsf(diff) > max_diff) {
    if (diff < 0.0) {
      diff = 0.9f;
    } else {
      diff = -0.9f;
    }
  } else {
    diff /= max_diff;
  }

  hit_cor += diff * (pitch / 2.0);

  B2DEBUG(250, "          Diff: " << diff << " hit_cor: " << hit_cor);

  return (pitch * ((double) seed_strip) - hit_cor - offset);
}

/*
 * Do coordinate translation and add cluster to cluster map.
 */
void
SVDHoughTrackingModule::clusterAdd(VxdID sensorID, bool p_side, short seed_strip, int size)
{
  Sensor sensor;
  double pitch, corr = 0.0;
  TVector3 pos, local_pos, first_pos;
  const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));

  local_pos.SetX(0.0);
  local_pos.SetY(sensorInfo->getUCellPosition(0));
  local_pos.SetZ(sensorInfo->getUCellPosition(0));

  first_pos = sensorInfo->pointToGlobal(local_pos);

  pos.SetX(first_pos.X());
  if (!p_side) {
    //pitch = sensorInfo->getUPitch();
    pitch = sensorInfo->getVPitch();

    /* // This condition makes no sense for me
    if (sensorID.getLayerNumber() == 5) {
      corr = 4.8 * Unit::mm;
    } else {
      corr = 0.0;
    }
    */

    pos.SetY(0.0);
    //pos.SetZ(clusterPosition(pitch, seed_strip, size, (sensorInfo->getWidth() / 2.0)) + corr);
    pos.SetZ(clusterPosition(pitch, seed_strip, size, (sensorInfo->getLength() / 2.0)) + corr);
    n_clusters.insert(std::make_pair(n_idx, std::make_pair(sensorID, pos)));
    ++n_idx;
  } else {
    //pitch = sensorInfo->getVPitch();
    pitch = sensorInfo->getUPitch();
    pos.SetZ(0.0);
    //pos.SetY(clusterPosition(pitch, seed_strip, size, (sensorInfo->getLength() / 2.0)));
    pos.SetY(clusterPosition(pitch, seed_strip, size, (sensorInfo->getWidth() / 2.0)));
    p_clusters.insert(std::make_pair(p_idx, std::make_pair(sensorID, pos)));
    ++p_idx;
  }

  B2DEBUG(250, "  -> New Cluster size: " << size << " Sensor: " << sensorID << " Side: "
          << p_side << " Seed Strip: " << seed_strip << " Pitch: " << pitch << " Pos: "
          << pos.X() << " " << pos.Y() << " " << pos.Z() << " Sensor Info: "
          << sensorInfo->getLength() << " " << sensorInfo->getWidth());
  for (int i = 0; i < size;  ++i) {
    B2DEBUG(250, "       |- Peak sample " << i << ": " << peak_samples[i]);
  }
}

/*
 * Analyse the clusters and compare the extrapolated coordinates
 * with official Clusterizer.
 */
void
SVDHoughTrackingModule::analyseClusterStrips()
{
  TVector3 pos;
  svdClusterMapIter iter;
  svdClusterPair cluster_info;
  int nSVDClusters, sensor_cnt;
  int nTrueHit;
  double closest_dist, dist;
  VxdID sensorID, svd_sensorID;
  StoreArray<SVDCluster> storeClusters(m_storeSVDClusterName);
  const StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  SVDCluster* svdCluster = NULL;
  SVDTrueHit* svdTrueHit = NULL;

  nSVDClusters = storeClusters.getEntries();
  if (nSVDClusters == 0) {
    return;
  }
  nTrueHit = storeSVDTrueHits.getEntries();
  if (nTrueHit == 0) {
    return;
  }

  sensor_cnt = 0;
  closest_dist = 1.0e+100;

  B2DEBUG(2, "List of clusters");

  B2DEBUG(2, " N-side");
  for (iter = n_clusters.begin(); iter != n_clusters.end(); ++iter) {
    cluster_info = iter->second;
    pos = cluster_info.second;
    sensorID = cluster_info.first;
    B2DEBUG(2, "  Sensor ID: " << sensorID << " Position: "
            << pos.X() << " " << pos.Y() << " " << pos.Z());

    /* Compare SVD clusters with Hough clusters */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nSVDClusters; ++i) {
      svdCluster = storeClusters[i];
      //if (sensorID == svdCluster->getSensorID() && svdCluster->isUCluster()) {
      if (sensorID == svdCluster->getSensorID() && !svdCluster->isUCluster()) {
        dist = (svdCluster->getPosition() -  pos.Z());
        B2DEBUG(2, "    | Compare with Sensor ID: " << svdCluster->getSensorID() << " Size "
                << svdCluster->getSize() << " z-Position: " << svdCluster->getPosition()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      //m_histClusterU->Fill(closest_dist);
      m_histClusterV->Fill(closest_dist);
    }

    /* Compare Hough clusters with True hits */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nTrueHit; ++i) {
      svdTrueHit = storeSVDTrueHits[i];
      if (sensorID == svdCluster->getSensorID()) {
        //dist = (svdTrueHit->getU() -  pos.Z());
        dist = (svdTrueHit->getV() -  pos.Z());
        B2DEBUG(2, "    | Compare with True hit: " << svdTrueHit->getSensorID()
                << " z-Position: " << svdTrueHit->getU()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      //m_histTrueClusterU->Fill(closest_dist);
      m_histTrueClusterV->Fill(closest_dist);
    }
  }

  B2DEBUG(2, " P-side");
  for (iter = p_clusters.begin(); iter != p_clusters.end(); ++iter) {
    cluster_info = iter->second;
    pos = cluster_info.second;
    sensorID = cluster_info.first;
    B2DEBUG(2, "  Sensor ID: " << cluster_info.first << " Position: "
            << pos.X() << " " << pos.Y() << " " << pos.Z());

    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nSVDClusters; ++i) {
      svdCluster = storeClusters[i];
      //if (sensorID == svdCluster->getSensorID() && !svdCluster->isUCluster()) {
      if (sensorID == svdCluster->getSensorID() && svdCluster->isUCluster()) {
        dist = (svdCluster->getPosition() -  pos.Y());
        B2DEBUG(2, "    | Compare with Sensor ID: " << svdCluster->getSensorID() << " Size "
                << svdCluster->getSize() << " y-Position: " << svdCluster->getPosition()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      //m_histClusterV->Fill(closest_dist);
      m_histClusterU->Fill(closest_dist);
    }

    /* Compare Hough clusters with True hits */
    sensor_cnt = 0;
    closest_dist = 1.0e+100;
    for (int i = 0; i < nTrueHit; ++i) {
      svdTrueHit = storeSVDTrueHits[i];
      if (sensorID == svdCluster->getSensorID()) {
        //dist = (svdTrueHit->getV() -  pos.Y());
        dist = (svdTrueHit->getU() -  pos.Y());
        B2DEBUG(2, "    | Compare with True hit: " << svdTrueHit->getSensorID()
                << " y-Position: " << svdTrueHit->getV()
                << " Distance to current cluster: " << dist);
        if (fabs(closest_dist) > fabs(dist)) {
          closest_dist = dist;
        }
        ++sensor_cnt;
      }
    }
    if (closest_dist < 0.08) {
      //m_histTrueClusterV->Fill(closest_dist);
      m_histTrueClusterU->Fill(closest_dist);
    }
  }
}

/*
 * Top module for the Strip Cluster engine.
 */
void
SVDHoughTrackingModule::printClusters(svdClusterMap& cluster, bool p_side)
{
  svdClusterPair clusterInfo;
  TVector3 pos;
  VxdID sensorID;

  if (!p_side) {
    B2DEBUG(1, "Clusters N-Side: " << cluster.size());
  } else {
    B2DEBUG(1, "Clusters P-Side: " << cluster.size());
  }

  for (auto it = cluster.begin(); it != cluster.end(); ++it) {
    clusterInfo = it->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    B2DEBUG(1, "  " << pos.X() << " "  << pos.Y() << " " <<  pos.Z() << " in Sensor " << sensorID);
  }
}

/*
 * Top module for the Strip Cluster engine.
 */
void
SVDHoughTrackingModule::clusterStrips()
{
  StoreArray<SVDDigit> storeDigits(m_storeSVDDigitsName);
  int nDigits, sample_cnt, cluster_size, first;
  SVDDigit* digit;
  VxdID sensorID, last_sensorID;
  bool p_side, last_side;
  bool valid;
  short strip, last_strip;
  float sample;
  float samples[6];
  float cur_peak_sample;
  double pos;

  B2DEBUG(250, "\nStart Clustering" << endl);

  nDigits = storeDigits.getEntries();
  if (nDigits == 0) {
    return;
  }

  /* Clear cluster maps for p and n side */
  p_clusters.clear();
  n_clusters.clear();

  last_sensorID = VxdID(0);
  last_side = false;
  last_strip = 0;
  sample_cnt = 0;
  cluster_size = 1;
  first = 1;

  B2DEBUG(1, "Cluster info: total digits: " << nDigits << " included");

  /* Loop over all Digits and aggregate first all samples from a
   * single strip. Then run the noise filter on them. Only when
   * the noise filter accepts the strips add it to the list of
   * cluster candidates.
   */
  for (int i = 0; i < nDigits; i++) {
    digit = storeDigits[i];
    sensorID = digit->getSensorID();
    p_side = digit->isUStrip();
    strip = digit->getCellID();
    sample = digit->getCharge();
    pos = digit->getCellPosition();

    B2DEBUG(350, "  Sensor: " << sensorID << " Side: " << p_side << " Strip: " << strip
            << " Charge: " << sample << " Position: " << pos);


    //if ((last_sensorID == sensorID && last_side == n_side) || first) {
    if ((last_sensorID == sensorID && last_side != p_side) || first) {
      /* Same strip, add up all samples */
      if ((strip == last_strip && sample_cnt != 6) || first) {
        first = 0;
        samples[sample_cnt] = sample;
        ++sample_cnt;
      } else {
        /* New strip, so noise filter old strip first */
        valid = noiseFilter(samples, sample_cnt, &cur_peak_sample);
        if (!valid) {
          if (cluster_size > 1) {
            clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
            cluster_size = 1;
          }
          continue;
        }

        if (strip == (last_strip + 1) && cluster_size <= 4) {
          peak_samples[cluster_size - 1] = cur_peak_sample;
          ++cluster_size;
        } else {
          clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
          cluster_size = 1;
          peak_samples[cluster_size - 1] = cur_peak_sample;
        }

        sample_cnt = 0;
        samples[sample_cnt] = sample;
        ++sample_cnt;
      }
    } else {
      valid = noiseFilter(samples, sample_cnt, &cur_peak_sample);
      if (valid) {
        peak_samples[cluster_size - 1] = cur_peak_sample;
        clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
      } else {
        peak_samples[0] = cur_peak_sample;
      }
      cluster_size = 1;
      sample_cnt = 0;
      samples[sample_cnt] = sample;
      ++sample_cnt;
      //create_strip_cluster();
    }
    last_strip = strip;
    //last_side = n_side;
    last_side = p_side;
    last_sensorID = sensorID;
  }

  /* Last cluster */
  if (nDigits > 0) {
    if (noiseFilter(samples, sample_cnt, &cur_peak_sample)) {
      peak_samples[cluster_size - 1] = cur_peak_sample;
      clusterAdd(last_sensorID, last_side, last_strip, cluster_size); /* Add cluster */
    }
  }

  B2DEBUG(250, "End Clustering" << endl);
}
