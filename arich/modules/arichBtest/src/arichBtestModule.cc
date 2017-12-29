/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/modules/arichBtest/arichBtestModule.h>

//include <boost/format.hpp>
//include <boost/foreach.hpp>


// Framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>


#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>


// Hit classes
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include "arich/geometry/ARICHGeometryPar.h"
#include "arich/geometry/ARICHBtestGeometryPar.h"

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <arich/geometry/ARICHGeometryPar.h>
#include "arich/modules/arichBtest/arichBtestData.h"

#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TRandom.h>
#include <TVector3.h>
#include <TAxis.h>

#include <libxml/xmlreader.h>


// ifstream constructor.
#include <iostream>
#include <fstream>

const char* record_strings[] = { "Event", "Begin_run", "Pause", "Resume", "End_run" };

using namespace std;

namespace Belle2 {


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
  REG_MODULE(arichBtest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


  arichBtestModule::arichBtestModule() : Module(), m_end(0), m_events(0), m_file(NULL), m_mwpc(NULL)
  {
    //Set module properties
    setDescription("Module for the ARICH Beamtest data analysis. It creates track form the MWPC hits and reads the HAPD hits");

    //Parameter definition
    addParam("outputFileName", m_outFile, "Output Root Filename", string("output.root"));
    vector<string> defaultList;
    addParam("runList", m_runList, "Data Filenames.", defaultList);
    vector<int> defaultMask;
    addParam("mwpcTrackMask", m_MwpcTrackMask, "Create track from MWPC layers", defaultMask);
    m_fp = NULL;
    addParam("beamMomentum", m_beamMomentum, "Momentum of the beam [GeV]", 0.0);

    for (int i = 0; i < 32; i++) m_tdc[i] = 0;

  }

  TNtuple* m_tuple; /**< ntuple containing hapd hits */
  TH1F* hapd[6];    /**< histogram of hits for each hapd */
  TH1F* mwpc_tdc[4][5]; /**< tdc information from mwpcs */
  TH1F* mwpc_diff[4][2]; /**< tdc difference from mwpcs */
  TH1F* mwpc_sum[4][2];  /**< tdc sum from mwpcs */
  TH1F* mwpc_sum_cut[4][2]; /**< tdc sum from mwpcs, with sum cut applied */
  TH1F* mwpc_residuals[4][2]; /**< residuals from mwpcs */
  TH2F* mwpc_xy[4];           /**< calculated x-y track positions */
  TH2F* mwpc_residualsz[4][2]; /**< z-residuals from mwpcs */
  //TGraph* m_hapdmap;
  //TGraph* m_el2pos;

  void arichBtestModule::initialize()
  {
    B2INFO("arichBtestModule::initialize()");
    StoreArray<ARICHAeroHit> aeroHits;
    StoreArray<ARICHDigit> digits;
    aeroHits.registerInDataStore();
    digits.registerInDataStore();

    m_file = new TFile(m_outFile.c_str(), "RECREATE");
    m_tuple = new TNtuple("nt", "Btest data hits", "x:y:xrec:yrec:m:c:gx:gy");
    char hapdName[256];
    for (int i = 0; i < 6; i++) {
      sprintf(hapdName, "hapd%d", i);
      hapd[i] = new TH1F(hapdName, hapdName, 145, -0.5, 144.5);
    }
    char name[256];

    for (int i = 0; i < 4; i++) {
      for (int k = 0; k < 2; k++) {
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_diff[i][k]     = new TH1F(strcat(name, "diff"), name, 300, -150, 150);
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_sum[i][k]      = new TH1F(strcat(name, "sum"), name, 200, -0.5, 199.5);
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_sum_cut[i][k]      = new TH1F(strcat(name, "sum_cut"), name, 200, -0.5, 199.5);
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_residuals[i][k] = new TH1F(strcat(name, "resd"), name, 200, -100, 100);
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_residualsz[i][k] = new TH2F(strcat(name, "resd_z"), name, 200, -25, 25, 400, -1000, 1000);
      }
      for (int k = 0; k < 5; k++) {
        sprintf(name, "mwpc%d_a%d_", i, k);
        mwpc_tdc[i][k]     = new TH1F(strcat(name, "tdc"), name, 1024, -1024, 1024);
      }
      sprintf(name, "mwpc%d_", i);
      mwpc_xy[i] = new TH2F(strcat(name, "xy"), name, 120, -30, 30, 120, -30, 30);
    }
    //m_hapdmap = new TGraph("arich/modules/arichBtest/geometry/hapd.map");
    //m_el2pos = new TGraph("arich/modules/arichBtest/geometry/hapdchmap_v0.dat");

    /*
     arich::ARICHGeometryPar* _arichgp = arich::ARICHGeometryPar::Instance();

     ofstream dout;
     dout.open ("ChannelCenterGlob.txt");
     for (int i=0;i<6;i++){
      for (int k=0;k<144;k++){
        TVector3 r = _arichgp->getChannelCenterGlob(i + 1, k);
        dout  << r.x() << " " << r.y() << endl;
      }
     }
     dout.close();
     */
    time(&m_timestart);
  }

  void arichBtestModule::beginRun()
  {

    B2INFO("arichBtestModule::beginRun()");

    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    B2INFO("arichBtestModule::eventMetaDataPtr run:" << eventMetaDataPtr->getRun());
    B2INFO("arichBtestModule::eventMetaDataPtr exp:" << eventMetaDataPtr->getExperiment());

    ARICHBtestGeometryPar* _arichbtgp = ARICHBtestGeometryPar::Instance();
    m_mwpc = _arichbtgp->getMwpc();

    static int first = 1;
    if (first) {
      m_runCurrent = m_runList.begin();
      first = 0;
      m_mwpc[0].Print();
      m_mwpc[1].Print();
      m_mwpc[2].Print();
      m_mwpc[3].Print();

    }
    m_end = 1;

    if (m_runCurrent < m_runList.end()) {
      if (m_fp) {
        m_eveList.push_back(m_events);
        gzclose(m_fp);
      }
      m_fp = gzopen((*m_runCurrent).c_str(), "rb");
      if (m_fp == NULL) {
        B2INFO("Cannot open " << *m_runCurrent);
        m_end = 1;
      } else {
        B2INFO("File opened " << *m_runCurrent);
        m_end = 0;
      }
    }
    m_events = 0;
  }

  int arichBtestModule::skipdata(gzFile fp)
  {
    unsigned int u;
    gzread(fp, &u, sizeof(unsigned int));
    gzseek(fp, u * sizeof(unsigned int), SEEK_CUR);
    return u + 1;
  }

  void arichBtestModule::readmwpc(unsigned int* dbuf, unsigned int len)
  {

    const int print1290 = 0;
    const int unsigned MAXV1290 = 32;

    unsigned int edge;
    unsigned int tdcch;
    unsigned int tdcl;
    unsigned int nhits;

    for (int i = 0; i < 32; i++) m_tdc[i] = 0XFFFF;
    //TDC V1290
    //for (int i=0;i<len;i++)   printf("V1290 %d(%d) 0x%08x \n",i,len, dbuf[i],n);
    for (unsigned int i = 0; i < len; i++) {
      int rid = (dbuf[i] >> 27) & 0x1F;
      switch (rid) {
        case 0x18:   // Filler
          if (print1290) printf("Filler  0x%08x %u.data\n", dbuf[i], i);
          break;
        case 0x8:   // Global Header
          if (print1290) printf("Global Header  0x%08x %u.data\n", dbuf[i], i);
          break;
        case 0x10:  // Global Trailer --- Last word of data
          nhits = ((dbuf[i] >> 5) & 0xFFFF); //Word Count = bit 31...21< Word Count: 20 ... 5 > 4...0
          if (print1290) printf("Global Trailer  0x%08x %u.data  STATUS=0x%03x nhits=%u\n", dbuf[i], i, (dbuf[i] >> 24) & 0x7, nhits);

          if (nhits != len) {
            if (print1290) printf("V1290 nhits!=len %u %d\n", nhits, len);
          };
          break;
        case 0x11:  // Global Trigger TimeTag
          if (print1290) printf("Global Trigger TimeTag  0x%08x %u.data\n", dbuf[i], i);
          break;
        case 0x1:   // TDC header
          if (print1290) printf("TDC header  0x%08x %u.data evid=%d wc=%d\n", dbuf[i], i, (dbuf[i] >> 12) & 0xFFF, dbuf[i] & 0xFFF);

          break;
        case 0x3:   // TDC trailer
          if (print1290) printf("TDC trailer  0x%08x %u.data\n", dbuf[i], i);

          break;
        case 0x4:   // TDC Error
          if (print1290) printf("TDC Error  0x%08x %u.data ERR=0x%x\n", dbuf[i], i, dbuf[i] & 0x3FFF);

          break;
        case 0x0  :   // TDC data

          edge  = (dbuf[i] >> 26) & 0x1;
          tdcch = (dbuf[i] >> 21) & 0x1F;
          tdcl  =   dbuf[i] & 0x1FFFFF;
          if (tdcch < MAXV1290) {
            //if (tdcch>15) gV1290[tdcch]->Fill(tdcl/40);
            //if (tdcch>15) if (tdcl< trg[tdcch-16] && tdcl>16000 && tdcl<20000 ) trg[tdcch-16]=tdcl;
            if (print1290)
              printf("V1290 0x%08x %u. [ch=%2u] edge=%u data=%u \n", dbuf[i], i, tdcch, edge, tdcl);
            m_tdc[tdcch] = tdcl / 40;
          }

          break;

        default:
          if (print1290) printf("Unknown  0x%08x %u.data\n", dbuf[i], i);

          break;

      }
    }

  }

  int arichBtestModule::readhapd(unsigned int len, unsigned int* data)
  {

    ARICHGeometryPar* _arichgp = ARICHGeometryPar::Instance();
    ARICHBtestGeometryPar* _arichbtgp = ARICHBtestGeometryPar::Instance();
    //-----------------------------------------------------

    int bmask = 0xF;

    for (int module = 0; module < 6; module++) {
      int istart = 19 * module;
      int istop = 19 * module + 18;
      for (int i = istart; i < istop; i++) {
        for (int j = 0; j < 8; j++) {
          unsigned int kdata = data[i] & (bmask << (j * 4));
          if (kdata) {
            int channelID = j + 8 * (i - istart);

            if (_arichgp->isActive(module,  channelID)) {

              hapd[module]->Fill(channelID);

              StoreArray<ARICHDigit> arichDigits;
              double globalTime = 0;

              double rposx = 0, rposy = 0;
              pair<int, int> eposhapd(_arichbtgp->GetHapdElectronicMap(module * 144 + channelID));
              int channel = eposhapd.second;

              if ((channel < 108 && channel > 71) || channel < 36) channel = 108 - (int(channel / 6) * 2 + 1) * 6 +
                    channel;
              else channel = 144 - (int((channel - 36) / 6) * 2 + 1) * 6 + channel - 36;
              TVector2 loc = _arichgp->getChannelCenterLoc(channel);
              if (abs(loc.X()) > 2.3 || abs(loc.Y()) > 2.3) continue;

              arichDigits.appendNew(module + 1, channel, globalTime);

              TVector3 rechit = _arichgp->getChannelCenterGlob(module + 1, channel);
              pair<double, double> poshapd(_arichbtgp->GetHapdChannelPosition(module * 144 + channelID));
              m_tuple ->Fill(-poshapd.first, poshapd.second, rechit.x(), rechit.y(), module, channelID, rposx, rposy);
            }
          }
        }
      }
    }

    return len;
  }


  int arichBtestModule::getTrack(int mask, TVector3& r, TVector3& dir)
  {
    int retval = 0;
    //const int trgch = 13;
    const double  t0 = 0;// m_tdc[trgch];
    for (int i = 0; i < 4; i++) {
      ARICHTracking* w = &m_mwpc[i];

      for (int k = 0; k < 4; k++) mwpc_tdc[i][k]->Fill(m_tdc[w->tdc[k]] - t0);
      mwpc_tdc[i][4]->Fill(m_tdc[w->atdc] - t0);


      for (int k = 0; k < 2; k++) {  // axis x,y
        w->status[k] = 1;
        w->diff[k] = m_tdc[w->tdc[2 * k]]  - m_tdc[w->tdc[2 * k + 1]];
        w->sum[k] = m_tdc[w->tdc[2 * k + 1]] + m_tdc[w->tdc[2 * k]] - 2 * m_tdc[w->atdc];
        mwpc_sum[i][k]->Fill(w->sum[k]);
        if (w->sum[k] < w->cutll[k] || w->sum[k] > w->cutul[k]) continue;
        mwpc_sum_cut[i][k]->Fill(w->sum[k]);
        w->status[k] = 0;
        w->reco[k] = w->diff[k] * w->slp[k] + w->offset[k];
        w->reco[k] += w->pos[k];

        mwpc_diff[i][k]->Fill(w->diff[k]);
      }

      w->reco[2] = w->pos[2]; // update z axis
      if (!w->status[0] &&  !w->status[1])  mwpc_xy[i]->Fill(w->reco[0], w->reco[1]);

      if (mask & (1 << i)) {
        if (!w->status[0] &&  !w->status[1]) {
          // add point to a fitter
        } else {
          retval = 1;
        }
      }
    }

    // replace by fitter
    int ii[4] = {0, 1, 0, 0};
    int ncnt = 0;
    for (int i = 0; i < 4; i++) {
      if (mask & (1 << i)) {
        //B2INFO(ncnt << " " << i << " Mask " << mask);
        ii[ncnt++] = i;
      }
    }
    int i0 = ii[0];
    int i1 = ii[1];

    r.SetXYZ(m_mwpc[i0].reco[1], m_mwpc[i0].reco[0], m_mwpc[i0].reco[2]);
    dir.SetXYZ(m_mwpc[i1].reco[1] - m_mwpc[i0].reco[1],
               m_mwpc[i1].reco[0] - m_mwpc[i0].reco[0],
               m_mwpc[i1].reco[2] - m_mwpc[i0].reco[2]);

    dir = dir.Unit();

// end replace by fitter
    if (dir.z() != 0) {
      for (int i = 0; i < 4; i++) {
        ARICHTracking* w = &m_mwpc[i];
        double l = (w->reco[2] - r.z()) / dir.z() ;
        TVector3 rext = r + dir * l;
        if (!w->status[0])  mwpc_residuals[i][0]->Fill(w->reco[0] - rext.y());
        if (!w->status[1])  mwpc_residuals[i][1]->Fill(w->reco[1] - rext.x());

        TAxis* axis =  mwpc_residualsz[i][1]->GetYaxis();
        for (int k = 0; k < axis->GetNbins(); k++) {
          double ll = (w->reco[2] + axis->GetBinCenter(k + 1) - r.z()) / dir.z();
          TVector3 rextt = r + dir * ll;
          mwpc_residualsz[i][0]->Fill(w->reco[0] - rextt.y(), axis->GetBinCenter(k + 1));
          mwpc_residualsz[i][1]->Fill(w->reco[1] - rextt.x(), axis->GetBinCenter(k + 1));

        }
      }
    }

    return retval;
  }

  int arichBtestModule::readdata(gzFile fp, int rec_id, int)
  {

    unsigned int len, data[10000];
    gzread(fp, &len, sizeof(unsigned int));
    //if (print) printf( "[%3d]   %d: ", len, rec_id );
    gzread(fp, data, sizeof(unsigned int)*len);

    TVector3 r;
    TVector3 dir;
    if (rec_id == 1) {
      readmwpc(data, len);
      int retval = getTrack(*(m_MwpcTrackMask.begin()), r, dir);
      //dir = TVector3(0,0,1);

      if (!retval) {
        // global transf, add track to datastore
        StoreArray<ARICHAeroHit> arichAeroHits;

        int particleId = 11;// geant4
        dir *= m_beamMomentum * Unit::GeV;
        r *= Unit::mm /*/ CLHEP::mm*/;
        static ARICHBtestGeometryPar* _arichbtgp = ARICHBtestGeometryPar::Instance();
        static TVector3 dr =  _arichbtgp->getTrackingShift();

        r += dr;

        //----------------------------------------
        // Track rotation
        //
        TRotation rot  =  _arichbtgp->getFrameRotation();
        TVector3  rc   =  _arichbtgp->getRotationCenter();

        TVector3 rrel  =  rc - rot * rc;
        r = rot * r + rrel;
        dir = rot * dir;
        r.SetX(-r.X()); dir.SetX(-dir.X());

        //
        // end track rotation
        //----------------------------------------
        r[1]  = -r.y();
        dir[1] = -dir.y();
        B2DEBUG(50, "-----------> " <<  rc.x() <<  " " << rc.y() << " " <<   rc.z() << "::::" << rrel.x() <<  " " << rrel.y() << " " <<
                rrel.z()  << " ----> R " <<   r.x() <<  " " << r.y() << " " <<   r.z() << " ----> S " <<   dir.x() <<  " " << dir.y() << " " <<
                dir.z());

        // Add new ARIHCAeroHit to datastore
        arichAeroHits.appendNew(particleId, r, dir);

      }
    }

    if (rec_id == 2) {
      readhapd(len, data);
    }

    for (unsigned int j = 0; j < len; j++) {
      //if( j%8==0 && j!= 0 ) printf( "       " );
      //printf( " %08x", data[j] );
      //if( j%8==7 || j==len-1 ) putchar('\n');
    }
    return len + 1;
  }




  void arichBtestModule::event()
  {





    if (!m_fp) return;
    unsigned int hdr[2];
    EventRec rec;
    BeginRec beginrec;
    BeginRec endrec;
    static char msg[1024];
    int len, type;

    const int sint = sizeof(unsigned int);
    do {
      if (gzread(m_fp, hdr, sint * 2) != 2 * sint  && m_end) {
        B2INFO("[" << m_events << "] End of file: " << *m_runCurrent);
        ++m_runCurrent;
        if (m_runCurrent == m_runList.end()) {
          StoreObjPtr<EventMetaData> eventMetaDataPtr;
          eventMetaDataPtr->setEndOfData();
          return;
        }
        beginRun();
      }

    } while (m_end &&  m_runCurrent != m_runList.end());
    if (m_events % 1000 == 0)  {
      time_t m_time;
      time(&m_time);
      B2INFO("neve= [" << m_events << "] in " << (double)(m_time - m_timestart) / 60. << " min (" << int(
               m_time - m_timestart) << "s) from " << *m_runCurrent);

    }
    m_events++;

    type = hdr[0];
    len  = hdr[1];
    int print = 0;
    if (print) {
      sprintf(msg, "type = %d, len = %d sint=%d", type, len , sint);
      B2INFO(msg);
    }
    gzseek(m_fp, -2 * sizeof(unsigned int), SEEK_CUR);
    switch (type) {
      case BEGIN_RECORD_TYPE: {
        gzread(m_fp, &beginrec, sizeof(beginrec));
        time_t t = beginrec.time;
        sprintf(msg, "BeginRec run %u time %s", beginrec.runno, ctime(&t));
        B2INFO(msg);
        break;
      }
      case END_RECORD_TYPE: {
        gzread(m_fp, &endrec, sizeof(endrec));
        time_t t = endrec.time;
        sprintf(msg, "EndRec run %u time %s", endrec.runno, ctime(&t));
        B2INFO(msg);
        break;
      }
      case EVENT_RECORD_TYPE: {
        gzread(m_fp, &rec, sizeof(rec));
        print = !(rec.evtno % 10000);
        time_t t = rec.time;
        if (print) {
          sprintf(msg, "EventRec run %u evt %u mstime %u, time %s", rec.runno, rec.evtno, rec.mstime, ctime(&t));
          B2INFO(msg);
        }
        /* if you just want to jump to the end */
        int pos = gztell(m_fp);
        /* try to read inside data */
        int buflen = rec.len - sizeof(rec) / sizeof(int);

        //if (print) printf ("%d: buflen\n",buflen);
        int n[5] = { 0 };
        int i(0), j(0);
        while (i < buflen && j < 5) {
          n[j] = readdata(m_fp, j, print);
          //      n[j] = skipdata( m_fp );
          i += n[j];
          j++;
        }
        if (gzseek(m_fp, pos + buflen * sizeof(unsigned int), SEEK_SET) < 0) B2ERROR("gzseek returns -1 ");
        break;
      }
      default:
        B2ERROR("IO error unknown record type " << type);
        break;
    }
    if (gzeof(m_fp)) m_end = 1;
  }

  void arichBtestModule::endRun()
  {
    B2INFO(" arichBtestModule: End Run !!!");

    m_file->Write();
    m_file->Close();

    if (m_fp) {
      gzclose(m_fp);
      m_eveList.push_back(m_events);
    }
  }

  void arichBtestModule::terminate()
  {
    int j = 1;
    BOOST_FOREACH(const string & fname, m_runList) {
      B2INFO(m_eveList[j] << " events processed from file " << fname);
      j++;
    }
    for (int i = 0; i < 4; i++) {
      //ARICHTracking* w = &m_mwpc[i];
      B2INFO(i << " a1=" << m_mwpc[i].tdc[0] << " a2="  << m_mwpc[i].tdc[1] << " a3=" << m_mwpc[i].tdc[2] << " a2="  << m_mwpc[i].tdc[3]
             << " A=" << m_mwpc[i].atdc);
    }

  }

}
