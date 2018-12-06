/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/modules/pxdDQM/PXDRawDQMCorrModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <vector>
#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDQMCorr)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawDQMCorrModule::PXDRawDQMCorrModule() : HistoModule(), m_storeRawHits()
{
  //Set module properties
  setDescription("PXD DQM Correlation module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdrawcorr"));
}


PXDRawDQMCorrModule::~PXDRawDQMCorrModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDRawDQMCorrModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  int nPixelsU1 = 0;
  int nPixelsV1 = 0;
  int nPixelsU2 = 250;
  int nPixelsV2 = 768;
  m_CorrelationU = new TH2F("RawCorrelationU", "Raw Correlation of U;U1/cells;U2/cells", nPixelsU1 + nPixelsU2, nPixelsU1, nPixelsU2,
                            nPixelsU1 + nPixelsU2, nPixelsU1, nPixelsU2);
  m_CorrelationV = new TH2F("RawCorrelationV", "Raw Correlation of V;V1/cells;V2/cells", nPixelsV1 + nPixelsV2, nPixelsV1, nPixelsV2,
                            nPixelsV1 + nPixelsV2, nPixelsV1, nPixelsV2);
  m_DeltaU = new TH1F("RawDeltaU", "Raw Correlation of U2-U1;Udiff/cells", 2 * nPixelsU1 + 2 * nPixelsU2, -nPixelsU2 + nPixelsU1,
                      nPixelsU2 - nPixelsU1);
  m_DeltaV = new TH1F("RawDeltaV", "Raw Correlation of V2-V1;Vdiff/cells", 2 * nPixelsV1 + 2 * nPixelsV2, -nPixelsV2 + nPixelsV1,
                      nPixelsV2 - nPixelsV1);

  m_In1CorrelationU = new TH2F("RawIn1CorrelationU", "Raw In1 Correlation of U;U1/cells;U2/cells", nPixelsU1 + nPixelsU2, nPixelsU1,
                               nPixelsU2, nPixelsU1 + nPixelsU2, nPixelsU1, nPixelsU2);
  m_In1CorrelationV = new TH2F("RawIn1CorrelationV", "Raw In1 Correlation of V;V1/cells;V2/cells", nPixelsV1 + nPixelsV2, nPixelsV1,
                               nPixelsV2, nPixelsV1 + nPixelsV2, nPixelsV1, nPixelsV2);
  m_In1DeltaU = new TH1F("RawIn1DeltaU", "Raw In1 Correlation of U2-U1;Udiff/cells", 2 * nPixelsU1 + 2 * nPixelsU2,
                         -nPixelsU2 + nPixelsU1, nPixelsU2 - nPixelsU1);
  m_In1DeltaV = new TH1F("RawIn1DeltaV", "Raw In1 Correlation of V2-V1;Vdiff/cells", 2 * nPixelsV1 + 2 * nPixelsV2,
                         -nPixelsV2 + nPixelsV1, nPixelsV2 - nPixelsV1);

  m_In2CorrelationU = new TH2F("RawIn2CorrelationU", "Raw In2 Correlation of U;U1/cells;U2/cells", nPixelsU1 + nPixelsU2, nPixelsU1,
                               nPixelsU2, nPixelsU1 + nPixelsU2, nPixelsU1, nPixelsU2);
  m_In2CorrelationV = new TH2F("RawIn2CorrelationV", "Raw In2 Correlation of V;V1/cells;V2/cells", nPixelsV1 + nPixelsV2, nPixelsV1,
                               nPixelsV2, nPixelsV1 + nPixelsV2, nPixelsV1, nPixelsV2);
  m_In2DeltaU = new TH1F("RawIn2DeltaU", "Raw In2 Correlation of U2-U1;Udiff/cells", 2 * nPixelsU1 + 2 * nPixelsU2,
                         -nPixelsU2 + nPixelsU1, nPixelsU2 - nPixelsU1);
  m_In2DeltaV = new TH1F("RawIn2DeltaV", "Raw In2 Correlation of V2-V1;Vdiff/cells", 2 * nPixelsV1 + 2 * nPixelsV2,
                         -nPixelsV2 + nPixelsV1, nPixelsV2 - nPixelsV1);

  // cd back to root directory
  oldDir->cd();
}


void PXDRawDQMCorrModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  m_storeRawHits.isRequired(m_storeRawHitsName);
}

void PXDRawDQMCorrModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_CorrelationU->Reset();
  m_CorrelationV->Reset();
  m_DeltaU->Reset();
  m_DeltaV->Reset();

  m_In1CorrelationU->Reset();
  m_In1CorrelationV->Reset();
  m_In1DeltaU->Reset();
  m_In1DeltaV->Reset();

  m_In2CorrelationU->Reset();
  m_In2CorrelationV->Reset();
  m_In2DeltaU->Reset();
  m_In2DeltaV->Reset();

}


void PXDRawDQMCorrModule::event()
{
  for (auto& hit1 : m_storeRawHits) {
    int iPlane1 = hit1.getSensorID().getLayerNumber();
    if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
    int index1 = planeToIndex(iPlane1);
    if (index1 == 0) {
      for (auto& hit2 : m_storeRawHits) {
        int iPlane2 = hit2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        int index2 = planeToIndex(iPlane2);
        if (index2 == 1) {
          m_CorrelationU->Fill(hit1.getColumn(), hit2.getColumn());
          m_CorrelationV->Fill(hit1.getRow(), hit2.getRow());
          m_DeltaU->Fill(hit2.getColumn() - hit1.getColumn());
          m_DeltaV->Fill(hit2.getRow() - hit1.getRow());
        } else { // index2=0
          if (hit1.getColumn() != hit2.getColumn()) {
            m_In1CorrelationU->Fill(hit1.getColumn(), hit2.getColumn());
            m_In1DeltaU->Fill(hit2.getColumn() - hit1.getColumn());
          }
          if (hit1.getRow() !=  hit2.getRow()) {
            m_In1CorrelationV->Fill(hit1.getRow(), hit2.getRow());
            m_In1DeltaV->Fill(hit2.getRow() - hit1.getRow());
          }
        }
      }
    } else { // index 1=1
      for (auto& hit2 : m_storeRawHits) {
        int iPlane2 = hit2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        int index2 = planeToIndex(iPlane2);
        if (index2 == 1) {
          if (hit1.getColumn() != hit2.getColumn()) {
            m_In2CorrelationU->Fill(hit1.getColumn(), hit2.getColumn());
            m_In2DeltaU->Fill(hit2.getColumn() - hit1.getColumn());
          }
          if (hit1.getRow() !=  hit2.getRow()) {
            m_In2CorrelationV->Fill(hit1.getRow(), hit2.getRow());
            m_In2DeltaV->Fill(hit2.getRow() - hit1.getRow());
          }
        }
      }
    }
  }
}


void PXDRawDQMCorrModule::endRun()
{
}


void PXDRawDQMCorrModule::terminate()
{
}
