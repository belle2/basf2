/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearboxIOXML.h>
#include <framework/datastore/Units.h>

#include <cmath>
#include <string>

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <boost/lexical_cast.hpp>

#include <boost/format.hpp>

#include <TMath.h>

using namespace std;
using namespace Belle2;


GearboxIOXML::GearboxIOXML()
{
  m_xmlDocument = NULL;
  m_enableParamCheck = true;

  setLengthUnitMap();
  setAngleUnitMap();
}


GearboxIOXML::~GearboxIOXML()
{
  close();
}


bool GearboxIOXML::open(const string& filename)
{
  if (filename.empty()) return false;

  m_xmlDocument = xmlParseFile(filename.c_str());
  xmlXIncludeProcess(m_xmlDocument);

  if (m_xmlDocument == NULL) return false;
  else return true;
}


bool GearboxIOXML::close()
{
  if (m_xmlDocument != NULL) {
    xmlFreeDoc(m_xmlDocument);
    xmlCleanupParser();
    m_xmlDocument = NULL;
    return true;
  } else return false;
}


bool GearboxIOXML::isOpen() const
{
  return (m_xmlDocument != NULL);
}


bool GearboxIOXML::isPathValid(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) return false;

  xmlXPathObjectPtr result;
  result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) return false;

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    return false;
  }

  xmlXPathFreeObject(result);
  return true;
}


bool GearboxIOXML::isParamAvailable(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  //1. Check if the path to the parameter exists, by
  //   evaluating the whole path except the last node
  string pathOnly = path.substr(0, path.find_last_of("/"));
  if (!isPathValid(pathOnly)) throw(GearboxPathNotValidError() << path);

  //2. Check if the parameter exists
  return isPathValid(path);
}


int GearboxIOXML::getNumberNodes(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxPathResultNotValidError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isPathValid(path)) throw(GearboxPathNotValidError() << path);
  }

  xmlXPathObjectPtr result;

  boost::format queryString("count(%1%)");
  queryString % path;

  result = getNodeSet(m_xmlDocument, (xmlChar*)queryString.str().c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << queryString.str());
  if (result->type != XPATH_NUMBER) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << queryString.str());
  }

  int numberNodes = (int)result->floatval;
  xmlXPathFreeObject(result);

  return numberNodes;
}


double GearboxIOXML::getParamLength(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  double resultLength;
  int currentUnitType;

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  getDoubleWithUnit(resultLength, currentUnitType, path, c_CM, m_lengthUnitMap);

  //--- Convert length value ---
  switch (currentUnitType) {
    case c_UM:
      resultLength *= um;
      break;
    case c_MM:
      resultLength *= mm;
      break;
    case c_CM:
      resultLength *= cm;
      break;
    case c_M:
      resultLength *= m;
      break;
    case c_KM:
      resultLength *= km;
      break;
  }
  return resultLength;
}


double GearboxIOXML::getParamAngle(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  double resultAngle;
  int currentUnitType;

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  getDoubleWithUnit(resultAngle, currentUnitType, path, c_Rad, m_angleUnitMap);

  //--- Convert angle value ---
  switch (currentUnitType) {
    case c_Deg:
      resultAngle *= deg;
      break;
    case c_Rad:
      break;
    case c_MRad:
      resultAngle *= mrad;
      break;
  }
  return resultAngle;
}


double GearboxIOXML::getParamNumValue(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  xmlXPathObjectPtr result;
  result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << path);
  if (result->type != XPATH_NODESET) throw(GearboxPathResultNotValidError() << path);
  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << path);
  }

  xmlNodeSetPtr nodeSet;
  nodeSet = result->nodesetval;

  xmlChar *resultChar = xmlNodeListGetString(m_xmlDocument, nodeSet->nodeTab[0]->xmlChildrenNode, 1);
  double resultValue = 0.0;

  try {
    resultValue = boost::lexical_cast<double>((char*)resultChar);
  } catch (boost::bad_lexical_cast &) {
    xmlXPathFreeObject(result);
    throw(GearboxStringNumConversionError() << string((char*)resultChar));
  }

  xmlXPathFreeObject(result);
  return resultValue;
}


std::string GearboxIOXML::getParamString(const std::string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  xmlXPathObjectPtr result;
  result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << path);

  if (result->type != XPATH_NODESET) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << path);
  }

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << path);
  }

  xmlNodeSetPtr nodeSet;
  nodeSet = result->nodesetval;

  xmlChar *resultChar = xmlNodeListGetString(m_xmlDocument, nodeSet->nodeTab[0]->xmlChildrenNode, 1);

  xmlXPathFreeObject(result);
  return (char*)resultChar;
}


//============================================================================
//                              Private methods
//============================================================================

xmlXPathObjectPtr GearboxIOXML::getNodeSet(xmlDocPtr document, xmlChar *xpath) const
{
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;
  context = xmlXPathNewContext(document);
  if (context == NULL) return NULL;

  result = xmlXPathEvalExpression(xpath, context);
  xmlXPathFreeContext(context);
  if (result == NULL) return NULL;
  return result;
}


void GearboxIOXML::getDoubleWithUnit(double& value, int& unit, const string& xpath,
                                     int defaultUnit, const std::map<std::string, int>& unitMap) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (xpath.empty()) throw(GearboxPathNotValidError() << "");

  xmlXPathObjectPtr result;
  result = getNodeSet(m_xmlDocument, (xmlChar*)xpath.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << xpath);

  if (result->type != XPATH_NODESET) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << xpath);
  }

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << xpath);
  }

  xmlNodeSetPtr nodeSet;
  nodeSet = result->nodesetval;

  //--- Get the unit ---
  xmlAttrPtr attribute = nodeSet->nodeTab[0]->properties;
  bool unitFound = false;

  while ((!unitFound) && (attribute != NULL)) {
    unitFound = !xmlStrcmp(attribute->name, (const xmlChar*)"unit");
    if (!unitFound) attribute = attribute->next;
  }

  if (unitFound) {
    string unitString = (char*)attribute->children->content;

    map<string, int>::const_iterator mapIter;
    mapIter = unitMap.find(unitString);
    if (mapIter != unitMap.end()) unit = mapIter->second;
    else unit = defaultUnit;
  } else unit = defaultUnit;

  //--- Get the value ---
  xmlChar *valueChar = xmlNodeListGetString(m_xmlDocument, nodeSet->nodeTab[0]->xmlChildrenNode, 1);
  value = 0.0;

  try {
    value = boost::lexical_cast<double>((char*)valueChar);
  } catch (boost::bad_lexical_cast &) {
    xmlXPathFreeObject(result);
    throw(GearboxStringNumConversionError() << string((char*)valueChar));
  }

  xmlXPathFreeObject(result);
}


void GearboxIOXML::setLengthUnitMap()
{
  m_lengthUnitMap.clear();
  m_lengthUnitMap.insert(make_pair("um", c_UM)); //Micrometer
  m_lengthUnitMap.insert(make_pair("mm", c_MM)); //Millimeter
  m_lengthUnitMap.insert(make_pair("cm", c_CM)); //Centimeter
  m_lengthUnitMap.insert(make_pair("m", c_M));   //Meter
  m_lengthUnitMap.insert(make_pair("km", c_KM)); //Kilometer
}


void GearboxIOXML::setAngleUnitMap()
{
  m_angleUnitMap.clear();
  m_angleUnitMap.insert(make_pair("deg", c_Deg));  //Degree
  m_angleUnitMap.insert(make_pair("rad", c_Rad));  //Radian
  m_angleUnitMap.insert(make_pair("mrad", c_MRad)); //milliradian
}
