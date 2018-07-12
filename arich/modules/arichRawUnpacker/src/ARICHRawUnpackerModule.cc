/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichRawUnpacker/ARICHRawUnpackerModule.h>
#include <arich/dataobjects/ARICHRawDigit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Dataobject classes
#include <rawdata/dataobjects/RawARICH.h>

#include <framework/database/DBObjPtr.h>

#include <sstream>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHRawUnpacker);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHRawUnpackerModule::ARICHRawUnpackerModule() : HistoModule()
  {
    m_debug = false;
  }

  ARICHRawUnpackerModule::~ARICHRawUnpackerModule()
  {
  }

  void ARICHRawUnpackerModule::defineHisto()
  {
    h_rate_a_all = new TH1F("h_rate_a_all", ";Channel ID", 144 * 6, 0, 144 * 6); //yone
    h_rate_b_all = new TH1F("h_rate_b_all", ";Channel ID", 144 * 6, 0, 144 * 6); //yone
    h_rate_c_all = new TH1F("h_rate_c_all", ";Channel ID", 144 * 6, 0, 144 * 6); //yone
    h_rate_d_all = new TH1F("h_rate_d_all", ";Channel ID", 144 * 6, 0, 144 * 6); //yone
  }

  void ARICHRawUnpackerModule::initialize()
  {
    REG_HISTOGRAM;
    StoreArray<RawARICH> rawdata;
    rawdata.isRequired();
    StoreArray<ARICHRawDigit> rawdigit;
    rawdigit.registerInDataStore();
  }

  void ARICHRawUnpackerModule::beginRun()
  {
  }

  void ARICHRawUnpackerModule::event()
  {
    StoreArray<RawARICH> rawdata;
    StoreArray<ARICHRawDigit> rawdigits;
    for (auto& raw : rawdata) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buf = (const int*)raw.GetDetectorBuffer(0, finesse);
        int bufSize = raw.GetDetectorNwords(0, finesse);
        if (bufSize < 1) continue;
        m_ibyte = 0;
        // read merger header
        int type = calbyte(buf);
        int ver = calbyte(buf);
        int boardid = calbyte(buf);
        int febno = calbyte(buf);
        unsigned int length_all = calword(buf);
        unsigned int mrg_evtno = calword(buf);
        ARICHRawDigit* rawdigit = rawdigits.appendNew(type, ver, boardid, febno, length_all, mrg_evtno);
        rawdigit->setCopperId(raw.GetNodeID(0));
        rawdigit->setHslbId(finesse);
        int nfebs = 0;
        while (m_ibyte < length_all) {
          int type_feb = calbyte(buf);
          ver = calbyte(buf);
          boardid = calbyte(buf);
          febno = calbyte(buf);
          unsigned int length = calword(buf);
          int evtno = calword(buf);
          unsigned int ibyte = 0;
          std::stringstream ss;
          ss << "type=" << type_feb << ", ver=" << ver << " "
             << ", boardid=" << boardid << ", febno=" << febno
             << ", length=" << length << ", evtno=" << evtno << " ";
          bool hasHit = false;
          long long feb_trigno = 0;
          for (int i = 0; i < 10; i++) {
            int val = calbyte(buf);
            ibyte++;
            if (i < 6) {
              feb_trigno |= (0xff & val) << (5 - i) * 8;
            }
          }
          ARICHRawDigit::FEBDigit feb;
          nfebs++;
          if (type_feb == 0x02) {//Raw mode
            int ch = 143;
            //B2INFO("raw mode");
            while (ibyte < length) {
              int val = calbyte(buf);
              if (val != 0) {
                ibyte++;
                ss << "ch# " << ch << "(" << val << ") ";
                hasHit = true;
                if (febno < 0 || febno > 6) {
                  B2ERROR("FEB is bad : " << febno << " hslb-" << finesse << ":"
                          << " type=" << type_feb << ", ver=" << ver << " "
                          << ", boardid=" << boardid << ", febno=" << febno
                          << ", length=" << length << ", evtno=" << evtno);
                }
                feb.push_back(ch, val);
              }
              ch--;
              if (ch < 0) break;
            }
          } else if (type_feb == 0x01) { // Suppressed mode
            if (length > 144 * 2 + 10) B2FATAL("error " << length);
            //B2INFO("suppreed mode");
            while (ibyte < length) {
              int ch = calbyte(buf);
              ibyte++;
              int val = calbyte(buf);
              ibyte++;
              if (val != 0) {
                ss << "ch# " << ch << "(" << val << ") ";
                hasHit = true;
                if (febno < 0 || febno > 6) {
                  B2ERROR("FEB is bad : " << febno << " hslb-" << finesse << ":"
                          << " type=" << type_feb << ", ver=" << ver << " "
                          << ", boardid=" << boardid << ", febno=" << febno
                          << ", length=" << length << ", evtno=" << evtno);
                  return;
                }
                feb.push_back(ch, val);
              }
            }
          }
          rawdigit->addFEB(feb, type, ver, boardid, febno, length, evtno, feb_trigno);
          if (m_debug && hasHit) {
            B2INFO(ss.str());
          }
        }
      }
    }
  }


  void ARICHRawUnpackerModule::endRun()
  {
  }

  void ARICHRawUnpackerModule::terminate()
  {
  }

} // end Belle2 namespace

