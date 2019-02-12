/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/hepmc/HepMCReader.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;


void HepMCReader::open(const string& filename)
{
  B2INFO("Reading HEPMC inputfile at " << filename);
  m_input.open(filename.c_str());
  if (!m_input) { throw (HepMCCouldNotOpenFileError() << filename); }
}


int HepMCReader::getEvent(MCParticleGraph& graph, double& eventWeight)
{
  int eventID = -1;

  HepMC::GenEvent evt;

  readNextEvent(evt);

  eventID = evt.event_number();
  eventWeight = 1; // why is evt.weights() a std::vector?
  const int nparticles = evt.particles_size();

  B2DEBUG(10, "Found eventID " << eventID << " with " << nparticles << " particles.");
  //evt.print(); // print pythia event content
  if (nparticles <= 0) {
    throw (HepMCEmptyEventError() << nparticles);
  }

  // Fill particle graph with this event
  const int event_offset = graph.size(); //offset
  //Make list of particles; Prepare graph
  for (int i = 0; i < nparticles; i++) {
    graph.addParticle();
  }

  std::unordered_map<int, int> hash_index_map;
  HepMC::GenEvent::particle_iterator tmp_particle = evt.particles_begin();
  for (int i = 0; i < nparticles; ++i) {
    //the barcode seems to just do 0++ etc. but im not sure if there are exceptions
    const int hash = (*tmp_particle)->barcode();
    hash_index_map[hash] = i;
    ++tmp_particle;
  }

  auto read_particle = evt.particles_begin();
  //Read particles from file
  for (int i = 0; i < nparticles; ++i) {
    auto* decay_vertex = (*read_particle)->end_vertex();
    auto* production_vertex = (*read_particle)->production_vertex();

    MCParticleGraph::GraphParticle& p = graph[event_offset + i];

    const int status = (*read_particle)->status();
    const bool isFinalstate =  !decay_vertex && status == 1;
    const bool isVirtual  = (status == 4) || (status == 21) || (status == 22) || (status == 23); //TODO refine
    const int pdg_code = (*read_particle)->pdg_id() ;
    const double mass = (*read_particle)->generated_mass();
    const auto momentum = (*read_particle)->momentum();

    B2DEBUG(10, "Read particle: status " << status << " isFinal " << isFinalstate << " isVirtual " << isVirtual << " pdg " << pdg_code
            << " mass " << mass << " px " << momentum.x() << " py " << momentum.y() << " px " << momentum.z() << " E " << momentum.t());

    p.setPDG(pdg_code);
    p.setMomentum(TVector3(momentum.x(), momentum.y(), momentum.z()));
    p.setEnergy(momentum.t());
    p.setMass(mass);
    if (production_vertex) {
      const auto pos = production_vertex->position();
      p.setProductionVertex(TVector3(pos.x(), pos.y(), pos.z()) * Unit::mm);
      p.setProductionTime(pos.t() * Unit::mm / Const::speedOfLight);
      p.setValidVertex(true);
    }

    // number of particles attached to this particles decay vertex
    //int ndaughters = 0;
    //if (decay_vertex) {
    //  ndaughters = decay_vertex->particles_out_size();
    //  //std::cout << "  Found vertex " << pdg_code << " ----------------------------"   << std::endl;
    //  int count = 0;
    //  for (auto daughter = decay_vertex->particles_begin(HepMC::children);
    //       daughter != decay_vertex->particles_end(HepMC::children); ++daughter) {
    //    std::cout << "     Daughter nr " << ++count << " pdg " << (*daughter)->pdg_id() <<" hash " << (*daughter)->barcode() <<  std::endl;
    //    //(*daughter)->print();
    //  }
    //}
    if (decay_vertex) {
      int daughter_idx = 0;
      for (auto daughter = decay_vertex->particles_begin(HepMC::children);
           daughter != decay_vertex->particles_end(HepMC::children); ++daughter) {
        const int daughter_index_in_graph = hash_index_map[(*daughter)->barcode()];
        p.decaysInto(graph[event_offset + daughter_index_in_graph]);
        daughter_idx++;
      }
    }
    //boost particles to lab frame:
    TLorentzVector p4 = p.get4Vector();
    if (m_wrongSignPz) { // this means we have to mirror Pz
      p4.SetPz(-1.0 * p4.Pz());
    }
    p4 = m_labboost * p4;
    p.set4Vector(p4);

    ////check if particle should be made virtual according to steering options:
    if (i < m_nVirtual || isVirtual) {
      p.setVirtual();
    }
    ++read_particle;
  }
  eventID += 1;
  B2DEBUG(10, "Returning event id " << eventID);
  return eventID;
}

void HepMCReader::readNextEvent(HepMC::GenEvent& evt)
{
  // walk through the IOStream
  if (m_input) {
    evt.read(m_input);
    if (evt.is_valid()) {
      const int nparticles = evt.particles_size();
      B2DEBUG(10, "Found valid event.i N particles " << nparticles);
      return; //
    } else {
      B2DEBUG(10, "The next event was invalid. Will stop reading now.");
    }
  }
  return;
}

int HepMCReader::countEvents(const std::string& filename)
{
  //different way to read file for consitency check
  HepMC::IO_GenEvent ascii_in(filename.c_str(), std::ios::in);
  int count = 0;
  HepMC::GenEvent* evt = ascii_in.read_next_event();
  while (evt) {
    evt = ascii_in.read_next_event();
    count++;
  }
  B2INFO("Counted " << count  << " events in  " << filename << ".");
  return count;
}


