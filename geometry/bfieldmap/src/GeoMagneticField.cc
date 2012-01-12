/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/GeoMagneticField.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <geometry/bfieldmap/BFieldComponentConstant.h>
#include <geometry/bfieldmap/BFieldComponentRadial.h>
#include <geometry/bfieldmap/BFieldComponentQuad.h>
#include <geometry/bfieldmap/BFieldComponentBeamline.h>
#include <geometry/CreatorFactory.h>


#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

geometry::CreatorFactory<GeoMagneticField> GeoMagneticFieldFactory("GeoMagneticField");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoMagneticField::GeoMagneticField() : CreatorBase()
{
  //Add the function pointers called for reading the components to the map
  m_componentTypeMap.insert(make_pair("Constant", boost::bind(&GeoMagneticField::readConstantBField, this, _1)));
  m_componentTypeMap.insert(make_pair("Radial",   boost::bind(&GeoMagneticField::readRadialBField,   this, _1)));
  m_componentTypeMap.insert(make_pair("Quad",     boost::bind(&GeoMagneticField::readQuadBField,     this, _1)));
  m_componentTypeMap.insert(make_pair("Beamline", boost::bind(&GeoMagneticField::readBeamlineBField, this, _1)));
}


GeoMagneticField::~GeoMagneticField()
{

}


void GeoMagneticField::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
{
  //Read the magnetic field components
  GearDir components(content, "Components/Component");

  //Loop over all components of the magnetic field
  CompTypeMap::iterator findIter;
  BOOST_FOREACH(const GearDir & component, content.getNodes("Components/Component")) {
    //Get the type of the magnetic field and call the appropriate function
    string compType = component.getString("attribute::type");
    B2DEBUG(10, "GeoMagneticField creator: Loading the parameters for the component type'" << compType << "'")

    findIter = m_componentTypeMap.find(compType);
    if (findIter != m_componentTypeMap.end()) {
      findIter->second(component);
    } else {
      B2ERROR("The magnetic field component type '" << compType << "' is unknown !")
    }
  }
}


//============================================================
//                   Protected methods
//============================================================

void GeoMagneticField::readConstantBField(const GearDir& component)
{
  double xValue = component.getDouble("X");
  double yValue = component.getDouble("Y");
  double zValue = component.getDouble("Z");

  BFieldComponentConstant& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentConstant>();
  bComp.setMagneticFieldValues(xValue, yValue, zValue);
}


void GeoMagneticField::readRadialBField(const GearDir& component)
{
  string mapFilename   = component.getString("MapFilename");
  int mapSizeR         = component.getInt("NumberGridPointsR");
  int mapSizeZ         = component.getInt("NumberGridPointsZ");

  double mapRegionMinZ = component.getLength("ZMin");
  double mapRegionMaxZ = component.getLength("ZMax");
  double mapOffset   = component.getLength("ZOffset");

  double mapRegionMinR = component.getLength("RadiusMin");
  double mapRegionMaxR = component.getLength("RadiusMax");

  double gridPitchR    = component.getLength("GridPitchR");
  double gridPitchZ    = component.getLength("GridPitchZ");

  BFieldComponentRadial& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentRadial>();
  bComp.setMapFilename(mapFilename);
  bComp.setMapSize(mapSizeR, mapSizeZ);
  bComp.setMapRegionZ(mapRegionMinZ, mapRegionMaxZ, mapOffset);
  bComp.setMapRegionR(mapRegionMinR, mapRegionMaxR);
  bComp.setGridPitch(gridPitchR, gridPitchZ);
}

void GeoMagneticField::readQuadBField(const GearDir& component)
{
  string mapFilenameHER = component.getString("MapFilenameHER");
  string mapFilenameLER = component.getString("MapFilenameLER");
  string apertFilenameHER = component.getString("ApertFilenameHER");
  string apertFilenameLER = component.getString("ApertFilenameLER");

  int mapSizeHER        = component.getInt("MapSizeHER");
  int mapSizeLER        = component.getInt("MapSizeLER");
  int apertSizeHER      = component.getInt("ApertSizeHER");
  int apertSizeLER      = component.getInt("ApertSizeLER");

  /* save beam energy in [eV] */
  double beamEnergyHER    = component.getEnergy("BeamEnergyHER") / Unit::eV ;
  double beamEnergyLER    = component.getEnergy("BeamEnergyLER") / Unit::eV ;

  BFieldComponentQuad& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentQuad>();
  bComp.setMapFilename(mapFilenameHER, mapFilenameLER);
  bComp.setApertFilename(apertFilenameHER, apertFilenameLER);
  bComp.setMapSize(mapSizeHER, mapSizeLER);
  bComp.setApertSize(apertSizeHER, apertSizeLER);
  bComp.setBeamEnergy(beamEnergyHER, beamEnergyLER);
}

void GeoMagneticField::readBeamlineBField(const GearDir& component)
{
  string mapFilenameHER = component.getString("MapFilenameHER");
  string mapFilenameLER = component.getString("MapFilenameLER");
  string interFilenameHER = component.getString("InterFilenameHER");
  string interFilenameLER = component.getString("InterFilenameLER");

  double mapRegionMinZ = component.getLength("ZMin");
  double mapRegionMaxZ = component.getLength("ZMax");

  double mapRegionMinR = component.getLength("RadiusMin");
  double mapRegionMaxR = component.getLength("RadiusMax");

  double beamAngle = component.getLength("BeamAngle");

  BFieldComponentBeamline& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentBeamline>();
  bComp.setMapFilename(mapFilenameHER, mapFilenameLER);
  bComp.setInterpolateFilename(interFilenameHER, interFilenameLER);
  bComp.setMapRegionZ(mapRegionMinZ, mapRegionMaxZ);
  bComp.setMapRegionR(mapRegionMinR, mapRegionMaxR);
  bComp.setBeamAngle(beamAngle);
}
