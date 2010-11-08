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
    B2ERROR("Could not close the connection of the GearboxIO object !")
  }
}


GearboxIOAbs& Gearbox::getGearboxIO() const throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return *m_gearboxIO;
}


GearDir Gearbox::getContent(const std::string& paramSetType, EGearboxContentType contentType) throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();

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
throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->enableParamCheck(paramCheck);
}


bool Gearbox::isPathValid(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->isPathValid(path);
}


bool Gearbox::isParamAvailable(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->isParamAvailable(path);
}


int Gearbox::getNumberNodes(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxPathResultNotValidError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->getNumberNodes(path);
}


double Gearbox::getParamLength(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->getParamLength(path);
}


double Gearbox::getParamAngle(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->getParamAngle(path);
}


double Gearbox::getParamNumValue(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->getParamNumValue(path);
}


std::string Gearbox::getParamString(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  if ((m_gearboxIO == NULL) || (!m_gearboxIO->isOpen())) throw GearboxIOAbs::GearboxIONotConnectedError();
  return m_gearboxIO->getParamString(path);
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
