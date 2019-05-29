/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDROIDQMModule.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::VXD;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIDQMModule::PXDROIDQMModule() : HistoModule() , m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Monitor ROIs");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdrawroi"));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of PXDRawROIs to be processed", std::string(""));
  addParam("eachModule", m_eachModule, "create for each module", false);
}

void PXDROIDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  hrawROIcount = new TH1F("hrawROIcount", "ROI count;Nr per Event", 250, 0, 250);
  hrawROItype = new TH1F("hrawROItype", "ROI type;Nr per Event", 2, 0, 2);

  hrawROIHLTmap  = new TH2F("hrawROIHLTmap", "HLT ROI Middle Map;Ucell;Vcell", 250 / 5, 0, 250, 768 / 4, 0, 768);
  hrawROIHLTsize  = new TH2F("hrawROIHLTsize", "HLT ROI Size Map;;Ucell;Vcell", 50, 0, 200, 50, 0, 200);
  hrawROIHLT_DHHID = new TH1F("hrawROIHLT_DHHID", "HLT ROI DHHID;ROIs per Module;DHH ID", 64, 0, 64);
  hrawROIHLTminV = new TH1F("hrawROIHLTminV", "HLT ROI minV;V", 768, 0, 768);
  hrawROIHLTmaxV = new TH1F("hrawROIHLTmaxV", "HLT ROI maxV;V", 768, 0, 768);
  hrawROIHLTminU = new TH1F("hrawROIHLTminU", "HLT ROI minU;U", 250, 0, 250);
  hrawROIHLTmaxU = new TH1F("hrawROIHLTmaxU", "HLT ROI maxU;U", 250, 0, 250);
  hrawROIHLTsizeV = new TH1F("hrawROIHLTsizeV", "HLT ROI size;V", 768, 0, 768);
  hrawROIHLTsizeU = new TH1F("hrawROIHLTsizeU", "HLT ROI size;U", 250, 0, 250);

  hrawROIDCmap  = new TH2F("hrawROIDCmap", "DATCON ROI Middle Map ;U;V", 250 / 5, 0, 250, 768 / 4, 0, 768);
  hrawROIDCsize  = new TH2F("hrawROIDCsize", "DATCON ROI Size Map ;U;V",  50, 0, 200, 50, 0, 200);
  hrawROIDC_DHHID = new TH1F("hrawROIDC_DHHID", "DATCON ROI DHHID;ROIs per Module; DHH ID", 64, 0, 64);
  hrawROIDCminV = new TH1F("hrawROIDCminV", "DATCON ROI minV;V", 768, 0, 768);
  hrawROIDCmaxV = new TH1F("hrawROIDCmaxV", "DATCON ROI maxV;V", 768, 0, 768);
  hrawROIDCminU = new TH1F("hrawROIDCminU", "DATCON ROI minU;U", 250, 0, 250);
  hrawROIDCmaxU = new TH1F("hrawROIDCmaxU", "DATCON ROI maxU;U", 250, 0, 250);
  hrawROIDCsizeV = new TH1F("hrawROIDCsizeV", "DATCON ROI size;V", 768, 0, 768);
  hrawROIDCsizeU = new TH1F("hrawROIDCsizeU", "DATCON ROI size;U", 250, 0, 250);

  hrawROINrDCvsNrHLT = new TH2F("hrawROINrDCvsNrHLT", "Nr DATCON ROI vs Nr HLT ROI; Nr HLT ROI;Nr DATCON ROI", 100, 0, 100, 100, 0,
                                100);
  hrawROIEVTsWithOneSource = new TH2F("hrawROIEVTsWithOneSource", "ROI sources in events; HLT;DATCON", 2, 0, 2, 2,
                                      0, 2);
  if (m_eachModule) {
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& avxdid : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
      if (info.getType() != VXD::SensorInfoBase::PXD) continue;
      // Only interested in PXD sensors

      TString buff = (std::string)avxdid;
      TString bufful = buff;
      bufful.ReplaceAll(".", "_");

      int dhh_id = ((avxdid.getLayerNumber() - 1) << 5) | ((avxdid.getLadderNumber()) << 1) | (avxdid.getSensorNumber() - 1);

      hrawROIHLTmapModule[dhh_id] = new TH2F("hrawROIHLTmap_" + bufful,
                                             "HLT ROI Middle Map  " + buff + ";Ucell;Vcell", 250, 0, 250, 768, 0, 768);
      hrawROIHLTsizeModule[dhh_id] = new TH2F("hrawROIHLTsize" + bufful,
                                              "HLT ROI Size Map " + buff + ";Ucell;Vcell",  50, 0, 200, 50, 0, 200);
      hrawROIDCmapModule[dhh_id] = new TH2F("hrawROIDCmap_" + bufful,
                                            "DC ROI Middle Map  " + buff + ";Ucell;Vcell", 250, 0, 250, 768, 0, 768);
      hrawROIDCsizeModule[dhh_id] = new TH2F("hrawROIDCsize" + bufful,
                                             "DC ROI Size Map " + buff + ";Ucell;Vcell",  50, 0, 200, 50, 0, 200);
    }
  }

  // cd back to root directory
  oldDir->cd();
}

void PXDROIDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeROIs.isRequired(m_PXDRawROIsName);
}

void PXDROIDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  hrawROIcount->Reset();
  hrawROItype->Reset();

  for (auto& a : hrawROIHLTmapModule) if (a.second) a.second->Reset();
  for (auto& a : hrawROIHLTsizeModule) if (a.second) a.second->Reset();
  hrawROIHLTmap->Reset();
  hrawROIHLTsize->Reset();
  hrawROIHLT_DHHID->Reset();
  hrawROIHLTminV->Reset();
  hrawROIHLTmaxV->Reset();
  hrawROIHLTminU->Reset();
  hrawROIHLTmaxU->Reset();
  hrawROIHLTsizeV->Reset();
  hrawROIHLTsizeU->Reset();

  for (auto& a : hrawROIDCmapModule) if (a.second) a.second->Reset();
  for (auto& a : hrawROIDCsizeModule) if (a.second) a.second->Reset();
  hrawROIDCmap->Reset();
  hrawROIDCsize->Reset();
  hrawROIDC_DHHID->Reset();
  hrawROIDCminV->Reset();
  hrawROIDCmaxV->Reset();
  hrawROIDCminU->Reset();
  hrawROIDCmaxU->Reset();
  hrawROIDCsizeV->Reset();
  hrawROIDCsizeU->Reset();

  hrawROINrDCvsNrHLT->Reset();
  hrawROIEVTsWithOneSource->Reset();
}

void PXDROIDQMModule::event()
{
  int nr_HLT = 0;
  int nr_DC = 0;
  for (auto& it : m_storeROIs) {
    int nr;
    nr = it.getNrROIs();
    hrawROIcount->Fill(nr);
    for (auto j = 0; j < nr; j++) {
      hrawROItype->Fill(it.getType(j));
      int Vmin, Vmax, Umin, Umax, Vmean, Umean, Vsize, Usize;
      Vmin = it.getMinVid(j);
      Vmax = it.getMaxVid(j);
      Vsize = Vmax - Vmin;
      Vmean = (Vmin + Vmax) / 2;
      Umin = it.getMinUid(j);
      Umax = it.getMaxUid(j);
      Usize = Umax - Umin;
      Umean = (Umin + Umax) / 2;
      auto id = it.getDHHID(j);
      if (it.getType(j)) {
        nr_DC++;
        hrawROIDC_DHHID->Fill(id);
        if (hrawROIDCmapModule[id]) hrawROIDCmapModule[id]->Fill(Umean, Vmean);
        if (hrawROIDCsizeModule[id]) hrawROIDCsizeModule[id]->Fill(Usize, Vsize);
        hrawROIDCmap->Fill(Umean, Vmean);
        hrawROIDCsize->Fill(Usize, Vsize);
        hrawROIDCminV->Fill(Vmin);
        hrawROIDCmaxV->Fill(Vmax);
        hrawROIDCminU->Fill(Umin);
        hrawROIDCmaxU->Fill(Umax);
        hrawROIDCsizeV->Fill(Vsize);
        hrawROIDCsizeU->Fill(Usize);
      } else {
        nr_HLT++;
        hrawROIHLT_DHHID->Fill(id);
        if (hrawROIHLTmapModule[id]) hrawROIHLTmapModule[id]->Fill(Umean, Vmean);
        if (hrawROIHLTsizeModule[id]) hrawROIHLTsizeModule[id]->Fill(Usize, Vsize);
        hrawROIHLTmap->Fill(Umean, Vmean);
        hrawROIHLTsize->Fill(Usize, Vsize);
        hrawROIHLTminV->Fill(Vmin);
        hrawROIHLTmaxV->Fill(Vmax);
        hrawROIHLTminU->Fill(Umin);
        hrawROIHLTmaxU->Fill(Umax);
        hrawROIHLTsizeV->Fill(Vsize);
        hrawROIHLTsizeU->Fill(Usize);
      }
    }
  }
  hrawROINrDCvsNrHLT->Fill(nr_HLT, nr_DC);
  if (nr_DC == 0 && nr_HLT == 0) {
    hrawROIEVTsWithOneSource->Fill("NO", "NO", 1);
  }
  if (nr_DC == 0 && nr_HLT != 0) {
    hrawROIEVTsWithOneSource->Fill("YES", "NO", 1);
  }
  if (nr_DC != 0 && nr_HLT == 0) {
    hrawROIEVTsWithOneSource->Fill("NO", "YES", 1);
  }
  if (nr_DC != 0 && nr_HLT != 0) {
    hrawROIEVTsWithOneSource->Fill("YES", "YES", 1);
  }
}
