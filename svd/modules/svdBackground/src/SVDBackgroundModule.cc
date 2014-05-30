#include "svd/modules/svdBackground/SVDBackgroundModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <simulation/background/BkgNeutronWeight.h>

#include <framework/core/InputController.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <set>
#include <algorithm>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDBackground)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDBackgroundModule::SVDBackgroundModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD background module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("componentNames", m_componentNames, "List of background component names to process", m_componentNames);
  addParam("componentTimes", m_componentTimes, "List of background component times", m_componentTimes);
}


SVDBackgroundModule::~SVDBackgroundModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDBackgroundModule::defineHisto()
{
}


void SVDBackgroundModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits);

  //Store names to speed up creation later
  m_storeMCParticlesName = storeMCParticles.getName();
  m_storeSimHitsName = storeSimHits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relDigitsMCParticlesName = relDigitsMCParticles.getName();
  m_relDigitsTrueHitsName = relDigitsTrueHits.getName();

  // Initialize m_data:
  for (unsigned int iComp = 0; iComp < m_componentNames.size(); ++iComp) {
    string componentName = m_componentNames[iComp];
    m_data[componentName].m_componentName = componentName;
    m_data[componentName].m_componentTime = 1.0 * m_componentTimes[iComp] * Unit::us / Unit::s;
  }
}

void SVDBackgroundModule::beginRun()
{
}


void SVDBackgroundModule::event()
{

  // Check if current component name has changed
  string currentFileName = InputController::getCurrentFileName();
  size_t beginNameMark = currentFileName.find_last_of('/') + 1;
  size_t endNameMark = currentFileName.find_last_of('_');
  string componentName = currentFileName.substr(beginNameMark, endNameMark - beginNameMark);
  if (componentName != m_currentComponentName) {
    B2INFO("Current component: " << componentName);
    m_currentComponentName = componentName;
    m_componentChanged = true;
  }

  //Register collections
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDSimHit> storeSimHits(m_storeSimHitsName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles, m_relDigitsMCParticlesName);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits, m_relDigitsTrueHitsName);

  // Fill expo histograms
  BackgroundData& bData = m_data[m_currentComponentName];
  double currentComponentTime = bData.m_componentTime;
  VxdID currentSensorID(3, 1, 1);
  double currentSensorMass = getSensorMass(currentSensorID);
  double currentSensorArea = getSensorArea(currentSensorID);
  VxdID currentLayerID(currentSensorID.getLayerNumber(), 0, 0);

  for (const SVDSimHit & hit : storeSimHits) {
    VxdID sensorID = hit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorMass = getSensorMass(currentSensorID);
      currentSensorArea = getSensorArea(currentSensorID);
      currentLayerID.setLayerNumber(currentSensorID.getLayerNumber());
    }
    double value = hit.getElectrons() * Const::ehEnergy / currentSensorMass / currentComponentTime * c_smy / Unit::J;
    bData.m_layerData[currentLayerID].m_dose += value;
    bData.m_layerData[currentLayerID].m_expo += hit.getElectrons() * Const::ehEnergy;
  }

  for (const SVDDigit & digit : storeDigits) {
    VxdID sensorID = digit.getSensorID();
    if (sensorID != currentSensorID) {
      currentSensorID = sensorID;
      currentSensorArea = getSensorArea(currentSensorID);
      currentLayerID.setLayerNumber(currentSensorID.getLayerNumber());
    }
    double value = 1.0 / 6.0 / currentComponentTime / currentSensorArea;
    if (digit.isUStrip())
      bData.m_layerData[currentLayerID].m_firedU += value;
    else
      bData.m_layerData[currentLayerID].m_firedV += value;
  }
}

void SVDBackgroundModule::endRun()
{
  // Write out m_data
  for (auto cmpBkg : m_data) {
    string componentName = cmpBkg.first;
    BackgroundData bgData = cmpBkg.second;
    for (auto vxdLayer : bgData.m_layerData) {
      int iLayer = vxdLayer.first.getLayerNumber();
      B2INFO(componentName.c_str() << " " << iLayer << " "
             << vxdLayer.second.m_dose << " "
             << vxdLayer.second.m_expo << " "
             << vxdLayer.second.m_firedU << " "
             << vxdLayer.second.m_firedV);
    }
  }
}


void SVDBackgroundModule::terminate()
{
}
