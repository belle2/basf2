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
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GearReader.h>

#include <boost/format.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;


GearDir::GearDir(const string& path) : GearboxIOAbs(), m_dirPath(path)
{

}


GearDir::GearDir(GearDir& gearDir, const string& path) : GearboxIOAbs()
{
  m_dirPath = gearDir.getDirPath() + path;
}


GearDir::GearDir(GearDir& gearDir, int index) : GearboxIOAbs()
{
  append(gearDir.getDirPath(), index);
}


GearDir::~GearDir()
{

}


bool GearDir::close()
{
  return false;
}


bool GearDir::isOpen() const
{
  return Gearbox::Instance().getGearboxIO().isOpen();
}


void GearDir::enableParamCheck(bool paramCheck)
{
  Gearbox::Instance().getGearboxIO().enableParamCheck(paramCheck);
}


bool GearDir::isPathValid(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  return Gearbox::Instance().getGearboxIO().isPathValid(m_dirPath + path);
}


bool GearDir::isParamAvailable(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError)
{
  return Gearbox::Instance().getGearboxIO().isParamAvailable(m_dirPath + path);
}


string GearDir::getNodeName(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  return Gearbox::Instance().getGearboxIO().getNodeName(m_dirPath + path);
}


int GearDir::getNumberNodes(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxPathResultNotValidError)
{
  return Gearbox::Instance().getGearboxIO().getNumberNodes(m_dirPath + path);
}


double GearDir::getParamLength(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  return Gearbox::Instance().getGearboxIO().getParamLength(m_dirPath + path);
}


double GearDir::getParamAngle(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  return Gearbox::Instance().getGearboxIO().getParamAngle(m_dirPath + path);
}


double GearDir::getParamEnergy(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  return Gearbox::Instance().getGearboxIO().getParamEnergy(m_dirPath + path);
}


double GearDir::getParamDensity(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  return Gearbox::Instance().getGearboxIO().getParamDensity(m_dirPath + path);
}


double GearDir::getParamNumValue(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  return Gearbox::Instance().getGearboxIO().getParamNumValue(m_dirPath + path);
}


string GearDir::getParamString(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  return Gearbox::Instance().getGearboxIO().getParamString(m_dirPath + path);
}


TGeoMaterial* GearDir::readMaterial(const string& path)
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  GearDir materialDir(*this, path);
  return GearReader::readMaterial(materialDir);
}


void GearDir::setDirPath(const string& path)
{
  m_dirPath = path;
}


void GearDir::append(const string& path)
{
  m_dirPath += path;
}


void GearDir::append(const string& path, int index)
{
  //If there is a trailing '/' use the string except the last character
  if (path[path.length()-1] == '/') {
    m_dirPath += (path.substr(0, path.length() - 1) + (format("[%1%]/") % (index)).str());
  } else m_dirPath += (path + (format("[%1%]/") % (index)).str());
}


void GearDir::append(GearDir& gearDir)
{
  m_dirPath += gearDir.getDirPath();
}
