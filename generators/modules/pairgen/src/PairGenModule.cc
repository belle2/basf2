/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/pairgen/PairGenModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/MCInitialParticles.h>

#include <TMath.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TLorentzRotation.h>

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
  m_initialParticleGeneration.initialize();
}

void PairGenModule::event()
{
  try {
    m_particleGraph.clear();
    MCInitialParticles initial = m_initialParticleGeneration.generate();

    //Distribution values:
    //generate phi uniformly in CMS and theta with 1 + Cos(Theta)^2 :
    double phi = gRandom->Uniform(0, 2.0 * Pi());
    double theta = 1.57;
    double value = 2;
    while (1 + Cos(theta)*Cos(theta) < value) {
      theta = gRandom->Uniform(0, 1.0 * Pi());
      value = gRandom->Uniform(0, 2.0);
    }
    double CMSEnergy = initial.getMass();
    TLorentzRotation boostCMSToLab = initial.getCMSToLab();

    int nParticles = 1;
    if (m_saveBoth) {
      nParticles = 2;
    }
    for (int n = 1; n <= nParticles; n++) {
      //Make list of particles
      MCParticleGraph::GraphParticle& p = m_particleGraph.addParticle();
      int sign = (n % 2) * 2 - 1;
      p.setStatus(MCParticle::c_PrimaryParticle);
      p.setPDG(sign * m_PDG);
      p.setMassFromPDG();
      p.setFirstDaughter(0);
      p.setLastDaughter(0);

      double m  = p.getMass();
      double momentum = Sqrt(CMSEnergy * CMSEnergy / 4 - m * m);

      double pz = momentum * Cos(theta);
      double px = momentum * Sin(theta) * Cos(phi);
      double py = momentum * Sin(theta) * Sin(phi);
      double e  = Sqrt(momentum * momentum + m * m);

      TLorentzVector vp(sign * px, sign * py, sign * pz, e);
      vp.Transform(boostCMSToLab);

      p.setMomentum(vp(0), vp(1), vp(2));
      p.setEnergy(vp(3));
      p.setProductionVertex(initial.getVertex());
      p.addStatus(MCParticle::c_StableInGenerator);
      //Particle is stable in generator. We could use MCParticleGraph options to
      //do this automatically but setting it here makes the particle correct
      //independent of the options
      p.setDecayTime(numeric_limits<double>::infinity());

      // set time offset to check fit bias in e.g. the ECL waveform fits
      p.setProductionTime(initial.getTime());
    }

    m_particleGraph.generateList();
  } catch (runtime_error& e) {
    B2ERROR(e.what());
  }
}

