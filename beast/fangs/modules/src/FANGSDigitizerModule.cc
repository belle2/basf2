/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/fangs/modules/FANGSDigitizerModule.h>
#include <beast/fangs/dataobjects/FANGSSimHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
//#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace Belle2;
using namespace fangs;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FANGSDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FANGSDigitizerModule::FANGSDigitizerModule() : Module()
{
  // Set module properties
  setDescription("FANGS digitizer module");

  //Default values are set here. New values can be in FANGS.xml.
  addParam("LowerTimingCut", m_lowerTimingCut, "Lower timing cut", 0.);
  addParam("UpperTimingCut", m_upperTimingCut, "Upper timing cut", 1000000.);
}

FANGSDigitizerModule::~FANGSDigitizerModule()
{
}

void FANGSDigitizerModule::initialize()
{
  B2INFO("FANGSDigitizer: Initializing");
  m_fangsHit.registerInDataStore();

  //get xml data
  getXMLData();

  //converter: electron number to TOT part I
  fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);
  //converter: electron number to TOT part II
  fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);
}

void FANGSDigitizerModule::beginRun()
{
}

void FANGSDigitizerModule::event()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<FANGSSimHit> FANGSSimHits;
  m_nFANGS = 15;
  std::vector<double> T0(m_nFANGS,
                         m_upperTimingCut);

  //Find the start time
  for (const auto& FANGSSimHit : FANGSSimHits) {
    const int lad = FANGSSimHit.getLadder();
    const int sen = FANGSSimHit.getSensor();
    const int detNb = (lad - 1) * 5 + sen - 1;
    const TVector3 trackPosition =  FANGSSimHit.getLocalPosEntry();
    const double z = trackPosition.Y() + m_sensor_width / 2.; //cm
    if (z < T0[detNb]) {
      T0[detNb] = z;
    }
  }

  for (auto& val : T0) {
    if (m_lowerTimingCut < val && val < m_upperTimingCut) {
      val = val / m_v_sensor; //cm / (cm / ns) = ns
    } else {
      val = -1.;
    }
  }

  //loop on all entries to store in 3D the ionization for each FANGS
  for (const auto& FANGSSimHit : FANGSSimHits) {

    const int lad = FANGSSimHit.getLadder();
    const int sen = FANGSSimHit.getSensor();
    const int detNb = (lad - 1) * 5 + sen - 1;
    const TVector3 simHitPosition =  FANGSSimHit.getLocalPosEntry();
    const double edep = FANGSSimHit.getEnergyDep() * 1e9; //GeV to eV

    const TVector3 chipPosition(//cm
      simHitPosition.X(),
      simHitPosition.Z(),
      simHitPosition.Y() + m_sensor_width / 2.);

    //If new detector filled the chip
    if (olddetNb != detNb && m_dchip_map.size() > 0) {
      Pixelization();
      olddetNb = detNb;
      m_dchip_map.clear();
      m_dchip.clear();
      m_dchip_detNb_map.clear();
      m_dchip_pdg_map.clear();
      m_dchip_trkID_map.clear();
    }

    //check if ionization within sensitive volume
    if ((-m_ChipColumnX < chipPosition.X() && chipPosition.X() < m_ChipColumnX) &&
        (-m_ChipRowY < chipPosition.Y() && chipPosition.Y() <  m_ChipRowY) &&
        (0. < chipPosition.Z() && chipPosition.Z() <  m_sensor_width) &&
        (m_lowerTimingCut < T0[detNb] && T0[detNb] < m_upperTimingCut)) {

      if (edep <  m_Workfct) break;
      ////////////////////////////////
      // check if enough energy to ionize
      else if (edep >  m_Workfct) {

        const double meanEl = edep / m_Workfct;
        const double sigma = sqrt(m_Fanofac * meanEl);
        const int NbEle = (int)gRandom->Gaus(meanEl, sigma);

        int col = (int)((chipPosition.X() + m_ChipColumnX) / (2. * m_ChipColumnX / (double)m_ChipColumnNb));
        int row = (int)((chipPosition.Y() + m_ChipRowY) / (2. * m_ChipRowY / (double)m_ChipRowNb));
        int pix = col +  m_ChipColumnNb * row;
        int quT = gRandom->Uniform(-1, 1);
        int bci = (int)((chipPosition.Z() / m_v_sensor - T0[detNb]) / (double)m_PixelTimeBin) + quT;
        if (bci < 0)bci = 0;

        //check if amplified drifted electron are within pixel boundaries
        if ((0 <= col && col < m_ChipColumnNb) &&
            (0 <= row && row < m_ChipRowNb) &&
            (0 <= pix && pix < m_ChipColumnNb * m_ChipRowNb)  &&
            (0 <= bci && bci < MAXtSIZE)) {
          //store info into 3D array for each FANGSs
          m_dchip_map[std::tuple<int, int>(col, row)] = 1;
          m_dchip_detNb_map[std::tuple<int, int>(col, row)] = detNb;
          m_dchip_pdg_map[std::tuple<int, int>(col, row)] = FANGSSimHit.getPDG();
          m_dchip_trkID_map[std::tuple<int, int>(col, row)] = FANGSSimHit.gettrkID();
          m_dchip[std::tuple<int, int, int>(col, row, bci)] += (int)(NbEle);
        }
      }
    }
  }

  if (m_dchip_map.size() > 0) Pixelization();
  m_dchip_map.clear();
  m_dchip.clear();
  m_dchip_detNb_map.clear();
  m_dchip_pdg_map.clear();
  m_dchip_trkID_map.clear();
}

void FANGSDigitizerModule::Pixelization()
{
  std::vector<int> t0;
  std::vector<int> col;
  std::vector<int> row;
  std::vector<int> ToT;
  std::vector<int> bci;

  StoreArray<FANGSHit> FANGSHits;

  for (auto& keyValuePair : m_dchip_map) {
    const auto& key = keyValuePair.first;
    //column
    int i = std::get<0>(key);
    //raw
    int j = std::get<1>(key);

    if (m_dchip_map[std::tuple<int, int>(i, j)] == 1) {

      int k0 = 1e9;
      const int quE = gRandom->Uniform(0, 2);
      const double thresEl = m_PixelThreshold + gRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
      int kcounter = 0;
      //determined t0 ie first time above pixel threshold
      for (auto& keyValuePair2 : m_dchip) {
        const auto& key2 = keyValuePair2.first;
        int k = std::get<2>(key2);
        if (m_dchip[std::tuple<int, int, int>(i, j, k)] > thresEl) {
          if (k0 > k)k0 = k;
          kcounter ++;
        }
      }
      //determined nb of bc per pixel
      //if good t0
      if (k0 != 1e9) {
        int ik = 0;
        int NbOfEl = 0;
        for (auto& keyValuePair2 : m_dchip) {
          const auto& key2 = keyValuePair2.first;
          int k = std::get<2>(key2);
          //sum up charge with 16 cycles
          if (ik < 16) {
            NbOfEl += m_dchip[std::tuple<int, int, int>(i, j, k)];
          } else {
            //calculate ToT
            int tot = -1;
            if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
              tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
            } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
              tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
            } else if (NbOfEl > 800.*m_TOTQ1) {
              tot = 14;
            }
            if (tot > 13) {
              tot = 14;
            }
            if (tot >= 0) {
              ToT.push_back(tot);
              t0.push_back(k0);
              col.push_back(i);
              row.push_back(j);
              bci.push_back(k0);
            }
            ik = 0;
            NbOfEl = 0;
          }
          ik++;
        }

        if (kcounter < 16) {
          //calculate ToT
          int tot = -1;
          if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
            tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
          } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
            tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
          } else if (NbOfEl > 800.*m_TOTQ1) {
            tot = 14;
          }
          if (tot > 13) {
            tot = 14;
          }
          if (tot >= 0) {
            ToT.push_back(tot);
            t0.push_back(k0);
            col.push_back(i);
            row.push_back(j);
            bci.push_back(k0);
          }
        }

      }
    } //end loop on row
  } // end loop on col

  //bool PixHit = false;
  //if entry
  if (bci.size() > 0) {
    //PixHit = true;
    //find start time
    sort(t0.begin(), t0.end());

    //loop on nb of hit
    for (int j = 0; j < (int)bci.size(); j++) {
      if ((bci[j] - t0[0]) > (m_PixelTimeBinNb - 1)) {
        continue;
      }
      //create FANGSHit
      FANGSHits.appendNew(FANGSHit(col[j], row[j], bci[j] - t0[0], ToT[j],
                                   m_dchip_detNb_map[std::tuple<int, int>(col[j], row[j])],
                                   m_dchip_pdg_map[std::tuple<int, int>(col[j], row[j])],
                                   m_dchip_trkID_map[std::tuple<int, int>(col[j], row[j])]));
    } //end if entry
  }
  //return PixHit;
}
//read tube centers, impulse response, and garfield drift data filename from FANGS.xml
void FANGSDigitizerModule::getXMLData()
{
  //const GearDir& content;
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"FANGS\"]/Content/");

  //get the location of the tubes

  m_PixelThreshold = content.getInt("PixelThreshold");
  m_PixelThresholdRMS = content.getInt("PixelThresholdRMS");
  m_PixelTimeBinNb = content.getInt("PixelTimeBinNb");
  m_PixelTimeBin = content.getDouble("PixelTimeBin");
  m_ChipColumnNb = content.getInt("ChipColumnNb");
  m_ChipRowNb = content.getInt("ChipRowNb");
  m_ChipColumnX = content.getDouble("ChipColumnX");
  m_ChipRowY = content.getDouble("ChipRowY");
  m_TOTA1 = content.getDouble("TOTA1");
  m_TOTB1 = content.getDouble("TOTB1");
  m_TOTC1 = content.getDouble("TOTC1");
  m_TOTQ1 = content.getDouble("TOTQ1");
  m_TOTA2 = content.getDouble("TOTA2");
  m_TOTB2 = content.getDouble("TOTB2");
  m_TOTC2 = content.getDouble("TOTC2");
  m_TOTQ2 = content.getDouble("TOTQ2");
  m_v_sensor = content.getDouble("v_sensor");
  m_sensor_width = content.getDouble("sensor_width");
  m_Workfct = content.getDouble("Workfct");
  m_Fanofac = content.getDouble("Fanofac");

  B2INFO("FANGSDigitizer: Aquired FANGS locations and gas parameters");
  B2INFO("              from FANGS.xml. There are " << m_nFANGS << " FANGSs implemented");

}

void FANGSDigitizerModule::endRun()
{
}

void FANGSDigitizerModule::terminate()
{
}


