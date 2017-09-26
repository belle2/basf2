#ifndef BREMFINDING_H
#define BREMFINDING_H

#include <ecl/modules/eclTrackBremFinder/ECLTrackBremFinderModule.h>
#include <ecl/modules/eclTrackBremFinder/BestMatchContainer.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <genfit/AbsFitterInfo.h>
#include <set>

#include <TMath.h>

#include <functional>

namespace Belle2 {

  class BremFinding {
  public:
    BremFinding(float clusterAcceptanceFactor, ECLCluster cluster, genfit::MeasuredStateOnPlane measuredStateOnPlane) :
      m_clusterAcceptanceFactor(clusterAcceptanceFactor),
      m_eclCluster(cluster),
      m_measuredStateOnPlane(measuredStateOnPlane)
    {}

    void setECLCluster(ECLCluster cluster) {m_eclCluster = cluster;}

    void setMeasuredStateOnPlane(genfit::MeasuredStateOnPlane measuredstate) {m_measuredStateOnPlane = measuredstate;}

    void setClusterAcceptanceFactor(float acceptanceFactor) {m_clusterAcceptanceFactor = acceptanceFactor;}

    void setAngleCorrectionTrue() {m_angleCorrection = true;}

    bool isMatch();

    double getDistanceHitCluster() {return m_distanceHitCluster;}
  private:
    float m_clusterAcceptanceFactor;

    ECLCluster m_eclCluster;

    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;

    bool m_angleCorrection = false;

    double m_distanceHitCluster;


  };

} //Belle2
#endif