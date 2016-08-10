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
 * Create sector map
 */
void
SVDHoughTrackingModule::create_sector_map()
{
  int i, skip;
  unsigned short next_layer;
  bool valid;
  StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  clusterSensorMap::iterator iter;
  sensorMap map, next_map;
  sensorMap map_arr[4];
  VxdID cur_id;
  TVector3 cur_vec, vec, next_vec, distance, abs_pos;

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  for (iter = svd_real_cluster_map.begin(); iter != svd_real_cluster_map.end(); ++iter) {
    map = iter->second;
    next_layer = 4;
    i = 1;
    if (map.first.getLayerNumber() == 3) {
      B2DEBUG(250, "New Seed: " << map.first << " Coord: " << cur_vec.X()
              << " " << cur_vec.Y()
              << cur_vec.Z());
      next_map = map;
      map_arr[0] = map;
      skip = 0;
      while (next_layer <= 6) {
        next_map = get_next_hit(next_map, next_layer, &valid);
        if (!valid) {
          skip = 1;
          break;
        }
        map_arr[i] = next_map;
        ++i;
        ++next_layer;
      }
      if (!skip) {
        sector_track_fit(map_arr); /* Fit tracks */
      } else {
        B2DEBUG(250, "...skip");
      }
    }
  }
}

/*
 * Sector-Neighbour-Finder (see chapter 4.1 in Michael's PhD thesis)
 * Get next hit in the next layer
 */
sensorMap
SVDHoughTrackingModule::get_next_hit(sensorMap map, unsigned short layer, bool* valid)
{
  static int run = 0;
  double last_dist;
  clusterSensorMap::iterator iter;
  sensorMap closest_hit, inner_map;
  TVector3 cur_vec, vec, next_vec, distance;

  cur_vec = map.second;
  last_dist = 1e128;
  for (iter = svd_real_cluster_map.begin(); iter != svd_real_cluster_map.end(); ++iter) {
    inner_map = iter->second;

    if (inner_map.first.getLayerNumber() == layer) {
      next_vec = inner_map.second;
      distance = cur_vec - next_vec;
      if (distance.Mag() < last_dist) {
        last_dist = distance.Mag();
        closest_hit = inner_map;

        if (m_histDist && layer == 4) {
          m_histDist->Fill(last_dist);
        }
      }
    }
  }

  ++run;
  if (last_dist > dist_thres[(layer - 4)]) {
    B2DEBUG(200, " Next: " << layer << " hit " << closest_hit.first << " distance: "
            << last_dist << " Angle: " << closest_hit.second.Angle(cur_vec) << " failed: "
            << dist_thres[(layer - 4)] << " < " << last_dist);
    *valid = 0;
  } else {
    B2DEBUG(200, " Next: " << layer << " hit " << closest_hit.first << " distance: "
            << last_dist << " Angle: " << closest_hit.second.Angle(cur_vec) << " valid");
    *valid = 1;
  }

  return (closest_hit);
}

/*
 * Sector-Neighbour-Finder (see chapter 4.1 in Michael's PhD thesis)
 * Fit track
 */
int
SVDHoughTrackingModule::sector_track_fit(sensorMap* map)
{
  int i;
  double m1, a1, m2, a2, m_x, m_z;
  clusterSensorMap::iterator iter;
  TVector3 cur_vec, next_vec, diff;
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);

  m1 = 0;
  a1 = 0;
  m2 = 0;
  a2 = 0;
  for (i = 0; i < 2 - 1; ++i) {
    cur_vec = map[i].second;
    next_vec = map[i + 1].second;
    diff = next_vec - cur_vec;
    m_x = diff.Y() / diff.X();
    m1 += m_x;
    a1 += -1.0 * m_x * cur_vec.X() + cur_vec.Y();
    m_z = diff.Z() / diff.Y();
    m2 += m_z;
    a2 += -1.0 * m_z * cur_vec.Y() + cur_vec.Z();
  }
  /*m1 /= 4.0;
  a1 /= 4.0;
  m2 /= 4.0;
  a2 /= 4.0;*/

  storeHoughTrack.appendNew(SVDHoughTrack(m1, a1, m2, a2));
  B2DEBUG(200, "Track parameters: m1 = " << m1 << " a1 = " << a1 << " m2 = " << m2 << " a2 = " << a2);
  return (0);
}


