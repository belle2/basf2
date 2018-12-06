/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/gearbox/GearboxModule.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
// needed for complex module parameter
#include <framework/core/ModuleParam.templateDetails.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Gearbox)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GearboxModule::GearboxModule() : Module()
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters from an XML document.");
  setPropertyFlags(c_ParallelProcessingCertified);


  m_backends.push_back("file:");

  //Parameter definition
  addParam("backends",  m_backends, "The backends to use when looking for xml "
           "data. A backend can also contain a search path after ':'. (If none "
           "is given, '/data' will be used.)", m_backends);
  addParam("fileName", m_fileName, "The filename of the main xml file",
           string("geometry/Belle2.xml"));
  addParam("override", m_unitOverrides, "Override single values from the XML "
           "file. This should be a list of tuples containing an xpath "
           "expression, a value and a unit (which can be empty). The xpath "
           "expression must resolve to exactly one node in the XML tree which "
           "does not contain any children except text. The supplied value and "
           "unit will be set for this node. See "
           "framework/examples/gearbox_override.py", m_unitOverrides);
  addParam("overrideMultiple", m_multipleOverrides, "Same as override but the "
           "xpath expression may evaluate to more than one node in which case "
           "all occurances are set to the supplied value and unit",
           m_multipleOverrides);
  addParam("overridePrefix", m_overridePrefix, "Common prefix which is "
           "prepended to all overrides. Beware that '//' has a special meaning "
           "meaning in xpath so be careful with leading and trailing slashes "
           "in the overrides and the prefix respectively", std::string("/Detector"));
}

void GearboxModule::initialize()
{
  //gearbox might need exp/run numbers
  StoreObjPtr<EventMetaData>().isRequired();

  Gearbox& gearbox = Gearbox::getInstance();
  for (auto& unit : m_unitOverrides) {
    Gearbox::PathOverride poverride;
    poverride.path = m_overridePrefix + std::get<0>(unit);
    poverride.value = std::get<1>(unit);
    poverride.unit = std::get<2>(unit);
    gearbox.addOverride(poverride);
  }
  for (auto& multiple : m_multipleOverrides) {
    Gearbox::PathOverride poverride;
    poverride.path = m_overridePrefix + std::get<0>(multiple);
    poverride.value = std::get<1>(multiple);
    poverride.unit = std::get<2>(multiple);
    poverride.multiple = true;
    gearbox.addOverride(poverride);
  }

  gearbox.setBackends(m_backends);
  gearbox.open(m_fileName);
}

void GearboxModule::beginRun()
{
  //if only file backend is used, we don't want to reread the data in every run
  if (m_backends.size() == 1 && m_backends[0] == "file:")
    return;

  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.close();
  gearbox.open(m_fileName);
}
