/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleExtractorFromROE/ParticleExtractorFromROEModule.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

// analysis
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(ParticleExtractorFromROE);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParticleExtractorFromROEModule::ParticleExtractorFromROEModule() : Module()
{
  // set module description (e.g. insert text)
  setDescription("Extract Particles that belong to the ROE and fill them into ParticleLists.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  std::vector<std::string> defaultList;
  addParam("outputListNames", m_outputListNames,
           "list of ParticleList names to be created", defaultList);
  addParam("maskName", m_maskName,
           "List of all mask names for which the info will be printed.",
           std::string(RestOfEvent::c_defaultMaskName));

  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

}

void ParticleExtractorFromROEModule::initialize()
{
  StoreArray<RestOfEvent>().isRequired();

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;

  const int nLists = m_outputListNames.size();
  m_pdgCodes.resize(nLists);
  m_absPdgCodes.resize(nLists);
  m_outputAntiListNames.resize(nLists);
  m_pLists.resize(nLists);
  m_antiPLists.resize(nLists);

  for (int iList = 0; iList < nLists; iList++) {

    const auto listName = m_outputListNames[iList];

    DecayDescriptor dd;
    if (not dd.init(listName))
      B2ERROR("Invalid PariticleList name: " << listName);

    const DecayDescriptorParticle* mother = dd.getMother();

    const std::string antiListName = ParticleListName::antiParticleListName(listName);
    m_outputAntiListNames[iList] = antiListName;

    const int pdgCode = mother->getPDGCode();
    m_pdgCodes[iList] = pdgCode;
    m_absPdgCodes[iList] = abs(pdgCode);

    StoreObjPtr<ParticleList> particleList(listName);
    m_pLists[iList] = particleList;

    particleList.registerInDataStore(listName, flags);
    if (listName != antiListName) {
      StoreObjPtr<ParticleList> antiParticleList(antiListName);
      antiParticleList.registerInDataStore(antiListName, flags);

      m_antiPLists[iList] = antiParticleList;
    }
  }

}

void ParticleExtractorFromROEModule::event()
{
  for (int iList = 0; iList < m_outputListNames.size(); iList++) {

    const std::string listName = m_outputListNames[iList];
    const std::string antiListName = m_outputAntiListNames[iList];
    const int pdgCode = m_pdgCodes[iList];

    m_pLists[iList].create();
    m_pLists[iList]->initialize(pdgCode, listName);
    if (listName != antiListName) {

      m_antiPLists[iList].create();
      m_antiPLists[iList]->initialize(-1 * pdgCode, antiListName);

      m_pLists[iList]->bindAntiParticleList(*(m_antiPLists[iList]));
    }
  }

  StoreObjPtr<RestOfEvent> roe("RestOfEvent");
  if (not roe.isValid()) {
    B2ERROR("RestOfEvent object is not valid.");
    return;
  }

  auto particlesInROE = roe->getParticles(m_maskName, /* unpackComposite */ true);

  for (auto part : particlesInROE) {
    const int absPdg = abs(part->getPDGCode());

    auto result = std::find(m_absPdgCodes.begin(), m_absPdgCodes.end(), absPdg);

    if (result == m_absPdgCodes.end())
      continue;

    const int indexList = std::distance(m_absPdgCodes.begin(), result);
    m_pLists[indexList]->addParticle(part);
  }

}

