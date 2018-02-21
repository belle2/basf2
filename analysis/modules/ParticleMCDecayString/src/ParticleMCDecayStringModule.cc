/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Matt Barrett                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/ParticleMCDecayString/ParticleMCDecayStringModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/StringWrapper.h>
#include <analysis/utility/MCMatching.h>

#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace Belle2 {

  REG_MODULE(ParticleMCDecayString)

  ParticleMCDecayStringModule::ParticleMCDecayStringModule() : Module(), m_tree("", DataStore::c_Persistent), m_hashset("",
        DataStore::c_Persistent)
  {
    setDescription("Creates the Monte Carlo decay string of a Particle and its daughters. "
                   "The MC decay string of the particle is hashed and saved as a 32bit pattern in the extra info field decayHash of the particle.  "
                   "The MC decay string of the particel + its daughters is hashed as well and saved as another 32bit pattern in the extra info field decayHashExtended of the particle.  "
                   "The mapping hash <-> MC decay string in saved in a TTree by this module.  "
                   "The 32bit pattern must be saved as a float (because our extra info field, variable manager and ntuple output only supports float) "
                   "but they just represent 32 bits of a hash!  "
                   "The MC decay string can also be stored in an analysis ROOT file using the MCDecayString NtupleTool.  "
                   "Details on the MC decay string format can be found here: "
                   "https://confluence.desy.de/display/BI/Physics+MCDecayString");
    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
    addParam("listName", m_listName, "Particles from these ParticleList are used as input.");
    addParam("fileName", m_fileName, "Filename in which the hash strings are saved, if empty the strings are not saved",
             std::string(""));
    addParam("treeName", m_treeName, "Tree name in which the hash strings are saved", std::string("hashtable"));
    addParam("conciseString", m_useConciseString, "If set to true, the code will use a more concise format for the string.", false);
    addParam("identifiers", m_identifiers, "Identifiers used to identify particles in the concise format.",
             std::string("abcdefghijklmnopqrstuvwxyz"));

    m_file = nullptr;
  }

  void ParticleMCDecayStringModule::initialize()
  {
    StoreObjPtr<ParticleList>().isRequired(m_listName);

    //This might not work for non-default names of Particle array:
    StoreArray<Particle> particles;
    particles.isRequired();

    StoreArray<StringWrapper> stringWrapperArray;
    stringWrapperArray.registerInDataStore();
    particles.registerRelationTo(stringWrapperArray);


    // Initializing the output root file
    if (m_fileName != "") {
      m_file = new TFile(m_fileName.c_str(), "RECREATE");
      if (!m_file->IsOpen()) {
        B2WARNING("Could not create file " << m_fileName);
        return;
      }

      m_file->cd();

      // check if TTree with that name already exists
      if (m_file->Get(m_treeName.c_str())) {
        B2WARNING("Tree with this name already exists: " << m_fileName);
        return;
      }

      m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
      m_tree.construct(m_treeName.c_str(), "Decay Hash Map");
      m_tree->get().Branch("decayHash", &m_decayHash);
      m_tree->get().Branch("decayHashExtended", &m_decayHashExtended);
      m_tree->get().Branch("decayString", &m_decayString);
      m_tree->get().SetBasketSize("*", 1600);
      m_tree->get().SetCacheSize(100000);
    }

    m_hashset.registerInDataStore(m_fileName + m_treeName + "_hashset", DataStore::c_DontWriteOut);
    m_hashset.construct();

  }

  void ParticleMCDecayStringModule::event()
  {

    StoreObjPtr<ParticleList> pList(m_listName);
    StoreArray<StringWrapper> stringWrapperArray;

    for (unsigned iParticle = 0; iParticle < pList->getListSize(); ++iParticle) {
      Particle* particle = pList->getParticle(iParticle);

      const std::string decayString = getMCDecayStringFromMCParticle(particle->getRelatedTo<MCParticle>());
      std::string decayStringExtended = getDecayString(*particle); //removed const to allow string to be modified to a different format.

      if (m_useConciseString) {convertToConciseString(decayStringExtended);}

      uint32_t decayHash = m_hasher(decayString);
      uint32_t decayHashExtended = m_hasher(decayStringExtended);

      uint64_t m_decayHashFull = decayHash;
      m_decayHashFull <<= 32;
      m_decayHashFull += decayHashExtended;

      // Convert unsigned int decay hash into a float keeping the same bit pattern
      assert(sizeof(float) == sizeof(uint32_t));

      union convert {
        uint32_t i;
        float f;
      };
      convert bitconverter;

      bitconverter.i = decayHash;
      m_decayHash = bitconverter.f;
      particle->addExtraInfo(c_ExtraInfoName, m_decayHash);

      bitconverter.i = decayHashExtended;
      m_decayHashExtended = bitconverter.f;
      particle->addExtraInfo(c_ExtraInfoNameExtended, m_decayHashExtended);

      m_decayString = decayStringExtended;

      StringWrapper* stringWrapper = stringWrapperArray.appendNew();
      particle->addRelationTo(stringWrapper);
      stringWrapper->setString(m_decayString);

      auto it = m_hashset->get().find(m_decayHashFull);
      if (it == m_hashset->get().end()) {
        m_hashset->get().insert(m_decayHashFull);

        if (m_tree.isValid()) {
          m_tree->get().Fill();
        }
      }

    }
  }

  void ParticleMCDecayStringModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      if (m_tree.isValid()) {
        B2INFO("Writing NTuple " << m_treeName);
        m_tree->write(m_file);

        const bool writeError = m_file->TestBit(TFile::kWriteError);
        if (writeError) {
          //m_file deleted first so we have a chance of closing it (though that will probably fail)
          delete m_file;
          B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
        }

        B2INFO("Closing file " << m_fileName);
        delete m_file;
      }
    }
  }


  const MCParticle* ParticleMCDecayStringModule::getInitialParticle(const MCParticle* mcP)
  {
    const MCParticle* mcPMother = mcP->getMother();
    if (mcPMother == nullptr) {
      return mcP;
    } else {
      return getInitialParticle(mcPMother);
    }
  }

  /** defines what is a final state particle for this purpose. */
  bool isFSP(int pdg)
  {
    switch (abs(pdg)) {
      case 211:   //pi^+
      case 321:   //K^+
      case 11:    //e
      case 12:    //nu_e
      case 13:    //mu
      case 14:    //nu_mu
      case 16:    //nu_tau
      case 22:    //gamma
      case 310:   //K_S
      case 130:   //K_L
      case 2112:  //n
      case 2212:  //p
        return 1;
      default:
        return 0;
    }
  }

  std::string ParticleMCDecayStringModule::getDecayString(const Particle& p)
  {

    std::string output;
    output += getDecayStringFromParticle(&p) + " | ";
    output += getMCDecayStringFromParticle(&p);
    return output;

  }

  std::string ParticleMCDecayStringModule::getDecayStringFromParticle(const Particle* p)
  {

    std::string output = " ";

    output += std::to_string(p->getPDGCode());

    if (not isFSP(p->getPDGCode())) {
      output += " (-->";
      for (auto daughter : p->getDaughters()) {
        output += getDecayStringFromParticle(daughter);
      }
      output += ")";
    }

    return output;

  }

  std::string ParticleMCDecayStringModule::getMCDecayStringFromParticle(const Particle* p)
  {

    std::string output;

    output = getMCDecayStringFromMCParticle(p->getRelatedTo<MCParticle>());
    // Some FSPs can have daughters, e.g. converted Photons and K-Shorts
    if (not isFSP(p->getPDGCode())) {
      for (auto& daughter : p->getDaughters()) {
        output += " | " + getMCDecayStringFromParticle(daughter);
      }
    }

    return output;

  }

  std::string ParticleMCDecayStringModule::getMCDecayStringFromMCParticle(const MCParticle* mcPMatched)
  {

    if (mcPMatched == nullptr)
      return "(No match)";

    // TODO Performance can be optimized, this mcPMother does not change during the construction
    const MCParticle* mcPMother = getInitialParticle(mcPMatched);

    std::string decayString = buildMCDecayString(mcPMother, mcPMatched);

    if (mcPMatched->getPDG() == 10022)
      return decayString + " (Virtual gamma match)";
    return decayString;
  }


  std::string ParticleMCDecayStringModule::buildMCDecayString(const MCParticle* mcPMother, const MCParticle* mcPMatched)
  {

    std::stringstream ss;
    ss << " ";
    if (mcPMother->getArrayIndex() == mcPMatched->getArrayIndex()) {
      ss << "^";
    }

    ss << mcPMother->getPDG();

    if (not isFSP(mcPMother->getPDG())) {
      ss << " (-->";
      for (auto daughter : mcPMother->getDaughters()) {
        ss << buildMCDecayString(daughter, mcPMatched);
      }
      ss << ")";
    }

    return ss.str();
  }

  void ParticleMCDecayStringModule::convertToConciseString(std::string& string)
  {

    std::vector<std::string> decayStrings;
    boost::split(decayStrings, string, boost::is_any_of("|"));

    if (decayStrings.empty()) {
      B2WARNING("ParticleMCDecayStringModule: unable to convert decay string to concise format.");
      return;
    }

    unsigned int nParticles(decayStrings.size() - 1);
    if (nParticles > m_identifiers.size()) {
      B2WARNING("ParticleMCDecayStringModule: not enough identifiers have been specified to use the concise string format:"
                << std::endl << "Number of particles in your decay mode = " << nParticles << std::endl
                << "Available identifiers: " << m_identifiers << std::endl
                << "Standard format will be used instead.");
      return;
    }

    //Find positions of carets in original strings, store them, and then erase them.
    std::string mode("");
    std::vector<int> caretPositions;
    for (std::vector<std::string>::iterator iter(decayStrings.begin()); iter != decayStrings.end(); ++iter) {
      std::string thisString(*iter);
      if ("" == mode) {
        mode = thisString;
        continue;
      }

      int caretPosition(thisString.find("^")); // -1 if no match.
      caretPositions.push_back(caretPosition);
      if (caretPosition > -1) {
        iter->erase(caretPosition, 1);
      }
    }

    //Check if all of the decay strings are the same (except for No matches):
    std::string theDecayString("");
    for (std::vector<std::string>::iterator iter(decayStrings.begin()); iter != decayStrings.end(); ++iter) {
      std::string thisString(*iter);
      if (thisString == mode) {continue;}

      //last decay string does not have a space at the end, don't want this to stop a match.
      char finalChar(thisString.back());
      if (finalChar != ' ') {thisString = thisString + " ";}

      if (" (No match) " != thisString) {
        if ("" == theDecayString) {
          theDecayString = thisString;
        } else {
          if (theDecayString != thisString) {
            //TODO: add string format if multiple decay strings are present (e.g. pile-up events).
            return;
          }
        }
      }
    }

    std::string modifiedString(theDecayString);

    //insert identifiers in positions where carets were:
    int nStrings(caretPositions.size());
    for (int iString(0); iString < nStrings; ++iString) {
      std::string identifier(m_identifiers.substr(iString, 1));
      int insertPosition(caretPositions.at(iString));
      if (insertPosition > -1) {
        for (int jString(0); jString < iString; ++jString) {
          if (caretPositions.at(jString) > -1 && caretPositions.at(jString) <= caretPositions.at(iString)) {
            ++insertPosition;
          }
        }
        modifiedString.insert(insertPosition, identifier);
      }
    }

    modifiedString = mode + "|" + modifiedString;

    //add a list of the unmatched particles at the end of the string:
    bool noMatchStringAdded(false);
    for (int iString(0); iString < nStrings; ++iString) {
      int insertPosition(caretPositions.at(iString));
      if (-1 == insertPosition) {
        if (!noMatchStringAdded) {
          modifiedString += " | No match: ";
          noMatchStringAdded = true;
        }
        modifiedString += m_identifiers.substr(iString, 1);
      }
    }

    string = modifiedString;
    return;
  }

} // Belle2 namespace

