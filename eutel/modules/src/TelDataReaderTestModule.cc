#include <eutel/modules/TelDataReaderTestModule.h>

// include standard c++
#include <iomanip>

// load the BASF2 datastore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// load the datastore objects
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <testbeam/vxd/dataobjects/TelEventInfo.h>

// include boost
#include <boost/format.hpp>

using namespace Belle2;

REG_MODULE(TelDataReaderTest);

TelDataReaderTestModule::TelDataReaderTestModule() : Module(),
  m_file(NULL),
  m_maps(),
  m_planeToId()
{
  // Module Description
  setDescription("This is a module for testing/validating the TelDataReader Module.");

  addParam("outFileName", m_outFileName,
           "File name used for writing the validation histograms.\nWill overwrite already existing files.",
           std::string("TelDataReaderValidation.root"));

  addParam("numPxCol", m_nPxCol, "Number of pixels in column direction.", static_cast<unsigned short int>(1152));
  addParam("numPxRow", m_nPxRow, "Number of pixels in row direction.", static_cast<unsigned short int>(576));
  addParam("numPlanes", m_nPlanes, "Number of planes.", static_cast<unsigned short int>(6));
  std::vector<unsigned short int> planeNos = { 0, 1, 2, 3, 4, 5 };
  addParam("planeNos", m_planeNos, "Vector of plane numbers.", planeNos);

  addParam("numBinsCol", m_nBinsCol,
           "Number of bins in column direction. \nIf this is zero, use number of pixels in column direction.",
           static_cast<unsigned short>(15));
  addParam("numBinsRow", m_nBinsRow, "Number of bins in row direction. \nIf this is zero, use number of pixels in row direction.",
           static_cast<unsigned short>(7));

}


TelDataReaderTestModule::~TelDataReaderTestModule()
{
}


void TelDataReaderTestModule::initialize()
{
  // perform parameter checks
  assert(m_planeNos.size() == m_nPlanes);

  m_file = new TFile(m_outFileName.c_str(), "RECREATE");
  if (! m_file) {
    B2FATAL("Could not open root file \"" << m_outFileName << "\" for writing.");
  }
}

void TelDataReaderTestModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2DEBUG(75, "Started operating on run " << std::setw(10) << evtMetaData->getRun());

  m_maps.resize(m_nPlanes);
  for (size_t iHisto = 0; iHisto < m_nPlanes; ++iHisto) {
    m_maps.at(iHisto) = new TH2I((boost::format("h_SignalMaps_Run%d_Plane%d") % evtMetaData->getRun() %
                                  m_planeNos[ iHisto ]).str().c_str(),
                                 (boost::format("SignalMaps: Run %d: Plane %d") % evtMetaData->getRun() % m_planeNos[ iHisto ]).str().c_str(),
                                 m_nBinsCol, -0.5, m_nPxCol - 0.5,
                                 m_nBinsRow, -0.5, m_nPxRow - 0.5);
    if (! m_maps.at(iHisto)) {
      B2FATAL("Could not create histogram.");
    }

    m_planeToId[ m_planeNos[ iHisto ] ] = iHisto;

    // disable stats box
    m_maps.at(iHisto)->SetStats(false);

    // set axis labels
    m_maps.at(iHisto)->GetXaxis()->SetTitle("Column No. [Px]");
    m_maps.at(iHisto)->GetYaxis()->SetTitle("Row No. [Px]");
    m_maps.at(iHisto)->GetZaxis()->SetTitle("Entries / Px");

    // scale axis labels to bins
    m_maps.at(iHisto)->GetXaxis()->SetNdivisions(m_nBinsCol);
    m_maps.at(iHisto)->GetYaxis()->SetNdivisions(m_nBinsRow);
  }
}

void TelDataReaderTestModule::event()
{
  // get the data from the data store
  StoreObjPtr<EventMetaData> evtMetaData;
  StoreArray<TelDigit> telDigits("TelDigits");
  StoreObjPtr<TelEventInfo> telEvtInfo("TelEventInfo");

  B2DEBUG(15, "Read event: " << std::setw(10) << evtMetaData->getEvent());
  if (evtMetaData->getEvent() % 1000 == 0) {
    B2INFO("  ---> " << std::setw(10) << evtMetaData->getEvent());
  }

  for (int iDigit = 0; iDigit < telDigits.getEntries(); ++iDigit) {
    const TelDigit* telDigit = telDigits[ iDigit ];
    unsigned short int id = m_planeToId[ telDigit->getSensorID() ];
    m_maps.at(id)->Fill(telDigit->getUCellID(), telDigit->getVCellID());
  }
}

void TelDataReaderTestModule::endRun()
{
  B2DEBUG(75, "Finished run!");

  for (size_t iHisto = 0; iHisto < m_nPlanes; ++iHisto) {
    m_maps.at(iHisto)->Write();
    delete m_maps.at(iHisto);
  }
  m_maps.clear();
}


void TelDataReaderTestModule::terminate()
{
  if (m_file) {
    m_file->Write();
    m_file->Close();
    delete m_file;
    m_file = NULL;
  }
}
