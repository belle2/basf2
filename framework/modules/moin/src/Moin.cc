// This is a template module for the basf2 framework
// it contains a class that should be derived from some basf2 module base class

#include <../moin/Moin.h>
#include <fwcore/ModuleManager.h>

#include <gearbox/Gearbox.h>
#include <gearbox/GearDir.h>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(ModuleMoin)

ModuleMoin::ModuleMoin() : Module("Moin")
{
  setDescription("Moin World");

  setPropertyFlags(1);
}


ModuleMoin::~ModuleMoin()
{
}

void ModuleMoin::initialize()
{
  outputstream.open("output.txt", fstream::out);
}


void ModuleMoin::beginRun()
{
  cout << "beginRun called" << endl;
}


void ModuleMoin::event()
{
  StoreObjPtr<SimpleVec<int> > Pointer2;

  m_evtNum++;

  StoreObjPtr<Relation> RelPtr("RelPtr");
  TObject* object = RelPtr->getFrom();
  if (object) { INFO("Moin Relation");} else {INFO("No return Object");}

  SimpleVec<int>* Vec = (SimpleVec<int>*)(object);
  vector<int> vec = Vec->getVector();
  INFO("vec[0] is " << vec[0]);

  StoreArray<HitCDC> CDCArray("HitCDCArray");
  outputstream << CDCArray.GetEntries() << endl;
  for (int ii = 0; ii < CDCArray.GetEntries(); ii++) {
    outputstream << "CDC: " << CDCArray[ii]->getLayerId() << " " << CDCArray[ii]->getWireId() << " " <<  CDCArray[ii]->getTime()  << endl;
  }


  outputstream << "\n#Next event!#" << endl;

  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(13);
  WARNING(particle->Mass());
}


void ModuleMoin::endRun()
{
  cout << "endRun called" << endl;
}


void ModuleMoin::terminate()
{
  cout << "Term called" << endl;
}
