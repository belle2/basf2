/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/core/ModuleParamBase.h>

#include <string>

using namespace Belle2 ;

ModuleParamBase::ModuleParamBase(const std::string& typeInfo,
                                 const std::string& description,
                                 bool force)
  : m_typeInfo(typeInfo)
  , m_description(description)
  , m_forceInSteering(force)
  , m_setInSteering(false)
{
}

ModuleParamBase::~ModuleParamBase() = default;

const std::string& ModuleParamBase::getTypeInfo() const
{
  return m_typeInfo;
}

const std::string& ModuleParamBase::getDescription() const
{
  return m_description;
}

bool ModuleParamBase::isSetInSteering() const
{
  return m_setInSteering;
}

bool ModuleParamBase::isForcedInSteering() const
{
  return m_forceInSteering;
}
