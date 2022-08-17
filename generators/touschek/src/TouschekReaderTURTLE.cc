/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/touschek/TouschekReaderTURTLE.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace Belle2;


TouschekReaderTURTLE::TouschekReaderTURTLE(TGeoHMatrix* transMatrix, int pdg): m_transMatrix(transMatrix), m_pdg(pdg),
  m_lineNum(0)
{
}


TouschekReaderTURTLE::~TouschekReaderTURTLE()
{
  if (m_input) m_input.close();
}


void TouschekReaderTURTLE::open(const string& filename)
{
  m_lineNum = 0;
  m_input.open(filename.c_str());
  if (!m_input) throw(TouschekCouldNotOpenFileError() << filename);
}


int TouschekReaderTURTLE::getParticles(int number, MCParticleGraph& graph)
{
  if (m_transMatrix == NULL) {
    B2ERROR("The transformation matrix is NULL !");
  }

  int numParticles = 0;
  string currLine;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",; \t");
  double fields[7];
  double particlePosTouschek[3] = {0.0, 0.0, 0.0};
  double particlePosGeant4[3] = {0.0, 0.0, 0.0};
  double particleMomTouschek[3] = {0.0, 0.0, 0.0};
  double particleMomGeant4[3] = {0.0, 0.0, 0.0};

  do {
    getline(m_input, currLine);
    m_lineNum++;

    boost::trim(currLine);
    if (currLine == "") continue;

    tokenizer tokens(currLine, sep);

    int index = 0;
    for (const string& tok : tokens) {
      try {
        if (index >= 7) {
          B2WARNING("This Touschek file has more than 7 fields ! Only the first 7 fields were read.");
          break;
        }
        fields[index] = boost::lexical_cast<double>(tok);
        index++;
      } catch (boost::bad_lexical_cast&) {
        throw (TouschekConvertFieldError() << m_lineNum << index << tok);
      }
    }

    //Convert the position of the particle from local Touschek plane space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    particlePosTouschek[0] = fields[2] * Unit::m;
    particlePosTouschek[1] = -fields[3] * Unit::m;
    if (m_pdg == -11) particlePosTouschek[2] = 400.0; //forLER
    else if (m_pdg == 11) particlePosTouschek[2] = -400.0; //forHER
    else { B2WARNING("m_pdg is not 11/-11"); break;}

    m_transMatrix->LocalToMaster(particlePosTouschek, particlePosGeant4);

    //Convert the momentum of the particle from local Touschek plane space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    particleMomTouschek[0] = fields[4];
    particleMomTouschek[1] = -fields[5];
    if (m_pdg == -11) particleMomTouschek[2] = -fields[6]; //forLER
    else if (m_pdg == 11) particleMomTouschek[2] = fields[6]; //forHER
    else { B2WARNING("m_pdg is not 11/-11"); break;}

    m_transMatrix->LocalToMasterVect(particleMomTouschek, particleMomGeant4);

    double totalMom2 = particleMomGeant4[0] * particleMomGeant4[0];
    totalMom2 += particleMomGeant4[1] * particleMomGeant4[1];
    totalMom2 += particleMomGeant4[2] * particleMomGeant4[2];

    //Add particles to MCParticle collection
    MCParticleGraph::GraphParticle& particle = graph.addParticle();
    particle.setStatus(MCParticle::c_PrimaryParticle);
    particle.setPDG(m_pdg);
    particle.setMassFromPDG();
    particle.setMomentum(ROOT::Math::XYZVector(particleMomGeant4[0], particleMomGeant4[1], particleMomGeant4[2]));
    particle.setProductionVertex(ROOT::Math::XYZVector(particlePosGeant4[0], particlePosGeant4[1], particlePosGeant4[2]));
    particle.setEnergy(sqrt(totalMom2 + particle.getMass()*particle.getMass()));
    particle.setProductionTime(0.0);
    particle.setValidVertex(true);

    numParticles++;
  } while ((!m_input.eof()) && ((number > -1) && (numParticles < number)));

  return numParticles;
}
