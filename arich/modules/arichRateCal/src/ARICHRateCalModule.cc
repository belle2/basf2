/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <arich/modules/arichRateCal/ARICHRateCalModule.h>
#include <arich/dataobjects/ARICHThParam.h>
#include <arich/dataobjects/ARICHRawDigit.h>
#include <arich/dataobjects/ARICHInfo.h>

// framework - DataStore
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

/*
#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
*/

#include <sstream>
#include <TH2F.h>

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHRateCal);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHRateCalModule::ARICHRateCalModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("Fills ARICHHits collection from ARICHDigits");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("nrun", m_nrun, "# of scan runs", 100);
    addParam("nevents", m_nevents, "# of events per run", 1000);
    double v = 0;
    addParam("dth", m_dth, "dth", v);
    addParam("th0", m_th0, "th0", v);
    addParam("debug", m_debugmode, "debug mode", false);
    addParam("internal", m_internalmode, "Internal thscan mode", false);
    addParam("daqdb", m_daqdb, "daqdb config name", std::string(""));
  }

  ARICHRateCalModule::~ARICHRateCalModule()
  {
  }

  void ARICHRateCalModule::defineHisto()
  {
    /*
    if (m_daqdb.size() > 0) {
      ConfigFile config("slowcontrol");
      PostgreSQLInterface db(config.get("database.host"),
                             config.get("database.dbname"),
                             config.get("database.user"),
                             config.get("database.password"),
                             config.getInt("database.port"));
      DBObject obj = DBObjectLoader::load(db, "arich_th",
                                          StringUtil::replace(m_daqdb, ".", ":"));
      obj.print();
      m_dth = obj.getFloat("dth");
      m_th0 = obj.getFloat("th0");
      m_nrun = obj.getInt("nth");
      db.close();
    }
    */

    if (m_dth == 0) {
      B2WARNING("dth is set to 0");
    }
    for (int i = 0; i < 100; i++) {
      h_rate2D[i] = new TH2F(Form("h_rate2D_%d", i), Form("MRG#%d;Channel ID; Vth [mV]", i), 144 * 6, -0.5, -0.5 + 144 * 6,
                             m_nrun, (m_th0 - 0.5 * m_dth) * 1000, (m_th0 + (m_nrun - 0.5)*m_dth) * 1000);
    }
  }

  void ARICHRateCalModule::initialize()
  {

    REG_HISTOGRAM
    StoreArray<ARICHRawDigit> rawdata;
    rawdata.isRequired();
    //StoreArray<RawARICH> rawdata;
    //rawdata.isRequired();
  }

  void ARICHRateCalModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtmetadata;
    //int expno = evtmetadata->getExperiment();
    int runno = evtmetadata->getRun();
    if (runno == 100 && !m_internalmode) {
      terminate();
    }
  }

  void ARICHRateCalModule::event()
  {
    StoreObjPtr<EventMetaData> evtmetadata;
    StoreObjPtr<ARICHInfo> arichinfo;
    if (!arichinfo->getthscan_mode()) return;
    double vth_thscan = arichinfo->getvth_thscan();

    //int runno = m_internalmode ? m_run_count : evtmetadata->getRun();
    //int raw_evtno = m_internalmode ? m_evt_count : evtmetadata->getEvent();
    int runno = evtmetadata->getRun();
    //int raw_evtno = evtmetadata->getEvent();

    ARICHThParam param(runno, m_dth, m_th0, m_nrun);
    StoreArray<ARICHRawDigit> rawdigits;
    for (auto& rawdigit : rawdigits) {
      const int mrgid = rawdigit.getBoardId();
      //const int nfebs = rawdigit.getNFEBs();
      //B2INFO("MB="<<mrgid<<" nfeb="<<nfebs);
      std::vector<ARICHRawDigit::FEBDigit>& febs(rawdigit.getFEBs());
      for (auto& feb : febs) {
        const int febno = feb.febno;
        std::vector<ARICHRawDigit::FEBDigit::ChannelDigit>& channels(feb());
        for (auto& channel : channels) {
          if (channel.val > 0) {
            //B2INFO("MB="<<mrgid<<" ch="<< channel.chno);
            double vth = m_internalmode ? vth_thscan * 1000 : param.getVth() * 1000 ;
            h_rate2D[mrgid]->Fill(channel.chno + febno * 144, vth);
          }
        }
      }
    }

    // evt,run counter for internal thscan mode
    m_evt_count++;
    if (m_evt_count == m_nevents) {
      m_evt_count = 0;
      m_run_count++;
    }

  }

} // end Belle2 namespace
