/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Belle 2 headers. */
#include <klm/modules/KLMDQM2/KLMDQM2Module.h>
#include <framework/database/DBObjPtr.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMDigitRaw.h>
#include <klm/bklm/geometry/GeometryPar.h>


/* ROOT headers. */
#include <TCanvas.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TClass.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TDirectory.h>
#include <TFile.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>


/* C++ headers. */
#include <vector>
#include <algorithm>
#include <string>




using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KLMDQM2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KLMDQM2Module::KLMDQM2Module() :
  HistoModule(),
  m_MatchedHitsBKLM{nullptr},
  m_AllExtHitsBKLM{nullptr},
  m_PlaneEfficienciesBKLM{nullptr},
  m_MatchedHitsEKLM{nullptr},
  m_AllExtHitsEKLM{nullptr},
  m_PlaneEfficienciesEKLM{nullptr},
  m_MatchedHitsBKLMSector{nullptr},
  m_AllExtHitsBKLMSector{nullptr},
  m_PlaneEfficienciesBKLMSector{nullptr},
  m_MatchedHitsEKLMSector{nullptr},
  m_AllExtHitsEKLMSector{nullptr},
  m_PlaneEfficienciesEKLMSector{nullptr},
  m_GeometryBKLM{nullptr},
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers{&(EKLMElementNumbers::Instance())},
  m_PlaneArrayIndex(&(KLMPlaneArrayIndex::Instance()))
{
  // Set module properties
  setDescription(R"DOC("Additional Module for KLMDQM plots after HLT filters

    An additional module developed to display plane efficiencies for the KLM dduring runs (i.e. for online analyses). 
    This module would be called after HLT filter in order to use mumu-tight skim to select reasonable events. 
    The output histograms would be plane efficiences = MatchedDigits/AllExtits.
    )DOC");

  // Parameter definitions
  addParam("MinHitsForFlagging", m_MinHitsForFlagging, "Minimal number of hits in a channel required to flag it as 'masked' or 'hot'",
           50);
  addParam("MinProcessedEventsForMessages", m_MinProcessedEventsForMessages,
           "Minimal number of processed events required to print error messages", 10000.);
  addParam("MuonListName", m_MuonListName, "Muon list name.",
           std::string("mu+:all"));
  addParam("AllowedDistance1D", m_AllowedDistance1D,
           "Maximal distance in the units of strip number from ExtHit to "
           "matching KLMDigit.", double(8));
  addParam("MinimalMatchingDigits", m_MinimalMatchingDigits,
           "Minimal number of matching digits.", 0);
  addParam("MinimalMatchingDigitsOuterLayers",
           m_MinimalMatchingDigitsOuterLayers,
           "Minimal number of matching digits in outer layers.", 0);
  addParam("MinimalMomentumNoOuterLayers", m_MinimalMomentumNoOuterLayers,
           "Minimal momentum in case there are no hits in outer layers.", 0.0);
  addParam("RemoveUnusedMuons", m_RemoveUnusedMuons,
           "Whether to remove unused muons.", false);
  addParam("IgnoreBackwardPropagation", m_IgnoreBackwardPropagation,
           "Whether to ignore ExtHits with backward propagation.", false);
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_HistogramDirectoryName,
           "Directory for KLM DQM histograms in ROOT file.",
           std::string("KLM2"));

}




KLMDQM2Module::~KLMDQM2Module()
{
}

void KLMDQM2Module::defineHisto()
{

  TDirectory* newDirectory{gDirectory->mkdir(m_HistogramDirectoryName.c_str())};
  TDirectory::TContext context{gDirectory, newDirectory};



  int BKLMMaxSectors = BKLMElementNumbers::getMaximalSectorGlobalNumber();
  int EKLMMaxSectors = EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder();



  /* Number of hits per channel. */
  /* KLM General Related. */
  m_MatchedHitsBKLM = new TH1F("matched_hitsBKLM",
                               "Matched Hits in BKLM Plane",
                               m_PlaneNumBKLM, 0.5, 0.5 + m_PlaneNumBKLM);
  m_MatchedHitsBKLM->GetXaxis()->SetTitle("Layer Number");
  //m_MatchedHitsBKLM->SetOption("LIVE");

  m_AllExtHitsBKLM = new TH1F("all_ext_hitsBKLM",
                              "All ExtHits in BKLM Plane",
                              m_PlaneNumBKLM, 0.5, 0.5 + m_PlaneNumBKLM);
  m_AllExtHitsBKLM->GetXaxis()->SetTitle("Layer number");
  //m_AllExtHitsBKLM->SetOption("LIVE");


  m_PlaneEfficienciesBKLM = new TH1F("plane_eff_bklm",
                                     "Plane Efficiency in BKLM Planes",
                                     m_PlaneNumBKLM, 0.5, 0.5 + m_PlaneNumBKLM);
  m_PlaneEfficienciesBKLM->GetXaxis()->SetTitle("Layer number");
  m_PlaneEfficienciesBKLM->SetOption("LIVE");
  m_PlaneEfficienciesBKLM->SetOption("HIST");
  m_PlaneEfficienciesBKLM->SetStats(11);



  m_MatchedHitsEKLM = new TH1F("matched_hitsEKLM",
                               "Matched Hits in EKLM Plane",
                               m_PlaneNumEKLM, 0.5, m_PlaneNumEKLM + 0.5);
  m_MatchedHitsEKLM->GetXaxis()->SetTitle("Plane number");
  //m_MatchedHitsEKLM->SetOption("LIVE");

  m_AllExtHitsEKLM = new TH1F("all_ext_hitsEKLM",
                              "All ExtHits in EKLM Plane",
                              m_PlaneNumEKLM, 0.5, m_PlaneNumEKLM + 0.5);
  m_AllExtHitsEKLM->GetXaxis()->SetTitle("Plane number");
  //m_AllExtHitsEKLM->SetOption("LIVE");

  m_PlaneEfficienciesEKLM = new TH1F("plane_eff_eklm",
                                     "Plane Efficiency in EKLM Plane",
                                     m_PlaneNumEKLM, 0.5, m_PlaneNumEKLM + 0.5);
  m_PlaneEfficienciesEKLM->GetXaxis()->SetTitle("Plane number");
  m_PlaneEfficienciesEKLM->SetOption("LIVE");
  m_PlaneEfficienciesEKLM->SetOption("HIST");


  /********************/
  /********************/
  /* binned by sector */
  /********************/
  /********************/

  m_MatchedHitsBKLMSector = new TH1F("matched_hitsBKLMSector",
                                     "Matched Hits in BKLM Sector",
                                     BKLMMaxSectors, 0.5, 0.5 + BKLMMaxSectors);
  m_MatchedHitsBKLMSector->GetXaxis()->SetTitle("Sector Number");
  //m_MatchedHitsBKLMSector->SetOption("LIVE");

  m_AllExtHitsBKLMSector = new TH1F("all_ext_hitsBKLMSector",
                                    "All ExtHits in BKLM Sector",
                                    BKLMMaxSectors, 0.5, 0.5 + BKLMMaxSectors);
  m_AllExtHitsBKLMSector->GetXaxis()->SetTitle("Sector number");
  //m_AllExtHitsBKLMSector->SetOption("LIVE");

  m_PlaneEfficienciesBKLMSector = new TH1F("plane_eff_bklmSector",
                                           "Plane Efficiency in BKLM Sector",
                                           BKLMMaxSectors, 0.5, 0.5 + BKLMMaxSectors);
  m_PlaneEfficienciesBKLMSector->GetXaxis()->SetTitle("Sector number");
  m_PlaneEfficienciesBKLMSector->GetYaxis()->SetRange(0., 1.1);
  m_PlaneEfficienciesBKLMSector->SetOption("LIVE");
  m_PlaneEfficienciesBKLMSector->SetOption("HIST");



  m_MatchedHitsEKLMSector = new TH1F("matched_hitsEKLMSector",
                                     "Matched Hits in EKLM Sector",
                                     EKLMMaxSectors, 0.5, EKLMMaxSectors + 0.5);
  m_MatchedHitsEKLMSector->GetXaxis()->SetTitle("Sector number");
  //m_MatchedHitsEKLMSector->SetOption("LIVE");

  m_AllExtHitsEKLMSector = new TH1F("all_ext_hitsEKLMSector",
                                    "All ExtHits in EKLM Sector",
                                    EKLMMaxSectors, 0.5, EKLMMaxSectors + 0.5);
  m_AllExtHitsEKLMSector->GetXaxis()->SetTitle("Sector number");
  //m_AllExtHitsEKLMSector->SetOption("LIVE");

  m_PlaneEfficienciesEKLMSector = new TH1F("plane_eff_eklmSector",
                                           "Plane Efficiency in EKLM Sector",
                                           EKLMMaxSectors, 0.5, EKLMMaxSectors + 0.5);
  m_PlaneEfficienciesEKLMSector->GetXaxis()->SetTitle("Sector number");
  m_PlaneEfficienciesEKLMSector->SetOption("LIVE");
  m_PlaneEfficienciesEKLMSector->SetOption("HIST");



}//end of defineHisto



void KLMDQM2Module::initialize()
{
  REG_HISTOGRAM;
  //inputs
  m_softwareTriggerResult.isRequired();
  m_MuonList.isRequired(m_MuonListName);

  m_RawFtsws.isOptional();
  m_RawKlms.isOptional();
  m_Digits.isOptional();

  m_GeometryBKLM = bklm::GeometryPar::instance();


}

void KLMDQM2Module::beginRun()
{

  //start by restarting histograms

  /* KLM General Related. */
  m_MatchedHitsBKLM->Reset();
  m_AllExtHitsBKLM->Reset();
  m_PlaneEfficienciesBKLM->Reset();
  m_MatchedHitsEKLM->Reset();
  m_AllExtHitsEKLM->Reset();
  m_PlaneEfficienciesEKLM->Reset();

  m_MatchedHitsBKLMSector->Reset();
  m_AllExtHitsBKLMSector->Reset();
  m_PlaneEfficienciesBKLMSector->Reset();
  m_MatchedHitsEKLMSector->Reset();
  m_AllExtHitsEKLMSector->Reset();
  m_PlaneEfficienciesEKLMSector->Reset();


}




void KLMDQM2Module::event()
{
  if (triggerFlag()) {
    unsigned int nMuons = m_MuonList->getListSize();
    for (unsigned int i = 0; i < nMuons; ++i) {
      const Particle* muon = m_MuonList->getParticle(i);
      collectDataTrack(muon, m_MatchedHitsBKLM,
                       m_AllExtHitsBKLM, m_MatchedHitsEKLM, m_AllExtHitsEKLM,
                       m_MatchedHitsBKLMSector, m_AllExtHitsBKLMSector,
                       m_MatchedHitsEKLMSector, m_AllExtHitsEKLMSector);

    }
    m_PlaneEfficienciesBKLM->Divide(m_MatchedHitsBKLM,
                                    m_AllExtHitsBKLM, 1, 1, "B");

    m_PlaneEfficienciesEKLM->Divide(m_MatchedHitsEKLM,
                                    m_AllExtHitsEKLM, 1, 1, "B");

    m_PlaneEfficienciesBKLMSector->Divide(m_MatchedHitsBKLMSector,
                                          m_AllExtHitsBKLMSector, 1, 1, "B");

    m_PlaneEfficienciesEKLMSector->Divide(m_MatchedHitsEKLMSector,
                                          m_AllExtHitsEKLMSector, 1, 1, "B");


  }



}

void KLMDQM2Module::endRun()
{
}

void KLMDQM2Module::terminate()
{
}



bool KLMDQM2Module::triggerFlag()
{

  bool passed = false;
  if (m_softwareTriggerResult) {
    try {
      passed = (m_softwareTriggerResult->getResult("software_trigger_cut&skim&accept_mumutight") == SoftwareTriggerCutResult::c_accept) ?
               true : false;
    } catch (const std::out_of_range&) {
      passed = false;
    }
  }
  return passed;


}


void KLMDQM2Module::findMatchingDigit(
  struct HitData* hitData)
{
  for (const KLMDigit& digit : m_Digits) {
    /*
     * TODO: multi-strip digits are ignored for now.
     * It is necessary to take them into account.
     */
    if (digit.isMultiStrip())
      continue;
    if (!(digit.getSubdetector() == hitData->subdetector &&
          digit.getSection() == hitData->section &&
          digit.getLayer() == hitData->layer &&
          digit.getSector() == hitData->sector &&
          digit.getPlane() == hitData->plane))
      continue;
    if (fabs(digit.getStrip() - hitData->strip) < m_AllowedDistance1D) {
      hitData->digit = &digit;
      return;
    }
  }
}

void KLMDQM2Module::addHit(
  std::map<KLMPlaneNumber, struct HitData>& hitMap,
  KLMPlaneNumber planeGlobal, struct HitData* hitData)
{
  std::map<KLMPlaneNumber, struct HitData>::iterator it;
  it = hitMap.find(planeGlobal);
  /*
   * There may be more than one such hit e.g. if track crosses the edge
   * of the strips or WLS fiber groove. Select only one hit per plane.
   */
  if (it == hitMap.end()) {
    hitMap.insert(std::pair<KLMPlaneNumber, struct HitData>(
                    planeGlobal, *hitData));
  }
}

bool KLMDQM2Module::collectDataTrack(
  const Particle* muon, TH1F* MatchedHitsBKLM, TH1F* AllExtHitsBKLM,
  TH1F* MatchedHitsEKLM, TH1F* AllExtHitsEKLM, TH1F* MatchedHitsBKLMSec, TH1F* AllExtHitsBKLMSec,
  TH1F* MatchedHitsEKLMSec, TH1F* AllExtHitsEKLMSec)
{
  const int nExtrapolationLayers =
    KLMElementNumbers::getMaximalExtrapolationLayer();
  const Track* track = muon->getTrack();
  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  std::map<KLMPlaneNumber, struct HitData> selectedHits;
  std::map<KLMPlaneNumber, struct HitData>::iterator it;
  KLMChannelNumber channel;
  enum KLMChannelStatus::ChannelStatus status;
  struct HitData hitData, hitDataPrevious;
  TVector3 extHitPosition;
  CLHEP::Hep3Vector extHitPositionCLHEP, localPosition;
  int layer;
  int extHitLayer[nExtrapolationLayers] = {0};
  int digitLayer[nExtrapolationLayers] = {0};
  hitDataPrevious.subdetector = -1;
  for (const ExtHit& hit : extHits) {
    /*
     * Choose hits that exit the sensitive volume.
     * It is not possible to use entry hits because of a bug in Geant4:
     * the step does not always have a correct status (fGeomBoundary),
     * and, consequently, ExtHits are not created.
     */
    if (hit.getStatus() != EXT_EXIT)
      continue;
    /*
     * Ignore ExtHits with backward propagation. This affects cosmic events
     * only. The removal of hits with backward propagation is normally
     * not needed, however, it is added because of backward error propagation
     * bug in Geant4 10.6.
     */
    if (m_IgnoreBackwardPropagation) {
      if (hit.isBackwardPropagated())
        continue;
    }
    KLMPlaneNumber planeGlobal = 0;
    hitData.hit = &hit;
    hitData.digit = nullptr;
    if (hit.getDetectorID() == Const::EDetector::EKLM) {
      int stripGlobal = hit.getCopyID();
      hitData.subdetector = KLMElementNumbers::c_EKLM;
      m_eklmElementNumbers->stripNumberToElementNumbers(
        stripGlobal, &hitData.section, &hitData.layer, &hitData.sector,
        &hitData.plane, &hitData.strip);
      channel = m_ElementNumbers->channelNumberEKLM(
                  hitData.section, hitData.sector, hitData.layer,
                  hitData.plane, hitData.strip);
      status = m_ChannelStatus->getChannelStatus(channel);
      if (status == KLMChannelStatus::c_Unknown)
        B2FATAL("Incomplete KLM channel status data.");
      if (status == KLMChannelStatus::c_Normal) {
        layer = m_ElementNumbers->getExtrapolationLayer(
                  hitData.subdetector, hitData.layer);
        extHitLayer[layer - 1]++;
        planeGlobal = m_ElementNumbers->planeNumberEKLM(
                        hitData.section, hitData.sector, hitData.layer,
                        hitData.plane);
        addHit(selectedHits, planeGlobal, &hitData);
      }
    } else if (hit.getDetectorID() == Const::EDetector::BKLM) {
      int moduleNumber = hit.getCopyID();
      hitData.subdetector = KLMElementNumbers::c_BKLM;
      BKLMElementNumbers::moduleNumberToElementNumbers(
        moduleNumber, &hitData.section, &hitData.sector, &hitData.layer);
      if (hitData.layer < BKLMElementNumbers::c_FirstRPCLayer) {
        /*
         * For scintillators, the plane and strip numbers are recorded
         * in the copy number.
         */
        BKLMElementNumbers::channelNumberToElementNumbers(
          moduleNumber, &hitData.section, &hitData.sector, &hitData.layer,
          &hitData.plane, &hitData.strip);
        channel = m_ElementNumbers->channelNumberBKLM(
                    hitData.section, hitData.sector, hitData.layer,
                    hitData.plane, hitData.strip);
        status = m_ChannelStatus->getChannelStatus(channel);
        if (status == KLMChannelStatus::c_Unknown)
          B2FATAL("Incomplete KLM channel status data.");
        if (status == KLMChannelStatus::c_Normal) {
          layer = m_ElementNumbers->getExtrapolationLayer(
                    hitData.subdetector, hitData.layer);
          extHitLayer[layer - 1]++;
          planeGlobal = m_ElementNumbers->planeNumberBKLM(
                          hitData.section, hitData.sector, hitData.layer,
                          hitData.plane);
          addHit(selectedHits, planeGlobal, &hitData);
        }
      } else {
        /* For RPCs, the sensitive volume corresponds to both readout planes. */
        extHitPosition = hit.getPosition();
        extHitPositionCLHEP.setX(extHitPosition.X());
        extHitPositionCLHEP.setY(extHitPosition.Y());
        extHitPositionCLHEP.setZ(extHitPosition.Z());
        const bklm::Module* module =
          m_GeometryBKLM->findModule(hitData.section, hitData.sector,
                                     hitData.layer);
        localPosition = module->globalToLocal(extHitPositionCLHEP);
        hitData.plane = BKLMElementNumbers::c_ZPlane;
        hitData.strip = module->getZStrip(localPosition);
        /*
         * FIXME:
         * There are 2 hits per module in RPCs, but the plane information is
         * not available in ExtHit. For now, 2 entries are created (one for
         * each plane) for the first hit, and the second one is removed.
         */
        if (hitData.subdetector == hitDataPrevious.subdetector &&
            hitData.section == hitDataPrevious.section &&
            hitData.sector == hitDataPrevious.sector &&
            hitData.layer == hitDataPrevious.layer)
          continue;
        std::memcpy(&hitDataPrevious, &hitData, sizeof(struct HitData));
        /* The returned strip may be out of the valid range. */
        if (BKLMElementNumbers::checkChannelNumber(
              hitData.section, hitData.sector, hitData.layer, hitData.plane,
              hitData.strip, false)) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hitData.section, hitData.sector, hitData.layer,
                      hitData.plane, hitData.strip);
          status = m_ChannelStatus->getChannelStatus(channel);
          if (status == KLMChannelStatus::c_Unknown)
            B2FATAL("Incomplete KLM channel status data.");
          if (status == KLMChannelStatus::c_Normal) {
            layer = m_ElementNumbers->getExtrapolationLayer(
                      hitData.subdetector, hitData.layer);
            extHitLayer[layer - 1]++;
            hitData.localPosition = localPosition.z();
            planeGlobal = m_ElementNumbers->planeNumberBKLM(
                            hitData.section, hitData.sector, hitData.layer,
                            hitData.plane);
            addHit(selectedHits, planeGlobal, &hitData);
          }
        }
        hitData.plane = BKLMElementNumbers::c_PhiPlane;
        hitData.strip = module->getPhiStrip(localPosition);
        /* The returned strip may be out of the valid range. */
        if (BKLMElementNumbers::checkChannelNumber(
              hitData.section, hitData.sector, hitData.layer, hitData.plane,
              hitData.strip, false)) {
          channel = m_ElementNumbers->channelNumberBKLM(
                      hitData.section, hitData.sector, hitData.layer,
                      hitData.plane, hitData.strip);
          status = m_ChannelStatus->getChannelStatus(channel);
          if (status == KLMChannelStatus::c_Unknown)
            B2FATAL("Incomplete KLM channel status data.");
          if (status == KLMChannelStatus::c_Normal) {
            layer = m_ElementNumbers->getExtrapolationLayer(
                      hitData.subdetector, hitData.layer);
            extHitLayer[layer - 1]++;
            hitData.localPosition = localPosition.y();
            planeGlobal = m_ElementNumbers->planeNumberBKLM(
                            hitData.section, hitData.sector, hitData.layer,
                            hitData.plane);
            addHit(selectedHits, planeGlobal, &hitData);
          } //end of channel status check
        } //end of channel number check
      }//end of detector condition
    } else
      continue;
  }
  /* Find matching digits. */
  int nDigits = 0;
  std::map<int, int>::iterator it2;
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    findMatchingDigit(&(it->second));
    if (it->second.digit != nullptr) {
      nDigits++;
      layer = m_ElementNumbers->getExtrapolationLayer(
                it->second.subdetector, it->second.layer);
      digitLayer[layer - 1]++;
    }
  }
  if (nDigits < m_MinimalMatchingDigits)
    return false;
  /* Write efficiency histograms */
  for (it = selectedHits.begin(); it != selectedHits.end(); ++it) {
    int matchingDigits = 0;
    int matchingDigitsOuterLayers = 0;
    int extHitsOuterLayers = 0;
    layer = m_ElementNumbers->getExtrapolationLayer(
              it->second.subdetector, it->second.layer) - 1;
    for (int i = 0; i < nExtrapolationLayers; ++i) {
      if (i != layer)
        matchingDigits += digitLayer[i];
      if (i > layer) {
        matchingDigitsOuterLayers += digitLayer[i];
        extHitsOuterLayers += extHitLayer[i];
      }
    }
    /* Check the number of matching digits in other layers. */
    if (matchingDigits < m_MinimalMatchingDigits)
      continue;
    /*
     * Check the number of matching digits in outer layers relatively to
     * this hit.
     */
    if (matchingDigitsOuterLayers < m_MinimalMatchingDigitsOuterLayers) {
      /**
       * It should be possible to have the required number of digits, thus,
       * the number of ExtHits needs to be sufficient. This requirement
       * does not reject the outer layers of the detector.
       */
      if (extHitsOuterLayers >= m_MinimalMatchingDigitsOuterLayers)
        continue;
      /*
       * If the number of ExtHits is insufficient, then check the momentum.
       * The muons with sufficiently large momentum have a very small
       * probability to get absorbed in the detector.
       */
      if (muon->getMomentum().Mag() < m_MinimalMomentumNoOuterLayers)
        continue;
    }
    //Filling AddExtHits and MatchedHits histograms
    if (it->second.subdetector == KLMElementNumbers::c_EKLM) {
      int planeNum = m_eklmElementNumbers->planeNumber(it->second.section, it->second.layer, it->second.sector, it->second.plane);
      int sectorNum = (it->second.section - 1) * EKLMElementNumbers::getMaximalSectorNumber() + it->second.sector;
      AllExtHitsEKLM->Fill(planeNum);
      AllExtHitsEKLMSec->Fill(sectorNum);
      if (it->second.digit) {
        MatchedHitsEKLM->Fill(planeNum);
        MatchedHitsEKLMSec->Fill(sectorNum);
      }
    }//end of if loop


    else if (it->second.subdetector == KLMElementNumbers::c_BKLM) {
      int layerGlobal = BKLMElementNumbers::layerGlobalNumber(
                          it->second.section, it->second.sector, it->second.layer);
      int sectorGlobal = it->second.section * BKLMElementNumbers::getMaximalSectorNumber() + (it->second.sector);
      AllExtHitsBKLM->Fill(layerGlobal);
      AllExtHitsBKLMSec->Fill(sectorGlobal);
      if (it->second.digit) {
        MatchedHitsBKLM->Fill(layerGlobal);
        MatchedHitsBKLMSec->Fill(sectorGlobal);
      }
    } else {
      B2FATAL("Had a hit that didn't come from BKLM or EKLM?");
    }


  } //end of selectedHits for loop
  return true;
} //end of collectTrackData







