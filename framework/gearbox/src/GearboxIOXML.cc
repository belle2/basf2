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

#include <cmath>
#include <string>

#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <boost/lexical_cast.hpp>

#include <boost/format.hpp>

#include <TMath.h>

using namespace std;
using namespace Belle2;


GearboxIOXML::GearboxIOXML() : GearboxIOAbs()
{
  xmlInitParser();
  LIBXML_TEST_VERSION

  m_xmlDocument = NULL;
  m_enableParamCheck = true;
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

  //Speeds up XPath computation on static documents.
  xmlXPathOrderDocElems(m_xmlDocument);

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


bool GearboxIOXML::isPathValid(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) return false;

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) return false;

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    return false;
  }

  xmlXPathFreeObject(result);
  return true;
}


bool GearboxIOXML::isParamAvailable(const string& path) const
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


string GearboxIOXML::getNodeName(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isPathValid(path)) throw(GearboxPathNotValidError() << path);
  }

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << path);
  if (result->type != XPATH_NODESET) throw(GearboxPathResultNotValidError() << path);
  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << path);
  }

  xmlNodeSetPtr nodeSet = result->nodesetval;

  return (char*)(nodeSet->nodeTab[0]->name);
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

  boost::format queryString("count(%1%)");
  queryString % path;

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)queryString.str().c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << queryString.str());
  if (result->type != XPATH_NUMBER) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << queryString.str());
  }

  int numberNodes = (int)result->floatval;
  xmlXPathFreeObject(result);

  return numberNodes;
}


double GearboxIOXML::getParamLength(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }
  return getDoubleWithUnit(path, Unit::c_UnitLength);
}


double GearboxIOXML::getParamAngle(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }
  return getDoubleWithUnit(path, Unit::c_UnitAngle);
}


double GearboxIOXML::getParamEnergy(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }
  return getDoubleWithUnit(path, Unit::c_UnitEnergy);
}


double GearboxIOXML::getParamDensity(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }
  return getDoubleWithUnit(path, Unit::c_UnitDensity);
}


double GearboxIOXML::getParamNumValue(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxPathEmptyResultError, GearboxIOAbs::GearboxParamNotExistsError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << path);
  if (result->type != XPATH_NODESET) throw(GearboxPathResultNotValidError() << path);
  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << path);
  }

  xmlNodeSetPtr nodeSet = result->nodesetval;

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


string GearboxIOXML::getParamString(const string& path) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError,
      GearboxIOAbs::GearboxParamNotExistsError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (path.empty()) throw(GearboxPathNotValidError() << "");

  if (m_enableParamCheck) {
    if (!isParamAvailable(path)) throw(GearboxParamNotExistsError() << path);
  }

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)path.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << path);

  if (result->type != XPATH_NODESET) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << path);
  }

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << path);
  }

  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlChar *resultChar = xmlNodeListGetString(m_xmlDocument, nodeSet->nodeTab[0]->xmlChildrenNode, 1);

  xmlXPathFreeObject(result);
  return (char*)resultChar;
}


//============================================================================
//                              Private methods
//============================================================================

xmlXPathObjectPtr GearboxIOXML::getNodeSet(xmlDocPtr document, xmlChar *xpath) const
{
  xmlXPathContextPtr context = xmlXPathNewContext(document);
  if (context == NULL) return NULL;

  xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
  xmlXPathFreeContext(context);
  if (result == NULL) return NULL;
  return result;
}


double GearboxIOXML::getDoubleWithUnit(const string& xpath, Unit::EUnitTypes unitType) const
throw(GearboxIOAbs::GearboxIONotConnectedError, GearboxIOAbs::GearboxPathNotValidError, GearboxIOAbs::GearboxPathEmptyResultError,
      GearboxIOAbs::GearboxPathResultNotValidError, GearboxIOAbs::GearboxStringNumConversionError)
{
  if (!isOpen()) throw GearboxIONotConnectedError();
  if (xpath.empty()) throw(GearboxPathNotValidError() << "");

  xmlXPathObjectPtr result = getNodeSet(m_xmlDocument, (xmlChar*)xpath.c_str());

  if (result == NULL) throw(GearboxPathEmptyResultError() << xpath);

  if (result->type != XPATH_NODESET) {
    xmlXPathFreeObject(result);
    throw(GearboxPathResultNotValidError() << xpath);
  }

  if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    throw(GearboxPathEmptyResultError() << xpath);
  }

  xmlNodeSetPtr nodeSet = result->nodesetval;

  //--- Get the value ---
  xmlChar *valueChar = xmlNodeListGetString(m_xmlDocument, nodeSet->nodeTab[0]->xmlChildrenNode, 1);
  double value = 0.0;

  try {
    value = boost::lexical_cast<double>((char*)valueChar);
  } catch (boost::bad_lexical_cast &) {
    xmlXPathFreeObject(result);
    throw(GearboxStringNumConversionError() << string((char*)valueChar));
  }

  //--- Get the unit ---
  xmlAttrPtr attribute = nodeSet->nodeTab[0]->properties;
  bool unitFound = false;

  while ((!unitFound) && (attribute != NULL)) {
    unitFound = !xmlStrcmp(attribute->name, (const xmlChar*)"unit");
    if (!unitFound) attribute = attribute->next;
  }

  if (unitFound) {
    string unitString = (char*)attribute->children->content;
    value = Unit::convertValue(value, unitType, unitString);
  }

  xmlXPathFreeObject(result);
  return value;
}
