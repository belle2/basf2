/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/plume/modules/PlumeModule.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::plume;

//We have to register the module to the Framework. The "Module" part of the
//class name will be appended automatically so every module hast to be named
//XxxModule
REG_MODULE(Plume);


PlumeModule::PlumeModule() : Module(), m_intParameter(0), m_doubleParameter(0), m_stringParameter("")
{
  setDescription("Creates PLUME - sub-detector of BEASTII");

  //We can define parameters which can be set from the steering file. The arguments are:
  // name, reference to the variable where the value will be stored, description, default value
  //If the default value is omitted the user has to specify this parameter, otherwise an error is produced
  addParam("intParameter", m_intParameter,
           "Useless parameter of type integer", 0);
  addParam("doubleParameter", m_doubleParameter,
           "Useless parameter of type double", 0.0);
  addParam("stringParameter", m_stringParameter,
           "Useless parameter of type string", string(""));
  addParam("doubleListParameter", m_doubleListParameter,
           "Useless parameter of type vector<double>", vector<double>(3, 0));


  //Valid parameter types are int, double, string, bool and vectors of any of those
}

void PlumeModule::initialize()
{
  B2INFO("Plume: Initialize");
  //Here you can do some stuff before processing starts. If you want to
  //write to some collections of the DataStore you have to register these
  //here by using StoreArray<T>::registerPersistent() for collections which
  //should be written to the output file by default or
  //StoreArray<T>::registerTransient() for collections which will not be
  //saved by default. If one just wants to access collections one should
  //check if they were registered by using the isRequired member

  StoreArray<MCParticle>   mcParticles;
  StoreArray<PlumeSimHit>  simHits;
  RelationArray relMCSimHit(mcParticles, simHits);
  if (!(mcParticles.isRequired() && simHits.isRequired() && relMCSimHit.isRequired())) {
    //Fatal is not necessary here as the storeArrays should just look
    //empty if not registered but let's make sure everything is present
    B2FATAL("Not all collections found, exiting processing");
  }
}

void PlumeModule::beginRun()
{
  B2INFO("Plume: Begin of new run");
  //Here comes the initialisation specific to each run
}

void PlumeModule::event()
{
  B2INFO("Plume: Event is being processed");
  //Here comes the actual event processing

  StoreArray<MCParticle>   mcParticles;
  StoreArray<PlumeSimHit>  simHits;

  //RelationIndex is a readonly, bidirectional index for a Relation so that one
  //can easily use the RelationArray without looping over it manually.
  RelationIndex<MCParticle, PlumeSimHit> relMCSimHit(mcParticles, simHits);

  //Lets loop over all created PlumeSimHits:
  //int nSimHits = simHits.getEntries();
  //for (int i = 0; i < nSimHits; ++i) {
  //PlumeSimHit& hit = *simHits[i];
  //Find all MCParticles which point to that SimHit and the corresponding weight
  //RelationIndex<MCParticle, PlumeSimHit>::range_to range = relMCSimHit.getElementsTo(hit);
  //for (; range.first != range.second; ++range.first) {
  //And Print something about the relation
  //const RelationIndex<MCParticle, PlumeSimHit>::Element& relation = *range.to;
  //B2INFO("PlumeSimHit #" << i << " has an energy deposition of " << hit.getenergyDep()
  //       << " and is related to MCParticle #" << relation.indexFrom
  //       << " which has an PDG code of " << relation.from->getPDG());
  //}
  //}

  //Now let's do it the other way round:
  int nMCParticles = mcParticles.getEntries();
  for (int i = 0; i < nMCParticles; ++i) {
    MCParticle& mcp = *mcParticles[i];
    // Find all PlumeSimHits which point from that MCParticle.
    typedef RelationIndex<MCParticle, PlumeSimHit>::Element relMCSimHit_Element;
    for (const relMCSimHit_Element& relation : relMCSimHit.getElementsFrom(mcp)) {
      B2INFO("MCParticle #" << i << " created the AwesomSimHit #" << relation.indexTo
             << " which has an energy deposition of " << relation.to->getenergyDep());
    }
  }
}

void PlumeModule::endRun()
{
  B2INFO("Plume: End of run");
  //Here cleanup after each run
}


void PlumeModule::terminate()
{
  B2INFO("Plume: Terminate");
  //Here final cleanup
}
