#include <tracking/trackFindingVXD/sectorMapTools/NoKickRTSel.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>

using namespace Belle2;


void NoKickRTSel::hitToTrueXPBuilder(const RecoTrack& track)
{
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;
  StoreArray<MCParticle> MCParticles;
  StoreArray<RecoTrack> recoTracks;

  std::vector<Belle2::RecoHitInformation::UsedSVDHit*> clusterList = track.getSVDHitList();
  for (const SVDCluster* cluster : clusterList) {
    const MCParticle* particle = cluster->getRelationsTo<MCParticle>()[0];
    for (const SVDTrueHit& hit : cluster->getRelationsTo<SVDTrueHit>()) {
      VxdID trueHitSensorID = hit.getSensorID();
      const VXD::SensorInfoBase& sensorInfo = VXD::GeoCache::getInstance().getSensorInfo(trueHitSensorID);
      hitToTrueXPDerivate entry(hit, *cluster, *particle, sensorInfo);
      int NClusterU = 0;
      int NClusterV = 0;
      for (SVDCluster Ncluster : hit.getRelationsFrom<SVDCluster>()) {
        if (Ncluster.isUCluster()) NClusterU++;
        else NClusterV++;
      }
      entry.setClusterU(NClusterU);
      entry.setClusterV(NClusterV);

      bool isReconstructed(false);
      for (const RecoTrack& aRecoTrack : particle->getRelationsFrom<RecoTrack>())
        isReconstructed |= aRecoTrack.hasSVDHits();
      entry.setReconstructed(isReconstructed);


      m_setHitToTrueXP.insert(entry);
    }
  }
  for (auto element : m_setHitToTrueXP) {
    m_hitToTrueXP.push_back(element);
  }
}

void NoKickRTSel::hit8TrackBuilder(const RecoTrack& track)
{
  hitToTrueXPBuilder(track);
  for (const hitToTrueXP XP : m_hitToTrueXP) {
    if (m_8hitTrack.size() < 1) {
      m_8hitTrack.push_back(XP);
    } else if (XP.m_sensorLayer != m_8hitTrack.back().m_sensorLayer ||
               XP.m_sensorLadder != m_8hitTrack.back().m_sensorLadder ||
               XP.m_sensorSensor != m_8hitTrack.back().m_sensorSensor) {
      m_8hitTrack.push_back(XP);
    }
  }

}

bool NoKickRTSel::segmentSelector(hitToTrueXP hit1, hitToTrueXP hit2, std::vector<double> selCut, parameters par, bool is0)
{
  double deltaPar = 0;
  if (hit2.m_sensorLayer - hit1.m_sensorLayer > 1) return true;
  else {
    switch (par) {
      case 0:
        deltaPar = abs(hit1.getOmegaEntry() - hit2.getOmegaEntry());
        if (is0) deltaPar = abs(hit1.getOmega0() - hit2.getOmegaEntry());
        break;

      case 1:
        deltaPar = hit1.getD0Entry() - hit2.getD0Entry();
        if (is0) deltaPar = hit1.getD00() - hit2.getD0Entry();
        break;

      case 2:
        deltaPar = hit1.getPhi0Entry() - hit2.getPhi0Entry();
        if (is0) deltaPar = hit1.getPhi00() - hit2.getPhi0Entry();
        break;

      case 3:
        deltaPar = hit1.getZ0Entry() - hit2.getZ0Entry();
        if (is0) deltaPar = hit1.getZ00() - hit2.getZ0Entry();

        break;

      case 4:
        deltaPar = hit1.getTanLambdaEntry() - hit2.getTanLambdaEntry();
        if (is0) deltaPar = hit1.getTanLambda0() - hit2.getTanLambdaEntry();

        break;
    }
    if (deltaPar > selCut.at(0) && deltaPar < selCut.at(1)) return true;
    else return false;
  }
}



bool NoKickRTSel::trackSelector(const RecoTrack& track)
{
  initNoKickRTSel();
  hit8TrackBuilder(track);
  bool good = true;
  if (track.getMomentumSeed().Mag() > m_pmax) return good;
  for (int i = 0; i < (int)(m_8hitTrack.size() - 2); i++) {
    double sinTheta = abs(m_8hitTrack.at(i).m_momentum0.Y()) /
                      sqrt(pow(m_8hitTrack.at(i).m_momentum0.Y(), 2) +
                           pow(m_8hitTrack.at(i).m_momentum0.Z(), 2));

    double momentum =
      sqrt(pow(m_8hitTrack.at(i).m_momentum0.X(), 2) +
           pow(m_8hitTrack.at(i).m_momentum0.Y(), 2) +
           pow(m_8hitTrack.at(i).m_momentum0.Z(), 2));

    for (int j = 0; j < 5; j++) {
      std::vector<double> selCut = m_trackCuts.cutSelector(sinTheta, momentum, m_8hitTrack.at(i).m_sensorLayer,
                                                           m_8hitTrack.at(i + 1).m_sensorLayer, (NoKickCuts::parameter) j);
      bool goodSeg = segmentSelector(m_8hitTrack.at(i), m_8hitTrack.at(i + 1), selCut, (parameters) j);

      if (!goodSeg) {
        good = false;
      }
      if (i == 0) { //PXD crossing
        std::vector<double> selCut0 = m_trackCuts.cutSelector(sinTheta, momentum, 0, m_8hitTrack.at(i).m_sensorLayer,
                                                              (NoKickCuts::parameter) j);

        bool goodSeg0 = segmentSelector(m_8hitTrack.at(i), m_8hitTrack.at(i), selCut, (parameters) j, true);
        if (!goodSeg0) {
          good = false;
        }
      }

    }
  }
  return good;
}
