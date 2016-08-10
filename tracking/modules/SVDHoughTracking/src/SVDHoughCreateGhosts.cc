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

/* Options */
//#define INJECT_GHOSTS

/*
 * Create ghosts.
 */
void
SVDHoughTrackingModule::createGhosts()
{
  vector<TVector3> ghost;
  /* Layer 3 */
  ghost.push_back(TVector3(-3.18393, -0.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -0.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -0.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -0.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -0.573994, -3.8714322));

  ghost.push_back(TVector3(-3.18393, -1.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -1.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -1.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -1.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -1.573994, -3.8714322));

  ghost.push_back(TVector3(-3.18393, -2.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -2.573994, 0.2504667));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -2.573994, -3.8714322));
  ghost.push_back(TVector3(-3.18393, -2.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -2.573994, 1.7185263));

  ghost.push_back(TVector3(-3.18393, 2.573994, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 2.573994, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 2.573994, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 2.573994, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 2.573994, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 2.573994, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 2.133992, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 2.133992, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 2.133992, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 2.133992, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 2.133992, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 2.133992, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 1.231563, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 1.231563, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 1.231563, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 1.231563, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 1.231563, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 1.231563, 0.2504667));

  ghost.push_back(TVector3(-3.18393, 0.782551, -3.8714322));
  ghost.push_back(TVector3(-3.18393, 0.782551, 3.5824531));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.8941479));
  ghost.push_back(TVector3(-3.18393, 0.782551, 2.6434211));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.2504667));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.7185263));
  ghost.push_back(TVector3(-3.18393, 0.782551, 1.2177692));
  ghost.push_back(TVector3(-3.18393, 0.782551, 0.2504667));

  ghost.push_back(TVector3(-3.18393, -1.755843, 3.5824531));
  ghost.push_back(TVector3(-3.18393, -1.755843, -3.8714322));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.8941479));
  ghost.push_back(TVector3(-3.18393, -1.755843, 2.6434211));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.2504667));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.7185263));
  ghost.push_back(TVector3(-3.18393, -1.755843, 1.2177692));
  ghost.push_back(TVector3(-3.18393, -1.755843, 0.2504667));

  /* Layer 4 */
  ghost.push_back(TVector3(1.01896, 1.755843, 5.2504667));
  ghost.push_back(TVector3(1.01896, 1.755843, -2.2938452));
  ghost.push_back(TVector3(1.01896, 1.755843, +2.2938452));
  ghost.push_back(TVector3(1.01896, 1.755843, -1.7820537));

  ghost.push_back(TVector3(1.01896, 0.125458, -2.2938452));
  ghost.push_back(TVector3(1.01896, 0.125458, 5.2504667));
  ghost.push_back(TVector3(1.01896, 0.125458, +2.2938452));
  ghost.push_back(TVector3(1.01896, 0.125458, -1.7820537));

  ghost.push_back(TVector3(1.01896, -1.150386, +2.2938452));
  ghost.push_back(TVector3(1.01896, -1.150386, 5.2504667));
  ghost.push_back(TVector3(1.01896, -1.150386, -2.2938452));
  ghost.push_back(TVector3(1.01896, -1.150386, -1.7820537));

  ghost.push_back(TVector3(1.01896, -2.016503, -1.7820537));
  ghost.push_back(TVector3(1.01896, -2.016503, 5.2504667));
  ghost.push_back(TVector3(1.01896, -2.016503, -2.2938452));
  ghost.push_back(TVector3(1.01896, -2.016503, +2.2938452));

  /* Layer 5 */
  ghost.push_back(TVector3(3.51593, 0.855843, 5.2504667));
  ghost.push_back(TVector3(3.51593, 0.855843, -3.1234510));
  ghost.push_back(TVector3(3.51593, 0.855843, -1.1432447));
  ghost.push_back(TVector3(3.51593, 0.855843, +0.1456811));

  ghost.push_back(TVector3(3.51593, -1.257368, -3.1234510));
  ghost.push_back(TVector3(3.51593, -1.257368, 5.2504667));
  ghost.push_back(TVector3(3.51593, -1.257368, -1.1432447));
  ghost.push_back(TVector3(3.51593, -1.257368, +0.1456811));

  ghost.push_back(TVector3(3.51593, +0.145622, -1.1432447));
  ghost.push_back(TVector3(3.51593, +0.145622, 5.2504667));
  ghost.push_back(TVector3(3.51593, +0.145622, -3.1234510));
  ghost.push_back(TVector3(3.51593, +0.145622, +0.1456811));

  ghost.push_back(TVector3(3.51593, +0.023453, +0.1456811));
  ghost.push_back(TVector3(3.51593, +0.023453, -1.1432447));
  ghost.push_back(TVector3(3.51593, +0.023453, -3.1234510));
  ghost.push_back(TVector3(3.51593, +0.023453, 5.2504667));

  /* Layer 6 */
  ghost.push_back(TVector3(6.51591, 2.417032, -3.1254667));
  ghost.push_back(TVector3(6.51591, 2.417032, -0.9834291));
  ghost.push_back(TVector3(6.51591, 2.417032, -2.6748223));
  ghost.push_back(TVector3(6.51591, 2.417032, -3.1254667));
  ghost.push_back(TVector3(6.51591, 2.417032, +1.1245973));

  ghost.push_back(TVector3(6.51591, -0.185326, -0.9834291));
  ghost.push_back(TVector3(6.51591, -0.185326, -2.6748223));
  ghost.push_back(TVector3(6.51591, -0.185326, -3.1254667));
  ghost.push_back(TVector3(6.51591, -0.185326, +1.1245973));

  ghost.push_back(TVector3(6.51591, +2.732843, -2.6748223));
  ghost.push_back(TVector3(6.51591, +2.732843, +1.1245973));
  ghost.push_back(TVector3(6.51591, +2.732843, -0.9834291));
  ghost.push_back(TVector3(6.51591, +2.732843, -3.1254667));

  ghost.push_back(TVector3(6.51591, -1.039503, +1.1245973));
  ghost.push_back(TVector3(6.51591, -1.039503, -2.6748223));
  ghost.push_back(TVector3(6.51591, -1.039503, -0.9834291));
  ghost.push_back(TVector3(6.51591, -1.039503, -3.1254667));

  for (unsigned int i = 0; i < ghost.size(); ++i) {
    //cluster_map.insert(std::make_pair((iter->first) + 1 + i, ghost[i]));
  }
}