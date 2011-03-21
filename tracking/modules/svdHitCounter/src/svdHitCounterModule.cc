/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/svdHitCounter/svdHitCounterModule.h"



using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(svdHitCounter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

svdHitCounterModule::svdHitCounterModule() : Module()
{
  //Set module properties
  setDescription("svdHitCounterMod module");

  //Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("svdHitOut.txt"));
// addParam("cdcSimColName", m_cdcSimHitColName, "Name of collection of simulated CDC hits", string(DEFAULT_CDCSIMHITS));
// addParam("pxdSimColName", m_pxdSimHitColName, "Name of collection of simulated PXD hits", string(DEFAULT_PXDSIMHITS));
  addParam("mcPartColName", m_mcPartColName, "Name of collection of all generated particles", string(DEFAULT_MCPARTICLES));
  addParam("svdSimColName", m_svdSimHitColName, "Name of collection of simulated SVD hits", string(DEFAULT_SVDSIMHITS));
  addParam("tracks", m_nTracks, "Number of tracks in one event", int(1));
  addParam("svdLayerId", m_svdLayerId, "Id of relevant SVD layer that should have been hit (or not depending on the \"option\")", int(6));
  addParam("option", m_option, "say if svdLayerId must at least or not even be reached", string("atLeast"));
}


svdHitCounterModule::~svdHitCounterModule()
{

}

int testIndex = 1;
void svdHitCounterModule::initialize()
{
  m_dataOut.open(m_dataOutFileName.c_str(), ios::app);
  //m_dataOut2.open( "othertestfile" );

  //m_testVec.resize(10,0);
  //m_hitsInSvd = 4;
  //set mode
  if (m_option == "atLeast") {
    m_atLeast = true;
    m_dataOut << "#starting paremeters of particles that reach at least the SVD layer with Id " << m_svdLayerId << "\n#theta\tp_tot\n";
  }
  if (m_option == "notEven") {
    m_atLeast = false;
    m_dataOut << "#starting paremeters of particles that reach not even the SVD layer with Id " << m_svdLayerId << "\n#theta\tp_tot\n";
  }
}


void svdHitCounterModule::beginRun()
{

}


void svdHitCounterModule::event()
{
  //get the data
  StoreArray<SVDSimHit> aSvdSimHitArray(m_svdSimHitColName.c_str());
  StoreArray<Relation> relationArray(DEFAULT_SVDSIMHITSREL);
  StoreArray<MCParticle> aMcParticleArray(m_mcPartColName.c_str());
  int nSvdSimHits = aSvdSimHitArray->GetEntriesFast(); // number of hits in this event

  int nMcParticles = aMcParticleArray->GetEntriesFast();
  int nRels = relationArray->GetEntriesFast();
  m_dataOut << "#size of arrays: " << nMcParticles << " " << nSvdSimHits << " " << nRels << endl;
  //int nPxdSimHits = aPxdSimHitArray->GetEntriesFast();
  int iTracks = 0;
  int goodTracks = 0;
  bool writePartile = true;
  for (int i = 0; i not_eq nMcParticles; ++i) {
    MCParticle* aMCParticle = aMcParticleArray[i];
    bool primary = aMCParticle->hasStatus(MCParticle::c_PrimaryParticle);
    //bool inSvd = aMCParticle->hasStatus(MCParticle::c_SeenInSVD);
    //m_dataOut << "i= " << i << " primary= " << primary << " svd= " << inSvd << "\n";
    //m_dataOut2 << "i=" << i ;
    int hl3 = 0;
    int hl4 = 0;
    int hl5 = 0;
    int hl6 = 0;
    //int trackId = 0;
    int totalHits = 0;
    //bool passedSvd = false;
    if (primary == true) { //check if particle was created by the particle gun
      // determin which layers where hit by this particle
      //stringstream convTrId;
      //stringstream convLyId;
      //convTrId << "trid: ";
      //convLyId << "lyid: ";

      for (int j = 0; j not_eq nRels; ++j) {
        int layerNumber = -1;
        //int trackId = -1;
        if (relationArray[j]->getFromIndex() == i) {
          int indexToSvdHits = relationArray[j]->getToIndex();
          layerNumber = aSvdSimHitArray[indexToSvdHits]->getLayerID();
          //trackId = aSvdSimHitArray[indexToSvdHits]->getTrackID();
          //m_dataOut2 << layerNumber;
          if (layerNumber == 3) { hl3 = 1; }
          if (layerNumber == 4) { hl4 = 1; }
          if (layerNumber == 5) { hl5 = 1; }
          if (layerNumber == 6) { hl6 = 1; }
          //convTrId << trackId << " ";
          //convLyId << layerNumber << " ";
        }
        //m_dataOut << trackId << " ";
      }
      //convTrId << "\n";
      double pT = 0;
      double pTot = 0; //length of momentum vector
      double theta = 0;

      if (m_atLeast == true) {
        if (m_svdLayerId == 6) {
          totalHits = hl3 + hl4 + hl5 + hl6;
        } else if (m_svdLayerId == 5) {
          totalHits = hl3 + hl4 + hl5;
        } else if (m_svdLayerId == 4) {
          totalHits = hl3 + hl4;
        } else if (m_svdLayerId == 3) {
          totalHits = hl3;
        }
        if (totalHits == m_svdLayerId - 2) {
          //passedSvd = true;
          //write this track to file
          //get p_t, p_tot, theta
          TVector3 p = aMCParticle->getMomentum();
          pTot = p.Mag();
          theta = p.Theta();
          pT = p.Perp();
          //m_dataOut << theta / Unit::deg << "\t" << pTot << "\t" << convLyId.str() << convTrId.str() << " i: " << i << "\n";
          m_dataOut << theta / Unit::deg << "\t" << pTot << "\t" << pT << "\n";
          ++goodTracks;
        }
      } else {
        if (m_svdLayerId == 6) {
          writePartile = not hl6;
        } else if (m_svdLayerId == 5) {
          writePartile = not hl6 and not hl5;
        } else if (m_svdLayerId == 4) {
          writePartile = not hl6 and not hl5 and not hl4;
        } else if (m_svdLayerId == 3 or m_svdLayerId == 0) {
          writePartile = not hl6 and not hl5 and not hl4 and not hl3;
        }
        if (writePartile == true) {
          TVector3 p = aMCParticle->getMomentum();
          pTot = p.Mag();
          theta = p.Theta();
          pT = p.Perp();
          m_dataOut << theta / Unit::deg << "\t" << pTot << "\t" << pT << "\n";
          //m_dataOut << theta / Unit::deg << "\t" << pTot << "\t" << convLyId.str() << convTrId.str() << " i: " << i << "\n";
          ++goodTracks;

        }
      }
      //if (m_hitsInSvd == -1)
    }
    ++iTracks;
    if (iTracks == m_nTracks) { // this will make the loop stop as early as possible if the primary particles are the first particles in sMcParticleArray
      m_dataOut << "#quit after " << iTracks << " iterations. Found " << goodTracks << " good tracks" << endl;
      break;
    }
  }
}


void svdHitCounterModule::endRun()
{

}


void svdHitCounterModule::terminate()
{
  m_dataOut.close();
}
