/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/CreatorFactory.h>

#include "G4SDManager.hh"
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  using namespace geometry;

  namespace ECL {

//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    geometry::CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    void barrel(G4LogicalVolume*);
    void backward(G4LogicalVolume*);
    void forward(G4LogicalVolume*);


    GeoECLCreator::GeoECLCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector("ECLSensitiveDetector", (2 * 24)*CLHEP::eV, 10 * CLHEP::MeV);
      G4SDManager::GetSDMpointer()->AddNewDetector(m_sensitive);
      m_bkgsensitive.clear();
    }


    GeoECLCreator::~GeoECLCreator()
    {
      //      delete m_sensitive;
      for (BkgSensitiveDetector* sensitive : m_bkgsensitive) delete sensitive;
      m_bkgsensitive.clear();
    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      barrel(&topVolume);
      forward(&topVolume);
      backward(&topVolume);
    }

  }//ecl
}//belle2
