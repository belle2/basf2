/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/cdcCalibrationCollector/CDCDeadBoardDetectorModule.h>
#include <TH1F.h>
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(CDCDeadBoardDetector);

CDCDeadBoardDetectorModule::CDCDeadBoardDetectorModule() : CalibrationCollectorModule()
{
  //Set module properties
  setDescription("CDCDeadBoardDetector detects dead boards.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

CDCDeadBoardDetectorModule::~CDCDeadBoardDetectorModule()
{
}

void CDCDeadBoardDetectorModule::prepare()
{
  m_rawCDCs.isRequired();
  auto m_BoardIDs = new TH1F("CDCboardIDs", "CDC board IDs", 299,  0.5, 299.5);
  registerObject<TH1F>("CDCboardIDs", m_BoardIDs);
}

void CDCDeadBoardDetectorModule::collect()
{

  //
  // Proccess RawCDC data block.
  //

  const int nEntries = m_rawCDCs.getEntries();

  for (int i = 0; i < nEntries; ++i) {
    const int nEntriesRawCDC = m_rawCDCs[i]->GetNumEntries();
    for (int j = 0; j < nEntriesRawCDC; ++j) {
      int MaxNumOfCh = m_rawCDCs[i]->GetMaxNumOfCh(j);

      for (int iFiness = 0; iFiness < MaxNumOfCh; ++iFiness) {
        int* ibuf = (int*)m_rawCDCs[i]->GetDetectorBuffer(j, iFiness);
        const int nWord = m_rawCDCs[i]->GetDetectorNwords(j, iFiness);
        const int c_headearWords = 3;
        if (nWord < c_headearWords) {
          continue;
        }
        unsigned int boardID = (ibuf[0] & 0x01ff);
        if ((boardID > 0) && (boardID < 300)) {
          getObjectPtr<TH1F>("CDCboardIDs")->Fill(boardID);
        }
      }
    }
  }
}

