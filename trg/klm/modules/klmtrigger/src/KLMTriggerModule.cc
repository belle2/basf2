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
  addParam("MinLayers", m_minLayers,
           "Minimum number of fired layers for a track",
           int(10));
  addParam("TrgKLMTracks", m_klmtrackCollectionName, "Name of the StoreArray holding the track list", string("TRGKLMTracks"));
  addParam("TrgKLMHits", m_klmhitCollectionName, "Name of the StoreArray holding the hit list", string("TRGKLMHits"));
}

void KLMTriggerModule::initialize()
{
  StoreArray<BKLMDigit> bklmDigits;

  StoreArray<KLMTriggerHit> klmTriggerHits(m_klmhitCollectionName);
  klmTriggerHits.registerInDataStore();
  klmTriggerHits.registerRelationTo(bklmDigits);

  StoreArray<KLMTriggerTrack> klmTriggerTracks(m_klmtrackCollectionName);
  klmTriggerTracks.registerInDataStore();
  klmTriggerTracks.registerRelationTo(klmTriggerHits);
}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2DEBUG(100, "KLMTrigger: Experiment " << evtMetaData->getExperiment() << ", run " << evtMetaData->getRun());
  m_nEvents = 0;
  m_nTracks = 0;
}

void KLMTriggerModule::event()
{
  ++m_nEvents;

  fillHits();
  fillTracks();
  calcChisq();
}

void KLMTriggerModule::endRun()
{
//  B2INFO("KLMTrigger: Processed " << m_nEvents << " events, found " << m_nTracks << " tracks");
}

void KLMTriggerModule::fillHits()
{
  StoreArray<BKLMDigit> bklmDigits;
  if (!bklmDigits.isValid())
    return;

  StoreArray<KLMTriggerHit> klmTriggerHits(m_klmhitCollectionName);

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

        int xInt = 0, yInt = 0, zInt = 0;
        double x = 0.0, y = 0.0, z = 0.0;
        geometryConverter(fwd, sector, layer, phiStrip, zStrip, xInt, yInt, zInt);
        // covert 1/8 cm to cm
        x = (double)(xInt >> 3);
        y = (double)(yInt >> 3);
        z = (double)(zInt >> 3);

        KLMTriggerHit* hit = klmTriggerHits.appendNew(fwd, sector, layer, phiStrip, zStrip);
        hit->setX(x);
        hit->setY(y);
        hit->setZ(z);
        hit->setXInt(xInt);
        hit->setYInt(yInt);
        hit->setZInt(zInt);
        hit->addRelationTo(bklmDigits[i]);
        hit->addRelationTo(bklmDigits[j]);
      }
    }
  }
//    B2INFO("KLM TSIM: " << klmTriggerHits.getEntries() << " hits made");
}




void KLMTriggerModule::fillTracks()
{
  StoreArray<KLMTriggerHit> klmTriggerHits(m_klmhitCollectionName);
  if (!klmTriggerHits.isValid())
    return;

  StoreArray<KLMTriggerTrack> klmTriggerTracks(m_klmtrackCollectionName);

  std::unordered_map<int, KLMTriggerTrack*> trackMap;

  int nEntries = klmTriggerHits.getEntries();
//  B2INFO("KLM TSIM: " << nEntries << " hits found");
  for (int i = 0; i < nEntries; ++i) {
    const KLMTriggerHit* hit = klmTriggerHits[i];

    bool fwd = hit->isForward();
    int sector = hit->getSector();

    int fwdInt = fwd ? 1 : 0;
    int sectorID = fwdInt * c_TotalSectors + sector;

    if (trackMap.find(sectorID) == trackMap.end())
      trackMap[sectorID] = klmTriggerTracks.appendNew(fwd, sector);

    trackMap[sectorID]->addRelationTo(klmTriggerHits[i]);
  }
}


void KLMTriggerModule::calcChisq()
{
  StoreArray<KLMTriggerTrack> klmTriggerTracks(m_klmtrackCollectionName);
  if (!klmTriggerTracks.isValid())
    return;

  int nEntries = klmTriggerTracks.getEntries();
  for (int i = 0; i < nEntries; ++i) {
    KLMTriggerTrack* track = klmTriggerTracks[i];
    RelationVector<KLMTriggerHit> hits = track->getRelationsWith<KLMTriggerHit>(m_klmhitCollectionName);

    int nHits = hits.size();
//    B2INFO("KLM TSIM: " << nHits << " hits attached to track");
    int sumX = 0, sumY = 0, sumZ = 0, sumXX = 0, sumXY = 0, sumXZ = 0, sumYY = 0, sumZZ = 0;
    int nLayers = 0;
    int firstLayer = c_TotalLayers;
    int lastLayer = 0;
//    bool firedLayers[c_TotalLayers] = { };
    std::unordered_map<int, bool> layersMap;

    for (int j = 0; j < nHits; ++j) {
      const KLMTriggerHit* hit = hits[j];

      const int xInt = hit->getXInt();
      const int yInt = hit->getYInt();
      const int zInt = hit->getZInt();
      const int layer = hit->getLayer();

      // calculate sums
      sumX += xInt;
      sumY += yInt;
      sumZ += zInt;
      sumXX += xInt * xInt;
      sumXY += xInt * yInt;
      sumXZ += xInt * zInt;
      sumYY += yInt * yInt;
      sumZZ += zInt * zInt;

      // find minimum and maximum layers
      if (layer < firstLayer)
        firstLayer = layer;
      if (layer > lastLayer)
        lastLayer = layer;
//      firedLayers[layer] = true;
      layersMap[layer] = true;
    }

    // mimic firmware integer widths
//    if(abs(sumX)>262144) // 5 bits upto 18 bits
//      sumX = 0;
//    if(abs(sumY)>262144)
//      sumY = 0;
//    if(abs(sumZ)>262144)
//      sumZ = 0;

//    if(abs(sumXX)>5.37e8) // 10 bits upto 29 bits
//      sumXX = 0;
//    if(abs(sumXY)>5.37e8)
//      sumXY = 0;
//    if(abs(sumXZ)>5.37e8)
//      sumXZ = 0;
//    if(abs(sumYY)>5.37e8)
//      sumYY = 0;
//    if(abs(sumZZ)>5.37e8)
//      sumZZ = 0;

    sumX = sumX >> 5;
    sumY = sumY >> 5;
    sumZ = sumZ >> 5;

    sumXX = sumXX >> 10;
    sumXY = sumXY >> 10;
    sumXZ = sumXZ >> 10;
    sumYY = sumYY >> 10;
    sumZZ = sumZZ >> 10;

    // calculate chisq
    int denom = sumXX * nHits - sumX * sumX;
    double denomInversed = 0.0;
    double slopeXY = 0.0;
    double interceptXY = 0.0;
    double chisqXY = 0.0;
    double ipXY = 0.0;
    double slopeXZ = 0.0;
    double interceptXZ = 0.0;
    double chisqXZ = 0.0;
    double ipXZ = 0.0;

    denom = denom >> 4;
//    if(abs(denomInt)<1.074e6) { // 14 bits upto 30 bits
    if (denom != 0) {
      denomInversed = 1.0 / denom;

      slopeXY = denomInversed * (sumXY * nHits - sumX * sumY);
      interceptXY = denomInversed * (sumXX * sumY - sumX * sumXY);

      slopeXZ = denomInversed * (sumXZ * nHits - sumX * sumZ);
      interceptXZ = denomInversed * (sumXX * sumZ - sumX * sumXZ);

      // denominator shift compensation
      slopeXY /= 16;
      interceptXY /= 16;

      slopeXZ /= 16;
      interceptXZ /= 16;

      // to avoid additional division, and square root calculation,
      // in the firmware we calculate ip^2 and
      // approximate 1/(1+slope*slope) as (1-slope*slope)
      // mimic this behaviour here
      ipXY = interceptXY * interceptXY * (1.0 - slopeXY * slopeXY);
      chisqXY = slopeXY * slopeXY * sumXX + interceptXY * interceptXY * nHits + sumYY + 2.0 * slopeXY * interceptXY * sumX - 2.0 *
                slopeXY * sumXY - 2.0 * interceptXY * sumY;

      ipXZ = interceptXZ * interceptXZ * (1.0 - slopeXZ * slopeXZ);
      chisqXZ = slopeXZ * slopeXZ * sumXX + interceptXZ * interceptXZ * nHits + sumZZ + 2.0 * slopeXZ * interceptXZ * sumX - 2.0 *
                slopeXZ * sumXZ - 2.0 * interceptXZ * sumZ;

      // sumX, sumXX shift compensation (first factor)
      // and convert 1/8 cm -> cm (second factor)
      interceptXY *= (32 / 8);
      chisqXY *= (1024 / 64);
      ipXY *= (1024 / 64);

      interceptXZ *= (32 / 8);
      chisqXZ *= (1024 / 64);
      ipXZ *= (1024 / 64);
    }

//    // calculate number of fired layers
//    for(int i = 0; i < c_TotalLayers; ++i)
//      if(firedLayers[i] == true)
//        ++nLayers;
    nLayers = layersMap.size();

    track->setSlopeXY(slopeXY);
    track->setInterceptXY(interceptXY);
    track->setChisqXY(chisqXY);
    track->setImpactParameterXY(ipXY);

    track->setSlopeXZ(slopeXZ);
    track->setInterceptXZ(interceptXZ);
    track->setChisqXZ(chisqXZ);
    track->setImpactParameterXZ(ipXZ);

    track->setFirstLayer(firstLayer);
    track->setLastLayer(lastLayer);
    track->setNLayers(nLayers);

    // to avoid additional division, in the firmware we do not calculate chisq/n
    // we compare chisq to threshold*n
    // mimic this behaviour here
    if (chisqXY < m_maxChisq * nHits && ipXY < m_maxIP &&
        chisqXZ < m_maxChisq * nHits && ipXZ < m_maxIP &&
        nLayers > m_minLayers)
      track->setTrigger(true);
  }
  m_nTracks += nEntries;
}


//void KLMTriggerModule::geometryConverter(bool fwd, int sector, int layer, int phiStrip, int zStrip, double& x, double& y, double& z)
//{
//  // lengths are in centimeters
//  const double phi_width[c_TotalLayers] = {4.0, 4.0, 4.90, 5.11, 5.32, 5.53, 4.30, 4.46, 4.62, 4.77, 4.93, 5.09, 5.25, 5.40, 5.56};
//  const int phi_nstrips[c_TotalLayers] = {37, 42, 36, 36, 36, 36, 48, 48, 48, 48, 48, 48, 48, 48, 48};
//  const double cosine[c_TotalSectors] = {1.0, 0.707107, 0.0, -0.707107, -1.0, -0.707107, 0.0, 0.707107};
//  const double sine[c_TotalSectors] = {0.0, 0.707107, 1.0, 0.707107, 0.0, -0.707107, -1.0, -0.707107};
//  const double z_width = (layer < 2 ? 4.0 : 4.52);
//  const double z_offset = 47.0;
//  const double r0 = 201.9;
//  const double gap_height = 4.4;
//  const double first_gap_height = 4.1;
//  const double iron_height = 4.7;
//  const double frame_thickness = 0.3;
//
//  const double tan_pi_to_8 = 0.414214;
//  const double gap_iron_width = (layer == 0 ? 7.94 : 3.0);
//  const int y_offset_sign = (layer == 0 ? -1 : 1);
//  const double spacer_width = 0.6;
//  const double scint_height = 1.0;
//  const double ps_inner_height = 0.635;
//  const double ps_outer_height = 0.47625;
//  const double glass_height = 0.238125;
//  const double gas_height = 0.2;
//
//  if(layer == 0)
//    x = r0 + 0.5 * first_gap_height;
//  else
//    x = r0 + first_gap_height + iron_height + (layer - 1) * (gap_height + iron_height) + 0.5 * gap_height;
//  y = (0.5 + phiStrip - 0.5 * phi_nstrips[layer]) * phi_width[layer];
//  z = (0.5 + zStrip) * z_width + frame_thickness;
//
//  // some small corrections
//  if(layer < 2) { // scints
//    if(layer == 0)
//      y += ((x + 0.5 * first_gap_height) * tan_pi_to_8 - gap_iron_width - frame_thickness - 0.5 * phi_nstrips[layer] * phi_width[layer]) *
//        y_offset_sign;
//    else
//      y += ((x + 0.5 * gap_height) * tan_pi_to_8 - gap_iron_width - frame_thickness - 0.5 * phi_nstrips[layer] * phi_width[layer]) *
//        y_offset_sign;
//    // x correction should go later, since in y correction calculation previous x is used (ActiveMiddleRadius vs ModuleMiddleRadius)
//    x += -(0.5 * scint_height - 0.5 * (ps_inner_height - ps_outer_height));
//  } else { // RPCs
//    x += -(glass_height + 0.5 * gas_height);
//    z += spacer_width;
//  }
//
//  // backward part flip
//  if(!fwd) {
//    y = -y;
//    z = -z;
//  }
//
//  // detector asymmetry shift
//  z += z_offset;
//
//  // rotate the sector to its position
//  double x_tmp = x, y_tmp = y;
//  x_tmp = x_tmp*cosine[sector] - y_tmp*sine[sector];
//  y_tmp = x_tmp*sine[sector] + y_tmp*cosine[sector];
//}


void KLMTriggerModule::geometryConverter(bool fwd, int sector, int layer, int phiStrip, int zStrip, int& x, int& y, int& z)
{
  const int c_LayerXCoord[c_TotalLayers] = {1628, 1700, 1773, 1846, 1919, 1992, 2064, 2137, 2210, 2283, 2356, 2428, 2501, 2574, 2647};
  const int c_LayerY0[c_TotalLayers] = { -2403, -2566, -2744, -2862, -2979, -3097, -3234, -3354, -3474, -3587, -3708, -3828, -3948, -4061, -4181};
  const int c_PhiWidth[c_TotalLayers] = {128, 128, 157, 164, 170, 177, 138, 143, 148, 153, 158, 163, 168, 173, 178};

  const int c_Z01     = 18;
  const int c_Z02     = 25;
  const int c_ZWidth1 = 32;
  const int c_ZWidth2 = 36;
  const int c_ZOffset = 376;

  bool flipped = false;
  int dy = 0;
  int dz = 0;
  int z0 = 0;
  int zWidth = 0;

  // define if module is flipped
  if (layer == 0 && fwd == 0 && sector == 0) // layer 0, backward, sector 0
    flipped = true;
  else if (layer == 0 && fwd == 0 && sector == 1)
    flipped = true;
  else if (layer == 0 && fwd == 0 && sector == 2)
    flipped = false;
  else if (layer == 0 && fwd == 0 && sector == 3)
    flipped = false;
  else if (layer == 0 && fwd == 0 && sector == 4)
    flipped = false;
  else if (layer == 0 && fwd == 0 && sector == 5)
    flipped = false;
  else if (layer == 0 && fwd == 0 && sector == 6)
    flipped = true;
  else if (layer == 0 && fwd == 0 && sector == 7)
    flipped = true;
  else if (layer == 0 && fwd == 1 && sector == 0) // layer 0, forward, sector 0
    flipped = true;
  else if (layer == 0 && fwd == 1 && sector == 1)
    flipped = true;
  else if (layer == 0 && fwd == 1 && sector == 2)
    flipped = true;
  else if (layer == 0 && fwd == 1 && sector == 3)
    flipped = false;
  else if (layer == 0 && fwd == 1 && sector == 4)
    flipped = false;
  else if (layer == 0 && fwd == 1 && sector == 5)
    flipped = false;
  else if (layer == 0 && fwd == 1 && sector == 6)
    flipped = false;
  else if (layer == 0 && fwd == 1 && sector == 7)
    flipped = true;
  else if (layer == 1 && fwd == 0 && sector == 0) // layer 1, backward, sector 0
    flipped = false;
  else if (layer == 1 && fwd == 0 && sector == 1)
    flipped = false;
  else if (layer == 1 && fwd == 0 && sector == 2)
    flipped = false;
  else if (layer == 1 && fwd == 0 && sector == 3)
    flipped = true;
  else if (layer == 1 && fwd == 0 && sector == 4)
    flipped = true;
  else if (layer == 1 && fwd == 0 && sector == 5)
    flipped = true;
  else if (layer == 1 && fwd == 0 && sector == 6)
    flipped = true;
  else if (layer == 1 && fwd == 0 && sector == 7)
    flipped = false;
  else if (layer == 1 && fwd == 1 && sector == 0) // layer 1, forward, sector 0
    flipped = false;
  else if (layer == 1 && fwd == 1 && sector == 1)
    flipped = false;
  else if (layer == 1 && fwd == 1 && sector == 2)
    flipped = true;
  else if (layer == 1 && fwd == 1 && sector == 3)
    flipped = true;
  else if (layer == 1 && fwd == 1 && sector == 4)
    flipped = true;
  else if (layer == 1 && fwd == 1 && sector == 5)
    flipped = true;
  else if (layer == 1 && fwd == 1 && sector == 6)
    flipped = false;
  else if (layer == 1 && fwd == 1 && sector == 7)
    flipped = false;
  else if (layer > 2 && fwd == 0) // backward RPCs
    flipped = true;
  else
    flipped = false;

  // convert channels to x, y, z
  if (layer < 2) {
    z0     = c_Z01;
    zWidth = c_ZWidth1;
  } else {
    z0     = c_Z02;
    zWidth = c_ZWidth2;
  }

  // corrections to y && z
  if (layer == 2 || layer == 4 || layer == 8 ||
      layer == 10 || layer == 11 || layer == 13) {
    if (phiStrip > 45)
      dy = -9;
    else if (phiStrip > 40)
      dy = -8;
    else if (phiStrip > 35)
      dy = -7;
    else if (phiStrip > 29)
      dy = -6;
    else if (phiStrip > 24)
      dy = -5;
    else if (phiStrip > 18)
      dy = -4;
    else if (phiStrip > 13)
      dy = -3;
    else if (phiStrip > 8)
      dy = -2;
    else if (phiStrip > 2)
      dy = -1;
    else
      dy = 0;
  } else if (layer == 3 || layer == 6 or
             layer == 7 || layer == 9) {
    if (phiStrip > 47)
      dy = -18;
    else if (phiStrip > 45)
      dy = -17;
    else if (phiStrip > 42)
      dy = -16;
    else if (phiStrip > 39)
      dy = -15;
    else if (phiStrip > 36)
      dy = -14;
    else if (phiStrip > 34)
      dy = -13;
    else if (phiStrip > 31)
      dy = -12;
    else if (phiStrip > 28)
      dy = -11;
    else if (phiStrip > 25)
      dy = -10;
    else if (phiStrip > 23)
      dy = -9;
    else if (phiStrip > 20)
      dy = -8;
    else if (phiStrip > 17)
      dy = -7;
    else if (phiStrip > 15)
      dy = -6;
    else if (phiStrip > 12)
      dy = -5;
    else if (phiStrip > 9)
      dy = -4;
    else if (phiStrip > 6)
      dy = -3;
    else if (phiStrip > 4)
      dy = -2;
    else if (phiStrip > 1)
      dy = -1;
    else
      dy = 0;
  } else
    dy = 0;

  if (layer == 4)
    dy = -dy;

  if (layer >= 2) {
    if (zStrip > 45)
      dz = 8;
    else if (zStrip > 38)
      dz = 7;
    else if (zStrip > 32)
      dz = 6;
    else if (zStrip > 26)
      dz = 5;
    else if (zStrip > 20)
      dz = 4;
    else if (zStrip > 13)
      dz = 3;
    else if (zStrip > 7)
      dz = 2;
    else if (zStrip > 1)
      dz = 1;
    else
      dz = 0;
  } else
    dz = 0;

  y  = c_LayerY0[layer] + dy + phiStrip * c_PhiWidth[layer];
  if (flipped)
    y = -y;

  z  = z0 + dz + zStrip * zWidth;
  if (fwd == 1)
    z = z + c_ZOffset;
  else
    z = -z + c_ZOffset;

  x      = c_LayerXCoord[layer];
  y      = y >> 2; // y values are defined in 1/32 cm here, round up to 1/8 cm
//z      = z;
}
