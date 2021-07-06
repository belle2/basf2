/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <generators/lhe/LHEReader.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <TF1.h>
#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;

const boost::char_separator<char> LHEReader::sep(",; \t");

void LHEReader::open(const string& filename)
{
  m_lineNr = 0;
  m_input.open(filename.c_str());
  if (!m_input) throw(LHECouldNotOpenFileError() << filename);
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

  double r, x = 0, y = 0, z = 0, t = 0;
  //Read particles from file
  for (int i = 0; i < nparticles; ++i) {
    MCParticleGraph::GraphParticle& p = graph[first + i];
    int mother = readParticle(p);

    // add the mother
    if (mother > 0) {
      MCParticleGraph::GraphParticle* q = &graph[mother - 1];
      p.comesFrom(*q);
    }

    //move vertex position of selected particle and its daughters
    if (m_meanDecayLength > 0) {
      if (p.getPDG() == m_pdgDisplaced) {
        TF1 fr("fr", "exp(-x/[0])", 0, 1000000);
        TLorentzVector p4 = p.get4Vector();
        fr.SetRange(m_Rmin, m_Rmax);
        fr.SetParameter(0, m_meanDecayLength * p4.Gamma());
        r = fr.GetRandom();
        x = r * p4.Px() / p4.P();
        y = r * p4.Py() / p4.P();
        z = r * p4.Pz() / p4.P();
        p.setDecayVertex(TVector3(x, y, z));
        t = (r / Const::speedOfLight) * (p4.E() / p4.P());
        p.setDecayTime(t);
        p.setValidVertex(true);
      }

      if (mother > 0) {
        if (graph[mother - 1].getPDG() == m_pdgDisplaced) {
          p.setProductionVertex(TVector3(x, y, z));
          p.setProductionTime(t);
          p.setValidVertex(true);
        }
      }
    }

    // boost particles to lab frame: both momentum and vertex
    TLorentzVector p4 = p.get4Vector();
    TLorentzVector v4;
    if (m_wrongSignPz) // this means we have to mirror Pz
      p4.SetPz(-1.0 * p4.Pz());
    p4 = m_labboost * p4;
    p.set4Vector(p4);
    if (p.getPDG() == m_pdgDisplaced) {
      v4.SetXYZT(p.getDecayVertex().X(), p.getDecayVertex().Y(), p.getDecayVertex().Z(), Const::speedOfLight * p.getDecayTime());
      v4 = m_labboost * v4;
      p.setDecayVertex(v4.X(), v4.Y(), v4.Z());
      p.setDecayTime(v4.T() / Const::speedOfLight);
    } else if (mother > 0) {
      if (graph[mother - 1].getPDG() == m_pdgDisplaced) {
        v4.SetXYZT(p.getProductionVertex().X(), p.getProductionVertex().Y(), p.getProductionVertex().Z(),
                   Const::speedOfLight * p.getProductionTime());
        v4 = m_labboost * v4;
        p.setProductionVertex(v4.X(), v4.Y(), v4.Z());
        p.setProductionTime(v4.T() / Const::speedOfLight);
      }
    }


    // initial 2 (e+/e-), virtual 3 (Z/gamma*)
    // check if particle should be made virtual according to steering options:
    if (i < m_indexVirtual && i >= m_indexInitial)
      p.addStatus(MCParticle::c_IsVirtual);

    if (i < m_indexInitial)
      p.addStatus(MCParticle::c_Initial);

    if (m_indexVirtual < m_indexInitial) B2WARNING("IsVirtual particle requested but is overwritten by Initial");

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
