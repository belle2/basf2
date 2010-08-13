// This is a template module for the basf2 framework
// it contains a class that should be derived from some basf2 module base class

#include <framework/modules/hello/hello.h>
#include <framework/fwcore/ModuleManager.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(ModuleHello)

ModuleHello::ModuleHello() : Module("Hello")
{
  setDescription("Hello World");

  m_testParamInt = 10;
  addParam("testParamInt", m_testParamInt, 20);
  addParam("testParamDouble", m_testParamDouble, 12.3456789);
  addParam("testParamString", m_testParamString, string("Default"));
  addParam("testParamBool", m_testParamBool, true);

  std::vector<int> defaultIntList;
  defaultIntList.push_back(0);
  defaultIntList.push_back(-1);
  defaultIntList.push_back(-2);
  addParam("testParamIntList", m_testParamIntList, defaultIntList);

  std::vector<double> defaultDoubleList;
  defaultDoubleList.push_back(0.0);
  defaultDoubleList.push_back(-1.0);
  defaultDoubleList.push_back(-2.0);
  addParam("testParamDoubleList", m_testParamDoubleList, defaultDoubleList);

  std::vector<std::string> defaultStringList;
  defaultStringList.push_back("One");
  defaultStringList.push_back("Two");
  addParam("testParamStringList", m_testParamStringList, defaultStringList);

  std::vector<bool> defaultBoolList;
  defaultBoolList.push_back(true);
  addParam("testParamBoolList", m_testParamBoolList, defaultBoolList);

}


ModuleHello::~ModuleHello()
{

}

void ModuleHello::initialize()
{
  DEBUG(5, "Debug 5")
  DEBUG(111, "Debug 111")
  WARNING("WARNINGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG")

  cout << "Init called" << endl;
  cout << "m_testParamInt: " << m_testParamInt << endl;
  cout << "m_testParamDouble: " << m_testParamDouble << endl;
  cout << "m_testParamString: " << m_testParamString << endl;
  cout << "m_testParamBool: " << m_testParamBool << endl;

  cout << "---------" << endl;
  cout << "Int List:" << endl;
  for (vector<int>::iterator listIter = m_testParamIntList.begin(); listIter !=  m_testParamIntList.end(); listIter++) {
    cout << *listIter << endl;
  }
  cout << "---------" << endl;

  cout << "Double List:" << endl;
  for (vector<double>::iterator listIter = m_testParamDoubleList.begin(); listIter !=  m_testParamDoubleList.end(); listIter++) {
    cout << *listIter << endl;
  }
  cout << "---------" << endl;

  cout << "String List:" << endl;
  for (vector<string>::iterator listIter = m_testParamStringList.begin(); listIter !=  m_testParamStringList.end(); listIter++) {
    cout << *listIter << endl;
  }
  cout << "---------" << endl;

  cout << "Bool List:" << endl;
  for (vector<bool>::iterator listIter = m_testParamBoolList.begin(); listIter !=  m_testParamBoolList.end(); listIter++) {
    cout << *listIter << endl;
  }
  cout << "---------" << endl;

  //Get Gearbox parameters
  Gearbox& gearbox = Gearbox::Instance();

  GearDir gbxParams = gearbox.getContent("PXD");
  INFO(gbxParams.getParamString("Rotation"))

  m_evtNum = 0;
  TNamed* named = new TNamed("NAME of named", "TITLE of named");

  StoreObjPtr<TNamed> nameptr;
  nameptr.storeObject(named, "Named", c_Persistent);

}


void ModuleHello::beginRun()
{
  cout << "beginRun called" << endl;
}


void ModuleHello::event()
{
  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
  INFO("EXP NUMBER: " << eventMetaDataPtr->getExperiment());
  INFO("RUN NUMBER: " << eventMetaDataPtr->getRun());
  INFO("EVT NUMBER: " << eventMetaDataPtr->getEvent());


  StoreObjPtr<SimpleVec<int> > Pointer1("TestVec1");

  StoreObjPtr<SimpleVec<int> > Pointer2;
  Pointer2.assignObject("TestVec2", c_Event, true);

  m_evtNum++;
  cout << "#Event: " << m_evtNum << endl;

  vector<int> testVec;

  testVec.push_back(random.Poisson(10));
  testVec.push_back(10*m_evtNum);

  Pointer1->setVector(testVec);
  Pointer2->setVector(testVec);

  StoreArray<SimpleVec<int> > Array("TestArray");

  for (int ii = 0; ii < 10; ++ii) {
    new(Array->AddrAt(ii)) SimpleVec<int>(testVec);
  }


  for (int ii = 0; ii < 10; ++ii) {
    new(Array->AddrAt(ii)) SimpleVec<int>();
    Array[ii]->setVector(testVec);
  }
  setReturnValue(10);
  //setReturnValue(false);

  //setProcessRecordType(BEGIN_RUN);

  StoreObjPtr<Relation> RelPtr;

  /*  TObject* object = &(*(Pointer1));
    if (object) {INFO("There should be an object in the Relation");} else {INFO("There is no object in the Relation");}
    RelPtr->setFrom(object);
    object = &(*(Pointer2));
    RelPtr->setTo(&(*(Pointer2)));
    RelPtr.storeObject(Pointer1.relateTo(Pointer2),"RelPtr");

    TObject testObject = RelPtr->getFrom();
    if (testObject) { INFO("Hello Relation");} else {INFO("No return Object");}
    */
}


void ModuleHello::endRun()
{
  cout << "endRun called" << endl;
}


void ModuleHello::terminate()
{
  cout << "Term called" << endl;
}
