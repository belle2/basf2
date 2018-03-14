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
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIDQMModule::PXDROIDQMModule() : HistoModule() , m_storeROIs()
{
  //Set module properties
  setDescription("Monitor ROIs");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdrawroi"));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of PXDRawROIs to be processed", std::string(""));
}

void PXDROIDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  hrawROIcount = new TH1F("hrawROIcount", "ROI count;Nr per Event", 250, 0, 250);
  hrawROItype = new TH1F("hrawROItype", "ROI type;Nr per Event", 2, 0, 2);

  hrawROIHLTmap  = new TH2F("hrawROIHLTmap", "HLT ROI Middle Map ;U;V", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIHLTsize  = new TH2F("hrawROIHLTsize", "HLT ROI Size Map ;U;V", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIHLT_DHHID = new TH1F("hrawROIHLT_DHHID", "HLT ROI DHHID;Nr per Event", 64, 0, 64);
  hrawROIHLTminV = new TH1F("hrawROIHLTminV", "HLT ROI minV;V", 768, 0, 768);
  hrawROIHLTmaxV = new TH1F("hrawROIHLTmaxV", "HLT ROI maxV;V", 768, 0, 768);
  hrawROIHLTminU = new TH1F("hrawROIHLTminU", "HLT ROI minU;U", 250, 0, 250);
  hrawROIHLTmaxU = new TH1F("hrawROIHLTmaxU", "HLT ROI maxU;U", 250, 0, 250);
  hrawROIHLTsizeV = new TH1F("hrawROIHLTsizeV", "HLT ROI size;V", 768, 0, 768);
  hrawROIHLTsizeU = new TH1F("hrawROIHLTsizeU", "HLT ROI size;U", 250, 0, 250);

  hrawROIDCmap  = new TH2F("hrawROIDCmap", "DATCON ROI Middle Map ;U;V", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIDCsize  = new TH2F("hrawROIDCsize", "DATCON ROI Size Map ;U;V", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIDC_DHHID = new TH1F("hrawROIDC_DHHID", "DATCON ROI DHHID;Nr per Event", 64, 0, 64);
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

  hrawROIHLTmap->Reset();
  hrawROIHLTsize->Reset();
  hrawROIHLT_DHHID->Reset();
  hrawROIHLTminV->Reset();
  hrawROIHLTmaxV->Reset();
  hrawROIHLTminU->Reset();
  hrawROIHLTmaxU->Reset();
  hrawROIHLTsizeV->Reset();
  hrawROIHLTsizeU->Reset();

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
    //it.dump();
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
      if (it.getType(j)) {
        nr_DC++;
        hrawROIDC_DHHID->Fill(it.getDHHID(j));
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
        hrawROIHLT_DHHID->Fill(it.getDHHID(j));
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
