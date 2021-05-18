/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *               Martin Ritter                                            *
 *               Giacomo De Pietro                                        *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <awesome/modules/AWESOMEBasic/AWESOMEBasicModule.h>

/* Belle2 headers. */
#include <framework/logging/Logger.h>

/* --------- WARNING ------------------------------------------------------
 * If you have more complex parameter types in your class then simple int,
 * double or std::vector of those you might need to uncomment the following
 * include directive to avoid an undefined reference on compilation.
 * --------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;
using namespace Belle2::AWESOME;

/**
 * We have to register the module to the Framework. The "Module" part of the
 * class name will be appended automatically so every module has to be named
 * <NAME>Module.
 */
REG_MODULE(AWESOMEBasic)

AWESOMEBasicModule::AWESOMEBasicModule() :
  Module(),
  m_intParameter{0},
  m_doubleParameter{0},
  m_stringParameter{""}
{
  setDescription("Here you can enter a description of the module which can be displayed during runtime.");
  /* We can define parameters which can be set from the steering file. The arguments are:
   * name, reference to the veriable where the value will be stored, description, default value.
   * If the default value is ommited the user has to specify this parameter, otherwise an error is produced. */
  addParam("intParameter", m_intParameter,
           "Useless parameter of type integer", 0);
  addParam("doubleParameter", m_doubleParameter,
           "Useless parameter of type double", 0.0);
  addParam("stringParameter", m_stringParameter,
           "Useless parameter of type string", std::string{""});
  addParam("doubleListParameter", m_doubleListParameter,
           "Useless parameter of type vector<double>", std::vector<double> {3, 0});

  //Valid parameter types are int, double, string, bool and vectors of any of those
}

void AWESOMEBasicModule::initialize()
{
  B2INFO("AWESOMEBasic: initialize.");
  /**
   * Here you can do some stuff before processing starts. If you want to
   * write to some collections of the DataStore you have to register these
   * here by using StoreArray<T>::registerInDataStore() for collections which
   * should be written to the output file by default or
   * StoreArray<T>::registerInDataStore(DataStore::c_DontWriteOut) for collections which will not be
   * saved by default. If one just wants to access collections one should
   * check if they were registered by using the isRequired member.
   */
  m_MCParticles.isRequired();
  m_SimHits.isRequired();
  m_MCParticles.requireRelationTo(m_SimHits);
}

void AWESOMEBasicModule::beginRun()
{
  B2INFO("AWESOMEBasic: begin of new run.");
  /**
   * Here comes the initialization specific to each run. Usually some preliminrary
   * business related to payloads and the conditions database must be done here.
   */
}

void AWESOMEBasicModule::event()
{
  B2INFO("AWESOMEBasic: event is being processed.");
  /* Let's try a simple loop over the simulated hits. */
  for (const AWESOMESimHit& hit : m_SimHits) {
    /**
     * Check if there is an MCParticle associated to this hit.
     * Since we may not store MCParticles from secondary processes, better to check it.
     */
    int mcArrayIndex = -1;
    int pdgCode = 0;
    /* We assume there is only a single MCParticle related to the hit. */
    MCParticle* mcParticle = hit.getRelated<MCParticle>();
    if (mcParticle) {
      mcArrayIndex = mcParticle->getArrayIndex();
      pdgCode = mcParticle->getPDG();
    }
    B2INFO("AWESOMESimHit #" << hit.getArrayIndex()
           << " has an energy deposition of " << hit.getEnergyDep()
           << " and is related to MCParticle #" << mcArrayIndex
           << " which has a PDG code " << pdgCode);
  }
  /* Now let's do it the other way round. */
  for (const MCParticle& mcParticle : m_MCParticles) {
    /**
     * In principle we know if the relation is TO or FROM. In case we only set
     * a single relation between the two objects, let's use the more generic
     * getRelationsWith<>() instead of getRelationsTo<>() or getRelationsFrom<>().
     */
    for (const AWESOMESimHit& hit : mcParticle.getRelationsWith<AWESOMESimHit>())
      B2INFO("MCParticle #" << mcParticle.getArrayIndex()
             << " which has a PDG code " << mcParticle.getPDG()
             << " created the AWESOMESimHit #" << hit.getArrayIndex()
             << " which has an energy deposition of " << hit.getEnergyDep());
  }
}

void AWESOMEBasicModule::endRun()
{
  B2INFO("AWESOMEBasic: end of run.");
  /* Here you can do some cleanup after each run. */
}


void AWESOMEBasicModule::terminate()
{
  B2INFO("AWESOMEBasic: terminate.");
  /* Here you can do some final cleanup. */
}
