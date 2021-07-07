/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/MCParticle.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/HTML.h>

#include <TDatabasePDG.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace Belle2;

const double MCParticle::c_epsilon = 10e-7;


void MCParticle::setMassFromPDG()
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdg) == nullptr)
    throw(ParticlePDGNotKnownError() << m_pdg);
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdg)->Mass();
}


float MCParticle::getCharge() const
{
  // Geant4 "optical photon" (m_pdg == 0) is not known to TDatabasePDG::Instance().
  if (m_pdg == 0) {
    return 0.0;
  }

  if (TDatabasePDG::Instance()->GetParticle(m_pdg) == nullptr) {
    B2ERROR("PDG=" << m_pdg << " ***code unknown to TDatabasePDG");
    return 0.0;
  }

  return TDatabasePDG::Instance()->GetParticle(m_pdg)->Charge() / 3.0;
}


vector<MCParticle*> MCParticle::getDaughters() const
{
  vector<MCParticle*> result;
  if (m_firstDaughter > 0) {
    fixParticleList();
    if (m_lastDaughter > m_plist->GetEntriesFast()) throw LastChildIndexOutOfRangError();
    TClonesArray& plist = *m_plist;
    result.reserve(m_lastDaughter - m_firstDaughter + 1);
    for (int i = m_firstDaughter - 1; i < m_lastDaughter; i++) {
      result.push_back(static_cast<MCParticle*>(plist[i]));
    }
  }
  return result;
}

int MCParticle::getNDaughters() const
{
  if (getFirstDaughter() == 0) //no daughters
    return 0;
  return getLastDaughter() - getFirstDaughter() + 1;
}

void MCParticle::fixParticleList() const
{
  if (m_plist != 0) return;

  TClonesArray* plist(0);

  //Search default location
  //TODO: this could be replaced with RelationsObject::getArrayIndex()/getArrayName()
  StoreArray<MCParticle> MCParticles;
  if (MCParticles && MCParticles.getPtr()->IndexOf(this) >= 0) {
    plist = MCParticles.getPtr();
  } else {
    //Search all StoreArrays which happen to store MCParticles
    const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
    for (DataStore::StoreEntryConstIter iter = map.begin(); iter != map.end(); ++iter) {
      TClonesArray* value = dynamic_cast<TClonesArray*>(iter->second.ptr);
      if (value && value->GetClass() == Class() && value->IndexOf(this) >= 0) {
        plist = value;
        break;
      }
    }
  }
  //Could not find any collection, raise exception
  if (!plist) {
    B2ERROR("Could not determine StoreArray the MCParticle belongs to !");
    throw NoParticleListSetError();
  }

  //Set plist pointer and index for whole array
  for (int i = 0; i < plist->GetEntriesFast(); i++) {
    MCParticle& mc = *(static_cast<MCParticle*>(plist->At(i)));
    mc.m_plist = plist;
    mc.m_index = i + 1;
  }
}
std::string MCParticle::getName() const
{
  const TParticlePDG* p = TDatabasePDG::Instance()->GetParticle(m_pdg);
  if (p)
    return p->GetName();
  else //handle unknown PDG codes
    return std::to_string(m_pdg);
}
std::string MCParticle::getInfoHTML() const
{
  std::stringstream out;
  out << "<b>Charge</b>=" << (int)getCharge();
  out << ", <b>PDG</b>=" << getPDG();
  out << " (" << getName() << ")";
  out << "<br>";
  out << "<b>isPrimaryParticle</b>=" << isPrimaryParticle();
  out << ",<b>isInitial</b>=" << isInitial();
  out << ",<b>isVirtual</b>=" << isVirtual();
  out << "<br>";

  out << "<b>pT</b>=" << getMomentum().Pt();
  out << ", <b>pZ</b>=" << m_momentum_z;
  out << "<br>";
  std::string unitType = HTML::chooseUnitOfLength(getProductionVertex());
  int precision = 3;
  out << "<b>V</b>=" << HTML::getStringConvertToUnit(getProductionVertex(), precision, unitType);

  const MCParticle* mom = getMother();
  if (mom) {
    out << "<br>";
    out << "<b>Mother</b>: " << mom->getArrayName() << "[" << mom->getArrayIndex() << "] (" << mom->getName() << ")";
  }
  return out.str();
}
