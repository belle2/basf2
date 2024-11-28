/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/cdcUnpacker/CDCUnpackerModule.h>
// DB objects
#include <cdc/dbobjects/CDCChannelMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
// framework - Database
#include <framework/database/DBArray.h>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCUnpacker);

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
  addParam("relationRawHits", m_relationRawHits, "Enbale relation of CDCHits, CDCRawHits, and CDCRawHitWaveForms.", false);
  addParam("recoverBoardIdError", m_recoverBoardIdError, "Recover boardID errors", true);
}

CDCUnpackerModule::~CDCUnpackerModule()
{
}

void CDCUnpackerModule::initialize()
{
  m_dataLengthError = false;
  m_dataSizeError = false;
  m_channelMapFromDB = new DBArray<CDCChannelMap>;
  if ((*m_channelMapFromDB).isValid()) {
    //    B2INFO("Channel map is  valid");
  } else {
    B2FATAL("Channel map is not valid");
  }

  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: initialize() Called.");
  }

  m_rawCDCs.isRequired(m_rawCDCName);
  m_CDCRawHitWaveForms.registerInDataStore(m_cdcRawHitWaveFormName);
  m_CDCRawHits.registerInDataStore(m_cdcRawHitName);
  m_CDCHits.registerInDataStore(m_cdcHitName);

  if (m_relationRawHits == true) {
    m_CDCHits.registerRelationTo(m_CDCRawHitWaveForms);
    m_CDCHits.registerRelationTo(m_CDCRawHits);

    // Set default names for the relations.
    m_relCDCRawHitToCDCHitName = DataStore::relationName(
                                   DataStore::arrayName<CDCRawHit>(m_cdcRawHitName),
                                   DataStore::arrayName<CDCHit>(m_cdcHitName));

    m_relCDCRawHitWFToCDCHitName = DataStore::relationName(
                                     DataStore::arrayName<CDCRawHitWaveForm>(m_cdcRawHitWaveFormName),
                                     DataStore::arrayName<CDCHit>(m_cdcHitName));
  }

  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: " << LogVar("FADC threshold", m_fadcThreshold));
  }

  TrackFindingCDC::CDCWireTopology::getInstance();
}

void CDCUnpackerModule::beginRun()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: beginRun() called.");
  }


  loadMap();
  setADCPedestal();
}

void CDCUnpackerModule::event()
{
  if (m_enablePrintOut == true) {
    B2INFO("CDCUnpacker: event() started.");
  }

  // TDC count for the trigger scinti.
  int tdcCountTrig = m_tdcOffset;

  const TrackFindingCDC::CDCWireTopology& wireTopology = TrackFindingCDC::CDCWireTopology::getInstance();

  // Create Data objects.
  m_CDCHits.clear();

  if (m_relationRawHits == true) {
    RelationArray rawCDCsToCDCHits(m_CDCRawHits, m_CDCHits, m_relCDCRawHitToCDCHitName); // CDCRawHit <-> CDCHit
    RelationArray rawCDCWFsToCDCHits(m_CDCRawHitWaveForms, m_CDCHits, m_relCDCRawHitWFToCDCHitName); // CDCRawHitWaveForm <-> CDCHit
  }

  if (m_enableStoreCDCRawHit == true) {
    m_CDCRawHits.clear();
    m_CDCRawHitWaveForms.clear();
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

    B2DEBUG(99, LogVar("nEntries of rawCDC[i]", nEntriesRawCDC));

    for (int j = 0; j < nEntriesRawCDC; ++j) {
      int trigType = m_rawCDCs[i]->GetTRGType(j); // Get event type of L1 trigger.
      int nWords[48];
      int* data32tab[48];
      int MaxNumOfCh = m_rawCDCs[i]->GetMaxNumOfCh(j);
      string readoutName;
      if (MaxNumOfCh == 4) readoutName = "COPPER";
      else if (MaxNumOfCh == 48) readoutName = "PCIe40";
      else
        B2FATAL("CDC UnpackerModule: Invalid value of GetMaxNumOfCh from raw data: " << LogVar("Number of ch: ",
                m_rawCDCs[i]->GetMaxNumOfCh(j)));

      for (int k = 0; k < MaxNumOfCh; ++k) {
        nWords[k] = m_rawCDCs[i]->GetDetectorNwords(j, k);
        if (MaxNumOfCh == 48 && m_rawCDCs[i]->CheckOnlineRemovedDataBit(j, k) == true) { //for error flag in ff55 trailer
          if (nWords[k] != 0)
            B2FATAL("The data is not removed for the bad channel (" << j << "," << k << ") with error flag in ff55 trailer! ");
        }
        data32tab[k] = (int*)m_rawCDCs[i]->GetDetectorBuffer(j, k);
      }

      //
      // Search Data from Finess 0->MaxNumOfCh (4/48 for COPPER/PCIe40).
      //

      for (int iFiness = 0; iFiness < MaxNumOfCh; ++iFiness) {
        int* ibuf = data32tab[iFiness];
        const int nWord = nWords[iFiness];
        B2DEBUG(99, LogVar("nWords (from " + readoutName + " header)", nWord));

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
          B2INFO("CDCUnpacker : RawDataBlock(CDC) : Block #  "
                 << LogVar("Block", i)
                 << LogVar("Node", iNode)
                 << LogVar("Finness", iFiness));
        }

        setCDCPacketHeader(ibuf);

        // Skip invalid boardsIDs
        if (m_boardId > 300) {
          B2WARNING("Invalid board " << std::hex << m_boardId << std::dec << " readout buffer block: " << i << " block channel: " << iFiness);
          if (m_recoverBoardIdError) {
            m_boardId = m_boardId & 0x01ff;
            if (m_boardId > 300) {
              B2WARNING("Unrecoverable board " << std::hex << m_boardId);
              continue;
            }
          } else {
            continue;
          }
        }

        const int dataType = getDataType();
        const int dataLength = getDataLength() / 4; // Data length in int word (4bytes).
        const int swDataLength = dataLength * 2;   // Data length in short word (2bytes).


        if (dataLength != (nWord - c_headearWords)) {
          if (m_dataSizeError == false) {
            B2ERROR("Inconsistent data size between " + readoutName + " and CDC FEE."
                    << LogVar("data length", dataLength) << LogVar("nWord", nWord)
                    << LogVar("Node ID", iNode) << LogVar("Finness ID", iFiness));
            m_dataSizeError = true;
          } else {
            B2WARNING("Inconsistent data size between " + readoutName + " and CDC FEE."
                      << LogVar("data length", dataLength) << LogVar("nWord", nWord)
                      << LogVar("Node ID", iNode) << LogVar("Finness ID", iFiness));
          }
          continue;
        }
        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : " << LogVar("Data size", dataLength));
        }

        const int board = getBoardId();
        const int trgNumber = getTriggerNumber();
        const int trgTime = getTriggerTime();

        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : " << LogVar("Board", board) <<  LogVar("Trigger number", trgNumber)
                 << LogVar("Trigger time ", trgTime));
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
                m_CDCRawHitWaveForms.appendNew(status, trgNumber, iNode, iFiness, board, iCh, iSample, trgTime, fadc, tdc);
              }

            }

            if (tdc1 != 0x7fff) {
              // Store to the CDCHit object.
              const WireID  wireId = getWireID(board, iCh);
              if (wireTopology.isValidWireID(wireId)) {
                if (trgTime < tdc1) {
                  tdc1 = (trgTime | 0x8000) - tdc1;
                } else {
                  tdc1 = trgTime - tdc1;
                }
                CDCHit* firstHit = m_CDCHits.appendNew(tdc1, fadcSum, wireId);
                if (m_enable2ndHit == true) {
                  CDCHit* secondHit = m_CDCHits.appendNew(tdc2, fadcSum, wireId);
                  secondHit->setOtherHitIndices(firstHit);
                  secondHit->set2ndHitFlag();
                }
                if (m_enableStoreCDCRawHit == true) {
                  if (m_relationRawHits == true) {
                    for (int iSample = 0; iSample < nSamples; ++iSample) {
                      m_CDCHits[m_CDCHits.getEntries() - 1]->addRelationTo(m_CDCRawHitWaveForms[m_CDCRawHitWaveForms.getEntries() - 1 + iSample -
                                                                           (nSamples - 1) ]);
                    }
                  }
                }
              } else {
                B2WARNING("Skip invalid wire board, channel: " << board << ", " << iCh);
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

          const size_t bufSize = m_buffer.size();
          for (size_t it = 0; it < bufSize;) {
            unsigned short header = m_buffer.at(it);     // Header.
            unsigned short ch = (header & 0xff00) >> 8;  // Channel ID in FE.
            unsigned short length = (header & 0xff) / 2; // Data length in short word.

            if (header == 0xff02) {
              it++;
              continue;
            }

            if (!((length == 4) || (length == 5))) {
              if (m_dataLengthError == false) {
                B2ERROR("CDCUnpacker : data length should be 4 or 5 words."
                        << LogVar("data length", length)
                        << LogVar("board id", board)
                        << LogVar("channel", ch));
                m_dataLengthError = true;
              } else {
                B2WARNING("CDCUnpacker : data length should be 4 or 5 words."
                          << LogVar("data length", length)
                          << LogVar("board id", board)
                          << LogVar("channel", ch));
              }
              break;
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

            if (ch >= 48) {
              B2WARNING("Invalid channel "
                        << LogVar("channel", ch)
                        << LogVar("board", board)
                        << LogVar("buffer total size", m_buffer.size())
                        << LogVar("length", length)
                        << LogVar("tdc", tdc1)
                        << LogVar("adc", fadcSum)
                        << LogVar("tot", tot)
                       );
              it += length;
              continue;
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
                if (wireTopology.isValidWireID(wireId)) {

                  if (board == m_boardIDTrig && ch == m_channelTrig) {
                    tdcCountTrig = tdc1;
                  } else {
                    CDCHit* firstHit = m_CDCHits.appendNew(tdc1, fadcSum, wireId,
                                                           0, tot);
                    if (length == 5) {
                      if (m_enable2ndHit == true) {
                        CDCHit* secondHit = m_CDCHits.appendNew(tdc2, fadcSum, wireId,
                                                                0, tot);
                        secondHit->setOtherHitIndices(firstHit);
                        secondHit->set2ndHitFlag();
                      }
                    }
                  }

                  if (m_enableStoreCDCRawHit == true) {
                    // Store to the CDCRawHit object.
                    if (m_relationRawHits == true) {
                      CDCRawHit* rawHit = m_CDCRawHits.appendNew(status, trgNumber, iNode, iFiness, board, ch,
                                                                 trgTime, fadcSum, tdc1, tdc2, tot);
                      m_CDCHits[m_CDCHits.getEntries() - 1]->addRelationTo(rawHit);
                      if (m_enable2ndHit == true) {
                        m_CDCHits[m_CDCHits.getEntries() - 2]->addRelationTo(rawHit);
                      }
                    } else {
                      m_CDCRawHits.appendNew(status, trgNumber, iNode, iFiness, board, ch,
                                             trgTime, fadcSum, tdc1, tdc2, tot);
                    }
                  }
                } else {
                  B2WARNING("Skip invalid wire board, channel: " << board << ", " << ch);
                }
              } else {
                B2WARNING("Undefined board id is fired: " << LogVar("board id", board) << " " << LogVar("channel", ch));
              }
            }
            it += length;
          }

        } else {
          B2WARNING("CDCUnpacker :  Undefined CDC Data Block : Block #  " << LogVar("block id", i));
        }
      }
    }
  }

  //
  // t0 correction w.r.t. the timing of the trigger counter.
  //
  if (m_subtractTrigTiming == true) {
    for (auto& hit : m_CDCHits) {
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

  if (m_channelMapFromDB) delete m_channelMapFromDB;
  if (m_adcPedestalFromDB) delete m_adcPedestalFromDB;
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
      B2ERROR("CDC unpacker can't find a filename: " << LogVar("file name", fileName));
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
