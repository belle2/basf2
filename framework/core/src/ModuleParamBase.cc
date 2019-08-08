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
#include <utility>

using namespace Belle2 ;

// cppcheck-suppress passedByValue ; We take a value to move it into a member so no performance penalty
ModuleParamBase::ModuleParamBase(std::string  typeInfo, std::string  description, bool force)
  : m_typeInfo(std::move(typeInfo))
  , m_description(std::move(description))
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
