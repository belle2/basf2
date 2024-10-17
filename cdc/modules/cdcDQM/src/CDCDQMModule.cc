/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <cdc/modules/cdcDQM/CDCDQMModule.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>

#include <TF1.h>
#include <TMath.h>
#include <TDirectory.h>

#include <fstream>
#include <math.h>
#include <set>

#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(CDCDQM);

CDCDQMModule::CDCDQMModule() : HistoModule()
{
  // set module description (e.g. insert text)
  setDescription("Make summary of data quality.");
  addParam("MinHits", m_minHits, "Include only events with more than MinHits hits in CDC", 0);
  addParam("AdjustWireShift", m_adjustWireShift, "If true, gets the correct phi view of the boards", m_adjustWireShift);
  setPropertyFlags(c_ParallelProcessingCertified);
}

CDCDQMModule::~CDCDQMModule()
{
}

void CDCDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("CDC");
  oldDir->cd("CDC");
  m_hNEvents = new TH1F("hNEvents", "hNEvents", 10, 0, 10);
  m_hNEvents->GetXaxis()->SetBinLabel(1, "number of events");
  m_hBit = new TH2F("hBit", "m_hBit", 7, 0, 7.0, 48, 0, 48.0);
  m_hBit->SetTitle("CDC:Removed Data Bit;CDCRawIndex;Channell Index");
  m_hOcc = new TH1F("hOcc", "hOccupancy", 150, 0, 1.5);
  m_hADC = new TH2F("hADC", "hADC", 300, 0, 300, 200, 0, 1000);
  m_hADC->SetTitle("ADC vs CDC-Boards;Board index;ADC");
  m_hTDC = new TH2F("hTDC", "hTDC", 300, 0, 300, 1000, 4200, 5200);
  m_hTDC->SetTitle("TDC vs CDC-Boards;Board index;TDC");
  m_hHit = new TH2F("hHit", "hHit", 56, 0, 56, 400, 0, 400);
  m_hHit->SetTitle("CDC-hits;layer index;nhits");
  m_hPhi = new TH1F("hPhi", "", 360, -180.0, 180.0);
  m_hPhi->SetTitle("CDC-track-#phi;cdctrack #phi (IP tracks + all events);entries");
  m_hPhiIndex = new TH2F("hPhiIndex", "", 360, -180.0, 180.0, 8, 0, 8.0);
  m_hPhiIndex->SetTitle("CDC-track-#phi;cdctrack #phi vs skims;selection-index");
  m_hPhiEff = new TH2F("hPhiEff", "", 360, -180.0, 180.0, 100, 0, 100.0);
  m_hPhiEff->SetTitle("CDC-track-#phi;cdctrack #phi vs cdchits;ncdchits");
  m_hPhiHit = new TH2F("h2HitPhi", "h2HitPhi", 90, -180.0, 180.0, 56, 0, 56);
  m_hPhiHit->SetTitle("CDC-hits-map (#phi vs layer);Track-#phi;Layer index");
  m_hPhiNCDC = new TH2F("hPhiNCDC", "hPhiNCDC", 90, -180.0, 180.0, 100, -0.5, 99.5);
  m_hPhiNCDC->SetTitle("nCDCHits vs #phi;Track-#phi;nCDCHits;Track / bin");
  m_hTrackingWireEff = new TH2F("hTrackingWireEff", "title", 400, 0.5, 400 + 0.5, 56 * 2, -0.5, 56 * 2 - 0.5);
  m_hTrackingWireEff->SetTitle("Attached vs Expected hits for all layers;wire;layer;Track / bin");
  oldDir->cd();
}

void CDCDQMModule::initialize()
{
  REG_HISTOGRAM
  m_cdcHits.isOptional();
  m_cdcRawHits.isOptional();
  m_trgSummary.isOptional();

  if (!m_Tracks.isOptional()) {
    B2WARNING("Missing Tracks array");
    return;
  }
}

void CDCDQMModule::beginRun()
{
  if (!m_RecoTracks.isOptional()) {
    B2DEBUG(22, "Missing recoTracks array in beginRun() ");
    return;
  }

  m_hNEvents->Reset();
  m_hBit->Reset();
  m_hOcc->Reset();
  m_hADC->Reset();
  m_hTDC->Reset();
  m_hHit->Reset();
  m_hPhi->Reset();
  m_hPhiIndex->Reset();
  m_hPhiEff->Reset();
  m_hPhiHit->Reset();
  m_hPhiNCDC->Reset();
  m_hTrackingWireEff->Reset();
}

void CDCDQMModule::event()
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  const int nWires = 14336;
  setReturnValue(1);
  if (!m_trgSummary.isValid() || (m_trgSummary->getTimType() == Belle2::TRGSummary::TTYP_RAND)) {
    setReturnValue(0);
    return;
  }

  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha/mumu/hadron events skim");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if ((fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_mumu_tight_or_highm") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())) {
    B2WARNING("CDCDQMModule: Can't find required bhabha or mumu or hadron trigger identifier");
    return;
  }

  const bool IsBhabha = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                         SoftwareTriggerCutResult::c_accept);
  const bool IsHadron = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                         SoftwareTriggerCutResult::c_accept);
  const bool IsMumu = (m_TrgResult->getResult("software_trigger_cut&skim&accept_mumu_tight_or_highm") ==
                       SoftwareTriggerCutResult::c_accept);

  if (m_cdcHits.getEntries() < m_minHits) {
    setReturnValue(0);
    return;
  }

  m_nEvents += 1;
  m_hOcc->Fill(static_cast<float>(m_cdcHits.getEntries()) / nWires);

  for (const auto& hit : m_cdcHits) {
    int lay = hit.getICLayer();
    int wire = hit.getIWire();
    m_hHit->Fill(lay, wire);
  }

  // to record removed databits
  const int nEntries = m_rawCDCs.getEntries();
  B2DEBUG(99, "nEntries of RawCDCs : " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    const int nEntriesRawCDC = m_rawCDCs[i]->GetNumEntries();
    B2DEBUG(99, LogVar("nEntries of rawCDC[i]", nEntriesRawCDC));
    for (int j = 0; j < nEntriesRawCDC; ++j) {
      int MaxNumOfCh = m_rawCDCs[i]->GetMaxNumOfCh(j);
      if (MaxNumOfCh != 4 && MaxNumOfCh != 48) {
        B2ERROR("CDCDQM: Invalid value of GetMaxNumOfCh");
      } else if (MaxNumOfCh == 48) {
        for (int k = 0; k < MaxNumOfCh; ++k) {
          if (m_rawCDCs[i]->CheckOnlineRemovedDataBit(j, k) == true)m_hBit->SetBinContent(i + 1, k + 1, -0.5);
          else m_hBit->SetBinContent(i + 1, k + 1, 0.5);
        }
      }
    }
  }

  // ADC vs layer 2D histogram with only good track related hits
  double iselect = -1.0;

  for (const auto& b2track : m_Tracks) {

    const Belle2::TrackFitResult* fitresult = b2track.getTrackFitResultWithClosestMass(Const::pion);
    if (!fitresult) {
      B2WARNING("No track fit result found.");
      continue;
    }

    Belle2::RecoTrack* track = b2track.getRelatedTo<Belle2::RecoTrack>(m_recoTrackArrayName);
    if (!track) {
      B2WARNING("Can not access RecoTrack of this Belle2::Track");
      continue;
    }

    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs) {
      B2WARNING("Can not access FitStatus of this Track");
      continue;
    }

    // use tracks from IP for layer effi calculation
    if (std::fabs(fitresult->getD0()) < 1.0 && std::fabs(fitresult->getZ0()) < 1.0) {
      std::set<int> hitInSLayer;  // list of contributing layers for this track
      for (const RecoHitInformation::UsedCDCHit* hit : track->getCDCHitList()) {
        const genfit::TrackPoint* tp = track->getCreatedTrackPoint(track->getRecoHitInformation(hit));
        if (!tp) continue;
        hitInSLayer.insert(hit->getICLayer());
      }
      if (hitInSLayer.empty()) continue;
      auto helix = fitresult->getHelix();
      int nSLayers = cdcgeo.getNumberOfSenseLayers();
      for (int lay = 0; lay < nSLayers; lay++) {
        double layerR = cdcgeo.senseWireR(lay);
        // extrapolate to layer
        double arcLength = helix.getArcLength2DAtCylindricalR(layerR);
        if (std::isnan(arcLength)) continue;
        const auto& result = helix.getPositionAtArcLength2D(arcLength);
        if (result.Z() > cdcgeo.senseWireFZ(lay) || result.Z() < cdcgeo.senseWireBZ(lay)) continue;
        // get phi of extrapolation and fill
        double phi = getShiftedPhi(result, lay);
        int fillXbin = findThetaBin(phi, lay);
        m_hTrackingWireEff->Fill(fillXbin, lay);
        if (hitInSLayer.count(lay)) // if hit is attached in this layer
          m_hTrackingWireEff->Fill(fillXbin, lay + nSLayers);
      }
    }

    double phiDegree = fitresult->getPhi() / Unit::deg;

    m_hPhiNCDC->Fill(phiDegree, track->getNumberOfCDCHits());

    // require high NDF track
    int ndf = fs->getNdf();
    if (ndf < 20) continue;

    if (fabs(fitresult->getD0()) > 1.0 || fabs(fitresult->getZ0()) > 1.0) {
      //Off IP tracks
      m_hPhiIndex->Fill(phiDegree, 0.5); //all skims
      if (IsBhabha) iselect = 1.5;
      if (IsHadron) iselect = 2.5;
      if (IsMumu) iselect = 3.5;
      m_hPhiIndex->Fill(phiDegree, iselect);
    } else {
      //IP tracks
      m_hPhi->Fill(phiDegree);
      m_hPhiIndex->Fill(phiDegree, 4.5); //all skims
      if (IsBhabha) iselect = 5.5;
      if (IsHadron) iselect = 6.5;
      if (IsMumu) iselect = 7.5;
      m_hPhiIndex->Fill(phiDegree, iselect);

      //for tracking efficiency part
      double nsvdhits = fitresult->getHitPatternVXD().getNSVDHits();
      double ncdchits = fitresult->getHitPatternCDC().getNHits();
      if (nsvdhits > 6) {
        if (ncdchits >= 100)ncdchits = 99.5; //push to last bin
        m_hPhiEff->Fill(phiDegree, ncdchits);
      }
    }

    // Fill histograms of ADC/TDC if hits are associated with track
    for (const RecoHitInformation::UsedCDCHit* hit : track->getCDCHitList()) {

      const genfit::TrackPoint* tp = track->getCreatedTrackPoint(track->getRecoHitInformation(hit));
      if (!tp) {
        B2WARNING("Can not access TrackPoint of this hit");
        continue;
      }

      UChar_t lay = hit->getICLayer();
      UShort_t IWire = hit->getIWire();
      UShort_t adc = hit->getADCCount();
      unsigned short tdc = hit->getTDCCount();
      WireID wireid(lay, IWire);
      unsigned short bid = cdcgeo.getBoardID(wireid);

      m_hADC->Fill(bid, adc);
      m_hTDC->Fill(bid, tdc);
      m_hPhiHit->Fill(phiDegree, lay);
    }
  }
}

void CDCDQMModule::endRun()
{
  m_hNEvents->SetBinContent(1, m_nEvents);
}

void CDCDQMModule::terminate()
{
}

int CDCDQMModule::findThetaBin(double phi, const int& lay)
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  int nWires = cdcgeo.nWiresInLayer(lay);
  double offset = cdcgeo.offset(lay);
  double shift = (offset - 0.5) * 2 * TMath::Pi() / nWires;
  double binLow = shift;
  double binHigh = 2 * TMath::Pi() + shift;
  // wrap around
  if (phi < binLow) phi += 2 * TMath::Pi(); // Wrap around for values less than binLow
  else if (phi >= binHigh) phi -= 2 * TMath::Pi(); // Wrap around for values >= binHigh
  double binWidth = (binHigh - binLow) / nWires;
  int binIndex = (phi - binLow) / binWidth;
  // Return the bin index (1-based for ROOT histograms)
  return binIndex + 1;
}

double CDCDQMModule::getShiftedPhi(const ROOT::Math::XYZVector& position, const int& lay)
{
  double phi = TMath::ATan2(position.Y(), position.X());
  if (m_adjustWireShift) {
    static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
    int nWires = cdcgeo.nWiresInLayer(lay);
    int nShifts = cdcgeo.nShifts(lay);
    double fZ = cdcgeo.senseWireFZ(lay);
    double bZ = cdcgeo.senseWireBZ(lay);
    double R = cdcgeo.senseWireR(lay);
    double phiSize = 2 * TMath::Pi() / nWires;
    double phiF = phiSize * 0.5 * nShifts;
    double phiB = 0;
    B2Vector3D f(R * TMath::Cos(phiF), R * TMath::Sin(phiF), fZ);
    B2Vector3D b(R * TMath::Cos(phiB), R * TMath::Sin(phiB), bZ);
    B2Vector3D v = f - b;
    B2Vector3D u = v.Unit();
    double beta = (position.Z() - b.Z()) / u.Z();
    B2Vector3D p = b + beta * u;
    phi -= TMath::ATan2(p.Y(), p.X());
  }
  while (phi < 0) phi += (2 * TMath::Pi());
  while (phi >= 2 * TMath::Pi()) phi -= (2 * TMath::Pi());
  return phi;
}
