/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/Materials.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <geometry/dbobjects/GeoMaterial.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>

#include <memory>
#include <limits>
#include <boost/algorithm/string.hpp>

#include <G4Material.hh>
#include <G4Element.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4MaterialPropertiesTable.hh>

#include "CLHEP/Units/PhysicalConstants.h"

using namespace std;

namespace Belle2 {
  namespace geometry {
    namespace {
      /** add GeoMaterialProperties defined in XML to a payload object
       * @param object instance of GeoMaterial or GeoOpticalSurface to add the properties to
       * @param parameters gearbox parameters containing the <Property> definitions
       */
      template<class T> void addProperties(T& object, const gearbox::Interface& parameters)
      {
        // check if we have properties defined for this material
        if (parameters.getNumberNodes("Property") > 0) {
          //Apparantly we have properties, so lets add them to the Material
          for (const GearDir& property : parameters.getNodes("Property")) {
            string name;
            try {
              name = property.getString("@name");
            } catch (gearbox::PathEmptyError&) {
              B2ERROR("Property at " << property.getPath() << " does not have a name attribute, ignoring");
              continue;
            }
            const double conversionFactor = Unit::convertValue(1, property.getString("@unit", "GeV"));
            unsigned int nValues = property.getNumberNodes("value");
            std::vector<double> energies(nValues, 0);
            std::vector<double> values(nValues, 0);
            for (unsigned int i = 0; i < nValues; ++i) {
              GearDir value(property, "value", i + 1);
              energies[i] = value.getDouble("@energy") * conversionFactor;
              values[i] = value.getDouble();
            }
            object.addProperty({name, energies, values});
          }
        }
      }
    }

    G4MaterialPropertiesTable* Materials::createProperties(const std::vector<GeoMaterialProperty>& props)
    {
      if (!props.size()) return nullptr;
      auto* table = new G4MaterialPropertiesTable();
      m_PropTables.push_back(table);
      for (const GeoMaterialProperty& prop : props) {
        std::vector<double> energies = prop.getEnergies();
        std::vector<double> values = prop.getValues();
        const size_t n = values.size();
        // convert energies to Geant4 Units
        for (double& energy : energies) energy *= CLHEP::GeV / Unit::GeV;
        table->AddProperty(prop.getName().c_str(), energies.data(), values.data(), n);
      }
      return table;
    }

    Materials& Materials::getInstance()
    {
      static unique_ptr<Materials> instance(new Materials());
      return *instance;
    }

    Materials::~Materials()
    {
      clear();
    }

    void Materials::initBuilders()
    {
      if (!m_nistMaterialBuilder) {
        B2DEBUG(50, "Creating new Nist Builder instances");
        m_nistElementBuilder = new G4NistElementBuilder(0);
        m_nistMaterialBuilder = new G4NistMaterialBuilder(m_nistElementBuilder, 0);
      }
    }

    G4Material* Materials::findMaterial(const string& name)
    {
      G4Material* mat{nullptr};
      if (m_materialCache.retrieve(name, mat)) {
        return mat;
      }
      initBuilders();
      mat = m_nistMaterialBuilder->FindOrBuildMaterial(name);

      //Try different combinations of the Material name to fallback to predefined G4 Elements
      if (!mat && name.substr(0, 3) != "G4_") {
        //Mainly for materials from single elements, e.g. G4_Al, G4_Si
        mat = m_nistMaterialBuilder->FindOrBuildMaterial("G4_" + name);
        //For predefined materials, e.g. G4_AIR, G4_TEFLON
        if (!mat) mat = m_nistMaterialBuilder->FindOrBuildMaterial("G4_" + boost::to_upper_copy(name));
      }

      //Insert into cache
      m_materialCache.insert(name, mat);
      return mat;
    }

    G4Material* Materials::getMaterial(const string& name, bool showErrors)
    {
      G4Material* mat = findMaterial(name);
      if (!mat && showErrors) B2ERROR("Material '" << name << "' could not be found");
      // There is a chance that we want to scale densities of all materials for studies.
      // Now we have the original so we can look to the scaled one or create it if it's missing
      if (m_densityScale) {
        if (m_ignoreScaling.count(name) > 0) return mat;
        if (name.substr(0, 7) == "scaled:")
          B2WARNING("Requested an already scaled material ... scaling once more" << LogVar("material", name));
        std::string scaled = "scaled:" + std::to_string(*m_densityScale) + ":" + name;
        G4Material* scaledMat = findMaterial(scaled);
        if (!scaledMat) {
          scaledMat = new G4Material(scaled, mat->GetDensity() * *m_densityScale, mat,
                                     mat->GetState(), mat->GetTemperature(), mat->GetPressure());
          m_materialCache.insert(scaled, scaledMat);
        }
        return scaledMat;
      }
      return mat;
    }

    G4Element* Materials::getElement(const string& name)
    {
      initBuilders();
      G4Element* elm = m_nistElementBuilder->FindOrBuildElement(name);
      if (!elm) B2ERROR("Element '" << name << "' could not be found");
      return elm;
    }

    G4Material* Materials::createMaterial(const gearbox::Interface& parameters)
    {
      GeoMaterial material = createMaterialConfig(parameters);
      return createMaterial(material);
    }

    G4Material* Materials::createMaterial(const GeoMaterial& parameters)
    {
      G4Material* oldmat = getMaterial(parameters.getName(), false);
      if (oldmat) {
        //We cannot modify or delete materials, so we just use existing ones
        B2ERROR("Material with name " << parameters.getName() << " already existing");
        return oldmat;
      }
      B2DEBUG(10, "Creating Material " << parameters.getName());
      //If density is negative or smaller than epsilon we should calculate the
      //density from the used materials
      double density = parameters.getDensity() * CLHEP::g / CLHEP::cm3;
      if (density < 1e-25) {
        density = 0;
        for (const GeoMaterialComponent& component : parameters.getComponents()) {
          //Check if we can calculate density, only works if we just add materials,
          //elements do not have density
          if (component.getIselement()) {
            B2ERROR("createMaterial " << parameters.getName()
                    << ": Cannot calculate density when adding elements, please provde a density");
            return nullptr;
          }
          G4Material* mat = getMaterial(component.getName());
          if (!mat) {
            B2ERROR("createMaterial " << parameters.getName() << ": Material '" << component.getName() << "' not found");
            return nullptr;
          }
          density += mat->GetDensity() * component.getFraction();
        }
      }

      //Finally, create Material and add all components
      G4Material* mat = new G4Material(parameters.getName(), density, parameters.getComponents().size(),
                                       (G4State)parameters.getState(), parameters.getTemperature(), parameters.getPressure() * CLHEP::pascal);

      for (const GeoMaterialComponent& component : parameters.getComponents()) {
        if (component.getIselement()) {
          G4Element* cmp = getElement(component.getName());
          if (!cmp) {
            B2ERROR("Cannot create material " << parameters.getName() << ": element " << component.getName() << " not found");
            return nullptr;
          }
          mat->AddElement(cmp, component.getFraction());
        } else {
          G4Material* cmp = getMaterial(component.getName());
          if (!cmp) {
            B2ERROR("Cannot create material " << parameters.getName() << ": material " << component.getName() << " not found");
            return nullptr;
          }
          mat->AddMaterial(cmp, component.getFraction());
        }
      }

      //add properties
      mat->SetMaterialPropertiesTable(createProperties(parameters.getProperties()));

      //Insert into cache and return
      m_materialCache.insert(parameters.getName(), mat);
      return mat;
    }

    GeoMaterial Materials::createMaterialConfig(const gearbox::Interface& parameters)
    {
      //Get straightforward parameters from gearbox
      string name = parameters.getString("@name");
      B2DEBUG(10, "Creating Material Config " << name);
      string stateStr = parameters.getString("state", "undefined");
      double density = parameters.getDensity("density", 0); // * CLHEP::g / CLHEP::cm3;
      double temperature = parameters.getDouble("temperature", CLHEP::STP_Temperature);
      double pressure = parameters.getDouble("pressure", CLHEP::STP_Pressure / CLHEP::pascal);

      // set the simple ones like they are
      GeoMaterial material;
      material.setName(name);
      material.setPressure(pressure);
      material.setTemperature(temperature);
      material.setDensity(density);

      //Sum up fractions to see if they are normalized
      double sumFractions(0);

      // Add all component materials
      for (const GearDir& component : parameters.getNodes("Components/Material")) {
        const string componentName = component.getString();
        double fraction = component.getDouble("@fraction", 1.0);
        material.addComponent({componentName, false, fraction});
        sumFractions += fraction;
      }
      // Add all component elements
      for (const GearDir& element : parameters.getNodes("Components/Element")) {
        const std::string elementName = element.getString();
        double fraction = element.getDouble("@fraction", 1.0);
        material.addComponent({elementName, true, fraction});
        sumFractions += fraction;
      }

      //Warn if the fractions are not normalized
      if (abs(sumFractions - 1) > numeric_limits<double>::epsilon()) {
        B2WARNING("createMaterial " << name << ": Fractions not normalized, scaling by 1/" << sumFractions);
        for (GeoMaterialComponent& cmp : material.getComponents()) {
          cmp.setFraction(cmp.getFraction() / sumFractions);
        }
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
      material.setState(state);

      addProperties(material, parameters);
      // all done, return material description
      return material;
    }

    G4OpticalSurface* Materials::createOpticalSurface(const gearbox::Interface& parameters)
    {
      GeoOpticalSurface surface = createOpticalSurfaceConfig(parameters);
      return createOpticalSurface(surface);
    }

    G4OpticalSurface* Materials::createOpticalSurface(const GeoOpticalSurface& surface)
    {
      G4OpticalSurface* optSurf = new G4OpticalSurface(surface.getName(),
                                                       (G4OpticalSurfaceModel) surface.getModel(),
                                                       (G4OpticalSurfaceFinish) surface.getFinish(),
                                                       (G4SurfaceType) surface.getType(),
                                                       surface.getValue());
      optSurf->SetMaterialPropertiesTable(createProperties(surface.getProperties()));
      //m_OptSurfaces.push_back(optSurf);
      return optSurf;
    }

    GeoOpticalSurface Materials::createOpticalSurfaceConfig(const gearbox::Interface& parameters)
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
            B2FATAL("Unknown Optical Surface Model: " << modelString);
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
                                                                  B2FATAL("Unknown Optical Surface Finish: " << finishString);
                                                                }

      G4SurfaceType type;
      boost::to_lower(typeString);
      CHECK_ENUM_VALUE(type, dielectric_metal)
      else CHECK_ENUM_VALUE(type, dielectric_dielectric)
        else CHECK_ENUM_VALUE(type, dielectric_LUT)
          else CHECK_ENUM_VALUE(type, firsov)
            else CHECK_ENUM_VALUE(type, x_ray)
              else {
                B2FATAL("Unknown Optical Surface Type: " << typeString);
              }
#undef CHECK_ENUM_VALUE

      GeoOpticalSurface surface(name, model, finish, type, value);
      addProperties(surface, GearDir(parameters, "Properties"));
      return surface;
    }

    void Materials::clear()
    {
      // delete all property tables we have
      B2DEBUG(50, "Cleaning G4MaterialPropertiesTable");
      for (G4MaterialPropertiesTable* prop : m_PropTables) delete prop;
      m_PropTables.clear();
      // and last but not least: get rid of all materials
      B2DEBUG(50, "Cleaning G4Materials");
      G4MaterialTable& materials = *G4Material::GetMaterialTable();
      for (G4Material* mat : materials) delete mat;
      materials.clear();
      B2DEBUG(50, "Cleaning G4Elements");
      G4ElementTable& elements = *G4Element::GetElementTable();
      for (G4Element* elm : elements) delete elm;
      elements.clear();
      B2DEBUG(50, "Cleaning G4Isotopes");
      auto& isotopes = const_cast<G4IsotopeTable&>(*G4Isotope::GetIsotopeTable());
      for (G4Isotope* iso : isotopes) delete iso;
      isotopes.clear();
      // delete material and element builder as they keep indices to materials they created :/
      delete m_nistMaterialBuilder;
      delete m_nistElementBuilder;
      m_nistMaterialBuilder = nullptr;
      m_nistElementBuilder = nullptr;
      // finally, get rid of the cache, it's invalid now anyway
      B2DEBUG(50, "Clean up material cache");
      m_materialCache.clear();
    }

  } //geometry namespace
} //Belle2 namespace
