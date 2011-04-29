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

#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoMagneticField regGeoMagneticField;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoMagneticField::GeoMagneticField() : CreatorBase("GeoMagneticField")
{
  setDescription("Sets the magnetic field for the Belle II detector.");

  //Add the function pointers called for reading the components to the map
  m_componentTypeMap.insert(make_pair("Constant", boost::bind(&GeoMagneticField::readConstantBField, this, _1)));
  m_componentTypeMap.insert(make_pair("Radial",   boost::bind(&GeoMagneticField::readRadialBField,   this, _1)));
}


GeoMagneticField::~GeoMagneticField()
{

}


void GeoMagneticField::create(GearDir& content)
{
  //Read the magnetic field components
  GearDir components(content, "Components/Component");

  //Loop over all components of the magnetic field
  CompTypeMap::iterator findIter;
  int nComp = components.getNumberNodes();
  for (int iComp = 1; iComp <= nComp; ++iComp) {
    GearDir component(components, iComp);

    //Get the type of the magnetic field and call the appropriate function
    string compType = component.getParamString("attribute::type");
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

void GeoMagneticField::readConstantBField(GearDir& component)
{
  double xValue = component.getParamNumValue("X");
  double yValue = component.getParamNumValue("Y");
  double zValue = component.getParamNumValue("Z");

  BFieldComponentConstant& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentConstant>();
  bComp.setMagneticFieldValues(xValue, yValue, zValue);
}


void GeoMagneticField::readRadialBField(GearDir& component)
{
  string mapFilename   = component.getParamString("MapFilename");
  int mapSizeR         = component.getParamIntValue("NumberGridPointsR");
  int mapSizeZ         = component.getParamIntValue("NumberGridPointsZ");

  double mapRegionMinZ = component.getParamLength("ZMin");
  double mapRegionMaxZ = component.getParamLength("ZMax");
  double mapOffset   = component.getParamLength("ZOffset");

  double mapRegionMinR = component.getParamLength("RadiusMin");
  double mapRegionMaxR = component.getParamLength("RadiusMax");

  double gridPitchR    = component.getParamLength("GridPitchR");
  double gridPitchZ    = component.getParamLength("GridPitchZ");

  BFieldComponentRadial& bComp = BFieldMap::Instance().addBFieldComponent<BFieldComponentRadial>();
  bComp.setMapFilename(mapFilename);
  bComp.setMapSize(mapSizeR, mapSizeZ);
  bComp.setMapRegionZ(mapRegionMinZ, mapRegionMaxZ, mapOffset);
  bComp.setMapRegionR(mapRegionMinR, mapRegionMaxR);
  bComp.setGridPitch(gridPitchR, gridPitchZ);
}
