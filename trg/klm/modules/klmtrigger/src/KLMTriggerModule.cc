/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitri Liventsev                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/klm/modules/klmtrigger/KLMTriggerModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// event data
#include <framework/dataobjects/EventMetaData.h>

// digits
#include <bklm/dataobjects/BKLMDigit.h>

#include <trg/klm/dataobjects/KLMTriggerHit.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>

#include <unordered_map>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KLMTrigger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KLMTriggerModule::KLMTriggerModule() : Module()
{
  setDescription("KLM trigger simulation");
  addParam("MaxChisq", m_maxChisq,
           "Maximum chi squared for a track",
           double(7.0));
  addParam("MaxIP", m_maxIP,
           "Maximum impact parameter for a track",
           double(7.0));
  addParam("MinFiredLayers", m_minFiredLayers,
           "Minimum number of fired layers for a track",
           int(10));
}

void KLMTriggerModule::initialize()
{
  StoreArray<BKLMDigit> bklmDigits;

  StoreArray<KLMTriggerHit> klmTriggerHits;
  klmTriggerHits.registerInDataStore();
  klmTriggerHits.registerRelationTo(bklmDigits);

  StoreArray<KLMTriggerTrack> klmTriggerTracks;
  klmTriggerTracks.registerInDataStore();
  klmTriggerTracks.registerRelationTo(klmTriggerHits);
}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("KLMTrigger: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun());
}

void KLMTriggerModule::event()
{
  fillHits();
  fillTracks();
  calcChisq();
}


void KLMTriggerModule::fillHits()
{
  StoreArray<BKLMDigit> bklmDigits;
  if (!bklmDigits.isValid())
    return;

  StoreArray<KLMTriggerHit> klmTriggerHits;

  int nEntries = bklmDigits.getEntries();
  for (int i = 0; i < nEntries; ++i) {
    for (int j = i + 1; j < nEntries; ++j) {
      const BKLMDigit* bklmDigit_i = bklmDigits[i];
      const BKLMDigit* bklmDigit_j = bklmDigits[j];

      if (bklmDigit_i->isForward() == bklmDigit_j->isForward() &&
          bklmDigit_i->getSector() == bklmDigit_j->getSector() &&
          bklmDigit_i->getLayer() == bklmDigit_j->getLayer() &&
          bklmDigit_i->isPhiReadout() != bklmDigit_j->isPhiReadout()) {  // phi-theta match
        bool fwd = bklmDigit_i->isForward();
        int sector = bklmDigit_i->getSector() - 1; // zero-based
        int layer = bklmDigit_i->getLayer() - 1; // zero-based
        int phiStrip = 0;
        int zStrip = 0;
        if (bklmDigit_i->isPhiReadout()) {
          phiStrip = bklmDigit_i->getStrip() - 1; //zero-based
          zStrip = bklmDigit_j->getStrip() - 1; // zero-based
        } else {
          zStrip = bklmDigit_i->getStrip() - 1; // zero-based
          phiStrip = bklmDigit_j->getStrip() - 1; // zero-based
        }

        double x = 0.0, y = 0.0, z = 0.0;
        geometryConverter(fwd, sector, layer, phiStrip, zStrip, x, y, z);

        KLMTriggerHit* hit = klmTriggerHits.appendNew(fwd, sector, layer, phiStrip, zStrip);
        hit->setX(x);
        hit->setY(y);
        hit->setZ(z);
        hit->addRelationTo(bklmDigits[i]);
        hit->addRelationTo(bklmDigits[j]);
      }
    }
  }
}




void KLMTriggerModule::fillTracks()
{
  StoreArray<KLMTriggerHit> klmTriggerHits;
  if (!klmTriggerHits.isValid())
    return;

  StoreArray<KLMTriggerTrack> klmTriggerTracks;

  std::unordered_map<int, KLMTriggerTrack*> trackMap;

  int nEntries = klmTriggerHits.getEntries();
  for (int i = 0; i < nEntries; ++i) {
    const KLMTriggerHit* hit = klmTriggerHits[i];

    bool fwd = hit->isForward();
    int sector = hit->getSector();

    int fwdInt = fwd ? 1 : 0;
    int sectorID = fwdInt * 8 + sector;

    if (trackMap.find(sectorID) == trackMap.end())
      trackMap[sectorID] = klmTriggerTracks.appendNew(fwd, sector);

    trackMap[sectorID]->addRelationTo(klmTriggerHits[i]);
  }
}


void KLMTriggerModule::calcChisq()
{
  StoreArray<KLMTriggerTrack> klmTriggerTracks;
  if (!klmTriggerTracks.isValid())
    return;

  int nEntries = klmTriggerTracks.getEntries();
  for (int i = 0; i < nEntries; ++i) {
    KLMTriggerTrack* track = klmTriggerTracks[i];
    RelationVector<KLMTriggerHit> hits = track->getRelationsWith<KLMTriggerHit>();

    int nHits = hits.size();
    double sumX = 0.0, sumY = 0.0, sumZ = 0.0, sumXX = 0.0, sumXY = 0.0, sumXZ = 0.0, sumYY = 0.0, sumZZ = 0.0;
    int nFiredLayers = 0;
    bool firedLayers[15] = { };

    for (int i = 0; i < nHits; ++i) {
      const KLMTriggerHit* hit = hits[i];

      double x = hit->getX();
      double y = hit->getY();
      double z = hit->getZ();

      // calculate sums
      sumX += x;
      sumY += y;
      sumZ += z;
      sumXX += x * x;
      sumXY += x * y;
      sumXZ += x * z;
      sumYY += y * y;
      sumZZ += z * z;

      firedLayers[hit->getLayer()] = true;
    }

    // calculate chisq using sums
    double denom = sumXX * nHits - sumX * sumX;

    double slopeXY = (sumXY * nHits - sumX * sumY) / denom;
    double interceptXY = (sumXX * sumY - sumX * sumXY) / denom;
    double ipXY = interceptXY / sqrt(slopeXY * slopeXY + 1.0);
    double chisqXY = slopeXY * slopeXY * sumXX + interceptXY * interceptXY * nHits + sumYY + 2.0 * slopeXY * interceptXY * sumX - 2.0 *
                     slopeXY * sumXY - 2.0 * interceptXY * sumY;

    double slopeXZ = (sumXZ * nHits - sumX * sumZ) / denom;
    double interceptXZ = (sumXX * sumZ - sumX * sumXZ) / denom;
    double ipXZ = interceptXZ / sqrt(slopeXZ * slopeXZ + 1.0);
    double chisqXZ = slopeXZ * slopeXZ * sumXX + interceptXZ * interceptXZ * nHits + sumZZ + 2.0 * slopeXZ * interceptXZ * sumX - 2.0 *
                     slopeXZ * sumXZ - 2.0 * interceptXZ * sumZ;

    // calculate number of fired layers
    for (int i = 0; i < 15; ++i)
      if (firedLayers[i] == true)
        ++nFiredLayers;

    track->setSlopeXY(slopeXY);
    track->setInterceptXY(interceptXY);
    track->setChisqXY(chisqXY);
    track->setImpactParameterXY(ipXY);

    track->setSlopeXZ(slopeXZ);
    track->setInterceptXZ(interceptXZ);
    track->setChisqXZ(chisqXZ);
    track->setImpactParameterXZ(ipXZ);

    track->setNFiredLayers(nFiredLayers);

    if (chisqXY < m_maxChisq && ipXY < m_maxIP &&
        chisqXZ < m_maxChisq && ipXZ < m_maxIP &&
        nFiredLayers > m_minFiredLayers)
      track->setTrigger(true);
  }
}


void KLMTriggerModule::geometryConverter(bool fwd, int sector, int layer, int phiStrip, int zStrip, double& x, double& y, double& z)
{
  // lengths are in centimeters
  const double phi_width[15] = {4.0, 4.0, 4.90, 5.11, 5.32, 5.53, 4.30, 4.46, 4.62, 4.77, 4.93, 5.09, 5.25, 5.40, 5.56};
  const int phi_nstrips[15] = {37, 42, 36, 36, 36, 36, 48, 48, 48, 48, 48, 48, 48, 48, 48};
  const double cosine[8] = {1.0, 0.707107, 0.0, -0.707107, -1.0, -0.707107, 0.0, 0.707107};
  const double sine[8] = {0.0, 0.707107, 1.0, 0.707107, 0.0, -0.707107, -1.0, -0.707107};
  const double z_width = (layer < 2 ? 4.0 : 4.52);
  const double z_offset = 47.0;
  const double r0 = 201.9;
  const double gap_height = 4.4;
  const double first_gap_height = 4.1;
  const double iron_height = 4.7;
  const double frame_thickness = 0.3;

  const double tan_pi_to_8 = 0.414214;
  const double gap_iron_width = (layer == 0 ? 7.94 : 3.0);
  const int y_offset_sign = (layer == 0 ? -1 : 1);
  const double spacer_width = 0.6;
  const double scint_height = 1.0;
  const double ps_inner_height = 0.635;
  const double ps_outer_height = 0.47625;
  const double glass_height = 0.238125;
  const double gas_height = 0.2;

  if (layer == 0)
    x = r0 + 0.5 * first_gap_height;
  else
    x = r0 + first_gap_height + iron_height + (layer - 1) * (gap_height + iron_height) + 0.5 * gap_height;
  y = (0.5 + phiStrip - 0.5 * phi_nstrips[layer]) * phi_width[layer];
  z = (0.5 + zStrip) * z_width + frame_thickness;

  // some small corrections
  if (layer < 2) { // scints
    if (layer == 0)
      y += ((x + 0.5 * first_gap_height) * tan_pi_to_8 - gap_iron_width - frame_thickness - 0.5 * phi_nstrips[layer] * phi_width[layer]) *
           y_offset_sign;
    else
      y += ((x + 0.5 * gap_height) * tan_pi_to_8 - gap_iron_width - frame_thickness - 0.5 * phi_nstrips[layer] * phi_width[layer]) *
           y_offset_sign;
    // x correction should go later, since in y correction calculation previous x is used (ActiveMiddleRadius vs ModuleMiddleRadius)
    x += -(0.5 * scint_height - 0.5 * (ps_inner_height - ps_outer_height));
  } else { // RPCs
    x += -(glass_height + 0.5 * gas_height);
    z += spacer_width;
  }

  // backward part flip
  if (!fwd) {
    y = -y;
    z = -z;
  }

  // detector asymmetry shift
  z += z_offset;

  // rotate the sector to its position
  double x_tmp = x, y_tmp = y;
  x_tmp = x_tmp * cosine[sector] - y_tmp * sine[sector];
  y_tmp = x_tmp * sine[sector] + y_tmp * cosine[sector];
}

