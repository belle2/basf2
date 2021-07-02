/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

/************************SETTER***********************/

void FlavorTaggerInfo::addMethodMap(const std::string& method)
{
  StoreArray<FlavorTaggerInfoMap> flavTagInfoMap;
  if (m_methodMap.find(method) == m_methodMap.end()) {
    // create FlavorTaggerInfoMap object

    FlavorTaggerInfoMap* flavTagMap = flavTagInfoMap.appendNew();
    m_methodMap.insert(std::pair<std::string, FlavorTaggerInfoMap*>(method, flavTagMap));

  } else {
    B2FATAL("Method  " << method << " already added. Please use another name.");
  }
}

/************************GETTER***********************/

FlavorTaggerInfoMap* FlavorTaggerInfo::getMethodMap(const std::string& method) const
{
  auto it = m_methodMap.find(method);
  if (it == m_methodMap.end()) {
    B2FATAL("Method  " << method << " does not exist in the FlavorTaggerInfoMap. Please add it before to combinerMethods.");
  } else {
    return it->second;
  }
}
