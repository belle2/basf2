//+
// File : svdClsHistoManagerModule.cc
// Description : A module to create histogram ROOT file
//             : for SVD clusterizer study
//
// Author : Katsuro Nakamura, KEK
// Date : 15 - May - 2014
//-

#include <svd/modules/svdDump/svdDumpModule.h>

//#include <rawdata/dataobjects/RawFTSW.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(svdDump)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Implementations
svdDumpModule::svdDumpModule() : Module(), m_event(0)
{
  // Module description
  setDescription("Module to create SVD data file");
  setPropertyFlags(Module::c_HistogramManager);

  // Parameters
  addParam("outputFileName", m_outputFileName, "Name of output file.", string("svdDumpModule.dat"));

  addParam("svdRawName", m_svdRawName, "Name of the SVD Raw", string(""));

  addParam("nFtbHeader", m_nFtbHeader, "# of FTB header", int(2));
  addParam("nFtbTrailer", m_nFtbTrailer, "# of FTB header", int(1));

}

svdDumpModule::~svdDumpModule()
{
}

void svdDumpModule::initialize()
{
  B2INFO("svdDumpModule: initialize() is called.");

  m_rawSVD.isRequired(m_svdRawName);

  m_event = 0;

  m_outputFile = new ofstream(m_outputFileName.c_str(), ios::trunc);
  if (!(*m_outputFile)) {
    B2FATAL("Output file: " << m_outputFileName.c_str() << " cannot be opened.");
  }

  return;
}

void svdDumpModule::terminate()
{
  B2INFO("svdDumpModule: terminate() is called.");

  m_outputFile->close();
  delete m_outputFile; m_outputFile = NULL;

  return;
}


void svdDumpModule::beginRun()
{
  B2INFO("svdDumpModule: beginRun() is called.");
}

void svdDumpModule::endRun()
{
  B2INFO("svdDumpModule: endRun() is called.");
}

void svdDumpModule::event()
{
  StoreArray<RawSVD> rawSVD(m_svdRawName);

  unsigned int total_nWords = 0;
  for (int i = 0; i < rawSVD.getEntries(); i++) {
    for (int j = 0; j < rawSVD[ i ]->GetNumEntries(); j++) {
      unsigned int nWords = rawSVD[i]->Get1stDetectorNwords(j);
      if (((int)nWords - m_nFtbHeader - m_nFtbTrailer) < 0) {
        B2FATAL("Remaining data size is negative: " << ((int)nWords - m_nFtbHeader - m_nFtbTrailer));
      }
      unsigned int remaining_nWords = (nWords - m_nFtbHeader - m_nFtbTrailer);
      total_nWords += remaining_nWords;
    }
  }
  uint32_t  header = (0xf << 28);
  header |= ((total_nWords & 0xffff) << 12) + (m_event & 0xfff);
  m_outputFile->write((char*)(&header), sizeof(uint32_t));

  for (int i = 0; i < rawSVD.getEntries(); i++) {
    for (int j = 0; j < rawSVD[ i ]->GetNumEntries(); j++) {

      unsigned int nWords = rawSVD[i]->Get1stDetectorNwords(j);
      unsigned int remaining_nWords = (nWords - m_nFtbHeader - m_nFtbTrailer);

      uint32_t* data32 = (uint32_t*)rawSVD[i]->Get1stDetectorBuffer(j);

      m_outputFile->write((char*)(data32 + m_nFtbHeader), sizeof(uint32_t)*remaining_nWords);

    }
  }

  m_event++;

  return;
}
