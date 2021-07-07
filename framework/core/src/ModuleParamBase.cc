/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/core/ModuleParamBase.h>

#include <string>
#include <utility>

using namespace Belle2 ;

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
