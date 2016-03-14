#include <calibration/DBObjCalibrationConstMapBase.h>

using namespace Belle2;

ClassImp(DBObjCalibrationConstMapBase);

void DBObjCalibrationConstMapBase::dump()
{
  for (auto& elementParam : m_constants) {
    std::cout << " Element " << elementParam.first.first << " Param " << elementParam.first.second << " Value = " << elementParam.second
              << std::endl;
  }
}

DBObjCalibrationConstMapBase::~DBObjCalibrationConstMapBase()
{
  m_constants.clear();
}

double DBObjCalibrationConstMapBase::get(DBObjCalibrationConstMapBase::baseType id, DBObjCalibrationConstMapBase::baseType param)
{

  auto constant = m_constants.find({id, param});
  if (constant == m_constants.end()) {
    return 0.;
  }
  return m_constants[ {id, param}];
}

void DBObjCalibrationConstMapBase::set(DBObjCalibrationConstMapBase::baseType id, DBObjCalibrationConstMapBase::baseType param,
                                       double value)
{
  m_constants[ {id, param}] = value;
}

double DBObjCalibrationConstMapBase::add(DBObjCalibrationConstMapBase::baseType id, DBObjCalibrationConstMapBase::baseType param,
                                         double value, bool subtractInsteadOfAdd)
{
  double original = get(id, param);
  double newValue = (subtractInsteadOfAdd) ? (original - value) : (original + value);
  set(id, param, newValue);
  return newValue;
}

void DBObjCalibrationConstMapBase::Streamer(TBuffer& R__b)
{
  if (R__b.IsReading()) {
    m_constants.clear();
    unsigned int map_size = 0;
    R__b >> map_size;
    for (unsigned int i = 0; i < map_size; ++i) {
      baseType element = 0;
      baseType param = 0;
      double value = 0.;
      R__b >> element >> param >> value;
      m_constants[ {element, param}] = value;
    }
  } else {
    unsigned int map_size = m_constants.size();
    R__b << map_size;
    for (auto& map_entry : m_constants) {
      R__b << map_entry.first.first << map_entry.first.second << map_entry.second;
    }
  }
}