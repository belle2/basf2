/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma GCC diagnostic ignored "-Wstack-usage="

#include <pxd/modules/pxdUnpacking/PXDUnpackerDQMModule.h>

#include "TDirectory.h"
#include <TAxis.h>
#include <string>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDUnpackerDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDUnpackerDQMModule::PXDUnpackerDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("Monitor Unpacker errors");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdUnpackerError"));
  addParam("showStats", m_showStats, "If set statistics will be shown", false);
  addParam("RunName", m_runName, "Name of the processed run");
}

void PXDUnpackerDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  m_sizeOfErrorBlock = errors.get_Nrerrors();

  hUnpackErrorCount = new TH1F("hUnpackErrorCount", m_runName.c_str() , m_sizeOfErrorBlock, 0, m_sizeOfErrorBlock);
  hUnpackErrorCountWeight = new TH2F("hUnpackErrorCountWeight", m_runName.c_str() , 4, 0, 4, m_sizeOfErrorBlock, 0,
                                     m_sizeOfErrorBlock);
  oldDir->cd();
}

void PXDUnpackerDQMModule::initialize()
{
  REG_HISTOGRAM
  m_sizeOfErrorBlock = errors.get_Nrerrors();
  B2INFO("Size of error block UNPACKER DQM " << m_sizeOfErrorBlock);
  for (int i = 0 ; i < m_sizeOfErrorBlock ; i++) {
    err.push_back(errors.get_errTypes(i));
    B2INFO("Errors listed in Unpacker [" << i << "] " << err[i]);
  }
}

void PXDUnpackerDQMModule::beginRun()
{
  hUnpackErrorCount->Reset();
  hUnpackErrorCountWeight->Reset();
}

void PXDUnpackerDQMModule::event()
{
  const char* str_ptr[m_sizeOfErrorBlock];
  const char* errW;
  int error = 0;
  for (int i = 0 ; i < m_sizeOfErrorBlock ; i++) {
    error = errors.get_errors(i);
    errW = errors.get_errWeight(i).c_str();
    str_ptr[i] = err[i].c_str();
    if (error != 0) {
      hUnpackErrorCountWeight->Fill(errW, str_ptr[i], 1); //1 displays values as bar, 2 and bigger display points with error bars
      hUnpackErrorCount->Fill(str_ptr[i], 1);
    }
    error = 0;
  }
  hUnpackErrorCount->LabelsDeflate("X");
  hUnpackErrorCount->LabelsOption("v");

  hUnpackErrorCountWeight->LabelsDeflate("X");
  hUnpackErrorCountWeight->LabelsDeflate("Y");
  hUnpackErrorCountWeight->LabelsOption("v");
  if (!m_showStats) {hUnpackErrorCount->SetStats(0);}
}
