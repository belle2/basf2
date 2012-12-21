/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecGamma/ECLGammaReconstructorModule.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <framework/datastore/RelationArray.h>

#include <GFTrack.h>
#include <GFTrackCand.h>
#include <tracking/dataobjects/ExtRecoHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TVector3.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLGammaReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLGammaReconstructorModule::ECLGammaReconstructorModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLGamma from ECLShower.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);


//  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the reconstructed tracks", string(""));
//  addParam("ExtTrackCandsColName", m_extTrackCandsColName, "Name of collection holding the list of hits from each extrapolation", string(""));
//  addParam("ExtRecoHitsColName", m_extRecoHitsColName, "Name of collection holding the RecoHits from the extrapolation", string(""));

  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the reconstructed tracks", string("GFTracks"));
  addParam("ExtTrackCandsColName", m_extTrackCandsColName, "Name of collection holding the list of hits from each extrapolation", string("ExtTrackCands"));
  addParam("ExtRecoHitsColName", m_extRecoHitsColName, "Name of collection holding the RecoHits from the extrapolation", string("ExtRecoHits"));

  addParam("gammaEnergyCut", m_ecut, "gamma enegy threshold ", 0.02);
  addParam("gammaE9o25Cut", m_e925cut, "gamma E9o25 threshold ", 0.75);
  addParam("gammaWidthCut", m_widcut, "gamma Width threshold ", 6.0);
  addParam("gammaNhitsCut", m_nhcut, "gamma Nhits threshold ", 0.);


//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));

}


ECLGammaReconstructorModule::~ECLGammaReconstructorModule()
{

}

void ECLGammaReconstructorModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
  StoreArray<ECLGamma>::registerPersistent();
  RelationArray::registerPersistent<ECLGamma, ECLShower>("", "");


}

void ECLGammaReconstructorModule::beginRun()
{
}


void ECLGammaReconstructorModule::event()
{
  //Input Array
  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  StoreArray<ECLGamma> gammaArray;

  RelationArray eclGammaToShower(gammaArray, eclRecShowerArray);

  if (!eclRecShowerArray) {
    B2DEBUG(100, "ECLShowers in empty in event " << m_nEvent);
  }
  if (!eclHitAssignmentArray) {
    B2DEBUG(100, "ECLHitAssignment in empty in event " << m_nEvent);
  }

  const int hitNum = eclRecShowerArray->GetEntriesFast();
  const int hANum = eclHitAssignmentArray->GetEntriesFast();

  readExtrapolate();//m_TrackCellId[i] =1 => Extrapolated cell

  //cout<<"Event "<< m_nEvent<<" Total input number of Shower Array "<<hitNum<<endl;
  for (int iShower = 0; iShower < hitNum; iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    m_showerId = aECLShower->GetShowerId();
    m_energy = aECLShower->GetEnergy();
    m_theta = aECLShower->GetTheta();
    m_Theta  = m_theta * 180. / M_PI;
    m_phi = aECLShower->GetPhi();
    m_e9oe25 = aECLShower->GetE9oE25();
    m_width  = aECLShower->GetWidth();
    m_nhit      = aECLShower->GetNHits();
    m_quality   = aECLShower->GetStatus();
//    cout<<m_showerId<<" "<<m_energy<<" "<<m_Theta<<" "<<m_phi<<endl;;
    if (m_quality != 0)continue;
    if (!goodGamma(m_Theta, m_energy, m_nhit, m_e9oe25, m_width, m_ecut, m_e925cut, m_widcut, m_nhcut))continue;

    m_extMatch = false;

    for (int iHA = 0; iHA < hANum; iHA++) {

      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId();

      if (m_HAShowerId != m_showerId)continue;
      if (m_HAShowerId > m_showerId)break;
      if (m_HAShowerId == m_showerId) {
        if (m_TrackCellId[m_HAcellId]) {m_extMatch = true; break;}

      }//if HAShowerId == ShowerId
    }//for HA hANum

    if (!m_extMatch) { //no match to track => assign as gamma

      if (!gammaArray) gammaArray.create();
      m_GNum = gammaArray->GetLast() + 1;
      new(gammaArray->AddrAt(m_GNum)) ECLGamma();
      gammaArray[m_GNum]->setShowerId(m_showerId);

      eclGammaToShower.add(m_GNum, iShower);

      /*
            double px = m_energy * sin(m_theta) * cos(m_phi);
            double py = m_energy * sin(m_theta) * sin(m_phi);
            double pz = m_energy * cos(m_theta);

                  cout << "EventGamma  " << m_nEvent << " Gamma " << m_showerId << " " << sqrt(px * px + py * py + pz * pz) << " m_extMatch  " << m_extMatch << endl;
                                          cout<<"CellID ";

                                         for (int iHA = 0; iHA < hANum; iHA++) {

                                           ECLHitAssignment* aECLShower = eclHitAssignmentArray[iHA];
                                           int m_HAShowerId = aECLShower->getShowerId();
                                           int m_HAcellId = aECLShower->getCellId();
                                           if(m_showerId==m_HAShowerId)cout<<m_HAcellId<<" ";
                                         }//for HA hANum
                                          cout<<endl;
                               */
    }//if !m_extMatch


  }//for shower hitNum

  //cout << "Event " << m_nEvent << " Total output number of Gamma Array " << ++m_GNum << endl;
  m_nEvent++;
}

void ECLGammaReconstructorModule::endRun()
{
  m_nRun++;
}

void ECLGammaReconstructorModule::terminate()
{
}

bool ECLGammaReconstructorModule::goodGamma(double ftheta, double energy, double nhit, double fe9oe25, double fwidth, double ecut, double e925cut, double widcut, double nhcut)
{
  bool ret = true;
  if (ftheta <  17.0) ret = false;
  if (ftheta > 150.0) ret = false;
  if (energy < ecut) ret = false;

  if (energy < 0.5) {
    if (nhit <= nhcut) ret = false;
    if (fe9oe25 < e925cut) ret = false;
    if (fwidth > widcut) ret = false;
  }

  return ret;

}

void ECLGammaReconstructorModule::readExtrapolate()
{
  for (int i = 0; i < 8736; i++) {
    m_TrackCellId[i] = false ;
  }

  StoreArray<GFTrack> gfTracks(m_gfTracksColName);
  StoreArray<GFTrackCand> extTrackCands(m_extTrackCandsColName);
  StoreArray<ExtRecoHit> extRecoHits(m_extRecoHitsColName);

  if (extTrackCands) {

    int myDetID = 5; // ECL in this example
//  cout<<"GFTrack  "<<gfTracks.getEntries()<<endl;
    for (int t = 0; t < gfTracks.getEntries(); ++t) {
//GFTrack* track = gfTracks[i];
//for ( int hypothesis = 0; hypothesis < 5; ++hypothesis ) {
      int hypothesis = 0;
      GFTrackCand* cand = extTrackCands[t * 5 + hypothesis];
//    cout<<"GFTrackCand  "<<cand->getNHits()<<endl;
      for (unsigned int j = 0; j < cand->getNHits(); ++j) {
        int detID;
        int hitID;
        int planeID;
        cand->getHitWithPlane(j, detID, hitID, planeID);
        if ((detID != myDetID) || (planeID == 0)) continue;
        //cout<<" Ext CellId "<<planeID<<" ";
        m_TrackCellId[planeID] = 1;
      }//for cands
      //cout<<endl;
//}//for 5 hypothesis
    }//gfTracks.getEntries()

  }//if extTrackCands

}



