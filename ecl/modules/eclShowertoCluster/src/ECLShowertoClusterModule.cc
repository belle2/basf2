/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclShowertoCluster/ECLShowertoClusterModule.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>


#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>


#include <ctime>
#include <iomanip>

using namespace std;
using namespace Belle2;
using namespace ECL;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowertoCluster)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowertoClusterModule::ECLShowertoClusterModule() : Module()
{
  // Set module properties
  setDescription("To create ECLCluster (mdst data object) from ECLShower.  To be used for root file created without mdst format. ");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definitions

}

ECLShowertoClusterModule::~ECLShowertoClusterModule()
{
}

void ECLShowertoClusterModule::initialize()
{
  m_nRun = 0;
  m_nEvent = 0;
  m_timeCPU = clock() * Unit::us;

  StoreArray<ECLCluster>::registerPersistent();

  RelationArray::registerPersistent<ECLCluster, ECLShower>("", "");
  RelationArray::registerPersistent<ECLCluster, MCParticle>("", "");
  RelationArray::registerPersistent<ECLCluster, Track>("", "");



}

void ECLShowertoClusterModule::beginRun()
{
  B2INFO("ECLShowertoClusterModule: Processing run: " << m_nRun);
}


void ECLShowertoClusterModule::event()
{
  StoreArray<ECLShower> eclShowerArray;
  // std::cout << m_nEvent << endl;
  if (!eclShowerArray) {
    B2DEBUG(100, "ECLShower is empty in event " << m_nEvent);
    return;
  }

  StoreArray<MCParticle> MCArray;
  if (!MCArray) {
    B2DEBUG(100, "MCParticle is empty in event " << m_nEvent);
  }


  StoreArray<Track> TrackArray;
  if (!TrackArray) {
    B2DEBUG(100, "Tracks is empty in event " << m_nEvent);
  }


  cout.unsetf(ios::scientific);
  cout.precision(6);


  for (int i_Shower = 0; i_Shower < eclShowerArray.getEntries(); ++i_Shower) {
    ECLShower* eclShower = eclShowerArray[i_Shower];

    StoreArray<ECLCluster> eclMdstArray;

    if (i_Shower == 0) eclMdstArray.clear(); // This clar the previous
    // stored data object of ECLCluster
    if (!eclMdstArray) eclMdstArray.create();
    new(eclMdstArray.nextFreeAddress()) ECLCluster();
    int i_Mdst = eclMdstArray.getEntries() - 1;

    float Mdst_Error[6] = {
      eclShower->getEnergyError(),
      0,
      eclShower->getPhiError(),
      0,
      0,
      eclShower->getThetaError()
    };

    eclMdstArray[i_Mdst]->setEnedepSum(eclShower->getUncEnergy());
    eclMdstArray[i_Mdst]->setEnergy(eclShower->getEnergy());
    eclMdstArray[i_Mdst]->setTheta(eclShower->getTheta());
    eclMdstArray[i_Mdst]->setPhi(eclShower->getPhi());
    eclMdstArray[i_Mdst]->setR(eclShower->getR());
    eclMdstArray[i_Mdst]->setE9oE25(eclShower->getE9oE25());
    eclMdstArray[i_Mdst]->setTiming(eclShower->getTime());
    eclMdstArray[i_Mdst]->setError(Mdst_Error);

    //Problem in making relation from ECLCluster to ECLShower
    // Guess: May be due to earlier ECLClustertoShower.
    // To be work on Future: Vishal
    /*
    std::cout << i_Mdst << "\t" << i_Shower << endl;
    std::cout << eclMdstArray[i_Mdst]->getEnergy() << "\t"
        << eclShowerArray[i_Shower]->getEnergy() << "\n";
    */
    //    eclMdstArray[i_Mdst]->addRelationTo(eclShowerArray[i_Shower]);


    const MCParticle* mcparticle = eclShower->getRelated<MCParticle>();

    // Relation of ECLClusterto MCParticle using ArrayIndex
    if (mcparticle) {
      RelationArray ECLClustertoMCParticle(eclMdstArray, MCArray);
      int mc_index = mcparticle->getArrayIndex();
      // std::cout << i_Mdst << "\t" << mc_index << endl;
      ECLClustertoMCParticle.add(i_Mdst, mc_index);
      //eclMdstArray[i_Mdst]->addRelationTo(mcparticle);
    }




    int i_eclShower = -99;
    bool v_TrackHit = false;
    for (int i_track = 0; i_track < TrackArray.getEntries(); ++i_track) {
      Track* track = TrackArray[i_track];

      i_eclShower = -99;
      // Loop over the tracks and check for the ECLShower relation
      // If there is relation, get the showerId of ECLShower,
      // if the ShowerId matches with the above ECLShower loop
      // use it to get the ECLClustertoTracks
      const ECLShower* sho = track->getRelated<ECLShower>();

      if (sho) {
        i_eclShower = sho->getShowerId();
        if (i_eclShower == eclShower->getShowerId()) {
          RelationArray ECLClustertoTracks(eclMdstArray, TrackArray);
          //  std::cout << i_Mdst << "\t" << i_track << endl;
          ECLClustertoTracks.add(i_Mdst, i_track);
          //eclMdstArray[i_Mdst]->addRelationTo(track);
          v_TrackHit = true;
        } // logic to check the index of i_Shower and i_eclShower
      } // sho is not NULL
    } // i_track
    eclMdstArray[i_Mdst]->setisTrack(v_TrackHit);
  }// i_Shower

  m_nEvent++;

}

void ECLShowertoClusterModule::endRun()
{

  m_nRun++;
}

void ECLShowertoClusterModule::terminate()
{
  m_timeCPU = clock() * Unit::us - m_timeCPU;

}












