#include <tracking/modules/trackFinderVXDTests/CollectorTestModule.h>

#include <iostream>

using namespace std;
using namespace Belle2;


REG_MODULE(CollectorTest)

CollectorTestModule::CollectorTestModule() : Module()
{
  setDescription("CollectorTest.");
}

void CollectorTestModule::initialize()
{

}

void CollectorTestModule::event()
{

}

void CollectorTestModule::terminate()
{
  cerr << "\n";
}

