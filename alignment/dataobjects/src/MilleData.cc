#include <alignment/dataobjects/MilleData.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <limits.h>
#include <stdlib.h>

char* full_path = realpath("foo.dat", NULL);
using namespace std;
using namespace Belle2;

void MilleData::merge(const MergeableNamed* other)
{
  auto* data = dynamic_cast<const MilleData*>(other);
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

MilleData* MilleData::Clone(const char* newname) const
{
  MilleData* obj = new MilleData(*this);
  if (newname && strlen(newname)) {
    obj->SetName(newname);
  }
  return obj;
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
  if (m_absFilePaths)
    m_files.push_back(FileSystem::findFile(string(realpath(filename.c_str(), NULL))));
  else
    m_files.push_back(filename);

}
