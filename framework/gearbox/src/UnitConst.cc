/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck, Martin Ritter, Thomas Kuhr    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/**************************************************************************
 * This file contains the implementations for Unit.h and Const.h to avoid *
 * problems with the order of the initialization of constants.            *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TMath.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include <THashList.h>
#include <TExMap.h>

#include <algorithm>
#include <fstream>

using namespace Belle2;
using namespace std;

namespace {
  /** Helper class for setting additional TParticlePDG attributes. */
  class ParticlePDG : public TParticlePDG {
  public:
    ParticlePDG(const char* name, const char* title,
                Double_t mass, Bool_t stable,
                Double_t width, Double_t charge,
                const char* ParticleClass,
                Int_t PDGcode,
                Int_t Anti,
                Int_t TrackingCode,
                Double_t Lifetime,
                Double_t Spin): TParticlePDG(name, title, mass, stable, width, charge, ParticleClass, PDGcode, Anti, TrackingCode) {
      //and finally overwrite base class members
      fLifetime = Lifetime;
      fSpin = Spin;
    }
  };

  /**
   * Helper class for loading the particle properties from EvtGen.
   * This should be instantiated once (statically, in this file) to replace
   * the global TDatabasePDG instance with ourselves
   */
  class EvtGenDatabasePDG : public TDatabasePDG {
  public:
    static TDatabasePDG* Instance();  /**< Instance method that loads the EvtGen table. */
    void ReadEvtGenTable(const char* filename);

    /** copy&paste, except we create ParticlePDG to set lifetime and spin. */
    TParticlePDG* AddParticle(const char* name, const char* title,
                              Double_t mass, Bool_t stable,
                              Double_t width, Double_t charge,
                              const char* ParticleClass,
                              Int_t PDGcode,
                              Int_t Anti,
                              Int_t TrackingCode,
                              Double_t Lifetime,
                              Double_t Spin
                             ) {
      //
      //  Particle definition normal constructor. If the particle is set to be
      //  stable, the decay width parameter does have no meaning and can be set to
      //  any value. The parameters granularity, LowerCutOff and HighCutOff are
      //  used for the construction of the mean free path look up tables. The
      //  granularity will be the number of logwise energy points for which the
      //  mean free path will be calculated.
      //

      TParticlePDG* old = GetParticle(PDGcode);

      if (old) {
        printf(" *** TDatabasePDG::AddParticle: particle with PDGcode=%d already defined\n", PDGcode);
        return 0;
      }

      TParticlePDG* p = new ParticlePDG(name, title, mass, stable, width,
                                        charge, ParticleClass, PDGcode, Anti,
                                        TrackingCode,
                                        Lifetime, Spin);
      fParticleList->Add(p);
      if (fPdgMap)
        fPdgMap->Add((Long_t)PDGcode, (Long_t)p);

      TParticleClassPDG* pclass = GetParticleClass(ParticleClass);

      if (!pclass) {
        pclass = new TParticleClassPDG(ParticleClass);
        fListOfClasses->Add(pclass);
      }

      pclass->AddParticle(p);

      return p;
    }

  private:
    /** singleton. */
    EvtGenDatabasePDG() { }
  };
}

TDatabasePDG* EvtGenDatabasePDG::Instance()
{
  static bool instanceCreated = false;
  if (!instanceCreated) {
    if (fgInstance) {
      B2ERROR("TDatabasePDG instance found? Replacing existing instance...")
    }
    //let's set the instance pointer to ourselves
    fgInstance = new EvtGenDatabasePDG();

    //ok, now load the data
    std::string fileName = std::getenv("BELLE2_EXTERNALS_DIR");
    fileName += "/share/evtgen/evt.pdl";
    fgInstance->ReadEvtGenTable(fileName.c_str());
    instanceCreated = true;
  }
  return TDatabasePDG::Instance();
}

void EvtGenDatabasePDG::ReadEvtGenTable(const char* filename)
{
  // read list of particles from an EvtGen pdl file
  // if the particle list does not exist, it is created, otherwise
  // particles are added to the existing list

  // the code for reading the evt.pdl file is copied from EvtPartPropDb::readPDT

  if (fParticleList == 0) {
    fParticleList  = new THashList;
    fListOfClasses = new TObjArray;
  }

  std::ifstream indec(filename);

  char cmnd[100], xxxx[100], pname[100];
  int  stdhepid; //aka PDG code
  double mass, pwidth, pmaxwidth;
  int    chg3, spin2;
  double ctau;
  int    lundkc;

  if (!indec) {
    B2FATAL("EvtGenDatabasePDG::ReadPDGTable: Could not open PDG particle file " << filename);
    return;
  }

  std::map<int, TParticlePDG*> pdgToPartMap;
  do {

    char ch, ch1;

    do {

      indec.get(ch);
      if (ch == '\n') indec.get(ch);
      if (ch != '*') {
        indec.putback(ch);
      } else {
        while (indec.get(ch1), ch1 != '\n');
      }
    } while (ch == '*');

    indec >> cmnd;

    if (strcmp(cmnd, "end")) {

      if (!strcmp(cmnd, "add")) {

        indec >> xxxx >> xxxx >> pname >> stdhepid;
        indec >> mass >> pwidth >> pmaxwidth >> chg3 >> spin2 >> ctau >> lundkc;

        const double c_mm_per_s = Const::speedOfLight / (Unit::mm / Unit::s);
        TParticlePDG* part = AddParticle(pname,
                                         pname,
                                         mass,
                                         0,
                                         pwidth,
                                         chg3,
                                         "Unknown",
                                         stdhepid,
                                         0, 0, //anti, trackigCode
                                         ctau / c_mm_per_s, // in seconds
                                         spin2 / 2.0);
        pdgToPartMap[stdhepid] = part;
      }
    }
  } while (strcmp(cmnd, "end"));

  //fix up particle <-> antiparticle settings
  for (auto entry : pdgToPartMap) {
    int pdg = entry.first;
    TParticlePDG* part = entry.second;

    const auto& antiPartIter = pdgToPartMap.find(-pdg);
    if (antiPartIter != pdgToPartMap.end()) {
      //set anti-particle
      part->SetAntiParticle(antiPartIter->second);
    } else {
      //we are our own antiparticle
      part->SetAntiParticle(part);
    }
  }
}


/*** The implementation of the Unit class defined in Unit.h starts here ***/

namespace Belle2 {

  map<string, double> Unit::s_conversionFactors;

  /** Define a new Unit given a name for the conversion table */
#define DEFINE_UNIT_NAME(var,value,name) const double Unit::var = Unit::registerConversion(name,value)
  /** Define a new Unit and take the variable name as name for the conversion table */
#define DEFINE_UNIT(var,value) DEFINE_UNIT_NAME(var,value,#var)

// standard units
  DEFINE_UNIT(cm    , 1.); /**< Standard of [length] */
  DEFINE_UNIT(ns    , 1.); /**< Standard of [time] */
  DEFINE_UNIT(rad   , 1.); /**< Standard of [angle] */
  DEFINE_UNIT(GeV   , 1.); /**< Standard of [energy, momentum, mass] */
  DEFINE_UNIT(K     , 1.); /**< Standard of [temperature] */
  DEFINE_UNIT(T     , 1.); /**< Standard of [magnetic field] */
  DEFINE_UNIT(e     , 1.); /**< Standard of [electric charge] */
  DEFINE_UNIT_NAME(g_cm3 , 1., "g/cm3"); /**< Standard of [density] */

// length units
  DEFINE_UNIT(km  , Unit::cm * 1e5);   /**< [kilometers] */
  DEFINE_UNIT(m   , Unit::cm * 1e2);   /**< [meters] */
  DEFINE_UNIT(mm  , Unit::m  * 1e-3);  /**< [millimeters] */
  DEFINE_UNIT(um  , Unit::m  * 1e-6);  /**< [micrometers] */
  DEFINE_UNIT(nm  , Unit::m  * 1e-9);  /**< [nanometers] */
  DEFINE_UNIT(pm  , Unit::m  * 1e-12); /**< [picometers] */
  DEFINE_UNIT(fm  , Unit::m  * 1e-15); /**< [femtometers] */

// area units
  DEFINE_UNIT(m2  , Unit::m * Unit::m);      /**< [square meters] */
  DEFINE_UNIT(cm2 , Unit::cm * Unit::cm);    /**< [square centimeters] */
  DEFINE_UNIT(mm2 , Unit::mm * Unit::mm);    /**< [square millimeters] */

  DEFINE_UNIT(b   , Unit::m2 * 1e-28); /**< [barn] */
  DEFINE_UNIT(mb  , Unit::b  * 1e-3);  /**< [millibarn] */
  DEFINE_UNIT(ub  , Unit::b  * 1e-6);  /**< [microbarn] */
  DEFINE_UNIT(nb  , Unit::b  * 1e-9);  /**< [nanobarn] */
  DEFINE_UNIT(pb  , Unit::b  * 1e-12); /**< [picobarn] */
  DEFINE_UNIT(fb  , Unit::b  * 1e-15); /**< [femtobarn] */
  DEFINE_UNIT(ab  , Unit::b  * 1e-18); /**< [attobarn] */

// volume units
  DEFINE_UNIT(m3  , Unit::m * Unit::m * Unit::m);    /**< [cubic meters] */
  DEFINE_UNIT(cm3 , Unit::cm * Unit::cm * Unit::cm); /**< [cubic centimeters] */
  DEFINE_UNIT(mm3 , Unit::mm * Unit::mm * Unit::mm); /**< [cubic millimeters] */

// time units
  DEFINE_UNIT(s   , Unit::ns * 1e9);   /**< [second] */
  DEFINE_UNIT(ms  , Unit::s  * 1e-3);  /**< [millisecond] */
  DEFINE_UNIT(us  , Unit::s  * 1e-6);  /**< [microsecond] */
  DEFINE_UNIT(ps  , Unit::s  * 1e-12); /**< [picosecond] */
  DEFINE_UNIT(fs  , Unit::s  * 1e-15); /**< [femtosecond] */

// angle units
  DEFINE_UNIT(mrad , Unit::rad * 1e-3);  /**< [millirad] */
  DEFINE_UNIT(deg  , TMath::DegToRad()); /**< degree to radians */

// energy units
  DEFINE_UNIT(eV  , Unit::GeV * 1e-9); /**< [electronvolt] */
  DEFINE_UNIT(keV , Unit::eV  * 1e3);  /**< [kiloelectronvolt] */
  DEFINE_UNIT(MeV , Unit::eV  * 1e6);  /**< [megaelectronvolt] */
  DEFINE_UNIT(TeV , Unit::eV  * 1e12);  /**< [teraelectronvolt] */
  DEFINE_UNIT(J   , 1.0 / TMath::Qe() * Unit::eV); /**< [joule] */
  DEFINE_UNIT(V   , Unit::eV / Unit::e); /**< Standard of [voltage] */

// charge units
  DEFINE_UNIT(C  , 1.0 / TMath::Qe() * Unit::e); /**< [Coulomb] */
  DEFINE_UNIT(fC , Unit::C / 1.0e15);            /**< [femtoCoulomb] */

// density units
  DEFINE_UNIT_NAME(mg_cm3  , Unit::g_cm3 * 1e-3, "mg/cm3"); /**< [mg/cm^3] */
  DEFINE_UNIT_NAME(kg_cm3  , Unit::g_cm3 * 1e3, "kg/cm3");  /**< [kg/cm^3] */
  DEFINE_UNIT_NAME(g_mm3   , Unit::g_cm3 / Unit::mm3, "g/mm3");  /**< [g/mm^3] */
  DEFINE_UNIT_NAME(mg_mm3  , Unit::mg_cm3 / Unit::mm3, "mg/mm3"); /**< [mg/mm^3] */
  DEFINE_UNIT_NAME(kg_mm3  , Unit::kg_cm3 / Unit::mm3, "kg/mm3"); /**< [kg/mm^3] */

// magnetic field units
  DEFINE_UNIT(TinStdUnits  , Unit::V * Unit::s / Unit::m2); /**< [Vs/m^2] */

  double Unit::convertValue(double value, const std::string& unitString)
  {
    map<string, double>::const_iterator it = s_conversionFactors.find(unitString);
    if (it == s_conversionFactors.end()) {
      B2ERROR("Could not find conversion factor for unit " << unitString << ", assuming 1.0");
      return value;
    }
    return it->second * value;
  }

  double Unit::registerConversion(const string& name, double value)
  {
    if (!s_conversionFactors.insert(make_pair(name, value)).second) {
      B2ERROR("Conversion factor for unit " << name << " already exists, ignoring");
    }
    return value;
  }

}

ClassImp(Const::DetectorSet)

/*** The implementation of the Const class defined in Const.h starts here ***/

Const::DetectorSet operator + (const Const::DetectorSet& firstSet, const Const::DetectorSet& secondSet)
{
  Const::DetectorSet set(firstSet);
  set += secondSet;
  return set;
}

Const::DetectorSet operator - (const Const::DetectorSet& firstSet, const Const::DetectorSet& secondSet)
{
  Const::DetectorSet set(firstSet);
  set -= secondSet;
  return set;
}

Const::DetectorSet operator + (Const::EDetector firstDet, Const::EDetector secondDet)
{
  Const::DetectorSet set(firstDet);
  set += secondDet;
  return set;
}

unsigned short Const::DetectorSet::getBit(Const::EDetector det) const
{
  return (1 << (det - 1));
}

Const::EDetector Const::DetectorSet::getDetector(unsigned short bit) const
{
  switch (bit) {
    case 0x0001: return PXD;
    case 0x0002: return SVD;
    case 0x0004: return CDC;
    case 0x0008: return TOP;
    case 0x0010: return ARICH;
    case 0x0020: return ECL;
    case 0x0040: return KLM;
    case 0x0080: return IR;
    case 0x0100: return TRG;
    case 0x0200: return DAQ;
    case 0x0400: return BEAST;
    case 0x0800: return TEST;
    default: return invalidDetector;
  }
}

int Const::DetectorSet::getIndex(EDetector det) const
{
  unsigned short bit = getBit(det);
  if ((m_bits & bit) == 0) return -1;
  int index = 0;
  for (unsigned short setBit = 1; setBit < bit; setBit *= 2) {
    if ((m_bits & setBit) != 0) ++index;
  }
  return index;
}

Const::EDetector Const::DetectorSet::operator [](int index) const
{
  if (index < 0) return Const::invalidDetector;
  for (unsigned short setBit = 1; setBit < 0x1000; setBit *= 2) {
    if ((m_bits & setBit) != 0) --index;
    if (index < 0) return getDetector(setBit);
  }
  return Const::invalidDetector;
}

size_t Const::DetectorSet::size() const
{
  int size = 0;
  for (unsigned short setBit = 1; setBit < 0x1000; setBit *= 2) {
    if ((m_bits & setBit) != 0) ++size;
  }
  return size;
}


const Const::DetectorSet Const::VXDDetectors::c_set = Const::PXD + Const::SVD;

const Const::DetectorSet Const::TrackingDetectors::c_set = Const::PXD + Const::SVD + Const::CDC;

const Const::DetectorSet Const::PIDDetectors::c_set = Const::SVD + Const::CDC + Const::TOP + Const::ARICH + Const::ECL + Const::KLM;


const Const::DetectorSet Const::allDetectors = Const::IR + Const::PXD + Const::SVD + Const::CDC +
                                               Const::TOP + Const::ARICH + Const::ECL + Const::KLM +
                                               Const::TRG + Const::DAQ + Const::BEAST + Const::TEST;


bool Const::ParticleType::operator < (const Const::ParticleType& other) const
{
  return m_pdgCode < other.m_pdgCode;
}

Const::ParticleType& Const::ParticleType::operator++()
{
  if (!m_set) {
    *this = invalidParticle;
  } else {
    m_pdgCode = m_set->at(++m_index).getPDGCode();
  }

  return *this;
}

Const::ParticleType Const::ParticleType::operator++(int)
{
  Const::ParticleType p = *this;
  ++(*this);
  return p;
}

const TParticlePDG* Const::ParticleType::getParticlePDG() const
{
  return EvtGenDatabasePDG::Instance()->GetParticle(m_pdgCode);
}

double Const::ParticleType::getMass() const
{
  return getParticlePDG()->Mass();
}

const Const::ParticleSet Const::chargedStableSet = Const::ParticleType(211) + Const::ParticleType(321) + Const::ParticleType(2212) + Const::ParticleType(11) + Const::ParticleType(13);

const Const::ChargedStable Const::electron = Const::chargedStableSet.find(11);
const Const::ChargedStable Const::muon = Const::chargedStableSet.find(13);
const Const::ChargedStable Const::pion = Const::chargedStableSet.find(211);
const Const::ChargedStable Const::kaon = Const::chargedStableSet.find(321);
const Const::ChargedStable Const::proton = Const::chargedStableSet.find(2212);

const Const::ParticleType Const::photon = Const::ParticleType(22);
const Const::ParticleType Const::pi0 = Const::ParticleType(111);
const Const::ParticleType Const::neutron = Const::ParticleType(2112);
const Const::ParticleType Const::Kshort = Const::ParticleType(310);
const Const::ParticleType Const::Klong = Const::ParticleType(130);
const Const::ParticleType Const::invalidParticle = Const::ParticleType(9900000);

const double Const::electronMass = Const::electron.getMass();
const double Const::muonMass = Const::muon.getMass();
const double Const::pionMass = Const::pion.getMass();
const double Const::kaonMass = Const::kaon.getMass();
const double Const::protonMass = Const::proton.getMass();
const double Const::pi0Mass = Const::pi0.getMass();
const double Const::neutronMass = Const::neutron.getMass();
const double Const::K0Mass = Const::Kshort.getMass();

void Const::ParticleSet::add(const Const::ParticleType& p)
{
  if (contains(p))
    return;
  m_particles.push_back(Const::ParticleType(p.getPDGCode(), this, m_particles.size()));
}

bool Const::ParticleSet::contains(const Const::ParticleType& p) const
{
  return (std::find(m_particles.begin(), m_particles.end(), p) != m_particles.end());
}

Const::ParticleSet operator + (const Const::ParticleSet& firstSet, const Const::ParticleSet& secondSet)
{
  Const::ParticleSet result(firstSet);
  for (Const::ParticleType pdgIter = secondSet.begin(); pdgIter != secondSet.end(); ++pdgIter) {
    result.add(pdgIter);
  }
  return result;
}


const double Const::speedOfLight   = 29.9792458;
const double Const::kBoltzmann     = 8.617343 * 1.0e-5 * Unit::eV / Unit::K;
const double Const::ehEnergy       = 3.65 * Unit::eV;
const double Const::fineStrConst   = 1.0 / 137.036;
const double Const::permSi         = 11.9 * 8.8542 * 1e-18 * Unit::C / Unit::V / Unit::um;
const double Const::uTherm         = 0.026 * Unit::V;
const double Const::eMobilitySi    = 1415 * Unit::cm2 / Unit::V / Unit::s;
