/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * CDC unpacker module                                                    *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcUnpacker/CDCUnpackerModule.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <cdc/dataobjects/CDCRawHitWaveForm.h>
// DB objects
#include <cdc/dbobjects/CDCChannelMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
// framework - Database
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>


#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>

using namespace std;
using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCUnpackerModule::CDCUnpackerModule() : Module()
{
  //Set module properties
  setDescription("CDCUnpacker generates CDCHit from Raw data.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawCDCName", m_rawCDCName, "Name of the RawCDC List name..", string(""));
  addParam("cdcRawHitWaveFormName", m_cdcRawHitWaveFormName, "Name of the CDCRawHit (Raw data mode).", string(""));
  addParam("cdcRawHitName", m_cdcRawHitName, "Name of the CDCRawHit (Suppressed mode).", string(""));
  addParam("cdcHitName", m_cdcHitName, "Name of the CDCHit List name..", string(""));
  addParam("fadcThreshold", m_fadcThreshold, "Threshold count.", 1);

  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("enableStoreCDCRawHit", m_enableStoreCDCRawHit, "Enable to store to the CDCRawHit object", false);
  addParam("enablePrintOut", m_enablePrintOut, "Enable to print out the data to the terminal", false);
  addParam("boardIDTrig", m_boardIDTrig, "Board ID for the trigger.", 7);
  addParam("channelTrig", m_channelTrig, "Channel for the trigger.", 1);
  addParam("subtractTrigTiming", m_subtractTrigTiming, "Enable to subtract the trigger timing from TDCs.", false);
  addParam("tdcOffset", m_tdcOffset, "TDC offset (in TDC count).", 0);
  addParam("enableDatabase", m_enableDatabase, "Enable database to read the channel map.", true);
  addParam("enable2ndHit", m_enable2ndHit, "Enable 2nd hit timing as a individual CDCHit object.", false);
  addParam("tdcAuxOffset", m_tdcAuxOffset, "TDC auxiliary offset (in TDC count).", 0);
  addParam("pedestalSubtraction", m_pedestalSubtraction, "Enbale ADC pedestal subtraction.", m_pedestalSubtraction);

}

CDCUnpackerModule::~CDCUnpackerModule()
{
}

void CDCUnpackerModule::initialize()
{

  m_channelMapFromDB = new DBArray<CDCChannelMap>;
  if ((*m_channelMapFromDB).isValid()) {
    B2INFO("Channel map is  valid");
  } else {
    B2FATAL("Channel map is not valid");
  }

  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: initialize() Called.");
  }


  m_rawCDCs.isRequired(m_rawCDCName);
  StoreArray<CDCRawHitWaveForm> storeCDCRawHitWFs(m_cdcRawHitWaveFormName);
  storeCDCRawHitWFs.registerInDataStore();

  StoreArray<CDCRawHit> storeCDCRawHits(m_cdcRawHitName);
  storeCDCRawHits.registerInDataStore();

  StoreArray<CDCHit> storeDigit(m_cdcHitName);
  storeDigit.registerInDataStore();

  // Relation.
  storeDigit.registerRelationTo(storeCDCRawHitWFs);
  storeDigit.registerRelationTo(storeCDCRawHits);

  // Set default names for the relations.
  m_relCDCRawHitToCDCHitName = DataStore::relationName(
                                 DataStore::arrayName<CDCRawHit>(m_cdcRawHitName),
                                 DataStore::arrayName<CDCHit>(m_cdcHitName));

  m_relCDCRawHitWFToCDCHitName = DataStore::relationName(
                                   DataStore::arrayName<CDCRawHitWaveForm>(m_cdcRawHitWaveFormName),
                                   DataStore::arrayName<CDCHit>(m_cdcHitName));

  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: FADC threshold: " << m_fadcThreshold);
  }
}

void CDCUnpackerModule::beginRun()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: beginRun() called.");
  }


  loadMap();
  setADCPedestal();
  if ((*m_channelMapFromDB).isValid()) {
    delete m_channelMapFromDB;
  }
}

void CDCUnpackerModule::event()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: event() started.");
  }

  // TDC count for the trigger scinti.
  int tdcCountTrig = m_tdcOffset;

  // Create Data objects.

  StoreArray<CDCRawHitWaveForm> cdcRawHitWFs(m_cdcRawHitWaveFormName);
  StoreArray<CDCRawHit> cdcRawHits(m_cdcRawHitName);
  StoreArray<CDCHit> cdcHits(m_cdcHitName);
  cdcHits.clear();

  RelationArray rawCDCsToCDCHits(cdcRawHits, cdcHits, m_relCDCRawHitToCDCHitName); // CDCRawHit <-> CDCHit
  RelationArray rawCDCWFsToCDCHits(cdcRawHitWFs, cdcHits, m_relCDCRawHitWFToCDCHitName); // CDCRawHitWaveForm <-> CDCHit

  if (m_enableStoreCDCRawHit == true) {
    cdcRawHits.clear();
    cdcRawHitWFs.clear();
  }

  //
  // Proccess RawCDC data block.
  //

  const int nEntries = m_rawCDCs.getEntries();

  B2DEBUG(99, "nEntries of RawCDCs : " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    const int subDetectorId = m_rawCDCs[i]->GetNodeID(0);
    const int iNode = (subDetectorId & 0xFFFFFF);
    const int nEntriesRawCDC = m_rawCDCs[i]->GetNumEntries();

    B2DEBUG(99, "nEntries of rawCDC[i] : " << nEntriesRawCDC);
    for (int j = 0; j < nEntriesRawCDC; ++j) {
      int trigType = m_rawCDCs[i]->GetTRGType(j); // Get event type of L1 trigger.
      int nWords[4];
      nWords[0] = m_rawCDCs[i]->Get1stDetectorNwords(j);
      nWords[1] = m_rawCDCs[i]->Get2ndDetectorNwords(j);
      nWords[2] = m_rawCDCs[i]->Get3rdDetectorNwords(j);
      nWords[3] = m_rawCDCs[i]->Get4thDetectorNwords(j);

      int* data32tab[4];
      data32tab[0] = (int*)m_rawCDCs[i]->Get1stDetectorBuffer(j);
      data32tab[1] = (int*)m_rawCDCs[i]->Get2ndDetectorBuffer(j);
      data32tab[2] = (int*)m_rawCDCs[i]->Get3rdDetectorBuffer(j);
      data32tab[3] = (int*)m_rawCDCs[i]->Get4thDetectorBuffer(j);



      //
      // Search Data from Finess A to D (0->3).
      //

      for (int iFiness = 0; iFiness < 4; ++iFiness) {
        int* ibuf = data32tab[iFiness];
        const int nWord = nWords[iFiness];
        B2DEBUG(99, "nWords (from COPPER header) : " << nWord);

        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : Print out CDC data block.");
          printBuffer(ibuf, nWord);
        }

        const int c_headearWords = 3;

        if (nWord < c_headearWords) {
          if (m_enablePrintOut == true) {
            B2WARNING("CDCUnpacker : No CDC block header.");
          }
          continue;
        }

        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : RawDataBlock(CDC) : Block #  " << i);
          B2INFO("CDCUnpacker : Node ID " << iNode << ", Finness ID " << iFiness);
        }

        setCDCPacketHeader(ibuf);

        const int dataType = getDataType();
        const int dataLength = getDataLength() / 4; // Data length in int word (4bytes).
        const int swDataLength = dataLength * 2;   // Data length in short word (2bytes).


        if (dataLength != (nWord - c_headearWords)) {
          B2ERROR("Inconsistent data size between COPPER and CDC FEE.");
          B2ERROR("data length " << dataLength << " nWord " << nWord);
          continue;
        }
        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : Data size " << dataLength <<  " words.");
        }

        const int board = getBoardId();
        const int trgNumber = getTriggerNumber();
        const int trgTime = getTriggerTime();

        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : Board ID " << board <<  ", Trigger number " << trgNumber << ", Trigger time " << trgTime);
        }

        //
        // Check the data type (raw or supressed mode?).
        //

        if (dataType == 1) { //  Raw data mode.
          if (m_enablePrintOut == true) {
            B2INFO("CDCUnpacker : Raw data mode.");
          }

          m_buffer.clear();

          for (int it = 0; it < dataLength; ++it) {
            int index = it + c_headearWords;

            m_buffer.push_back(static_cast<unsigned short>((ibuf[index] & 0xffff0000) >> 16));
            m_buffer.push_back(static_cast<unsigned short>(ibuf[index] & 0xffff));
          }

          const int fadcTdcChannels = 48; // Total channels of FADC or TDC.
          const int nSamples = swDataLength / (2 * fadcTdcChannels); // Number of samplings.

          std::vector<unsigned short> fadcs;
          std::vector<unsigned short> tdcs;

          for (int iCh = 0; iCh < fadcTdcChannels; ++iCh) {
            const int offset = fadcTdcChannels;
            unsigned short fadcSum = 0;     // FADC sum below thereshold.
            unsigned short tdc1 = 0x7fff;   // Fastest TDC.
            unsigned short tdc2 = 0x7fff;   // 2nd fastest TDC.

            for (int iSample = 0; iSample < nSamples; ++iSample) {
              // FADC value for each sample and channel.

              unsigned short fadc = m_buffer.at(iCh + 2 * fadcTdcChannels * iSample);

              if (fadc > m_fadcThreshold) {
                fadcSum += fadc;
              }
              // TDC count for each sample and channel.

              unsigned short tdc = m_buffer.at(iCh + 2 * fadcTdcChannels * iSample + offset) & 0x7fff;
              unsigned short tdcIsValid = (m_buffer.at(iCh + 2 * fadcTdcChannels * iSample + offset) & 0x8000) >> 15;
              if (tdcIsValid == 1) { // good tdc data.
                if (tdc > 0) { // if hit timng is 0, skip.
                  if (tdc < tdc1) {
                    tdc2 = tdc1; // 2nd fastest hit
                    tdc1 = tdc;  // fastest hit.
                  }
                }
              }

              fadcs.push_back(fadc);
              tdcs.push_back(tdc);
              if (m_enableStoreCDCRawHit == true) {
                // Store to the CDCRawHitWaveForm object.
                const unsigned short status = 0;
                cdcRawHitWFs.appendNew(status, trgNumber, iNode, iFiness, board, iCh, iSample, trgTime, fadc, tdc);
              }

            }

            if (tdc1 != 0x7fff) {
              // Store to the CDCHit object.
              const WireID  wireId = getWireID(board, iCh);

              if (trgTime < tdc1) {
                tdc1 = (trgTime | 0x8000) - tdc1;
              } else {
                tdc1 = trgTime - tdc1;
              }
              CDCHit* firstHit = cdcHits.appendNew(tdc1, fadcSum, wireId);
              if (m_enable2ndHit == true) {
                CDCHit* secondHit = cdcHits.appendNew(tdc2, fadcSum, wireId);
                secondHit->setOtherHitIndices(firstHit);
                secondHit->set2ndHitFlag();
              }
            }




            if (m_enablePrintOut == true) {
              //
              // Print out (for debug).
              //

              printf("FADC ch %2d : ", iCh);
              for (int iSample = 0; iSample < nSamples; ++iSample) {
                printf("%4x ", fadcs.at(iSample));
              }
              printf("\n");

              printf("TDC ch %2d  : ", iCh);
              for (int iSample = 0; iSample < nSamples; ++iSample) {
                printf("%4x ", tdcs.at(iSample));
              }
              printf("\n");
            }

          }

        } else if (dataType == 2) { // Suppressed mode.
          if (m_enablePrintOut == true) {
            B2INFO("CDCUnpacker : Suppressed mode.");
          }

          // convert int array -> short array.
          m_buffer.clear();
          for (int it = 0; it < dataLength; ++it) {
            int index = it + c_headearWords;
            m_buffer.push_back(static_cast<unsigned short>((ibuf[index] & 0xffff0000) >> 16));
            m_buffer.push_back(static_cast<unsigned short>(ibuf[index] & 0xffff));
          }

          const int bufSize = static_cast<int>(m_buffer.size());
          for (int it = 0; it < bufSize;) {
            unsigned short header = m_buffer.at(it);     // Header.
            unsigned short ch = (header & 0xff00) >> 8;  // Channel ID in FE.
            unsigned short length = (header & 0xff) / 2; // Data length in short word.

            if (header == 0xff02) {
              it++;
              continue;
            }

            if (!((length == 4) || (length == 5))) {
              B2ERROR("CDCUnpacker : data length should be 4 or 5 words.");
              B2ERROR("CDCUnpacker : length " << length << " words.");
              it += length;
              continue;
            }

            unsigned short tot = m_buffer.at(it + 1);     // Time over threshold.
            unsigned short fadcSum = m_buffer.at(it + 2);  // FADC sum.
            if (m_pedestalSubtraction == true) {
              int diff = fadcSum - (*m_adcPedestalFromDB)->getPedestal(board, ch);
              if (diff <= m_fadcThreshold) {
                fadcSum = 0;
              } else {
                fadcSum =  static_cast<unsigned short>(diff);
              }
            }
            unsigned short tdc1 = 0;                  // TDC count.
            unsigned short tdc2 = 0;                  // 2nd TDC count.
            unsigned short tdcFlag = 0;               // Multiple hit or not (1 for multi hits, 0 for single hit).

            if (length == 4) {
              tdc1 = m_buffer.at(it + 3);
            } else if (length == 5) {
              tdc1 = m_buffer.at(it + 3);
              tdc2 = m_buffer.at(it + 4) & 0x7fff;
              tdcFlag = (m_buffer.at(it + 4) & 0x8000) >> 15;
            } else {
              B2ERROR("CDCUnpacker : Undefined data length (should be 4 or 5 short words) ");
            }

            if (m_enablePrintOut == true) {
              printf("%4x %4x %4x %4x %4x %4x %4x \n", ch, length, tot, fadcSum, tdc1, tdc2, tdcFlag);
            }
            if (length == 4 || length == 5) {

              //              const unsigned short status = 0;
              const unsigned short status = trigType; // temporally trigger type is stored, here.
              // Store to the CDCHit.
              const WireID  wireId = getWireID(board, ch);

              if (isValidBoardChannel(wireId)) {
                if (board == m_boardIDTrig && ch == m_channelTrig) {
                  tdcCountTrig = tdc1;
                } else {
                  CDCHit* firstHit = cdcHits.appendNew(tdc1, fadcSum, wireId);
                  if (length == 5) {
                    if (m_enable2ndHit == true) {
                      CDCHit* secondHit = cdcHits.appendNew(tdc2, fadcSum, wireId);
                      secondHit->setOtherHitIndices(firstHit);
                      secondHit->set2ndHitFlag();
                    }
                  }
                }

                if (m_enableStoreCDCRawHit == true) {
                  // Store to the CDCRawHit object.
                  cdcRawHits.appendNew(status, trgNumber, iNode, iFiness, board, ch, trgTime, fadcSum, tdc1, tdc2, tot);
                }

              } else {
                B2WARNING("Undefined board id is fired: " << board << " " << ch);
              }
            }
            it += static_cast<int>(length);
          }

        } else {
          B2WARNING("CDCUnpacker :  Undefined CDC Data Block : Block #  " << i);
        }
      }
    }
  }

  //
  // t0 correction w.r.t. the timing of the trigger counter.
  //
  if (m_subtractTrigTiming == true) {
    for (auto& hit : cdcHits) {
      int tdc = hit.getTDCCount();
      if (hit.is2ndHit()) {
        if (tdc != 0) {
          tdc  = tdc - (tdcCountTrig - m_tdcOffset);
        }
      } else {
        tdc  = tdc - (tdcCountTrig - m_tdcOffset);
      }

      tdc -= m_tdcAuxOffset;
      hit.setTDCCount(static_cast<unsigned short>(tdc));
    }
  }
}

void CDCUnpackerModule::endRun()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker : End run.");
  }
}

void CDCUnpackerModule::terminate()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker : Terminated.");
  }
}


WireID CDCUnpackerModule::getWireID(int iBoard, int iCh) const
{
  return m_map[iBoard][iCh];
}

void CDCUnpackerModule::loadMap()
{

  if (m_enableDatabase == false) {

    // Read the channel map from the local text.
    std::string fileName = FileSystem::findFile(m_xmlMapFileName);
    std::cout << fileName << std::endl;
    if (fileName == "") {
      B2ERROR("CDC unpacker can't find a filename: " << fileName);
      exit(1);
    }


    ifstream ifs;
    ifs.open(fileName.c_str());
    int isl;
    int icl;
    int iw;
    int iBoard;
    int iCh;

    while (!ifs.eof()) {
      ifs >>  isl >> icl >> iw >> iBoard >> iCh;
      const WireID  wireId(isl, icl, iw);
      m_map[iBoard][iCh] = wireId;
    }
  } else {
    for (const auto& cm : (*m_channelMapFromDB)) {
      const int isl = cm.getISuperLayer();
      const int il = cm.getILayer();
      const int iw = cm.getIWire();
      const int iBoard = cm.getBoardID();
      const int iCh = cm.getBoardChannel();
      const WireID  wireId(isl, il, iw);
      m_map[iBoard][iCh] = wireId;
    }
  }
}

void CDCUnpackerModule::setADCPedestal()
{
  if (m_pedestalSubtraction == true) {
    m_adcPedestalFromDB = new DBObjPtr<CDCADCDeltaPedestals>;
    if (!(*m_adcPedestalFromDB).isValid()) {
      m_pedestalSubtraction = false;
    }
  }

}

void CDCUnpackerModule::printBuffer(int* buf, int nwords)
{

  for (int j = 0; j < nwords; ++j) {
    printf(" %.8x", buf[j]);
    if ((j + 1) % 10 == 0) {
      printf("\n");
    }
  }
  printf("\n");

  return;
}
