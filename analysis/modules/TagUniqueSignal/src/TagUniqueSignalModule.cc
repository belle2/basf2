#include <analysis/modules/TagUniqueSignal/TagUniqueSignalModule.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Variables.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <set>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(TagUniqueSignal)

TagUniqueSignalModule::TagUniqueSignalModule() : Module(), m_targetVar(nullptr)
{
  setDescription("Mark true (target=1) candidates from input list via extra-info field.  Only the first true candidate associated with an MCParticle is marked.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("particleList", m_particleList, "Input ParticleList name");
  addParam("target", m_targetVariable, "Variable which defines signal and background.", std::string("isSignal"));
  addParam("extraInfoName", m_extraInfoName,
           "Extra-info field added to all particles in the input list. 1 for unique signal, 0 for background or duplicates.",
           string("uniqueSignal"));
}

void TagUniqueSignalModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);

  Variable::Manager& manager = Variable::Manager::Instance();

  m_targetVar = manager.getVariable(m_targetVariable);
  if (m_targetVar == nullptr) {
    B2ERROR("TagUniqueSignal: Variable::Manager doesn't have variable" <<  m_targetVariable);
  }
}

void TagUniqueSignalModule::event()
{
  const StoreObjPtr<ParticleList> inPList(m_particleList);
  if (!inPList)
    return;


  const int size = inPList->getListSize();
  std::set<const MCParticle*> foundMCParticles;
  for (int i = 0; i < size; i++) {
    Particle* part = inPList->getParticle(i);
    const MCParticle* mcp = part->getRelated<MCParticle>();
    float extraInfoValue = 0.0;
    if (mcp and m_targetVar->function(part)) {
      const bool was_inserted = foundMCParticles.insert(mcp).second;
      if (was_inserted)
        extraInfoValue = 1.0;
    }
    if (part->hasExtraInfo(m_extraInfoName)) {
      B2WARNING("Extra Info with given name is already set! This module can only be used once per particle.");
    } else {
      part->addExtraInfo(m_extraInfoName, extraInfoValue);
    }
  }
}
