/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/ParamBase.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ParamBase::ParamBase(const std::string& name, const std::string& description, bool forced)
  : m_name(name)
  , m_description(description)
  , m_forced(forced)
{
}

ParamBase::~ParamBase() = default;

const std::string&
ParamBase::getName() const
{
  return m_name;
}

const std::string&
ParamBase::getDescription() const
{
  return m_description;
}

bool
ParamBase::isForced() const
{
  return m_forced;
}
