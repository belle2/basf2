/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <masterclass/modules/MasterClassModule.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <analysis/dataobjects/Particle.h>

#include <TDirectory.h>
#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MasterClass);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MasterClassModule::MasterClassModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Module to write out data in a format for Belle II masterclasses)DOC");
  addParam("outputFileName", m_filename, "File name of root ntuple output.", std::string("masterclass.root"));
}

void MasterClassModule::initialize()
{
  m_tracks.isRequired();
  m_clusters.isRequired();

  m_event = new BEvent;
  m_file = TFile::Open(m_filename.c_str(), "RECREATE");
  m_tree = new TTree("T", "Event Tree");
  m_tree->Branch("BEvent", &m_event);
}

void MasterClassModule::event()
{
  m_event->Clear();
  m_event->EventNo(m_index++);

  for (auto& track : m_tracks) {
    auto pid = track.getRelated<PIDLikelihood>();
    const double priors[] = {0.05, 0.05, 0.65, 0.24, 0.01, 0};
    auto type = pid->getMostLikely(priors);

    Const::PIDDetectorSet detectorSet = Const::PIDDetectors::set();

    auto trackFit = track.getTrackFitResultWithClosestMass(type);
    auto p = trackFit->getMomentum();
    double m = type.getMass();
    SIMPLEPID id = ALL;
    switch (type.getPDGCode()) {
      case 11: id = ELECTRON; break;
      case 13: id = MUON; break;
      case 211: id = PION; break;
      case 321: id = KAON; break;
      case 2212: id = PROTON; break;
      default: id = ALL;
    }
    m_event->AddTrack(
      p.x(), p.y(), p.z(), sqrt(m * m + p.Mag2()),
      trackFit->getChargeSign(), id,
      pid->getLogL(Const::ChargedStable(11), detectorSet), // e
      pid->getLogL(Const::ChargedStable(13), detectorSet), // mu
      pid->getLogL(Const::ChargedStable(211), detectorSet), // pi
      pid->getLogL(Const::ChargedStable(321), detectorSet), // K
      pid->getLogL(Const::ChargedStable(2212), detectorSet), // p
      pid->getLogL(Const::ChargedStable(1000010020), detectorSet) // d
    );
  }

  for (auto& cluster : m_clusters) {
    if (!cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
    double E = cluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    if (E < 0.1) continue;
    ROOT::Math::XYZVector p = cluster.getClusterPosition();
    p *= (E / p.R());
    m_event->AddTrack(
      p.x(), p.y(), p.z(), E,
      0, PHOTON
    );
  }

  m_tree->Fill();
}

void MasterClassModule::terminate()
{
  m_file->cd();
  m_tree->Write();
  m_file->Close();
  delete m_file;
}
