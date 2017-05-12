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

bool NoKickRTSel::globalCut(const std::vector<hitToTrueXP>& track8)
{
  // int flag3 = 0;
  // int flag6 = 0;
  int flagd0 = 1;
  //int flagphi0 = 1;
  int flagz0 = 1;
  //int flagtanlambda = 1;
  int lay3 = 0;
  int lay4 = 0;
  int lay5 = 0;
  int lay6 = 0;
  for (hitToTrueXP XP : track8) {
    if (XP.getSensorLayer() == 3) lay3 = 1;
    if (XP.getSensorLayer() == 4) lay4 = 1;
    if (XP.getSensorLayer() == 5) lay5 = 1;
    if (XP.getSensorLayer() == 6) lay6 = 1;
    // if (XP.getSensorLayer() == 3) flag3 = 1;
    // if (XP.getSensorLayer() == 6) flag6 = 1;
    if (abs(XP.getD0Entry()) > 1.) flagd0 = 0;
    //if (abs(XP.getPhi0Entry()) > 4) flagphi0 = 0;
    if (abs(XP.getZ0Entry()) > 1.) flagz0 = 0;
    //if (abs(XP.getTanLambdaEntry()) > 4) flagtanlambda = 0;
  }
  // if(flag3 ==0) std::cout << "flag3" << std::endl;
  // if(flag6 ==0) std::cout << "flag6" << std::endl;
  // if(flagd0 ==0) std::cout << "flagd0" << std::endl;
  // if(flagphi0 ==0) std::cout << "flagphi0" << std::endl;
  // if(flagz0 ==0) std::cout << "flagz0" << std::endl;
  // if(flagtanlambda ==0) std::cout << "flagtanlambda" << std::endl;
  int N_lay = lay3 + lay4 + lay5 + lay6;
  if (N_lay >= 3) N_lay = 1;
  else N_lay = 0;
  // if(N_lay ==0) std::cout << "N_lay" << std::endl;
  //int flagTot = flagd0 * flagphi0 * flagz0 * flagtanlambda * N_lay;
  int flagTot = flagd0 * flagz0 * N_lay;
  if (flagTot == 1) return true;
  else return false;
}

bool NoKickRTSel::segmentSelector(hitToTrueXP hit1, hitToTrueXP hit2, std::vector<double> selCut, parameters par, bool is0)
{
  double deltaPar = 0;
  if (hit2.m_sensorLayer - hit1.m_sensorLayer > 1) return true;
//  if (hit2.m_sensorLayer !=4 || hit1.m_sensorLayer !=3) return true;
//  if(is0) return true;
  //if (hit2.m_sensorLayer == hit1.m_sensorLayer) return true;
  else {
    switch (par) {
      case 0:
        return true;//REMOVED OMEGA FROM CUTS
        deltaPar = abs(hit1.getOmegaEntry() - hit2.getOmegaEntry());
        if (is0) deltaPar = abs(hit1.getOmega0() - hit2.getOmegaEntry());
        break;

      case 1:
        deltaPar = hit1.getD0Entry() - hit2.getD0Entry();
        if (is0) deltaPar = hit1.getD00() - hit2.getD0Entry();
        break;

      case 2:
        //  return true;
        deltaPar = asin(sin(hit1.getPhi0Entry())) - asin(sin(hit2.getPhi0Entry()));
        if (is0) deltaPar = asin(sin(hit1.getPhi00())) - asin(sin(hit2.getPhi0Entry()));
        break;

      case 3:
        // return true;
        deltaPar = hit1.getZ0Entry() - hit2.getZ0Entry();
        if (is0) deltaPar = hit1.getZ00() - hit2.getZ0Entry();

        break;

      case 4:
        // return true;
        deltaPar = hit1.getTanLambdaEntry() - hit2.getTanLambdaEntry();
        if (is0) deltaPar = hit1.getTanLambda0() - hit2.getTanLambdaEntry();

        break;
    }
    //DEBUG-----------------------------------------
    double usedCut = 0;
    if (abs(selCut.at(0)) > abs(selCut.at(1))) {
      usedCut = abs(selCut.at(0));
    }
    //else usedCut =abs(selCut.at(1));
    else usedCut = abs(selCut.at(1));
    //if(par ==1 || par ==3) usedCut=0.5; else usedCut=0.2;
    if (deltaPar < -usedCut || deltaPar > usedCut) {
      //if(deltaPar < 1*selCut.at(0) || deltaPar > 1*selCut.at(1)){
      std::cout << "--------------------------" << std::endl;
      std::cout << "lay1=" << hit1.m_sensorLayer << std::endl;
      std::cout << "lay2=" << hit2.m_sensorLayer << std::endl;
      std::cout << "parametro=" << par << std::endl;
      std::cout << "Min=" << selCut.at(0) << std::endl;
      std::cout << "Max=" << selCut.at(1) << std::endl;
      std::cout << "deltapar=" << deltaPar << std::endl;
      std::cout << "momentum=" << hit1.m_momentum0.Mag() << std::endl;


    }

    if (deltaPar > -usedCut && deltaPar < usedCut) return true;
    //if (deltaPar > 1*selCut.at(0) && deltaPar < 1*selCut.at(1)) return true;
    else return false;
  }
}



bool NoKickRTSel::trackSelector(const RecoTrack& track)
{
  initNoKickRTSel();
  hit8TrackBuilder(track);
  bool good = true;
  good = globalCut(m_8hitTrack);
  if (good == false) return good;
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
