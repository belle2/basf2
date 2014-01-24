/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/pxdROIDQMModule.h>

#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pxdROIDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pxdROIDQMModule::pxdROIDQMModule() : HistoModule() , m_storeROIs()
{
  //Set module properties
  setDescription("Monitor ROIs");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void pxdROIDQMModule::defineHisto()
{
  hrawROIcount = new TH1F("hrawROIcount", "ROI count;Nr per Event", 256, 0, 256);
  hrawROImap  = new TH2F("hrawROImap", "ROI Map ;column;row", 256, 0, 256, 786, 0, 786);
  hrawROIDHHID = new TH1F("hrawROI DHHID", "ROI DHHID;Nr per Event", 64, 0, 64);
  hrawROItype = new TH1F("hrawROIytpe", "ROI type;Nr per Event", 3, -1, 1);
  hrawROIrow1 = new TH1F("hrawROIrow1", "ROI row1;Nr per Event", 786, 0, 786);
  hrawROIrow2 = new TH1F("hrawROIrow2", "ROI row2;Nr per Event", 786, 0, 786);
  hrawROIcol1 = new TH1F("hrawROIcol1", "ROI col1;Nr per Event", 256, 0, 256);
  hrawROIcol2 = new TH1F("hrawROIcol2", "ROI col2;Nr per Event", 256, 0, 256);
}

void pxdROIDQMModule::initialize()
{
  REG_HISTOGRAM
}

void pxdROIDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  hrawROIcount->Reset();
  hrawROImap->Reset();
  hrawROIDHHID->Reset();
  hrawROItype->Reset();
  hrawROIrow1->Reset();
  hrawROIrow2->Reset();
  hrawROIcol1->Reset();
  hrawROIcol2->Reset();
}

void pxdROIDQMModule::event()
{
  for (auto & it : m_storeROIs) {
    int nr;
    nr = it.getNrROIs();
    hrawROIcount->Fill(nr);
    for (auto j = 0; j < nr; j++) {
      hrawROImap->Fill(0., 0.);
      hrawROIDHHID->Fill(it.getDHHID(j));
      hrawROItype->Fill(it.getType(j));
      hrawROIrow1->Fill(it.getRow1(j));
      hrawROIrow2->Fill(it.getRow2(j));
      hrawROIcol1->Fill(it.getCol1(j));
      hrawROIcol2->Fill(it.getCol2(j));
    }
  }
}
