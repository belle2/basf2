/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportBase.h>
#include <framework/database/EventDependency.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void DBImportBase::addIntraRunDependency(unsigned long long int tag,
                                         EIntraRunDependency dependency)
{

  if (!m_object) return;

  if (dependency == c_None) m_dependency = dependency;
  if (dependency == m_dependency) {
    m_objects.push_back(m_object);
    m_tags.push_back(tag);
    m_object = 0;
  } else {
    B2FATAL("DBImportBase::addIntraRunDependency: " <<
            "intra run dependency cannot be of mixed types");
  }

}


bool DBImportBase::import(IntervalOfValidity& iov)
{

  switch (m_dependency) {
    case c_Event:
      return import<EventDependency>(iov);
    case c_TimeStamp:
      B2ERROR("DBImportBase::import: " <<
              "intra run dependency of type 'time stamp' not supported yet");
      return false;
    case c_Subrun:
      B2ERROR("DBImportBase::import: " <<
              "intra run dependency of type 'subrun' not supported yet");
      return false;
    default:
      if (!m_object) return false;
      return Database::Instance().storeData(m_package, m_module, m_object, iov);
  }

}


void DBImportBase::clear()
{
  if (m_object) {
    delete m_object;
    m_object = 0;
  }

  m_dependency = c_None;
  for (auto& object : m_objects) delete object;
  m_objects.clear();
  m_tags.clear();
}

