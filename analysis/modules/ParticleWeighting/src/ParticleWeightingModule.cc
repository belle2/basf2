/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleWeighting/ParticleWeightingModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>


// framework aux
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleWeighting)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleWeightingModule::ParticleWeightingModule() : Module()

  {
    setDescription("Weights particles according to LookUp table");
    addParam("tableName", m_tableName, "ID of table used for reweighing");
    addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidates");
  }


  // // Getting LookUp info for given particle in given event
  // WeightInfo ParticleWeightingModule::getInfo(const Particle* p)
  // {
  //   WeightMap usedWeightMap = m_ParticleWeightingLookUpTable->getWeightMap();
  //   double entryKey = this->getKey(p);
  //   if (usedWeightMap.find(entryKey) == usedWeightMap.end()) {
  //     if (entryKey == -1) {
  //       B2ERROR("This particle is out of range of the LookUp table, but weights for this region are not defined. Consider call 'defineOutOfRangeWeight()' function.");
  //     } else {
  //       B2ERROR("Bin '" << entryKey << "' is defined in ParticleWeightingKeyMap, but doesn't have any weight info.");
  //     }
  //   }
  //   return usedWeightMap.find(entryKey)->second;
  // }


  // // Get kinematic key for the particle
  // double ParticleWeightingModule::getKey(const Particle* p)
  // {
  //   ParticleWeightingKeyMap usedParticleWeightingKeyMap = m_ParticleWeightingLookUpTable->getParticleWeightingKeyMap();
  //   int nDim = usedParticleWeightingKeyMap.numberOfDimensions();
  //   std::vector<std::string> variables =  usedParticleWeightingKeyMap.getVarManagerNames();
  //   double var1_val;
  //   double var2_val=0;
  //   double var3_val=0;
  //   const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
  //   if (!var1) {
  //     B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
  //     return -1;
  //   } else {
  //     var1_val = var1->function(p);
  //   }
  //   if (nDim > 1){
  //     const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(variables[1]);
  //     if (!var2) {
  //       B2ERROR("Variable '" << variables[1] << "' is not available in Variable::Manager!");
  //       return -1;
  //     } else {
  //       var2_val = var2->function(p);
  //     }
  //   }
  //   if (nDim > 2){
  //     const Variable::Manager::Var* var3 = Variable::Manager::Instance().getVariable(variables[2]);
  //     if (!var3) {
  //       B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
  //       return -1;
  //     } else {
  //       var3_val = var3->function(p);
  //     }
  //   }

  //   return m_ParticleWeightingLookUpTable->getParticleWeightingKeyMap().getKey(var1_val, var2_val, var3_val);
  // }

  void ParticleWeightingModule::initialize()
  {
    StoreArray<Particle>().isRequired();
    m_inputList.isRequired(m_inputListName);
    // m_ParticleWeightingLookUpTable = DBObjPtr<ParticleWeightingLookUpTable>{m_tableName};
    // B2INFO("m_ParticleWeightingLookUpTable = DBObjPtr<Belle2::ParticleWeightingLookUpTable>{m_tableName};");
    DBObjPtr<ParticleWeightingLookUpTable> m_ParticleWeightingLookUpTable{m_tableName};
  }


  void ParticleWeightingModule::event()
  {
    // if (!m_inputList) {
    //   B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    //   return;
    // }
    // StoreArray<Particle> particles;
    // const unsigned int numParticles = m_inputList->getListSize();
    // for (unsigned int i = 0; i < numParticles; i++) {
    //   const Particle* ppointer = m_inputList->getParticle(i);
    //   double index = ppointer->getArrayIndex();
    //   Particle* p = particles[index];
    //   WeightInfo info = getInfo(p);
    //   for (auto entry : info){
    //     p->addExtraInfo(entry.first, entry.second);
    //   }
    // }
  }

} // end Belle2 namespace

