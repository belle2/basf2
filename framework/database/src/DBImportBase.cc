/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportBase.h>
#include <framework/database/EventDependency.h>
#include <framework/database/Database.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void DBImportBase::addIntraRunDependency(unsigned long long int tag,
                                         EIntraRunDependency dependency)
{

  if (!m_object) return;

  if (m_dependency == c_None) m_dependency = dependency;
  if (dependency == m_dependency) {
    m_objects.push_back(m_object);
    m_tags.push_back(tag);
    m_object = nullptr;
  } else {
    B2FATAL("DBImportBase::addIntraRunDependency: " <<
            "intra run dependency cannot be of mixed types");
  }

}


bool DBImportBase::import(const IntervalOfValidity& iov)
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
      return Database::Instance().storeData(m_name, m_object, iov);
  }

}


void DBImportBase::clear()
{
  if (m_object) {
    delete m_object;
    m_object = nullptr;
  }

  m_dependency = c_None;
  for (auto& object : m_objects) delete object;
  m_objects.clear();
  m_tags.clear();
}

bool DBImportBase::storeData(TObject* intraRun, const IntervalOfValidity& iov)
{
  return Database::Instance().storeData(m_name, intraRun, iov);
}

DBImportBase::~DBImportBase()
{
  for (auto& object : m_objects) delete object;
}
