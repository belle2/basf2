/******************************************************************************
* BASF2 (Belle Analysis Framework 2)                                          *
* Copyright(C) 2010 - Belle II Collaboration                                  *
*                                                                             *
* Author: The Belle II Collaboration                                          *
* Contributors: Jarek Wiechczynski, Jacek Stypula, Peter Kvasnicka            *
*               Giulia Casarosa, Eugenio Paoloni                              *
*                                                                             *
* This software is provided "as is" without any warranty.                     *
* Beware! Do not expose to open flames it can explode                         *
******************************************************************************/

#include <svd/modules/svdUnpacker/SVDUnpackerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <arpa/inet.h>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::augmented_crc
#include <boost/cstdint.hpp>  // for boost::uint16_t
#define CRC16POLYREV 0x8005   // CRC-16 polynomial, normal representation 

#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

std::string Belle2::SVD::SVDUnpackerModule::m_xmlFileName = std::string("SVDChannelMapping.xml");

SVDUnpackerModule::SVDUnpackerModule() : Module(),
  m_generateOldDigits(false),
  m_mapping(m_xmlFileName),
  m_shutUpFTBError(0),
  m_FADCTriggerNumberOffset(0)
{
  //Set module properties
  setDescription("Produce SVDDigits from RawSVD. NOTE: only zero-suppressed mode is currently supported!");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  addParam("GenerateOldDigits", m_generateOldDigits, "Generate SVDDigits", bool(false));
  addParam("svdShaperDigitListName", m_svdShaperDigitListName, "Name of the SVDShaperDigits list", string(""));
  addParam("shutUpFTBError", m_shutUpFTBError,
           "if >0 is the number of reported FTB header ERRORs before quiet operations. If <0 full log produced.", -1);
  addParam("FADCTriggerNumberOffset", m_FADCTriggerNumberOffset,
           "number to be added to the FADC trigger number to match the main trigger number", 0);
  addParam("svdDAQDiagnosticsListName", m_svdDAQDiagnosticsListName, "Name of the DAQDiagnostics  list", string(""));
  addParam("softwarePipelineAddressEmulation", m_emulatePipelineAddress, "Estimate emulated pipeline address", bool(true));
  addParam("killDigitsFromUpsetAPVs", m_killUpsetDigits, "Delete digits from upset APVs", bool(false));
  addParam("silentlyAppend", m_silentAppend, "Append digits to a pre-existing non-empty storeArray", bool(false));
  addParam("badMappingFatal", m_badMappingFatal, "Throw B2FATAL if there's a wrong payload in the database", bool(false));
}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{
  m_eventMetaDataPtr.isRequired();
  // Don't panic if no SVD data.
  m_rawSVD.isOptional(m_rawSVDListName);
  StoreArray<SVDDAQDiagnostic> storeDAQDiagnostics(m_svdDAQDiagnosticsListName);
  storeDAQDiagnostics.registerInDataStore();
  m_svdDAQDiagnosticsListName = storeDAQDiagnostics.getName();

  if (m_generateOldDigits) {
    m_svdDigit.registerInDataStore(m_svdDigitListName);
  }

  StoreArray<SVDShaperDigit> storeShaperDigits(m_svdShaperDigitListName);
  storeShaperDigits.registerInDataStore();
  storeShaperDigits.registerRelationTo(storeDAQDiagnostics);
  m_svdShaperDigitListName = storeShaperDigits.getName();

}

void SVDUnpackerModule::beginRun()
{
  m_wrongFTBcrc = 0;
  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  //number of FADC boards
  nFADCboards = m_map->getFADCboardsNumber();

  //passing APV<->FADC mapping from SVDOnlineToOfflineMap object
  APVmap = &(m_map->APVforFADCmap);

}

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
void SVDUnpackerModule::event()
{
  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  if (!rawSVDList || !rawSVDList.getEntries())
    return;
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  StoreArray<SVDShaperDigit> shaperDigits(m_svdShaperDigitListName);
  StoreArray<SVDDAQDiagnostic> DAQDiagnostics(m_svdDAQDiagnosticsListName);

  if (!m_silentAppend && m_generateOldDigits && svdDigits && svdDigits.getEntries())
    B2FATAL("Unpacking SVDDigits to a non-empty pre-existing StoreArray.\n"
            << "This can lead to undesired behaviour. At least remember to\""
            << "use SVDDigitSorter in your path and set the \n"
            << "silentlyAppend parameter of SVDUnpacker to true.");

  if (!m_silentAppend && shaperDigits && shaperDigits.getEntries())
    B2FATAL("Unpacking SVDShaperDigits to a non-empty pre-existing \n"
            << "StoreArray. This can lead to undesired behaviour. At least\n"
            << "remember to use SVDShaperDigitSorter in your path and \n"
            << "set the silentlyAppend parameter of SVDUnpacker to true.");

  SVDDAQDiagnostic* currentDAQDiagnostic;
  vector<SVDDAQDiagnostic*> vDiagnostic_ptr;

  map<SVDShaperDigit, SVDDAQDiagnostic*> diagnosticMap;
  // Store encountered pipeline addresses with APVs in which they were observed
  map<unsigned short, set<pair<unsigned short, unsigned short> > > apvsByPipeline;

  if (!m_eventMetaDataPtr.isValid()) {  // give up...
    B2ERROR("Missing valid EventMetaData." << std::endl <<
            "No SVDDigit produced for this event");
    return;
  }

  if (! m_map) { //give up
    B2ERROR("SVD xml map not loaded." << std::endl <<
            "No SVDDigit produced for this event");
    return;
  }

  bool nFADCmatch = true;
  bool nAPVmatch = true;
  bool badMapping = false;
  bool badHeader = false;
  bool badTrailer = false;

  unsigned short nAPVheaders = 999;
  set<short> seenAPVHeaders = {};

  unsigned short nEntries_rawSVD = rawSVDList.getEntries();

  if (nEntries_rawSVD != nFADCboards) {
    B2WARNING("Number of RawSVD data objects do not match the number of FADC boards" << LogVar("#RawSVD",
              nEntries_rawSVD)  << LogVar("#FADCs", nFADCboards) << LogVar("Event number", m_eventMetaDataPtr->getEvent()));

    nFADCmatch = false;
  }

  for (unsigned int i = 0; i < nEntries_rawSVD; i++) {

    unsigned int numEntries_rawSVD = rawSVDList[ i ]->GetNumEntries();
    for (unsigned int j = 0; j < numEntries_rawSVD; j++) {

      unsigned short nWords[4];
      nWords[0] = rawSVDList[i]->Get1stDetectorNwords(j);
      nWords[1] = rawSVDList[i]->Get2ndDetectorNwords(j);
      nWords[2] = rawSVDList[i]->Get3rdDetectorNwords(j);
      nWords[3] = rawSVDList[i]->Get4thDetectorNwords(j);

      uint32_t* data32tab[4]; //vector of pointers

      data32tab[0] = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j); // points at the begining of the 1st buffer
      data32tab[1] = (uint32_t*)rawSVDList[i]->Get2ndDetectorBuffer(j);
      data32tab[2] = (uint32_t*)rawSVDList[i]->Get3rdDetectorBuffer(j);
      data32tab[3] = (uint32_t*)rawSVDList[i]->Get4thDetectorBuffer(j);


      unsigned short ftbError = 0;
      unsigned short trgType = 0;
      unsigned short trgNumber = 0;
      unsigned short cmc1;
      unsigned short cmc2;
      unsigned short apvErrors;
      unsigned short pipAddr;
      unsigned short ftbFlags;
      unsigned short apvErrorsOR;

      bool is3sampleData = false;

      for (unsigned int buf = 0; buf < 4; buf++) { // loop over 4 buffers

        //if (data32tab[buf] != NULL && &data32tab[buf][nWords[buf] - 1] != NULL)                              printB2Debug(data32tab[buf], data32tab[buf], &data32tab[buf][nWords[buf] - 1], nWords[buf]);

        uint32_t* data32_it = data32tab[buf];
        short fadc = 255, apv = 63, strip, sample[6];
        vector<uint32_t> crc16vec;


        for (; data32_it != &data32tab[buf][nWords[buf]]; data32_it++) {
          m_data32 = *data32_it; //put current 32-bit frame to union


          if (m_data32 == 0xffaa0000) {   // first part of FTB header
            crc16vec.clear(); // clear the input container for crc16 calculation
            crc16vec.push_back(m_data32);

            data32_it++; // go to 2nd part of FTB header
            crc16vec.push_back(*data32_it);

            m_data32 = *data32_it; //put the second 32-bit frame to union

            ftbError = m_FTBHeader.errorsField;

            if (ftbError != 240) {
              switch (ftbError - 240) {
                case 3:
                  B2WARNING("FADC Event Number is different from (FTB & TTD) Event Numbers");
                  break;
                case 5:
                  B2WARNING("TTD Event Number is different from (FTB & FADC) Event Numbers");
                  break;
                case 6:
                  B2WARNING("FTB Event Number is different from (TTD & FADC) Event Numbers");
                  break;
                case 7:
                  B2WARNING("(FTB, TTD & FADC) Event Numbers are different from each other");
                  break;
                default:
                  B2WARNING("Problem with errorsField variable in FTB Header" << LogVar("abnormal value", ftbError));
              }
            }

            if (m_FTBHeader.eventNumber !=
                (m_eventMetaDataPtr->getEvent() & 0xFFFFFF)) {
              if (m_shutUpFTBError) { //
                m_shutUpFTBError -= 1 ;

                B2ERROR("Trigger number mismatch detected!" << LogVar("Expected trigger number & 0xFFFFFF",
                                                                      (m_eventMetaDataPtr->getEvent() & 0xFFFFFF)) << LogVar("Trigger number in the FTB", m_FTBHeader.eventNumber));

              }
            }

            continue;
          }


          crc16vec.push_back(m_data32);


          if (m_MainHeader.check == 6) { // FADC header
            fadc = m_MainHeader.FADCnum;
            trgType = m_MainHeader.trgType;
            trgNumber = m_MainHeader.trgNumber;

            nAPVheaders = 0; // start counting APV headers for this FADC
            nAPVmatch = true; //assume correct # of APV headers
            badMapping = false; //assume correct mapping
            badHeader = false;
            badTrailer = false;

            is3sampleData = false;
            if (m_MainHeader.DAQMode == 1) is3sampleData = true;

            if (
              m_MainHeader.trgNumber !=
              ((m_eventMetaDataPtr->getEvent() - m_FADCTriggerNumberOffset) & 0xFF)) {
              B2ERROR(" Found a wrong FTB header of the SVD FADC " << LogVar("Event number", m_eventMetaDataPtr->getEvent()) << LogVar("FADC",
                      fadc) << LogVar("Trigger number LSByte reported by the FADC", m_MainHeader.trgNumber) << LogVar("+ offset",
                          m_FADCTriggerNumberOffset) << LogVar("expected", (m_eventMetaDataPtr->getEvent() & 0xFF)));
              badHeader = true;
              currentDAQDiagnostic = DAQDiagnostics.appendNew(0, 0, 0, 0, 0, 0, ftbError, nFADCmatch, nAPVmatch, badHeader, fadc, 0);
              vDiagnostic_ptr.push_back(currentDAQDiagnostic);
            }

            // create SVDModeByte object from MainHeader vars
            m_SVDModeByte = SVDModeByte(m_MainHeader.runType, m_MainHeader.evtType, m_MainHeader.DAQMode, m_MainHeader.trgTiming);
          }

          if (m_APVHeader.check == 2) { // APV header

            nAPVheaders++;
            apv = m_APVHeader.APVnum;
            seenAPVHeaders.insert(apv);

            cmc1 = m_APVHeader.CMC1;
            cmc2 = m_APVHeader.CMC2;
            apvErrors = m_APVHeader.apvErr;
            pipAddr = m_APVHeader.pipelineAddr;

            if (apvErrors != 0) B2WARNING("APV error has been detected." << LogVar("FADC", fadc) << LogVar("APV", apv) << LogVar("Error value",
                                            apvErrors));

            // temporary SVDDAQDiagnostic object (no info from trailers and APVmatch code)
            currentDAQDiagnostic = DAQDiagnostics.appendNew(trgNumber, trgType, pipAddr, cmc1, cmc2, apvErrors, ftbError, nFADCmatch, nAPVmatch,
                                                            badHeader,
                                                            fadc, apv);
            vDiagnostic_ptr.push_back(currentDAQDiagnostic);

            apvsByPipeline[pipAddr].insert(make_pair(fadc, apv));
          }

          if (m_data_A.check == 0) { // data
            strip = m_data_A.stripNum;

            sample[0] = m_data_A.sample1;
            sample[1] = m_data_A.sample2;
            sample[2] = m_data_A.sample3;


            sample[3] = 0;
            sample[4] = 0;
            sample[5] = 0;

            if (not is3sampleData) {

              data32_it++;
              m_data32 = *data32_it; // 2nd frame with data
              crc16vec.push_back(m_data32);

              sample[3] = m_data_B.sample4;
              sample[4] = m_data_B.sample5;
              sample[5] = m_data_B.sample6;
            }

            if (m_generateOldDigits) {
              for (unsigned int idat = 0; idat < 6; idat++) {
                // m_cellPosition member of the SVDDigit object is set to zero by NewDigit function
                SVDDigit* newDigit = m_map->NewDigit(fadc, apv, strip, sample[idat], idat);
                if (newDigit) {
                  svdDigits.appendNew(*newDigit);
                  delete newDigit;
                }
              }
            }

            // Generating SVDShaperDigit object
            SVDShaperDigit* newShaperDigit = m_map->NewShaperDigit(fadc, apv, strip, sample, 0.0, m_SVDModeByte);
            if (newShaperDigit) {
              diagnosticMap.insert(make_pair(*newShaperDigit, currentDAQDiagnostic));
              delete newShaperDigit;
            } else if (m_badMappingFatal) {
              B2FATAL("Respective FADC/APV combination not found -->> incorrect payload in the database! ");
            } else {
              badMapping = true;
            }

          }  //is data frame


          if (m_FADCTrailer.check == 14)  { // FADC trailer

            //additional check if we have a faulty/fake FADC that is not in the map
            if (APVmap->find(fadc) == APVmap->end()) badMapping = true;

            //comparing number of APV chips and the number of APV headers, for the current FADC
            unsigned short nAPVs = APVmap->count(fadc);

            if (nAPVheaders == 0) {
              currentDAQDiagnostic = DAQDiagnostics.appendNew(0, 0, 0, 0, 0, 0, ftbError, nFADCmatch, nAPVmatch, badHeader, fadc, 0);
              vDiagnostic_ptr.push_back(currentDAQDiagnostic);
            }

            if (nAPVs != nAPVheaders) {
              // There is an APV missing, detect which it is.
              for (const auto& fadcApv : *APVmap) {
                if (fadcApv.first != fadc) continue;
                if (seenAPVHeaders.find(fadcApv.second) == seenAPVHeaders.end()) {
                  // We have a missing APV. Look if it is a known one.
                  auto eventNo = m_eventMetaDataPtr->getEvent();
                  auto missingRec = m_missingAPVs.find(make_pair(fadcApv.first, fadcApv.second));
                  if (missingRec != m_missingAPVs.end()) {
                    // This is known to be missing, so keep quiet and just update event counters
                    if (missingRec->second.first > eventNo)
                      missingRec->second.first = eventNo;
                    if (missingRec->second.second < eventNo)
                      missingRec->second.second = eventNo;
                  } else {
                    // We haven't seen this previously.
                    m_missingAPVs.insert(make_pair(
                                           make_pair(fadcApv.first, fadcApv.second),
                                           make_pair(eventNo, eventNo)
                                         ));
                    B2WARNING("missing APV header! " << LogVar("Event number", eventNo) << LogVar("APV", int(fadcApv.second)) << LogVar("FADC",
                              int(fadcApv.first)));
                  }
                }
              }
              nAPVmatch = false;
            }
            seenAPVHeaders.clear();

            ftbFlags = m_FADCTrailer.FTBFlags;
            if ((ftbFlags >> 5) != 0) badTrailer = true;
            if (ftbFlags != 0) {
              B2WARNING(" FTB Flags variable has an active error bit(s)" << LogVar("on FADC number", fadc));

              if (ftbFlags & 16) B2WARNING("----> CRC error has been detected. Data might be corrupted!");
              if (ftbFlags & 8) B2WARNING("----> Bad Event indication has been detected. Data might be corrupted!");
              if (ftbFlags & 4) B2WARNING("----> Double Header has been detected. Data might be corrupted!");
              if (ftbFlags & 2) B2WARNING("----> Time Out has been detected. Data might be corrupted!");
              if (ftbFlags & 1) B2WARNING("----> Event Too Long! Data might be corrupted!");
            }

            apvErrorsOR = m_FADCTrailer.apvErrOR;
            for (auto p : vDiagnostic_ptr) {
              // adding remaining info to Diagnostic object
              p->setFTBFlags(ftbFlags);
              p->setApvErrorOR(apvErrorsOR);
              p->setAPVMatch(nAPVmatch);
              p->setBadMapping(badMapping);
              p->setBadTrailer(badTrailer);
              vDiagnostic_ptr.clear();
            }

          }// FADC trailer

          if (m_FTBTrailer.controlWord == 0xff55)  {// FTB trailer

            //check CRC16
            crc16vec.pop_back();
            unsigned short iCRC = crc16vec.size();
            uint32_t crc16input[iCRC];

            for (unsigned short icrc = 0; icrc < iCRC; icrc++)
              crc16input[icrc] = htonl(crc16vec.at(icrc));

            //verify CRC16
            boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
            bcrc.process_block(crc16input, crc16input + iCRC);
            unsigned int checkCRC = bcrc.checksum();

            if (checkCRC != m_FTBTrailer.crc16) {
              B2WARNING("FTB CRC16 checksum DOES NOT MATCH" << LogVar("for FADC no.", fadc));
              m_wrongFTBcrc++;
            }

          } // FTB trailer

        } // end loop over 32-bit frames in each buffer

      } // end iteration on 4 data buffers

      //m_runType = 0;

    } // end event loop

  }// end loop over RawSVD objects

  // Detect upset APVs and report/treat
  auto major_apv = max_element(apvsByPipeline.begin(), apvsByPipeline.end(),
                               [](const decltype(apvsByPipeline)::value_type & p1,
                                  const decltype(apvsByPipeline)::value_type & p2) -> bool
  { return p1.second.size() < p2.second.size(); }
                              );
  // We set emuPipelineAddress fields in diagnostics to this.
  if (m_emulatePipelineAddress)
    for (auto& p : DAQDiagnostics)
      p.setEmuPipelineAddress(major_apv->first);
  // And report any upset apvs or update records
  if (apvsByPipeline.size() > 1)
    for (const auto& p : apvsByPipeline) {
      if (p.first == major_apv->first) continue;
      for (const auto& fadcApv : p.second) {
        // We have an upset APV. Look if it is a known one.
        auto eventNo = m_eventMetaDataPtr->getEvent();
        auto upsetRec = m_upsetAPVs.find(make_pair(fadcApv.first, fadcApv.second));
        if (upsetRec != m_upsetAPVs.end()) {
          // This is known to be upset, so keep quiet and update event counters
          if (upsetRec->second.first > eventNo)
            upsetRec->second.first = eventNo;
          if (upsetRec->second.second < eventNo)
            upsetRec->second.second = eventNo;
        } else {
          // We haven't seen this one previously.
          m_upsetAPVs.insert(make_pair(
                               make_pair(fadcApv.first, fadcApv.second),
                               make_pair(eventNo, eventNo)
                             ));
          for (auto& pp : DAQDiagnostics) {

            if (pp.getFADCNumber() == fadcApv.first and pp.getAPVNumber() == fadcApv.second)
              pp.setUpsetAPV(true);
          }
          B2WARNING("Upset APV detected!!!" << LogVar("APV", int(fadcApv.second)) << LogVar("FADC",
                    int(fadcApv.first)) << LogVar("Event number", eventNo));
        }
      }
    }

  // Here we can delete digits coming from upset APVs. We detect them by comparing
  // actual and emulated pipeline address fields in DAQDiagnostics.
  for (auto& p : diagnosticMap) {

    if ((m_killUpsetDigits && p.second->getPipelineAddress() != p.second->getEmuPipelineAddress()) || p.second->getFTBError() != 240
        || p.second->getFTBFlags()) continue;
    shaperDigits.appendNew(p.first)->addRelationTo(p.second);
  }

} //end event function
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

void SVDUnpackerModule::endRun()
{
  // Summary report on missing APVs
  if (m_missingAPVs.size() > 0) {
    B2WARNING("SVDUnpacker summary 1: Missing APVs");
    for (const auto& miss : m_missingAPVs)
      B2WARNING(LogVar("Missing APV", miss.first.second) << LogVar("FADC", miss.first.first) << LogVar("since event",
                miss.second.first) << LogVar("to event", miss.second.second));
  }
  if (m_upsetAPVs.size() > 0) {
    B2WARNING("SVDUnpacker summary 2: Upset APVs");
    for (const auto& upst : m_upsetAPVs)
      B2WARNING(LogVar("Upset APV", upst.first.second) << LogVar("FADC", upst.first.first) <<
                LogVar("since event", upst.second.first) << LogVar("to event", upst.second.second));
  }
}


// additional printing function
void SVDUnpackerModule::printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords)
{

  uint32_t* min = std::max((data32 - nWords), data32_min);
  uint32_t* max = std::min((data32 + nWords), data32_max);

  size_t counter{0};
  std::stringstream os;
  os << std::hex << std::setfill('0');
  for (uint32_t* ptr = min; ptr <= max; ++ptr) {
    os << std::setw(8) << *ptr;
    if (++counter % 10 == 0) os << std::endl;
    else os << " ";
  }

  os << std::endl;
  B2INFO(os.str());
  return;

}
