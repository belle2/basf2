/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdHelper/PXDROIPlotModule.h>
#include <TBox.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TH2.h>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIPlot)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIPlotModule::PXDROIPlotModule() : Module()
{
  //Set module properties
  setDescription("Plot ROIs on PXD Hit/Charge Maps and write pictures");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
  // we do not want to have a default name, as the user should be very sure what he is plotting!
  addParam("ROIsName", m_ROIsName, "name of the list of ROIs (plotted in red)", string("__unusedROIs__"));
  addParam("DCROIsName", m_DCROIsName, "name of the list of DatCon ROIs (optional, plotted in blue)", string("__unusedROIs__"));
  addParam("HLTROIsName", m_HLTROIsName, "name of the list of HLT ROIs (optional, plotted in green)", string("__unusedROIs__"));
  addParam("Prefix", m_prefix, "prefix for plots", string(""));
}

void PXDROIPlotModule::initialize()
{
  m_eventMetaData.isRequired();
  m_storeRawHits.isRequired(m_storeRawHitsName);
  // all ROIs are optional, allowing to use always the same color for comparison even if one cathegory is not there
  m_storeROIs.isOptional(m_ROIsName);
  m_storeDCROIs.isOptional(m_DCROIsName);
  m_storeHLTROIs.isOptional(m_HLTROIsName);
}

void PXDROIPlotModule::event()
{
  unsigned int evtNr = m_eventMetaData->getEvent();
  unsigned int evtRun = m_eventMetaData->getRun();
  map <VxdID, bool> flag; // flag sensors with ROIs
  map <VxdID, vector <ROIid>> list;
  map <VxdID, vector <ROIid>> listDC;
  map <VxdID, vector <ROIid>> listHLT;
  string histoname;
  TCanvas* c1;
  gStyle->SetPalette(55);
  gStyle->SetOptStat(0);
  c1 = new TCanvas("c1", "c1", 1200, 600);
  TH2F* h = new TH2F("ChargemapWithROIs", "PXD Module Chargemap;VCell;UCell", 768, 0, 768, 250, 0, 250);
  h->SetContour(100);
  for (auto& it : m_storeROIs) {
    list[it.getSensorID()].push_back(it);
    flag[it.getSensorID()] = true;
  }
  for (auto& it : m_storeDCROIs) {
    listDC[it.getSensorID()].push_back(it);
    flag[it.getSensorID()] = true;
  }
  for (auto& it : m_storeHLTROIs) {
    listHLT[it.getSensorID()].push_back(it);
    flag[it.getSensorID()] = true;
  }

  for (auto& f : flag) {
    c1->Clear();
    c1->cd();
    h->Reset();

    VxdID currentVxdId = f.first;
    histoname = m_prefix + boost::str(boost::format("Run_%d_Evt_%d_") % evtRun % evtNr) + string(currentVxdId);
    h->SetTitle(histoname.data());

    for (auto& pix : m_storeRawHits) {

      if (currentVxdId != pix.getSensorID()) continue;
      h->Fill(pix.getRow(), pix.getColumn(), pix.getCharge());

    }

    h->Draw("colz");

    // Alpha Blending only works in ROOT's Batch Mode, but LineStyle won't.
    // LineStyle works in normal mode, but Alpha won't
    for (auto& it : list[currentVxdId]) {
      TBox* b;
      b = new TBox(it.getMinVid(), it.getMinUid(), it.getMaxVid(), it.getMaxUid());
      b->SetLineColorAlpha(kRed, 0.3);
      b->SetLineWidth(3);
      b->SetFillStyle(0);// Hollow
      b->Draw();
    }
    // we move the other box by more than half a bin. this is needed as alpha seems not to work
    for (auto& it : listDC[currentVxdId]) {
      TBox* b;
      b = new TBox(it.getMinVid() + 0.7, it.getMinUid() + 0.7, it.getMaxVid() + 0.7, it.getMaxUid() + 0.7);
      b->SetLineColorAlpha(kBlue, 0.3);
      b->SetLineWidth(2);
      b->SetLineStyle(2);
      b->SetFillStyle(0);// Hollow
      b->Draw();
    }
    // we move the other box by half a bin. this is needed as alpha seems not to work, in addition we use a dashed style
    // dashed style doesnt work with png export, thus if all ROIs are identical, lines might overlap completely
    for (auto& it : listHLT[currentVxdId]) {
      TBox* b;
      b = new TBox(it.getMinVid() - 0.7, it.getMinUid() - 0.7, it.getMaxVid() - 0.7, it.getMaxUid() - 0.7);
      b->SetLineColorAlpha(kGreen, 0.3);
      b->SetLineWidth(2);
      b->SetLineStyle(7);
      b->SetFillStyle(0);// Hollow
      b->Draw();
    }
    c1->Print((histoname + ".png").data());
    c1->Print((histoname + ".root").data());
  }
  delete h;
  delete c1;
}
