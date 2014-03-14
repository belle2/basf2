#pragma once


namespace Belle2 {

  /**
     * Small helper class to perform sorting of hits in the CDC pattern reco
     */

  class CDCLegendreTrackHit;

  struct CDCLegendreTrackingSortHit {

    CDCLegendreTrackingSortHit(int charge) : m_charge(charge) {}
    bool operator()(CDCLegendreTrackHit* hit1, CDCLegendreTrackHit* hit2);
    int m_charge;
  };

}
