/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <generators/lhe/LHEReader.h>

#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;

const boost::char_separator<char> LHEReader::sep(",; \t");

void LHEReader::open(const string& filename)
{
  m_lineNr = 0;
  m_input.open(filename.c_str());
  if (!m_input) throw(LHECouldNotOpenFileError() << filename);
  fr = new TF1("fr", "exp(-x/[0])", 0, 100000);
  tr = new TRandom();
}


int LHEReader::getEvent(MCParticleGraph& graph, double& eventWeight)
{
//   int eventID = -1;
//   int nparticles = readEventHeader(eventID, eventWeight);
  int nparticles = readEventHeader(eventWeight);
  if (nparticles <= 0) {
    throw (LHEEmptyEventError() << m_lineNr << nparticles);
  }

  int first = graph.size();
  //Make list of particles
  for (int i = 0; i < nparticles; i++) {
    graph.addParticle();
  }

  double r, x, y, z;
  //Read particles from file
  for (int i = 0; i < nparticles; ++i) {
    MCParticleGraph::GraphParticle& p = graph[first + i];
    int mother = readParticle(p);

    // add the mother
    if (mother > 0) {
      MCParticleGraph::GraphParticle* q = &graph[mother - 1];
      p.comesFrom(*q);
    }

    // boost particles to lab frame:
    TLorentzVector p4 = p.get4Vector();
    if (m_wrongSignPz) // this means we have to mirror Pz
      p4.SetPz(-1.0 * p4.Pz());
    p4 = m_labboost * p4;
    p.set4Vector(p4);

    //move vertex position of FSR particle and dark photon
    if (m_l0 > 0 && p.getPDG() != 22) {
      if (p.getPDG() == 9000008) {
        fr->SetParameter(0, m_l0 * p4.Gamma());
        r = fr->GetRandom();
        tr->Sphere(x, y, z, r);
      }
      p.setProductionVertex(TVector3(x, y, z));
      p.setValidVertex(true);
    }

    // initial 2 (e+/e-), virtual 3 (Z/gamma*)
    // check if particle should be made virtual according to steering options:
    if (i < m_nVirtual && i >= m_nInitial)
      p.addStatus(MCParticle::c_IsVirtual);

    if (i < m_nInitial)
      p.addStatus(MCParticle::c_Initial);

    if (m_nVirtual < m_nInitial) B2WARNING("IsVirtual particle requested but is overwritten by Initial");

  }
//   return eventID;
  return -1;
}


bool LHEReader::skipEvents(int n)
{
//   int eventID;
  double weight;
  for (int i = 0; i < n; i++) {
//     int nparticles = readEventHeader(eventID, weight);
    int nparticles = readEventHeader(weight);
    if (nparticles < 0) return false;
    for (int j = 0; j < nparticles; j++) getLine();
  }
  return true;
}


//===================================================================
//                  Protected methods
//===================================================================

std::string LHEReader::getLine()
{
  std::string line;
  do {
    getline(m_input, line);
    m_lineNr++;
    size_t commentPos = line.find_first_of('#');
    if (commentPos != string::npos) {
      line = line.substr(0, commentPos);
    }
    boost::trim(line);

  } while (line == "" && !m_input.eof());

  return line;
}


// int LHEReader::readEventHeader(int& eventID, double& eventWeight)
int LHEReader::readEventHeader(double& eventWeight)
{

  // Search for next <event>
  std::string line2;
  do {
    getline(m_input, line2);
    m_lineNr++;
    size_t commentPos = line2.find_first_of('#');
    if (commentPos != string::npos) {
      line2 = line2.substr(0, commentPos);
    }
    boost::trim(line2);

  } while (line2 != "<event>" && !m_input.eof());

  //Get number of particles from file
  int nparticles = -1;
  string line = getLine();

  if (line == "" || m_input.eof()) return -1;

  vector<double> fields;
  fields.reserve(15);

  tokenizer tokens(line, sep);
  int index(0);

  BOOST_FOREACH(const string & tok, tokens) {
    ++index;
    try {
      fields.push_back(boost::lexical_cast<double>(tok));
    } catch (boost::bad_lexical_cast& e) {
      throw (LHEConvertFieldError() << m_lineNr << index << tok);
    }
  }

  switch (fields.size()) {
    default:
      eventWeight = 1.0;
      nparticles = static_cast<int>(fields[0]); //other fields in LHE contain effective couplings
      break;
  }
  return nparticles;
}


int LHEReader::readParticle(MCParticleGraph::GraphParticle& particle)
{
  int mother = -1;

  string line = getLine();
  vector<double> fields;
  fields.reserve(15);

  tokenizer tokens(line, sep);
  int index(0);

  BOOST_FOREACH(const string & tok, tokens) {
    ++index;
    try {
      fields.push_back(boost::lexical_cast<double>(tok));
    } catch (boost::bad_lexical_cast& e) {
      throw (LHEConvertFieldError() << m_lineNr << index << tok);
    }
  }

  switch (fields.size()) {
    case 13:
      particle.addStatus(MCParticle::c_PrimaryParticle);
      particle.setPDG(static_cast<int>(fields[0]));
      mother = static_cast<int>(fields[2]);
      particle.setMomentum(TVector3(&fields[6]));
      particle.setMass(fields[10]);
      break;
    default:
      throw (LHEParticleFormatError() << m_lineNr << fields.size());
  }

  return mother;
}
