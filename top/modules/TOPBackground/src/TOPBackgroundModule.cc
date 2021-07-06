/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPBackground/TOPBackgroundModule.h>

#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/geometry/TOPGeometryPar.h>

#include <mdst/dataobjects/MCParticle.h>
#include <simulation/dataobjects/BeamBackHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <TCanvas.h>
#include <TLegend.h>
#include <TLine.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>

#include <geometry/Materials.h>
#include <G4Material.hh>

using namespace std;
using namespace boost;

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPBackground)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPBackgroundModule::TOPBackgroundModule() : Module(),
    m_rootFile(0),
    origingamma(0),
    originpe(0),
    peflux(0),
    nflux(0),
    rdose(0),
    zdist(0),
    genergy(0),
    genergy2(0),
    zdistg(0),
    originpt(0),
    nflux_bar(0),
    gflux_bar(0),
    cflux_bar(0),
    norigin(0),
    gorigin(0),
    corigin(0),
    nprim(0),
    gprim(0),
    cprim(0),
    origin_zx(0),
    origin_zy(0),
    prim_zx(0),
    prim_zy(0),
    module_occupancy(0),
    PCBmass(0),
    PCBarea(0),
    yearns(0),
    evtoJ(0),
    mtoc(0),
    count(0),
    count_occ(0),
    origingamma_x(0),
    origingamma_y(0),
    origingamma_z(0),
    originpe_x(0),
    originpe_y(0),
    originpe_z(0)
  {
    // Set description()
    setDescription("A module to analyze beam background simulations regarding TOP");

    // Add parameters
    addParam("Type", m_BkgType, "Backgound type" , string("Backgound"));
    addParam("Output", m_OutputFileName, "Name of the output file",
             string("Backgound.root"));
    addParam("TimeOfSimulation", m_TimeOfSimulation,
             "Real time in micro seconds that corresponds to simulated data", 5.);
  }

  TOPBackgroundModule::~TOPBackgroundModule()
  {

  }

  void TOPBackgroundModule::initialize()
  {

    // CPU time start

    // Initializing the output root file
    m_rootFile = TFile::Open(m_OutputFileName.c_str(), "RECREATE");
    origingamma = new TTree("origingamma", "tree");
    originpe = new TTree("originpe", "tree2");

    origingamma->Branch("x", &origingamma_x);
    origingamma->Branch("y", &origingamma_y);
    origingamma->Branch("z", &origingamma_z);
    originpe->Branch("x", &originpe_x);
    originpe->Branch("y", &originpe_y);
    originpe->Branch("z", &originpe_z);

    peflux = new TH1F("Photoelectron flux", "Photoelectron flux", 33, -11.25, 360);
    nflux = new TH1F("Neutron flux", "Neutron flux", 33, -11.25, 360);
    rdose = new TH1F("Radiation dose", "Radiation dose", 33, -11.25, 360);
    zdist = new TH1F("Photoelectron origin", "Photoelectron origin", 200, -400, 400);
    genergy = new TH1F("Energy distribution of photons", "Energy distribution of photons", 150, 0, 5);
    genergy2 = new TH1F("Energy distribution of gammas", "Energy distribution of gammas", 500, 0, 5);

    zdistg = new TH1F("Photoelectron flux z", "Photoelectron flux Z projection", 800, -400, 400);
    originpt = new TH1F("P_t of origin electron", "P_t of origin electron", 300, 0.06, 0.14);

    nflux_bar = new TH2F("Neutron flux on bar", "Neutron flux on bar", 32, -114.8, 211.5, 16, -0, 360);
    gflux_bar = new TH2F("Gamma flux on bar", "Gamma flux on bar MHz/cm^{2}", 32, -114.8, 211.5, 16, -0, 360);
    cflux_bar = new TH2F("Charged flux on bar", "Charged flux on bar MHz/cm^{2}", 32, -114.8, 211.5, 16, -0, 360);

    norigin = new TH1F("neutron(BAR) origin", "neutron(BAR) origin", 200, -400, 400);
    gorigin = new TH1F("gamma(BAR) origin", "gamma(BAR) origin", 200, -400, 400);
    corigin = new TH1F("charged(BAR) origin", "charged(BAR) origin", 200, -400, 400);

    nprim = new TH1F("neutron(BAR) primary", "neutron(BAR) primary", 200, -400, 400);
    gprim = new TH1F("gamma(BAR) primary", "gamma(BAR) primary", 200, -400, 400);
    cprim = new TH1F("charged(BAR) primary", "charged(BAR) primary", 200, -400, 400);

    origin_zx = new TGraph();
    origin_zy = new TGraph();

    prim_zx = new TGraph();
    prim_zy = new TGraph();
    module_occupancy = new TGraph();

    origin_zy->SetName("originZY");
    origin_zx->SetName("originZX");
    module_occupancy->SetName("occupancy");

    const auto& geo = TOP::TOPGeometryPar::Instance()->getGeometry()->getFrontEnd();
    double S1 = geo.getFrontBoardWidth() * geo.getFrontBoardHeight();
    double S2 = geo.getHVBoardWidth() * geo.getHVBoardLength();
    double V1 = S1 * geo.getFrontBoardThickness();
    double V2 = S2 * geo.getHVBoardThickness();
    G4Material* material1 = geometry::Materials::get(geo.getFrontBoardMaterial());
    if (!material1) B2FATAL("Material '" << geo.getFrontBoardMaterial() << "' not found");
    G4Material* material2 = geometry::Materials::get(geo.getHVBoardMaterial());
    if (!material2) B2FATAL("Material '" << geo.getHVBoardMaterial() << "' not found");
    double density1 = material1->GetDensity() / CLHEP::kg * CLHEP::cm * CLHEP::cm * CLHEP::cm;
    double density2 = material2->GetDensity() / CLHEP::kg * CLHEP::cm * CLHEP::cm * CLHEP::cm;

    PCBarea = S1 + S2; // [cm^2], old value was: 496.725
    PCBmass = V1 * density1 + V2 * density2; // [kg], old value was: 0.417249

    yearns = 1.e13;
    evtoJ = 1.60217653 * 1e-10;
    mtoc = 1.97530864197531;
    count = 0;
    count_occ = 0;

    origingamma_x = 0;
    origingamma_y = 0;
    origingamma_z = 0;
    originpe_x = 0;
    originpe_y = 0;
    originpe_z = 0;
  }

  void TOPBackgroundModule::beginRun()
  {
    // Print run number
    B2INFO("TOPBackground: Processing:");

  }

  void TOPBackgroundModule::event()
  {
    StoreArray<TOPSimHit>  topSimhits;
    StoreArray<MCParticle> mcParticles;
    StoreArray<TOPDigit> topDigits;
    StoreArray<TOPBarHit> topTracks;

    int nHits = topDigits.getEntries();
    for (int i = 0; i < nHits; i++) {
      TOPDigit* aDigit = topDigits[i];
      int barID = aDigit->getModuleID();

      peflux->AddBinContent(barID * 2, 1. / m_TimeOfSimulation / 32.0);

      const TOPSimHit* simHit = DataStore::getRelated<TOPSimHit>(aDigit);
      if (!simHit) continue;
      int PMTID = simHit->getPmtID();

      module_occupancy->SetPoint(count_occ, PMTID , barID);
      count_occ++;

      genergy->Fill(simHit->getEnergy());

      const MCParticle* particle = DataStore::getRelated<MCParticle>(simHit);

      if (particle) {
        const MCParticle* currParticle = particle;

        const MCParticle* mother = currParticle->getMother();

        int mm = 0;
        while (mother) {
          const MCParticle* pommother = mother->getMother();
          if (!pommother) {

            zdist->Fill(mother->getVertex().Z());
            zdistg->Fill(mother->getVertex().Z(), 1. / m_TimeOfSimulation / 32.0 / 16.0);
            originpe_x = mother->getVertex().X();
            originpe_y = mother->getVertex().Y();
            originpe_z = mother->getVertex().Z();
            originpe->Fill();
            TVector3 momentum = mother->getMomentum();
            if (m_BkgType.at(m_BkgType.size() - 3) == 'L') momentum.RotateY(0.0415);
            else if (m_BkgType.at(m_BkgType.size() - 3) == 'H') momentum.RotateY(-0.0415);
            double px = momentum.X();
            double py = momentum.Y();
            double pt = sqrt(px * px + py * py);
            originpt->Fill(pt);
          }
          mother = pommother;
          mm++;
        }
      }
    }


    StoreArray<BeamBackHit> beamBackHits;
    nHits = beamBackHits.getEntries();

    for (int iHit = 0; iHit < nHits; ++iHit) {
      BeamBackHit* tophit = beamBackHits[iHit];
      int subdet = tophit->getSubDet();
      if (subdet != 5) continue;

      TVector3 pos = tophit->getPosition();
      double phi = pos.XYvector().Phi_0_2pi(pos.XYvector().Phi()) / 3.14159265358979 * 180.;
      int barID = int (phi / 22.5 + 0.5);
      if (barID == 16) {
        barID = 0;
      }
      barID++;


      if (tophit->getPDG() == Const::neutron.getPDGCode()) {
        double w = tophit->getNeutronWeight();
        double tlen = tophit->getTrackLength();

        nflux->AddBinContent(barID * 2, w / m_TimeOfSimulation / PCBarea * yearns * tlen / 0.2);

      } else {
        double edep = tophit->getEnergyDeposit();
        rdose->AddBinContent(barID * 2, edep / m_TimeOfSimulation * yearns / PCBmass * evtoJ);
      }
    }

    nHits = topTracks.getEntries();
    for (int iHit = 0; iHit < nHits; ++iHit) {
      TOPBarHit* toptrk = topTracks[iHit];

      int PDG = toptrk->getPDG();
      int barID = toptrk->getModuleID();

      if (PDG == Const::neutron.getPDGCode()) {
        nflux_bar->Fill(toptrk->getPosition().Z(), (barID - 1) * 22.5,
                        1. / 917.65 / m_TimeOfSimulation * yearns * 2.);
        norigin->Fill(toptrk->getProductionPoint().Z());
      } else {
        if (PDG == Const::photon.getPDGCode()) {
          gflux_bar->Fill(toptrk->getPosition().Z(), (barID - 1) * 22.5,
                          1. / 917.65 / m_TimeOfSimulation * 2.);
          gorigin->Fill(toptrk->getProductionPoint().Z());
          genergy2->Fill(toptrk->getMomentum().Mag() * 1000);
          origin_zx->SetPoint(count, toptrk->getProductionPoint().Z(),
                              toptrk->getProductionPoint().X());
          origin_zy->SetPoint(count, toptrk->getProductionPoint().Z() / 0.999143,
                              toptrk->getProductionPoint().Y());
          origingamma_x = toptrk->getProductionPoint().X();
          origingamma_y = toptrk->getProductionPoint().Y();
          origingamma_z = toptrk->getProductionPoint().Z();
          origingamma->Fill();
          count++;

        } else {
          cflux_bar->Fill(toptrk->getPosition().Z(), (barID - 1) * 22.5,
                          1. / 917.65 / m_TimeOfSimulation * 2.);
          corigin->Fill(toptrk->getProductionPoint().Z());
        }
      }

    }
  }


  void TOPBackgroundModule::myprint(TH1F* histo, const char* path, const char* xtit = "", const char* ytit = "", double tresh = 0)
  {

    gROOT->Reset();
    gStyle->SetOptStat("");
    gStyle->SetOptFit(1111);

    gStyle->SetCanvasColor(-1);
    gStyle->SetPadColor(-1);
    gStyle->SetFrameFillColor(-1);
    gStyle->SetHistFillColor(-1);
    gStyle->SetTitleFillColor(-1);
    gStyle->SetFillColor(-1);
    gStyle->SetFillStyle(4000);
    gStyle->SetStatStyle(0);
    gStyle->SetTitleStyle(0);
    gStyle->SetCanvasBorderSize(0);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    //  gStyle->SetTitleMode(0);
    gStyle->SetFrameBorderSize(0);
    gStyle->SetLegendBorderSize(0);
    gStyle->SetStatBorderSize(0);
    gStyle->SetTitleBorderSize(0);
    //gROOT->ForceStyle();*/



    TCanvas* c1 = new TCanvas("c1", "", 1920, 1200);

    double x1 = histo->GetBinLowEdge(1);
    double nb = histo->GetNbinsX();
    double bin = histo->GetBinWidth(1);
    double x2 = x1 + bin * nb;

    double max = histo->GetBinContent(histo->GetMaximumBin());

    if (max < tresh) {
      histo->GetYaxis()->SetRangeUser(0, tresh * 1.1);
    }

    TLine* line = new TLine(x1, tresh, x2, tresh);
    line->SetLineColor(1);
    line->SetLineWidth(3);
    line->SetLineStyle(2);


    histo->SetFillColor(2);
    histo->SetLineColor(1);

    gPad->SetTopMargin(0.08);
    gPad->SetBottomMargin(0.15);
    gPad->SetGridy();

    histo->GetXaxis()->SetLabelSize(0.06);
    histo->GetYaxis()->SetLabelSize(0.06);
    histo->GetXaxis()->SetTitleSize(0.06);
    histo->GetYaxis()->SetTitleSize(0.06);
    histo->GetXaxis()->SetTitle(xtit);
    histo->GetYaxis()->SetTitle(ytit);
    histo->GetXaxis()->SetTitleOffset(0.9);
    histo->GetYaxis()->SetTitleOffset(0.7);

    histo->Draw();

    TLegend* leg = new TLegend(0.75, 0.95, 0.90, 1.00);
    leg->AddEntry(histo, m_BkgType.c_str(), "pf");
    leg->Draw("SAME");
    if (tresh > 0.01) {
      line->Draw("SAME");
    }

    c1->Print(path);
  }



  void TOPBackgroundModule::endRun()
  {
    B2INFO("TOPBackground: Finished:");
  }

  void TOPBackgroundModule::terminate()
  {
    /*
     myprint(peflux, ("peflux_" + m_BkgType + ".pdf").c_str(), "#phi", "MHz / PMT", 1.);
     myprint(zdist, ("zdist_" + m_BkgType + ".pdf").c_str(), "z[cm]", "", 0.0);
     myprint(nflux, ("nflux_" + m_BkgType + ".pdf").c_str(), "#phi", "neutrons / cm^{2} / year", 0.0);
     myprint(rdose, ("rdose_" + m_BkgType + ".pdf").c_str(), "#phi", "Gy/year", 0.0);
     */

    m_rootFile->cd();
    origingamma->Write();
    originpe->Write();
    peflux->Write();
    zdist->Write();
    nflux->Write();
    rdose->Write();
    genergy->Write();
    genergy2->Write();
    nflux_bar->Write();
    gflux_bar->Write();
    origin_zx->Write();
    origin_zy->Write();
    module_occupancy->Write();
    gorigin->Write();
    norigin->Write();
    zdistg->Write();
    originpt->Write();
    m_rootFile->Close();

    // Announce
    B2INFO("TOPBackground finished");


  }

  void TOPBackgroundModule::printModuleParams() const
  {

  }

} // end Belle2 namespace
