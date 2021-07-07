/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/particledb/EvtGenParticlePDG.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <THashList.h>
#include <TExMap.h>
#include <TInterpreter.h>

using namespace Belle2;

EvtGenDatabasePDG* EvtGenDatabasePDG::Instance()
{
  static bool instanceCreated = false;
  if (!instanceCreated) {
    // problem with 6.14: ROOT cannot find includes this early. Fix promised for 6.16.
    // (https://root-forum.cern.ch/t/problem-with-loadclassinfo-for-class-instantiated-very-early/30831)
    gInterpreter->Declare("#include <framework/particledb/EvtGenParticlePDG.h>");
    //hope we are the first to create a TDatabasePDG instance (setting the instance pointer in the process)
    auto instance = new EvtGenDatabasePDG();
    //ok, now load the data
    instance->ReadEvtGenTable();
    instanceCreated = true;
    if (instance != TDatabasePDG::Instance())
      B2FATAL("EvtGenDatabasePDG created too late!");
  }
  //this cast should be safe as we stop execution above if we couldn't create the first instance ourselves
  return static_cast<EvtGenDatabasePDG*>(TDatabasePDG::Instance());
}

EvtGenParticlePDG* EvtGenDatabasePDG::AddParticle(const char* name, const char* title, Double_t mass, Bool_t stable, Double_t width,
                                                  Double_t charge, const char* ParticleClass, Int_t PDGcode, Int_t Anti, Int_t TrackingCode,
                                                  Double_t Lifetime, Double_t Spin, Double_t maxWidth, Int_t pythiaID)
{
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
    B2ERROR("EvtGenDatabasePDG::AddParticle: particle with PDGcode=" << PDGcode << " already defined");
    return nullptr;
  }

  if (std::strpbrk(name, " \t\n\v\f\r")) {
    B2ERROR("EvtGenDatabasePDG::AddParticle: invalid particle name '" << name << "'. Names may not contain Whitespace");
    return nullptr;
  }

  auto* p = new EvtGenParticlePDG(name, title, mass, stable, width, charge, ParticleClass,
                                  PDGcode, Anti, TrackingCode, Lifetime, Spin, maxWidth, pythiaID);

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

void EvtGenDatabasePDG::ReadEvtGenTable(const char* filename)
{
  //ok, now load the data
  std::string defaultFilename = FileSystem::findFile("data/framework/particledb/evt.pdl");
  if (defaultFilename.empty())
    B2FATAL("Cannot find the default particle database file (evt.pdl).");
  if (!filename) {
    filename = defaultFilename.c_str();
  } else {
    if (filename != defaultFilename) {
      B2RESULT("Loading non-standard evt.pdl file '" << filename << "'");
    }
  }

  // do we have lists already?
  if (fParticleList == nullptr) {
    //if not create them
    fParticleList  = new THashList;
    fListOfClasses = new TObjArray;
  } else {
    //otherwise clear them
    fParticleList->Delete();
    fListOfClasses->Delete();
    //and also reset the map from pdgcode to particle
    delete fPdgMap;
    fPdgMap = nullptr;
  }
  std::ifstream indec(filename);

  char cmnd[100], xxxx[100], pname[100];
  int  stdhepid; //aka PDG code
  double mass, pwidth, pmaxwidth;
  int    chg3, spin2;
  double ctau;
  int    lundkc;

  if (!indec) {
    B2ERROR("EvtGenDatabasePDG::ReadEvtGenTable: Could not particle data file '" << filename << "'");
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
        TParticlePDG* part = AddParticle(pname, pname, mass, false, pwidth, chg3, "Unknown", stdhepid, 0, 0,
                                         ctau / c_mm_per_s, spin2 / 2.0, pmaxwidth, lundkc);
        pdgToPartMap[stdhepid] = part;
        if (!part) {
          B2FATAL("EvtGenDatabasePDG::ReadPDGTable: Problem creating particle '" << pname << "'");
        }
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

void EvtGenDatabasePDG::WriteEvtGenTable(std::ostream& out)
{
  const double c_mm_per_s = Const::speedOfLight / (Unit::mm / Unit::s);
  if (fParticleList) {
    for (TObject* obj : *fParticleList) {
      auto* part = dynamic_cast<EvtGenParticlePDG*>(obj);
      if (!part) {
        B2FATAL("EvtGenDatabasePDG::WriteEvtgenTable: Particle does not inherit from EventGenParticlePDG");
      }
      out << "add p Particle " << part->GetName() << " " << part->PdgCode()
          << " " << std::scientific << std::setprecision(7) << part->Mass()
          << " " << std::scientific << std::setprecision(7) << part->Width()
          << " " << std::scientific << std::setprecision(7) << part->MaxWidth()
          << " " << (int)(part->Charge())
          << " " << (int)(part->Spin() * 2)
          << " " << std::scientific << std::setprecision(7) << part->Lifetime() * c_mm_per_s
          << " " << part->PythiaID()
          << std::endl;
    }
  }
  out << "end" << std::endl << std::flush;
}

void EvtGenDatabasePDG::WriteEvtGenTable(const char* filename)
{
  std::ofstream out(filename);
  if (!out) {
    B2FATAL("Cannot write evtgen pdl to '" << filename << "'");
  }
  WriteEvtGenTable(out);
}
