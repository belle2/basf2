// This is a template module for the basf2 framework
// it contains a class that should be derived from some basf2 module base class

#include <framework/modules/moin/Moin.h>
#include <framework/fwcore/ModuleManager.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(ModuleMoin)

ModuleMoin::ModuleMoin() : Module("Moin")
{
  setDescription("Test Module for Martin's work on framework issues.");

  addParam("fill_datastore", m_fillDataStore, 1, "Fill stuff into the DataStore");

  DEBUG(100, "Constructor of Moin done");
}


ModuleMoin::~ModuleMoin()
{
  DEBUG(100, "Destructor of Moin done");
}

void ModuleMoin::initialize()
{
  m_evtNum = 0;
//  outputstream.open("output.txt", fstream::out);
}


void ModuleMoin::beginRun()
{
  DEBUG(100, "BeginRun of Moin called.");
}


void ModuleMoin::event()
{
  // Vectors of int
  StoreObjPtr<SimpleVec<int> > testVector1("testVector1");
  StoreObjPtr<SimpleVec<int> > testVector2("testVector2");

  // Relation
  StoreObjPtr<Relation> relation("relation");


  m_evtNum++;

  if (m_fillDataStore) {
    // fill some values
    vector<int> vector1;
    vector<int> vector2;
    for (int ii = 0; ii < 10; ++ii) {
      vector1.push_back(random.Poisson(10));
      vector2.push_back(random.Poisson(m_evtNum * ii));

//      INFO ("vector1[" << ii << "] : " << vector1[ii]);
    }
    testVector1->setVector(vector1);
    INFO("vector1[0]" << vector1[0]);
    testVector2->setVector(vector2);

    relation->setFrom(&(*testVector1));
    relation->setTo(&(*testVector2));
  } else {

    StoreObjPtr<Relation> relation("relation");
    TObject* object = relation->getFrom();
    SimpleVec<int>* vectorFromRelation = dynamic_cast<SimpleVec<int>* >(object);

    vector<int> vector1 = vectorFromRelation->getVector();

    INFO("This number should be Poisson distributed around 10 and is:" << vector1[0])

  }


//  StoreArray<HitCDC> CDCArray("HitCDCArray");
//  outputstream << CDCArray.GetEntries() << endl;
//  for (int ii = 0; ii < CDCArray.GetEntries(); ii++) {
//    outputstream << "CDC: " << CDCArray[ii]->getLayerId() << " " << CDCArray[ii]->getWireId() << " " <<  CDCArray[ii]->getTime()  << endl;
//  }


//  outputstream << "\n#Next event!#" << endl;

  /*  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(13);
    WARNING(particle->Mass());*/
}


void ModuleMoin::endRun()
{
  DEBUG(100, "EndRun of Moin called.");
}


void ModuleMoin::terminate()
{
  DEBUG(100, "Terminate of Moin called.");
}
