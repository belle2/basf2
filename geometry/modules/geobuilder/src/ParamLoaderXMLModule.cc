/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/ParamLoaderXMLModule.h>

#include <framework/core/ModuleUtils.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearboxIOXML.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParamLoaderXML)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParamLoaderXMLModule::ParamLoaderXMLModule() : Module()
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters from a XML document.");

  //Parameter definition
  addParam("InputFileXML",  m_filenameXML, "The filename of the XML input file which should be loaded.");
  addParam("ParamCheck",  m_paramCheck, "Turns on/off parameter validation. Turn it off for speedup.", true);
}


ParamLoaderXMLModule::~ParamLoaderXMLModule()
{

}


void ParamLoaderXMLModule::initialize()
{
  //Check parameters
  if (!ModuleUtils::fileNameExists(m_filenameXML)) {
    B2ERROR("Parameter <InputFileXML>: The specified filename " << m_filenameXML << " does not exist !")
  }

  GearboxIOXML* gearboxIOXML = new GearboxIOXML();
  bool result = gearboxIOXML->open(m_filenameXML);

  if (result) {
    Gearbox& gearbox = Gearbox::Instance();
    gearbox.connect(gearboxIOXML);

    gearbox.enableParamCheck(m_paramCheck);

  } else {
    B2ERROR("Could not open XML file: " << m_filenameXML)
  }
}
