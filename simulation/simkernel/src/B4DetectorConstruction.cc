/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geodetector/GeoDetector.h>

#include <simkernel/B4DetectorConstruction.h>
#include <simkernel/B4MagneticField.h>
#include <logging/Logger.h>

using namespace Belle2;

B4DetectorConstruction* B4DetectorConstruction::m_instance = 0;

B4DetectorConstruction* B4DetectorConstruction::Instance()
{
  if (!m_instance) m_instance = new B4DetectorConstruction();
  return m_instance;
}

B4DetectorConstruction::B4DetectorConstruction() : m_magneticField(NULL)
{
  m_magneticField = new B4MagneticField();
}

B4DetectorConstruction::~B4DetectorConstruction()
{
  if (m_magneticField) delete m_magneticField;
}

void B4DetectorConstruction::Initialize(TG4RootDetectorConstruction *dc)
{
  //--------------------------------------------------------------------
  // Get pointers of sub-detector construction and call ralated
  // functions to implement sesitive detector, user limits and so on.
  //--------------------------------------------------------------------

  std::list<std::string> creatorList = GeoDetector::Instance().getCalledCreators();

  for (unsigned int idriver = 0; idriver < m_registeredDrivers.size(); idriver++) {
    if (find(creatorList.begin(), creatorList.end(), m_registeredDrivers[idriver]->getName()) != creatorList.end()) {
      m_registeredDrivers[idriver]->initialize(dc);
    }
  }
}

void B4DetectorConstruction::registerGeometryDriver(B4VSubDetectorDriver* aSubDetectorDriver)
{
  //--------------------------------
  // Register sub-detector drivers.
  //--------------------------------
  for (unsigned int idriver = 0; idriver < m_registeredDrivers.size(); idriver++) {
    if (m_registeredDrivers[idriver]->isApplicable(aSubDetectorDriver->getName())) {

      FATAL("Trying to register twice the same geometry driver name "
            << aSubDetectorDriver->getName() << " !!!");
    }
  }
  m_registeredDrivers.push_back(aSubDetectorDriver);
}
