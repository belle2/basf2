/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTrackCalDigitMatch/ECLTrackCalDigitMatchModule.h>

//Framework
#include <framework/logging/Logger.h>
#include <framework/dataobjects/Helix.h>

//MDST
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>

//ECL
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLCalDigit.h>

//Analysis
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>

using namespace Belle2;
using namespace std;

#define TWOPI 6.28318530718

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTrackCalDigitMatch)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLTrackCalDigitMatchModule::ECLTrackCalDigitMatchModule() : Module()
{
  // Set module properties
  setDescription("Find ECLCalDigits closest to a track (this is *not* a track to cluster matcher)");
  addParam("extRadius", m_extRadius, "radius to which tracks are extrapolated [cm]", 130.00);
  addParam("angleFWDGap", m_angleFWDGap, "FWD gap angle [deg]", 31.80);
  addParam("angleBWDGap", m_angleBWDGap, "BWD gap angle [deg]", 129.70);
  addParam("trackHypothesis", m_trackHypothesis, "Track hypothesis (PDG code) used in extrapolation", 11);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLTrackCalDigitMatchModule::initialize()
{

  /** Required dataobjects */
  m_eclCalDigits.isRequired();
  m_tracks.isRequired();

  /** output dataobjects */
  m_anaEnergyCloseToTrack.registerInDataStore();

  /** Register relations */
  m_tracks.registerRelationTo(m_anaEnergyCloseToTrack);

  // ECL geometry
  geom = ECL::ECLGeometryPar::Instance();

  // based on the (fix) endcap gap positions, get the fwd and bwd z positions
  m_extZFWD = m_extRadius / std::tan(m_angleFWDGap * 0.0174533);
  m_extZBWD = m_extRadius / std::tan(m_angleBWDGap * 0.0174533);
  B2DEBUG(50, "m_extZFWD: " << m_extZFWD << ", m_extZBWD: " << m_extZBWD);

  // fill maps that related phi id to list of cell ids (ECLGeometryPar returns IDs from 0..)
  for (unsigned int i = 0; i < 144; ++i) {
    const unsigned thetaFWDEndcap = 12;
    const unsigned thetaFWDBarrel = 13;
    m_FWD3Barrel[i] = { geom->GetCellID(thetaFWDBarrel, (i == 0) ? 143 : i - 1) + 1,
                        geom->GetCellID(thetaFWDBarrel, i) + 1,
                        geom->GetCellID(thetaFWDBarrel, (i == 143) ? 0 : i + 1) + 1
                      };

    m_FWD3Endcap[i] = { geom->GetCellID(thetaFWDEndcap, (i == 0) ? 143 : i - 1) + 1,
                        geom->GetCellID(thetaFWDEndcap, i) + 1,
                        geom->GetCellID(thetaFWDEndcap, (i == 143) ? 0 : i + 1) + 1
                      };

    const unsigned thetaBWDEndcap = 59;
    const unsigned thetaBWDBarrel = 58;
    m_BWD3Barrel[i] = { geom->GetCellID(thetaBWDBarrel, (i == 0) ? 143 : i - 1) + 1,
                        geom->GetCellID(thetaBWDBarrel, i) + 1,
                        geom->GetCellID(thetaBWDBarrel, (i == 143) ? 0 : i + 1) + 1
                      };

    m_BWD3Endcap[i] = { geom->GetCellID(thetaBWDEndcap, (i == 0) ? 143 : i - 1) + 1,
                        geom->GetCellID(thetaBWDEndcap, i) + 1,
                        geom->GetCellID(thetaBWDEndcap, (i == 143) ? 0 : i + 1) + 1
                      };
  }

  m_eclCalDigitsArray.resize(8736 + 1);
}

void ECLTrackCalDigitMatchModule::event()
{

  // Fill a vector that can be used to map cellid -> store array position for eclCalDigits.
  memset(&m_eclCalDigitsArray[0], -1, m_eclCalDigitsArray.size() * sizeof m_eclCalDigitsArray[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_eclCalDigitsArray[m_eclCalDigits[i]->getCellId()] = i;
  }

  for (const Track& track : m_tracks) {

    const auto anainfo = m_anaEnergyCloseToTrack.appendNew();
    track.addRelationTo(anainfo);

    // get the trackfit results for this track
    Const::ChargedStable chargedStable(abs(m_trackHypothesis));
    auto closestMassFitResult = track.getTrackFitResultWithClosestMass(chargedStable);
    if (closestMassFitResult == nullptr) continue;

    // get the track parameters
    const double z0        = closestMassFitResult->getZ0();
    const double tanlambda = closestMassFitResult->getTanLambda();

    // use the helix class
    Helix h = closestMassFitResult->getHelix();

    // extrapolate to radius
    const double lHelixRadius = h.getArcLength2DAtCylindricalR(m_extRadius) > 0 ? h.getArcLength2DAtCylindricalR(m_extRadius) : 999999.;

    // extrapolate to FWD z
    const double lFWD = (m_extZFWD - z0) / tanlambda > 0 ? (m_extZFWD - z0) / tanlambda : 999999.;

    // extrapolate to backward z
    const double lBWD = (m_extZBWD - z0) / tanlambda > 0 ? (m_extZBWD - z0) / tanlambda : 999999.;

    // pick smalles arclength
    const double l = std::min(std::min(lHelixRadius, lFWD), lBWD);

    B2DEBUG(50, lHelixRadius << " " << lFWD << " " << lBWD << " -> " << l);

    TVector3 posHelix = h.getPositionAtArcLength2D(l);
    const double exttheta = atan2(posHelix.Perp(), posHelix.Z());
    const double extphi = atan2(posHelix.Y(), posHelix.X());

    // check if the extrapolation reaches the requested radius
    if (!isnan(exttheta)) {
      // get the correct phi bin for this track (phi: -PI..PI, but phi-ids in ECL from 0..144)
      double extphitwopi = extphi;
      if (extphi < 0) extphitwopi = TWOPI + extphi;
      const int phiid = extphitwopi / (TWOPI / 144);

      // get the energy sums
      double sum3FWDBarrel = 0.0;
      double sum3FWDEndcap = 0.0;
      double sum3BWDBarrel = 0.0;
      double sum3BWDEndcap = 0.0;

      for (int i = 0; i < 3; ++i) {
        int pos = m_eclCalDigitsArray[m_FWD3Barrel[phiid][i]];
        if (pos >= 0) {
          sum3FWDBarrel += m_eclCalDigits[pos]->getEnergy();
        }
        B2DEBUG(50, phiid << " " << i << " " << m_FWD3Barrel[phiid][i] << " " << sum3FWDBarrel);

        pos = m_eclCalDigitsArray[m_FWD3Endcap[phiid][i]];
        if (pos >= 0) {
          sum3FWDEndcap += m_eclCalDigits[pos]->getEnergy();
        }
        B2DEBUG(50, phiid << " " << i << " " << m_FWD3Endcap[phiid][i] << " " << sum3FWDEndcap);

        pos = m_eclCalDigitsArray[m_BWD3Barrel[phiid][i]];
        if (pos >= 0) {
          sum3BWDBarrel += m_eclCalDigits[pos]->getEnergy();
        }
        B2DEBUG(50, phiid << " " << i << " " << m_BWD3Barrel[phiid][i] << " " << sum3BWDBarrel);

        pos = m_eclCalDigitsArray[m_BWD3Endcap[phiid][i]];
        if (pos >= 0) {
          sum3BWDEndcap += m_eclCalDigits[pos]->getEnergy();
        }
        B2DEBUG(50, phiid << " " << i << " " << m_BWD3Endcap[phiid][i] << " " << sum3BWDEndcap);
      }

      anainfo->setEnergy3FWDBarrel(sum3FWDBarrel);
      anainfo->setEnergy3FWDEndcap(sum3FWDEndcap);
      anainfo->setEnergy3BWDBarrel(sum3BWDBarrel);
      anainfo->setEnergy3BWDEndcap(sum3BWDEndcap);

      anainfo->setExtTheta(exttheta);
      anainfo->setExtPhi(extphi);
      anainfo->setExtPhiId(phiid);

    }

  }
}
