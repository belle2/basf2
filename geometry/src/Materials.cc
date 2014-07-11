/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/Materials.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <memory>
#include <limits>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4Material.hh>
#include <G4Element.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>

#include "CLHEP/Units/PhysicalConstants.h"

using namespace std;

namespace Belle2 {
  namespace geometry {

    Materials& Materials::getInstance()
    {
      static auto_ptr<Materials> instance(new Materials());
      return *instance;
    }

    G4Material* Materials::getMaterial(const string& name, bool showErrors) const
    {
      G4Material* mat;
      if (m_materialCache.retrieve(name, mat)) {
        return mat;
      }
      mat = G4NistManager::Instance()->FindOrBuildMaterial(name);

      //Try different combinations of the Material name to fallback to predefined G4 Elements
      if (!mat && name.substr(0, 3) != "G4_") {
        if (showErrors) {
          B2DEBUG(100, "Could not find Material '" << name << "', searching Geant4 names G4_"
                  << name << " and G4_" << boost::to_upper_copy(name));
        }
        //Mainly for materials from single elements, e.g. G4_Al, G4_Si
        mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_" + name);
        //For predefined materials, e.g. G4_AIR, G4_TEFLON
        if (!mat) mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_" + boost::to_upper_copy(name));
      }

      //Insert into cache
      m_materialCache.insert(name, mat);

      if (!mat && showErrors) B2ERROR("Material '" << name << "' could not be found");
      return mat;
    }

    G4Element* Materials::getElement(const string& name) const
    {
      G4Element* elm = G4NistManager::Instance()->FindOrBuildElement(name);
      if (!elm) B2ERROR("Element '" << name << "' could not be found");
      return elm;
    }

    G4Material* Materials::createMaterial(const gearbox::Interface& parameters)
    {
      //Get straightforward parameters from gearbox
      string name = parameters.getString("@name");
      G4Material* oldmat = getMaterial(name, false);
      if (oldmat) {
        //We cannot modify or delete materials, so we just use existing ones
        B2DEBUG(100, "Material with name " << name << " already existing, using existing material");
        return oldmat;
      }
      B2INFO("Creating Material " << name);
      string stateStr = parameters.getString("state", "undefined");
      double density = parameters.getDensity("density", 0) * CLHEP::g / CLHEP::cm3;
      double temperature = parameters.getDouble("temperature", CLHEP::STP_Temperature);
      double pressure = parameters.getDouble("pressure", CLHEP::STP_Pressure / CLHEP::pascal);
      //If density is negative or smaller than epsilon we should calculate the
      //density from the used materials
      bool deductDensity = density < 1e-25;

      //Check if we can calculate density, only works if we just add materials,
      //elements do not have density
      if (deductDensity && parameters.getNumberNodes("Components/Element")) {
        B2ERROR("createMaterial " << name
                << ": Cannot calculate density when adding elements, please provde a density");
        return 0;
      }

      //Sum up fractions to see if they are normalized
      double sumFractions(0);

      //Prepare vector with all component materials
      vector<G4Material*> componentMaterials;
      vector<double> fractionMaterials;
      BOOST_FOREACH(const GearDir & material, parameters.getNodes("Components/Material")) {
        G4Material* mat = getMaterial(material.getString());
        double fraction = material.getDouble("@fraction", 1.0);
        if (!mat) {
          B2ERROR("createMaterial " << name << ": Material '" << material.getString() << "' not found");
          return 0;
        }
        componentMaterials.push_back(mat);
        fractionMaterials.push_back(fraction);
        if (deductDensity) density += mat->GetDensity() * fraction;
        sumFractions += fraction;
      }
      //Normalize density if we calculate it
      if (deductDensity) density /= sumFractions;

      //Prepare vector with all component elements
      vector<G4Element*> componentElements;
      vector<double> fractionElements;
      BOOST_FOREACH(const GearDir & element, parameters.getNodes("Components/Element")) {
        G4Element* elm = getElement(element.getString());
        double fraction = element.getDouble("@fraction", 1.0);
        if (!elm) {
          B2ERROR("createMaterial " << name << ": Element '" << element.getString() << "' not found");
          return 0;
        }
        componentElements.push_back(elm);
        fractionElements.push_back(fraction);
        sumFractions += fraction;
      }

      //Warn if the fractions are not normalized
      if (abs(sumFractions - 1) > numeric_limits<double>::epsilon()) {
        B2WARNING("createMaterial " << name << ": Fractions not normalized, scaling by 1/" << sumFractions);
      }

      //Determine material state
      boost::to_lower(stateStr);
      G4State state = kStateUndefined;
      if (stateStr == "solid") {
        state = kStateSolid;
      } else if (stateStr == "liquid") {
        state = kStateLiquid;
      } else if (stateStr == "gas") {
        state = kStateGas;
      } else if (stateStr != "undefined") {
        B2WARNING("createMaterial " << name << ": Unknown state '" << stateStr << "', using undefined");
      }

      //Finally, create Material and add all components
      G4Material* mat = new G4Material(name, density, componentElements.size() + componentMaterials.size(),
                                       state, temperature, pressure * CLHEP::pascal);

      for (size_t i = 0; i < componentMaterials.size(); ++i) {
        mat->AddMaterial(componentMaterials[i], fractionMaterials[i] / sumFractions);
      }
      for (size_t i = 0; i < componentElements.size(); ++i) {
        mat->AddElement(componentElements[i], fractionElements[i] / sumFractions);
      }

      //check if we have properties defined for this material
      G4MaterialPropertiesTable* g4PropTable = createProperties(parameters);
      if (g4PropTable) mat->SetMaterialPropertiesTable(g4PropTable);

      //Insert into cache and return
      m_materialCache.insert(name, mat);
      return mat;
    }

    G4MaterialPropertiesTable* Materials::createProperties(const gearbox::Interface& parameters)
    {
      if (parameters.getNumberNodes("Property") > 0) {
        //Apparantly we have properties, so lets add them to the Material
        G4MaterialPropertiesTable* g4PropTable = new G4MaterialPropertiesTable();
        BOOST_FOREACH(const GearDir & property, parameters.getNodes("Property")) {
          string name;
          try {
            name = property.getString("@name");
          } catch (gearbox::PathEmptyError) {
            B2ERROR("Property at " << property.getPath() << " does not have a name attribute, ignoring");
            continue;
          }
          double conversionFactor = Unit::convertValue(1, property.getString("@unit", "GeV"));
          //Geant4 uses MeV as default energy
          conversionFactor /= Unit::MeV;
          unsigned int nValues = property.getNumberNodes("value");
          double energies[nValues];
          double values[nValues];
          for (unsigned int i = 0; i < nValues; ++i) {
            GearDir value(property, "value", i + 1);
            energies[i] = value.getDouble("@energy") * conversionFactor;
            values[i] = value.getDouble();
          }
          g4PropTable->AddProperty(name.c_str(), energies, values, nValues);
        }
        return g4PropTable;
      }
      return 0;
    }

    G4OpticalSurface* Materials::createOpticalSurface(const gearbox::Interface& parameters)
    {
      string name         = parameters.getString("@name", "OpticalSurface");
      string modelString  = parameters.getString("Model", "glisur");
      string finishString = parameters.getString("Finish", "polished");
      string typeString   = parameters.getString("Type", "dielectric_dielectric");
      double value        = parameters.getDouble("Value", 1.0);

#define CHECK_ENUM_VALUE(name,value) if (name##String == #value) { name = value; }
      G4OpticalSurfaceModel model;
      boost::to_lower(modelString);
      CHECK_ENUM_VALUE(model, glisur)
      else CHECK_ENUM_VALUE(model, unified)
        else CHECK_ENUM_VALUE(model, LUT)
          else {
            B2ERROR("Unknown Optical Surface Model: " << modelString);
            return 0;
          }

      G4OpticalSurfaceFinish finish;
      boost::to_lower(finishString);
      CHECK_ENUM_VALUE(finish, polished)
      else CHECK_ENUM_VALUE(finish, polishedfrontpainted)
        else CHECK_ENUM_VALUE(finish, polishedbackpainted)
          else CHECK_ENUM_VALUE(finish, ground)
            else CHECK_ENUM_VALUE(finish, groundfrontpainted)
              else CHECK_ENUM_VALUE(finish, groundbackpainted)
                else CHECK_ENUM_VALUE(finish, polishedlumirrorair)
                  else CHECK_ENUM_VALUE(finish, polishedlumirrorglue)
                    else CHECK_ENUM_VALUE(finish, polishedair)
                      else CHECK_ENUM_VALUE(finish, polishedteflonair)
                        else CHECK_ENUM_VALUE(finish, polishedtioair)
                          else CHECK_ENUM_VALUE(finish, polishedtyvekair)
                            else CHECK_ENUM_VALUE(finish, polishedvm2000air)
                              else CHECK_ENUM_VALUE(finish, polishedvm2000glue)
                                else CHECK_ENUM_VALUE(finish, etchedlumirrorair)
                                  else CHECK_ENUM_VALUE(finish, etchedlumirrorglue)
                                    else CHECK_ENUM_VALUE(finish, etchedair)
                                      else CHECK_ENUM_VALUE(finish, etchedteflonair)
                                        else CHECK_ENUM_VALUE(finish, etchedtioair)
                                          else CHECK_ENUM_VALUE(finish, etchedtyvekair)
                                            else CHECK_ENUM_VALUE(finish, etchedvm2000air)
                                              else CHECK_ENUM_VALUE(finish, etchedvm2000glue)
                                                else CHECK_ENUM_VALUE(finish, groundlumirrorair)
                                                  else CHECK_ENUM_VALUE(finish, groundlumirrorglue)
                                                    else CHECK_ENUM_VALUE(finish, groundair)
                                                      else CHECK_ENUM_VALUE(finish, groundteflonair)
                                                        else CHECK_ENUM_VALUE(finish, groundtioair)
                                                          else CHECK_ENUM_VALUE(finish, groundtyvekair)
                                                            else CHECK_ENUM_VALUE(finish, groundvm2000air)
                                                              else CHECK_ENUM_VALUE(finish, groundvm2000glue)
                                                                else {
                                                                  B2ERROR("Unknown Optical Surface Finish: " << finishString);
                                                                  return 0;
                                                                }

      G4SurfaceType type;
      boost::to_lower(typeString);
      CHECK_ENUM_VALUE(type, dielectric_metal)
      else CHECK_ENUM_VALUE(type, dielectric_dielectric)
        else CHECK_ENUM_VALUE(type, dielectric_LUT)
          else CHECK_ENUM_VALUE(type, firsov)
            else CHECK_ENUM_VALUE(type, x_ray)
              else {
                B2ERROR("Unknown Optical Surface Type: " << typeString);
                return 0;
              }
#undef CHECK_ENUM_VALUE

      G4OpticalSurface* optSurf = new G4OpticalSurface(name, model, finish, type, value);

      G4MaterialPropertiesTable* g4PropTable = createProperties(GearDir(parameters, "Properties"));
      if (g4PropTable) optSurf->SetMaterialPropertiesTable(g4PropTable);

      return optSurf;
    }


  } //geometry namespace
} //Belle2 namespace
