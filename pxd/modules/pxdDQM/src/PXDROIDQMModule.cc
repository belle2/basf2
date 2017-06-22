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

  hrawROIHLTmap  = new TH2F("hrawROIHLTmap", "HLT ROI Middle Map ;column;row", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIHLTsize  = new TH2F("hrawROIHLTsize", "HLT ROI Size Map ;column;row", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIHLT_DHHID = new TH1F("hrawROIHLT_DHHID", "HLT ROI DHHID;Nr per Event", 64, 0, 64);
  hrawROIHLTrow1 = new TH1F("hrawROIHLTrow1", "HLT ROI row1;row", 768, 0, 768);
  hrawROIHLTrow2 = new TH1F("hrawROIHLTrow2", "HLT ROI row2;row", 768, 0, 768);
  hrawROIHLTcol1 = new TH1F("hrawROIHLTcol1", "HLT ROI col1;column", 250, 0, 250);
  hrawROIHLTcol2 = new TH1F("hrawROIHLTcol2", "HLT ROI col2;column", 250, 0, 250);
  hrawROIHLTrowSize = new TH1F("hrawROIHLTrowSize", "HLT ROI size row;column", 768, 0, 768);
  hrawROIHLTcolSize = new TH1F("hrawROIHLTcolSize", "HLT ROI size col;column", 250, 0, 250);

  hrawROIDCmap  = new TH2F("hrawROIDCmap", "DATCON ROI Middle Map ;column;row", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIDCsize  = new TH2F("hrawROIDCsize", "DATCON ROI Size Map ;column;row", 250 / 4, 0, 250, 768 / 4, 0, 768);
  hrawROIDC_DHHID = new TH1F("hrawROIDC_DHHID", "DATCON ROI DHHID;Nr per Event", 64, 0, 64);
  hrawROIDCrow1 = new TH1F("hrawROIDCrow1", "DATCON ROI row1;row", 768, 0, 768);
  hrawROIDCrow2 = new TH1F("hrawROIDCrow2", "DATCON ROI row2;row", 768, 0, 768);
  hrawROIDCcol1 = new TH1F("hrawROIDCcol1", "DATCON ROI col1;column", 250, 0, 250);
  hrawROIDCcol2 = new TH1F("hrawROIDCcol2", "DATCON ROI col2;column", 250, 0, 250);

  hrawROINrDCvsNrHLT = new TH2F("hrawROINrDCvsNrHLT", "Nr DATCON ROI vs Nr HLT ROI; Nr HLT ROI;Nr DATCON ROI", 100, 0, 100, 100, 0,
                                100);
  hrawROIEVTsWithOneSource = new TH2F("hrawROIEVTsWithOneSource", "Events with only one ROI source; HLT ROI;DATCON ROI", 2, 0, 2, 2,
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
  hrawROIHLTrow1->Reset();
  hrawROIHLTrow2->Reset();
  hrawROIHLTcol1->Reset();
  hrawROIHLTcol2->Reset();
  hrawROIHLTrowSize->Reset();
  hrawROIHLTcolSize->Reset();

  hrawROIDCmap->Reset();
  hrawROIDCsize->Reset();
  hrawROIDC_DHHID->Reset();
  hrawROIDCrow1->Reset();
  hrawROIDCrow2->Reset();
  hrawROIDCcol1->Reset();
  hrawROIDCcol2->Reset();

  hrawROINrDCvsNrHLT->Reset();
  hrawROIEVTsWithOneSource->Reset();
}

void PXDROIDQMModule::event()
{
  for (auto& it : m_storeROIs) {
    int nr;
    int nr_HLT = 0;
    int nr_DC = 0;
    nr = it.getNrROIs();
    //it.dump();
    hrawROIcount->Fill(nr);
    for (auto j = 0; j < nr; j++) {
      hrawROItype->Fill(it.getType(j));
      int r1, r2, c1, c2, rm, cm, rs, cs;
      r1 = it.getRow1(j);
      r2 = it.getRow2(j);
      rs = r2 - r1;
      rm = (r1 + r2) / 2;
      c1 = it.getCol1(j);
      c2 = it.getCol2(j);
      cs = c2 - c1;
      cm = (c1 + c2) / 2;
      if (it.getType(j)) {
        nr_DC++;
        hrawROIDC_DHHID->Fill(it.getDHHID(j));
        hrawROIDCmap->Fill(cm, rm);
        hrawROIDCsize->Fill(cs, rs);
        hrawROIDCrow1->Fill(r1);
        hrawROIDCrow2->Fill(r2);
        hrawROIDCcol1->Fill(c1);
        hrawROIDCcol2->Fill(c2);
      } else {
        nr_HLT++;
        hrawROIHLT_DHHID->Fill(it.getDHHID(j));
        hrawROIHLTmap->Fill(cm, rm);
        hrawROIHLTsize->Fill(cs, rs);
        hrawROIHLTrow1->Fill(r1);
        hrawROIHLTrow2->Fill(r2);
        hrawROIHLTcol1->Fill(c1);
        hrawROIHLTcol2->Fill(c2);
        hrawROIHLTrowSize->Fill(rs);
        hrawROIHLTcolSize->Fill(cs);
      }
    }
    hrawROINrDCvsNrHLT->Fill(nr_HLT, nr_DC);
    if (nr_DC == 0 && nr_HLT == 0) {
      hrawROIEVTsWithOneSource->Fill(nr_HLT, nr_DC);
    }
    if (nr_DC == 0 && nr_HLT != 0) {
      hrawROIEVTsWithOneSource->Fill(1, nr_DC);
    }
    if (nr_DC != 0 && nr_HLT == 0) {
      hrawROIEVTsWithOneSource->Fill(nr_HLT, 1);
    }
    if (nr_DC != 0 && nr_HLT != 0) {
      hrawROIEVTsWithOneSource->Fill(1, 1);
    }
    nr_HLT = 0;
    nr_DC = 0;
  }
}
