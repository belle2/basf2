/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLDBTool                                                              *
 *                                                                        *
 * Utility designed to read / write object from / to database.            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// FRAMEWORK
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
// ECL
#include <ecl/utility/ECLDBTool.h>
using namespace Belle2;
// Constructor.
ECLDBTool::ECLDBTool(bool isLocal,
                     const char* dbName,
                     const char* payloadName):
  m_isLocal(isLocal),
  m_dbName(dbName),
  m_payloadName(payloadName)
{
}
// Destructor.
ECLDBTool::~ECLDBTool()
{
}
// Connect to a database.
void ECLDBTool::connect() const
{
  if (m_isLocal) {
    LocalDatabase::reset();
    LocalDatabase::createInstance(m_dbName.c_str());
  } else {
    ConditionsDatabase::reset();
    ConditionsDatabase::createDefaultInstance(m_dbName.c_str());
  }
}
// Write object and validity interval to a database.
void ECLDBTool::write(TObject* const obj,
                      const IntervalOfValidity& iov) const
{
  Database::Instance().storeData(m_payloadName.c_str(),
                                 obj, iov);
}
// Read object and validity interval from a database.
void ECLDBTool::read(TObject** obj,
                     IntervalOfValidity** iov,
                     const EventMetaData& event) const
{
  auto data = Database::Instance().
              getData(event, m_payloadName.c_str());
  *obj = std::get<0>(data);
  *iov = new IntervalOfValidity(std::get<1>(data));
}
// Read just validity interval from a database.
void ECLDBTool::read(IntervalOfValidity** iov,
                     const EventMetaData& event) const
{
  auto data = Database::Instance().
              getData(event, m_payloadName.c_str());
  auto obj = std::get<0>(data);
  *iov = new IntervalOfValidity(std::get<1>(data));
  delete obj;
}
// Change validity interval (Make copy of current
// payload with other validity interval).
void ECLDBTool::changeIoV(const EventMetaData& event,
                          const IntervalOfValidity& iov) const
{
  auto data = Database::Instance().
              getData(event, m_payloadName.c_str());
  auto obj = std::get<0>(data);
  write(obj, iov);
  delete obj;
}
