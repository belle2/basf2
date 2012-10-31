/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

const TParticlePDG* Const::ParticleType::particlePDG() const
{
  return Const::EvtGenDatabasePDG::instance()->GetParticle(m_pdgCode);
}

const Const::ParticleType Const::electron = Const::ParticleType(11);
const Const::ParticleType Const::muon = Const::ParticleType(13);
const Const::ParticleType Const::pion = Const::ParticleType(211);
const Const::ParticleType Const::kaon = Const::ParticleType(321);
const Const::ParticleType Const::proton = Const::ParticleType(2212);

const double Const::electronMass = Const::electron.particlePDG()->Mass();
const double Const::muonMass = Const::muon.particlePDG()->Mass();
const double Const::pionMass = Const::pion.particlePDG()->Mass();
const double Const::kaonMass = Const::kaon.particlePDG()->Mass();
const double Const::protonMass = Const::proton.particlePDG()->Mass();

const double Const::speedOfLight   = 29.9792458;
const double Const::kBoltzmann     = 8.617343 * 1.0e-5 * Unit::eV / Unit::K;
const double Const::ehEnergy       = 3.65 * Unit::eV;
const double Const::fineStrConst   = 1.0 / 137.036;
const double Const::permSi         = 11.9 * 8.8542 * 1e-18 * Unit::C / Unit::V / Unit::um;
const double Const::uTherm         = 0.026 * Unit::V;
const double Const::eMobilitySi    = 1415 * Unit::cm2 / Unit::V / Unit::s;


TDatabasePDG* Const::EvtGenDatabasePDG::instance()
{
  if (!fgInstance) {
    std::string fileName = std::getenv("BELLE2_EXTERNALS_DIR");
    fileName += "/share/evtgen/evt.pdl";
    TDatabasePDG::Instance()->ReadEvtGenTable(fileName.c_str());
  }
  return TDatabasePDG::Instance();
}
