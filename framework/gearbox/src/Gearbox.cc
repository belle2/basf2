/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearboxIOXML.h>
#include <framework/logging/Logger.h>

#include <boost/filesystem.hpp>

using namespace Belle2;
using namespace std;
using namespace boost::filesystem;
using namespace boost::python;

Gearbox* Gearbox::m_instance = NULL;


Gearbox& Gearbox::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) {
    m_instance = new Gearbox();
  }
  return *m_instance;
}


void Gearbox::connect(GearboxIOAbs* gearboxIO)
{
  //Close any open GearboxIO object
  if (close()) {
    m_gearboxIO = gearboxIO;
  } else {
    ERROR("Could not close the connection of the GearboxIO object !")
  }
}


GearboxIOAbs& Gearbox::getGearboxIO() const throw(GbxExcIONotConnected)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return *m_gearboxIO;
}


GearDir Gearbox::getContent(const std::string& paramSetType, EGearboxContentType contentType) throw(GbxExcIONotConnected)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();

  switch (contentType) {
    case c_GbxGlobal:       return GearDir(getGlobalParamPath(paramSetType));
    case c_GbxMaterial:     return GearDir(getMaterialParamPath(paramSetType));
    case c_GbxSubdetectors: return GearDir(getSubdetectorPath(paramSetType));
    default:                return GearDir();
  }
  return GearDir();
}


std::string Gearbox::getGlobalParamPath(const std::string& globalParam) const
{
  return "/Detector/Global/ParamSet[@type=\"" + globalParam + "\"]/Content/";
}


std::string Gearbox::getMaterialParamPath(const std::string& materialParam) const
{
  return "/Detector/MaterialSets/ParamSet[@type=\"" + materialParam + "\"]/Content/";
}


std::string Gearbox::getSubdetectorPath(const string& subdetector) const
{
  return "/Detector/Subdetectors/ParamSet[@type=\"" + subdetector + "\"]/Content/";
}


void Gearbox::enableParamCheck(bool paramCheck)
throw(GbxExcIONotConnected)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->enableParamCheck(paramCheck);
}


bool Gearbox::isPathValid(const std::string& path) const
throw(GbxExcIONotConnected)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->isPathValid(path);
}


bool Gearbox::isParamAvailable(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->isParamAvailable(path);
}


int Gearbox::getNumberNodes(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult, GbxExcPathResultNotValid)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->getNumberNodes(path);
}


double Gearbox::getParamLength(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->getParamLength(path);
}


double Gearbox::getParamAngle(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->getParamAngle(path);
}


double Gearbox::getParamNumValue(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->getParamNumValue(path);
}


std::string Gearbox::getParamString(const std::string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GbxExcIONotConnected();
  return m_gearboxIO->getParamString(path);
}


//=====================================================================
//                          Python API
//=====================================================================

void translateGbxExcPathNotValid(GbxExcPathNotValid const& exc)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "The path \"" + exc.getXPathStatement() + "\" is not valid !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void translateGbxExcPathEmptyResult(GbxExcPathEmptyResult const& exc)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "The result of the path \"" + exc.getXPathStatement() + "\" is empty !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void translateGbxExcPathResultNotValid(GbxExcPathResultNotValid const& exc)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "The result of the path \"" + exc.getXPathStatement() + "\" is not valid !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void translateGbxExcIONotAvailable(GbxExcIONotAvailable const&)
{
  // Use the Python 'C' API to set up an exception object
  string excMessage = "The GearboxIO object is not available. Please make sure you have the Gearbox module in your path !";
  PyErr_SetString(PyExc_RuntimeError, excMessage.c_str());
}


void Gearbox::exposePythonAPI()
{
  //Register exception translators
  register_exception_translator<GbxExcPathNotValid>(&translateGbxExcPathNotValid);
  register_exception_translator<GbxExcPathEmptyResult>(&translateGbxExcPathEmptyResult);
  register_exception_translator<GbxExcPathResultNotValid>(&translateGbxExcPathResultNotValid);
  register_exception_translator<GbxExcIONotAvailable>(&translateGbxExcIONotAvailable);
}


//============================================================================
//                              Private methods
//============================================================================

bool Gearbox::close()
{
  bool result = true;
  if (m_gearboxIO != NULL) {
    if (m_gearboxIO->isOpen()) {
      result = m_gearboxIO->close();
    }

    if (result) {
      delete m_gearboxIO;
      m_gearboxIO = NULL;
    }
  }
  return result;
}


Gearbox::Gearbox()
{
  m_gearboxIO = NULL;
}


Gearbox::~Gearbox()
{
  close();
}
