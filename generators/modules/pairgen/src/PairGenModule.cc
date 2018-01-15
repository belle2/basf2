/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/pairgen/PairGenModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <TMath.h>

#include <TRandom3.h>
#include <TLorentzVector.h>

using namespace TMath;
using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PairGen)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PairGenModule::PairGenModule() : Module()
{
  //Set module properties
  setDescription(
    "Simple module to generate tracks of given PDG back to back in CMS.\n"
  );
  setPropertyFlags(c_Input);

  //Set default values for parameters
  m_PDG = 11;

  //Parameter definition
  addParam("pdgCode", m_PDG,
           "PDG code for generated particles", 11);
  addParam("saveBoth", m_saveBoth,
           "Store both particles if true, one if false", true);
}

void PairGenModule::initialize()
{
  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();
}

void PairGenModule::event()
{
  try {
    m_particleGraph.clear();

    //Make list of particles
    MCParticleGraph::GraphParticle& p = m_particleGraph.addParticle();
    p.setStatus(MCParticle::c_PrimaryParticle);
    p.setPDG(m_PDG);
    p.setMassFromPDG();
    p.setFirstDaughter(0);
    p.setLastDaughter(0);

    MCParticleGraph::GraphParticle& q = m_particleGraph.addParticle();
    q.setStatus(MCParticle::c_PrimaryParticle);
    q.setPDG(-1 * m_PDG);
    q.setMassFromPDG();
    q.setFirstDaughter(0);
    q.setLastDaughter(0);

    //generate the phi uniformly in CMS and theta with 1 + Cos(Theta)^2 :
    double phi = gRandom->Uniform(0, 2.0 * Pi());
    double theta = 0;
    double value = 0;
    while (1 + Cos(theta)*Cos(theta) > value) {
      theta = gRandom->Uniform(0, 1.0 * Pi());
      value = gRandom->Uniform(0, 2.0);
    }

    double m  = p.getMass();

    double momentum = Sqrt(10.583 * 10.583 / 4 - m * m);

    double pz = momentum * Cos(theta);
    double px = momentum * Sin(theta) * Cos(phi);
    double py = momentum * Sin(theta) * Sin(phi);
    double e  = Sqrt(momentum * momentum + m * m);

    TLorentzVector vp(px, py, pz, e);
    vp.Boost(0, 0, 0.272727);
    TLorentzVector vq(-px, -py, -pz, e);
    vq.Boost(0, 0, 0.272727);

    p.setMomentum(vp(0), vp(1), vp(2));
    p.setEnergy(vp(3));
    p.setProductionVertex(0, 0, 0);
    p.addStatus(MCParticle::c_StableInGenerator);
    //Particle is stable in generator. We could use MCParticleGraph options to
    //do this automatically but setting it here makes the particle correct
    //independent of the options
    p.setDecayTime(numeric_limits<double>::infinity());

    // set time offset to check fit bias in e.g. the ECL waveform fits
    p.setProductionTime(0);

    q.setMomentum(vq(0), vq(1), vq(2));
    q.setEnergy(vq(3));
    q.setProductionVertex(0, 0, 0);
    if (m_saveBoth) {
      q.addStatus(MCParticle::c_StableInGenerator);
    } else {
      q.addStatus(MCParticle::c_IsVirtual);
    }
    //Particle is stable in generator. We could use MCParticleGraph options to
    //do this automatically but setting it here makes the particle correct
    //independent of the options
    q.setDecayTime(numeric_limits<double>::infinity());

    // set time offset to check fit bias in e.g. the ECL waveform fits
    q.setProductionTime(0);

    m_particleGraph.generateList();
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}

