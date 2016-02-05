#include <topcaf/dataobjects/TopConfigurations.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TFile.h>
#include <TList.h>

#include <iostream>
#include <string>
#include <utility>

using namespace Belle2;

TopConfigurations::TopConfigurations()
{

}

TopConfigurations::~TopConfigurations()
{

}

void TopConfigurations::ReadTopConfigurations(std::string filename)
{

  TFile* fTopConfigurationsFile = TFile::Open(filename.c_str(), "read");




  TopPixelRefMap* PixeltoRowP;
  TopPixelRefMap* PixeltoColumnP;
  TopPixelRefMap* PixeltoPMTP;
  TopPixelRefMap* PixeltoPMTChannelP;
  TopPixelRefMap* PixeltoScrodP;
  TopPixelRefMap* PixeltoAsicRowP;
  TopPixelRefMap* PixeltoAsicColumnP;
  TopPixelRefMap* PixeltoAsicChannelP;
  TopPixelRetMap* HardwareIDtoPixelP;
  TopElectronicRetMap*  ScrodtoElectronicModuleP;
  TopPixelRefElectronicRetMap* PixeltoElectronicModuleP;
  TopElectronicRefMap*  ElectronicModuletoScrodP;
  std::vector<TopElectronicConstructionName>* TopModuleElectronicConstructionsP;
  TopParameter* TDCUnit_ns;


  fTopConfigurationsFile->GetObject("PixeltoRow",               PixeltoRowP);
  fTopConfigurationsFile->GetObject("PixeltoColumn",      PixeltoColumnP);
  fTopConfigurationsFile->GetObject("PixeltoPMT",         PixeltoPMTP);
  fTopConfigurationsFile->GetObject("PixeltoPMTChannel",      PixeltoPMTChannelP);
  fTopConfigurationsFile->GetObject("PixeltoElectronicModule",  PixeltoElectronicModuleP);
  fTopConfigurationsFile->GetObject("PixeltoScrod",  PixeltoScrodP);
  fTopConfigurationsFile->GetObject("PixeltoAsicRow",     PixeltoAsicRowP);
  fTopConfigurationsFile->GetObject("PixeltoAsicColumn",      PixeltoAsicColumnP);
  fTopConfigurationsFile->GetObject("PixeltoAsicChannel",     PixeltoAsicChannelP);
  fTopConfigurationsFile->GetObject("HardwareIDtoPixel",  HardwareIDtoPixelP);
  fTopConfigurationsFile->GetObject("ScrodtoElectronicModule", ScrodtoElectronicModuleP);
  fTopConfigurationsFile->GetObject("ElectronicModuletoScrod", ElectronicModuletoScrodP);
  fTopConfigurationsFile->GetObject("TopModuleElectronicConstructions", TopModuleElectronicConstructionsP);
  fTopConfigurationsFile->GetObject("TDCUnit_ns", TDCUnit_ns);

  m_PixeltoRow =               *PixeltoRowP;
  m_PixeltoColumn =              *PixeltoColumnP;
  m_PixeltoPMT =               *PixeltoPMTP;






  m_PixeltoPMTChannel =              *PixeltoPMTChannelP;
  m_PixeltoScrod =                     *PixeltoScrodP;
  m_PixeltoElectronicModule =          *PixeltoElectronicModuleP;
  m_PixeltoAsicRow =             *PixeltoAsicRowP;
  m_PixeltoAsicColumn =              *PixeltoAsicColumnP;
  m_PixeltoAsicChannel =             *PixeltoAsicChannelP;

  m_HardwareIDtoPixel =             *HardwareIDtoPixelP;
  m_TDCUnit_ns = *TDCUnit_ns;

  //  B2INFO("TDCUnit_ns: " << m_TDCUnit_ns.first);


  m_ScrodtoElectronicModule =          *ScrodtoElectronicModuleP;
  m_ElectronicModuletoScrod =          *ElectronicModuletoScrodP;
  m_TopModuleElectronicConstructions = *TopModuleElectronicConstructionsP;


}


