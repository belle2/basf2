/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013-2020 - Belle II Collaboration                        *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo, Anze Zupanc, Sam Cunliffe               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/ParticleStats/ParticleStatsModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <nlohmann/json.hpp>
#include <fstream>


using namespace Belle2;

// Register module in the framework
REG_MODULE(ParticleStats)

ParticleStatsModule::ParticleStatsModule() : Module()
{
  // Set module properties and parameters
  setDescription("Make a summary of specific ParticleLists.");
  addParam("outputFile", m_outputFile, "Name of output file", std::string());
  addParam("printPassMatrix", m_printPassMatrix, "Should we also calculate and print the pass matrix?", true);
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", std::vector<std::string>());
}

void ParticleStatsModule::initialize()
{
  m_particles.isRequired();
  m_nLists = m_strParticleLists.size();

  // check all particle lists are valid
  for (unsigned i = 0; i < m_nLists; ++i) {
    bool valid = m_decaydescriptor.init(m_strParticleLists[i]);
    if (!valid)
      B2ERROR("Invalid input list name: " << m_strParticleLists[i]);

    if (m_decaydescriptor.getNDaughters() > 0)
      B2ERROR("ParticleStatsModule::initialize Invalid input DecayString " << m_strParticleLists[i]
              << ". DecayString should not contain any daughters, only the mother particle.");
  }
  B2INFO("Number of ParticleLists to be studied: " << m_nLists << " ");

  m_PassMatrix = new TMatrix(m_nLists, m_nLists + 1);
  m_MultiplicityMatrix = new TMatrix(m_nLists, 4); // 0 All particles; 1 Negative; 2 Positive; 3 SelfConjugated
}

void ParticleStatsModule::event()
{
  bool unique = true;
  bool pass = false;

  // first loop over all particle lists we are interested in
  for (unsigned iList = 0; iList < m_nLists; ++iList) {

    // check that the list exists and is not empty
    StoreObjPtr<ParticleList> particlelist(m_strParticleLists[iList]);
    if (!particlelist) {
      B2ERROR("ParticleListi " << m_strParticleLists[iList] << " not found");
      continue;
    }
    if (!particlelist->getListSize()) continue;

    pass = true;

    /**************************************************************************
     * For the multiplicity table, count the number of candidates in each
     * list. Add to the total for all events processed so far.
     */

    // all particles and anti-particles, or self-conjugate
    (*m_MultiplicityMatrix)(iList, 0) = (*m_MultiplicityMatrix)(iList, 0)
                                        + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle)
                                        + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true)
                                        + particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle);

    // particles alone
    if (particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle))
      (*m_MultiplicityMatrix)(iList, 1) = (*m_MultiplicityMatrix)(iList, 1)
                                          + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle);

    // anti-particles alone
    if (particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true))
      (*m_MultiplicityMatrix)(iList, 2) = (*m_MultiplicityMatrix)(iList, 2)
                                          + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true);

    // self-conjugate particles alone
    if (particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle))
      (*m_MultiplicityMatrix)(iList, 3) = (*m_MultiplicityMatrix)(iList, 3)
                                          + particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle);

    /**************************************************************************
     * Now, loop again over all particle lists to fill the pass matrix for this
     * event. Add to the totals for all events processed so far.
     */
    for (unsigned jList = 0; jList < m_nLists; ++jList) {

      // check that the list exists and is not empty
      StoreObjPtr<ParticleList> particlelistj(m_strParticleLists[jList]);
      if (!particlelistj) {
        B2INFO("ParticleListj " << m_strParticleLists[jList] << " not found");
        continue;
      }
      if (!particlelistj->getListSize()) continue;

      // if the event passed both the i and j list selection then increment
      (*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) + 1.;
      if (iList != jList) unique = false;

    } // end inner loop over all particle lists

    if (unique)(*m_PassMatrix)(iList, m_nLists) = (*m_PassMatrix)(iList, m_nLists) + 1.;

  } // end outer loop over all particle lists

  // finally count the particles created in this event, and the number of events
  // passing any of the list selections that we were interested in
  m_nParticles += m_particles.getEntries();
  if (pass) m_nPass++;
}

// finish up the calculation and print a table to B2INFO
void ParticleStatsModule::terminate()
{
  auto nEvents = (float)Environment::Instance().getNumberOfEvents();
  B2INFO("ParticleStats Summary:");
  std::ostringstream stream;

  nlohmann::json json;

  // divide every entry of the pass matrix by the number of events processed
  // (entrywise) to get the fraction of events
  for (unsigned iList = 0; iList < m_nLists; ++iList)
    for (unsigned jList = 0; jList < m_nLists + 1; ++jList)
      (*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) / nEvents;

  for (unsigned iList = 0; iList < m_nLists; ++iList)
    for (unsigned jList = 0; jList < m_nLists + 1; ++jList)
      if (iList != jList && (*m_PassMatrix)(iList, iList) > 0.)
        (*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) / (*m_PassMatrix)(iList, iList) ;

  if (m_printPassMatrix && m_nLists > 1) {

    // format the pass matrix table
    stream << "=======================================================================\n";
    stream << " Pass matrix (for i, j: fraction passing selection i also passes selection j)\n\n";
    stream << "                  ";

    // table headings
    for (unsigned iList = 0; iList < m_nLists; ++iList)
      stream << "|" << Form("  %5d", iList);
    stream << "| Unique|\n";

    // table data
    for (unsigned iList = 0; iList < m_nLists; ++iList) {
      // first column is particle list name
      stream <<  Form("%14s(%2d)", m_strParticleLists[iList].c_str(), iList) << "|";
      // next N columns are particle list survival fractions (need to use tabs here)
      for (unsigned jList = 0; jList < m_nLists + 1; ++jList) { // matrix
        if (iList != jList) {
          stream << Form(" %6.4f|", (*m_PassMatrix)(iList, jList));
          std::string jName = (jList < m_nLists ? m_strParticleLists[jList].c_str() : "Unique");
          json["Pass matrix"][m_strParticleLists[iList].c_str()][jName.c_str()] = (*m_PassMatrix)(iList, jList);
        }
        if (iList == jList) {
          stream << Form(" %6.4f|", 1.0);
          json["Pass matrix"][m_strParticleLists[iList].c_str()][m_strParticleLists[jList].c_str()] = 1.0;
        }

      }
      stream << "\n";
    }
    B2INFO(stream.str()); // print the pass matrix table


  } // end if

  // clear the stream and format the multiplicity table
  stream.str("");
  stream << "\n======================================================================\n";
  stream << " Average Candidate Multiplicity (ACM) and ACM for Passed Events (ACMPE) \n\n";

  // table headings
  stream << "                             |   All Particles   |     Particles     |  Anti-particles   |  Self-conjugates  |\n";
  stream << "                  | Retention|      ACM|    ACMPE|      ACM|    ACMPE|      ACM|    ACMPE|      ACM|    ACMPE|\n";

  // table data
  for (unsigned iList = 0; iList < m_nLists; ++iList) {
    // first column is particle list name
    stream <<  Form("%14s(%2d)", m_strParticleLists[iList].c_str(), iList) << "|";
    // second column is retention (no need to use tabs here because it can't be
    // more than 1.{followed by 4 digits})
    stream << Form("    %6.4f|", (*m_PassMatrix)(iList, iList));

    std::string pName  = m_strParticleLists[iList].c_str();
    float retRate = (*m_PassMatrix)(iList, iList);
    json["Retention"][pName]["Retention"] = retRate;
    std::string flavs[4] = {"All Particles", "Particles", "Anti Particles",  "Self-conjugates"};
    // now the ACM and ACMPE
    for (int iFlav = 0; iFlav < 4; ++iFlav) {
      stream << Form(" %8.4f|", (*m_MultiplicityMatrix)(iList, iFlav) / nEvents);
      json["Retention"][pName][Form("%s ACM", flavs[iFlav].c_str())] = (*m_MultiplicityMatrix)(iList, iFlav) / nEvents;

      stream << Form(" %8.4f|", (*m_MultiplicityMatrix)(iList, iFlav) / nEvents / (*m_PassMatrix)(iList, iList));
      json["Retention"][pName][Form("%s ACPME", flavs[iFlav].c_str())] = (*m_MultiplicityMatrix)(iList,
          iFlav) / nEvents / (*m_PassMatrix)(iList,
                                             iList);
    }
    stream << "\n";
  }
  B2INFO(stream.str()); // print the multiplicity table

  // now print some global information
  stream.str("");
  stream << "\n======================================================================\n";
  stream << "Total Retention: " << m_nPass << " events passing / " << nEvents << " events processed = " << Form("%6.4f\n",
         (float)m_nPass / (float)nEvents);
  stream << "Total Number of Particles created in the DataStore: " << m_nParticles;
  stream << "\n======================================================================\n";
  json["Total retention"] = m_nPass / nEvents;
  json["Events passing"] = m_nPass;
  json["Events processed"] = nEvents;
  json["Total particles number"] = m_nParticles;
  B2INFO(stream.str());
  if (m_outputFile != "") {
    std::ofstream jsonFile(m_outputFile);
    jsonFile << json.dump(2) << std::endl;
  }

  delete m_PassMatrix;
  delete m_MultiplicityMatrix;
}
