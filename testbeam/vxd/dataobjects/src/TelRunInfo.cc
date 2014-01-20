#include <testbeam/vxd/dataobjects/TelRunInfo.h>

using namespace Belle2;

ClassImp(TelRunInfo)

TelRunInfo::TelRunInfo(ULong64_t run /* = 0 */,
                       ULong64_t numEvents /* = 0 */,
                       const std::string& fileName /* = "" */,
                       const std::string& detectorName /* = "" */)
  : m_run(run),
    m_numEvents(numEvents),
    m_fileName(fileName),
    m_detectorName(detectorName)
{

}

bool TelRunInfo::operator==(const TelRunInfo& telRunInfo) const
{
  return ((m_run == telRunInfo.getRunNumber()) &&
          (m_numEvents == telRunInfo.getNumEvents()) &&
          (m_fileName == telRunInfo.getFileName()) &&
          (m_detectorName == telRunInfo.getDetectorName()));
}

bool TelRunInfo::operator!=(const TelRunInfo& telRunInfo) const
{
  return !(* this == telRunInfo);
}
