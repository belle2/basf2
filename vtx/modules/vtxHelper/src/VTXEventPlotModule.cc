/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/modules/vtxHelper/VTXEventPlotModule.h>
#include <TText.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TH2.h>
#include <boost/format.hpp>
#include <vxd/geometry/GeoCache.h>
#include <vtx/geometry/SensorInfo.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::VTX;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VTXEventPlot)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VTXEventPlotModule::VTXEventPlotModule() : Module()
{
  //Set module properties
  setDescription("Plot VTX Hit/Charge Maps and write pictures");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("VTXDigitsName", m_storeDigitsName, "The name of the StoreArray of VTXDigits to be processed", string(""));
}

void VTXEventPlotModule::initialize()
{
  m_eventMetaData.isRequired();
  m_storeDigits.isRequired(m_storeDigitsName);
}

void VTXEventPlotModule::beginRun()
{
  // Loop over all VTX sensors and find the maximum values for uCell and vCell.
  m_maxVCell = 0;
  m_maxUCell = 0;

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (VxdID layer : geo.getLayers(SensorInfo::VTX)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensorID : geo.getSensors(ladder)) {
        const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
        if (m_maxVCell < info.getVCells()) m_maxVCell = info.getVCells()  ;
        if (m_maxUCell < info.getUCells()) m_maxUCell = info.getUCells()  ;
      }
    }
  }


}

void VTXEventPlotModule::event()
{
  unsigned int evtNr = m_eventMetaData->getEvent();
  unsigned int evtRun = m_eventMetaData->getRun();
  map <VxdID, bool> flag; // flag sensors with digits
  string histoname;
  string labeltext;
  TCanvas* c1;
  gStyle->SetPalette(55);
  gStyle->SetOptStat(0);
  c1 = new TCanvas("c1", "c1", 1200, 600);
  TH2F* h = new TH2F("Chargemap", "VTX Module Chargemap;VCell;UCell",  m_maxVCell, 0,  m_maxVCell, m_maxUCell, 0, m_maxUCell);
  h->SetContour(100);

  for (auto& it : m_storeDigits) {
    flag[it.getSensorID()] = true;
  }

  for (auto& f : flag) {
    c1->Clear();
    c1->cd();
    h->Reset();

    VxdID currentVxdId = f.first;
    histoname = boost::str(boost::format("Run_%d_Evt_%d_") % evtRun % evtNr) + string(currentVxdId);
    h->SetTitle(histoname.data());

    double occ = 0;

    for (auto& pix : m_storeDigits) {

      if (currentVxdId != pix.getSensorID()) continue;
      h->Fill(pix.getVCellID(), pix.getUCellID(), pix.getCharge());
      occ++;
    }

    const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(currentVxdId));
    occ = 100.0 * occ / info.getVCells() / info.getUCells();

    h->Draw("colz");

    TText* label = new TText();
    label->SetNDC();
    label->SetTextFont(1);
    label->SetTextColor(1);
    label->SetTextSize(0.03);
    label->SetTextAlign(22);
    label->SetTextAngle(0);
    labeltext = boost::str(boost::format("Occ. %f %%") % occ);
    label->DrawText(0.8, 0.8, labeltext.c_str());

    c1->Print((histoname + ".png").data());
    c1->Print((histoname + ".root").data());

    B2RESULT("The occupancy for VTX sensor " << currentVxdId << " is: " << occ);
  }
  delete h;
  delete c1;
}
