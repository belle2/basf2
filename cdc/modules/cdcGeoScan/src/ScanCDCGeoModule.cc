/******************************************************************
* BASF2 (Belle II Analysis Software Framework)                    *
* Copyright(C) YEAR Belle II Collaboration                        *
*                                                                 *
* Test module to get CDC geometry parameters into a .root files   *
* To run this module use script: cdc/examples/runScanGeo.py       *
*******************************************************************/
#include <cdc/modules/cdcGeoScan/ScanCDCGeoModule.h>
#include <geometry/GeometryManager.h>
#include <framework/gearbox/GearDir.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include "TH1F.h"
#include "TCanvas.h"
#include "TVectorF.h"

using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(ScanCDCGeo);

ScanCDCGeoModule::ScanCDCGeoModule()
{
  setDescription("This module fills CDC geometry information in histo/tree format to a root file");
}

void ScanCDCGeoModule::initialize()
{

  bookOutput();

  TH1F* h_nwires = new TH1F("h_nwires", "Number of wires in layer;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_stereo = new TH1F("h_stereo", "Stereo angle in this layer;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_swire_posF_phi = new TH1F("h_swire_posF_phi", "#phi of sense wires in forward pos;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_swire_posF_theta = new TH1F("h_swire_posF_theta", "#theta of sense wires in forward pos;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_swire_posB_phi = new TH1F("h_swire_posB_phi", "#phi of sense wires in backward pos;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_swire_posB_theta = new TH1F("h_swire_posB_theta", "#theta of sense wires in backward pos;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_fwire_iradius = new TH1F("h_fwire_iradius", "inner radius of field wires;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_fwire_oradius = new TH1F("h_fwire_oradius", "outer radius of field wires;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_width = new TH1F("h_width", "Cell Width;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_height = new TH1F("h_height", "Cell Height;CDC layer#", 56, 0.5, 56.5);
  TH1F* h_length = new TH1F("h_length", "length of the wires;CDC layer#", 56, 0.5, 56.5);

  B2INFO("Creating CDCGeometryPar object");
  CDCGeometryPar::Instance();
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // Print some info
  cout << "------| Summary-1 " << endl;
  cout << left << "Number of CDC wire layers" << setw(15) << " " << cdcgeo.nWireLayers() << endl;
  cout << left << "Sense Wire Diameter" << setw(15) << " " << cdcgeo.senseWireDiameter() << endl;
  cout << left << "Field Wire Diameter" << setw(15) << " " << cdcgeo.fieldWireDiameter() << endl;

  cout << "------| Summary-2 " << endl;
  cout << left
       << setw(4) << ""
       << setw(10) << "Layer"
       << setw(15) << "nwire"
       << setw(15) << "radii sense"
       << setw(15) << "dRdown"
       << setw(15) << "dRup"
       << setw(15) << "Delta"
       << endl;

  for (unsigned int i = 0; i < cdcgeo.nWireLayers(); ++i) {

    fnWires = cdcgeo.nWiresInLayer(i);
    h_nwires->SetBinContent(i + 1, fnWires);

    // forward position of the input sense wire
    const TVector3& wirePosF = cdcgeo.wireForwardPosition(i, 0);
    fswire_posF_phi = wirePosF.Phi();
    h_swire_posF_phi->SetBinContent(i + 1, fswire_posF_phi);
    fswire_posF_theta = wirePosF.Theta();
    h_swire_posF_theta->SetBinContent(i + 1, fswire_posF_theta);

    const TVector3& wirePosB = cdcgeo.wireBackwardPosition(i, 0);
    fswire_posB_phi = wirePosB.Phi();
    h_swire_posB_phi->SetBinContent(i + 1, fswire_posB_phi);
    fswire_posB_theta = wirePosB.Theta();
    h_swire_posB_theta->SetBinContent(i + 1, fswire_posB_theta);

    const TVector3 wireDir = (wirePosF - wirePosB);
    fstereoAng = wireDir.Theta();
    if (wirePosF.Phi() < wirePosB.Phi())fstereoAng *= -1;
    h_stereo->SetBinContent(i + 1, fstereoAng);

    fclength = wirePosF.Perp();
    h_length->SetBinContent(i + 1, fclength);

    // the width of the cell (trapezoidal)
    fcwidth = 2 * PI * wirePosF.Perp() / fnWires;
    h_width->SetBinContent(i + 1, fcwidth);

    fwire_iradius = cdcgeo.innerRadiusWireLayer()[i];
    h_fwire_iradius->SetBinContent(i + 1, fwire_iradius);

    fwire_oradius = cdcgeo.outerRadiusWireLayer()[i];
    h_fwire_oradius->SetBinContent(i + 1, fwire_oradius);

    fcheight = fwire_oradius - fwire_iradius;
    h_height->SetBinContent(i + 1, fcheight);

    //Printing some useful information
    double inradiusnext = cdcgeo.innerRadiusWireLayer()[i + 1];
    double delta = -99.0;
    if (i < cdcgeo.nWireLayers() - 1) {
      delta = fwire_oradius - inradiusnext;
    }

    cout << left
         << setw(4) << ""
         << setw(10) << i
         << setw(15) << fnWires
         << setw(15) << cdcgeo.senseWireR(i)
         << setw(15) << cdcgeo.senseWireR(i) - cdcgeo.innerRadiusWireLayer()[i]
         << setw(15) << cdcgeo.outerRadiusWireLayer()[i] - cdcgeo.senseWireR(i)
         << setw(15) << delta
         << endl;
    m_tree->Fill();

  }

  TDirectory* dhistos = m_file->mkdir("histos");
  dhistos->cd();
  h_nwires->Write();
  h_swire_posF_phi->Write();
  h_swire_posB_phi->Write();
  h_swire_posF_theta->Write();
  h_swire_posB_theta->Write();
  h_stereo->Write();
  h_fwire_iradius->Write();
  h_fwire_oradius->Write();
  h_width->Write();
  h_height->Write();
  h_length->Write();

  TDirectory* dvar = m_file->mkdir("vars");
  dvar->cd();

  TVectorF sWireDia(1);
  sWireDia[0] = cdcgeo.senseWireDiameter();
  sWireDia.Write("sWireDia");

  TVectorF fWireDia(1);
  fWireDia[0] = cdcgeo.fieldWireDiameter();
  fWireDia.Write("fWireDia");

}

void ScanCDCGeoModule::bookOutput()
{
  // register output root file
  m_file = new TFile("CDCGeometryScan.root", "RECREATE");
  m_tree = new TTree("tree", "CDC Geometry details");
  m_tree->SetDirectory(0);
  m_tree->Branch("lnwires", &fnWires, "lnwires/I");
  m_tree->Branch("lsteang", &fstereoAng, "lsteang/D");
  m_tree->Branch("lswire_fpos_phi", &fswire_posF_phi, "lswire_fpos_phi/D");
  m_tree->Branch("lswire_fpos_theta", &fswire_posF_theta, "lswire_fpos_theta/D");
  m_tree->Branch("lswire_bpos_phi", &fswire_posB_phi, "lswire_bpos_phi/D");
  m_tree->Branch("lswire_bpos_theta", &fswire_posB_theta, "lswire_bpos_theta/D");
  m_tree->Branch("lfwire_inr", &fwire_iradius, "lfwire_inr/D");
  m_tree->Branch("lfwire_or", &fwire_oradius, "lfwire_or/D");
  m_tree->Branch("lcwidth", &fcwidth, "lcwidth/D");
  m_tree->Branch("lclength", &fclength, "lclength/D");
  m_tree->Branch("lcheight", &fcheight, "lcheight/D");
}

void ScanCDCGeoModule::terminate()
{
  m_file->cd();
  m_tree->Write();
  m_file->Close();
}