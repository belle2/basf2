#include <alignment/dataobjects/MilleData.h>

#include <framework/utilities/FileSystem.h>
#include <TList.h>

using namespace std;
using namespace Belle2;

void MilleData::merge(const MilleData* other)
{
  auto* data = static_cast<const MilleData*>(other);
  const vector<string>& files = data->getFiles();
  m_numRecords += data->m_numRecords;
  for (auto& file : files) {
    bool exists = false;
    for (auto& myfile : m_files) {
      if (myfile == file) {
        exists = true;
        break;
      }
    }
    if (!exists)
      m_files.push_back(file);
  }
}

MilleData& MilleData::operator=(const MilleData& other)
{
  close();
  m_files = other.m_files;
  m_doublePrecision = other.m_doublePrecision;
  m_numRecords = other.m_numRecords;
  return *this;
}
void MilleData::close()
{
  if (m_binary) {
    delete m_binary;
    m_binary = nullptr;
  }
}

void MilleData::fill(gbl::GblTrajectory& trajectory)
{
  if (m_binary) {
    trajectory.milleOut(*m_binary);
    ++m_numRecords;
  }
}

void MilleData::open(string filename)
{
  if (m_binary) {
    close();
  }
  m_binary = new gbl::MilleBinary(filename, m_doublePrecision);
  m_files.push_back(filename);
}

Long64_t MilleData::Merge(TCollection* hlist)
{
  Long64_t nMerged = 0;
  if (hlist) {
    const MilleData* xh = 0;
    TIter nxh(hlist);
    while ((xh = dynamic_cast<MilleData*>(nxh()))) {
      // Add xh to me
      merge(xh);
      ++nMerged;
    }
  }
  return nMerged;
}
