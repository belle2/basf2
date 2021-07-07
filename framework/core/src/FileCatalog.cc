/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/EnvironmentVariables.h>
#include <boost/filesystem.hpp>
#include <fstream>


using namespace Belle2;
namespace fs = boost::filesystem;



FileCatalog& FileCatalog::Instance()
{
  static FileCatalog instance;
  return instance;
}


FileCatalog::FileCatalog() : m_fileName("")
{
  // check for BELLE2_FILECATALOG environment variable
  std::string fileCatalog = EnvironmentVariables::get("BELLE2_FILECATALOG", "");
  if (fileCatalog == "NONE") return;

  // check for file catalog in home directory
  if (fileCatalog.empty()) {
    const std::string path{"~/Belle2FileCatalog.xml"};
    if (fs::exists(path)) {
      fileCatalog = path;
    }
  }

  // use file catalog in current directory if nothing else found
  if (fileCatalog.empty()) {
    fileCatalog = "Belle2FileCatalog.xml";
  }

  // get absolute path name
  m_fileName = fs::absolute(fileCatalog, fs::initial_path<fs::path>()).c_str();
}


bool FileCatalog::readCatalog(FileCatalog::FileMap& fileMap)
{
  fileMap.clear();

  std::ifstream file(m_fileName.c_str());
  if (!file.is_open()) return false;

  try {
    while (!file.eof()) {
      FileMetaData entry;
      std::string physicalFileName;
      if (entry.read(file, physicalFileName)) fileMap[entry.getLfn()] = std::make_pair(physicalFileName, entry);
    }
  } catch (std::exception& e) {
    B2ERROR("Errors occured while reading " << m_fileName <<
            ", maybe it is corrupted? Note that your .root files should be unaffected. (Error details: " << e.what() << ")");
    return false;
  }

  return true;
}


bool FileCatalog::writeCatalog(const FileCatalog::FileMap& fileMap)
{
  std::ofstream file(m_fileName.c_str());
  if (!file.is_open()) return false;

  file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  file << "<FileCatalog>\n";
  for (const auto& entry : fileMap) entry.second.second.write(file, entry.second.first);
  file << "</FileCatalog>\n";

  return true;
}

bool FileCatalog::registerFile(const std::string& fileName, FileMetaData& metaData, const std::string& oldLFN)
{
  if (m_fileName.empty()) return false;

  // make sure we have an LFN
  if (metaData.getLfn().empty()) {
    B2ERROR("Cannot register a file without a valid LFN");
    return false;
  }

  // get lock for write access to file catalog
  FileSystem::Lock lock(m_fileName);
  if (!lock.lock()) {
    B2ERROR("Locking of file catalog " << m_fileName << " failed.");
    return false;
  }

  // read the file catalog
  FileMap fileMap;
  if (!readCatalog(fileMap)) {
    B2ERROR("Failed to read file catalog " << m_fileName);
    return false;
  }

  // check whether a file with this name is already registered and remove if so
  for (auto it = fileMap.begin(); it != fileMap.end(); ++it) {
    auto&& [lfn, value] = *it;
    if (!oldLFN.empty() and oldLFN == lfn) {
      // old LFN exists and we requested an update so no warning, just remove
      fileMap.erase(it);
      break;
    }
    if (metaData.getLfn() == lfn) {
      B2WARNING("A file with the same LFN is already registered and will be overwritten in the catalog."
                << LogVar("LFN", lfn) << LogVar("old PFN", value.first) << LogVar("new PFN", fileName));
      fileMap.erase(it);
      break;
    }
  }

  // add the new entry and write the file catalog
  fileMap[metaData.getLfn()] = std::make_pair(fileName, metaData);

  if (!writeCatalog(fileMap)) {
    B2ERROR("Failed to write file catalog " << m_fileName);
    return false;
  }

  return true;
}


bool FileCatalog::getMetaData(std::string& fileName, FileMetaData& metaData)
{
  metaData = FileMetaData();
  if (m_fileName.empty()) return false;
  if (!fs::exists(m_fileName)) return false;

  // get lock for read access to file catalog
  FileSystem::Lock lock(m_fileName, true);
  if (!lock.lock()) {
    B2ERROR("Locking of file catalog " << m_fileName << " failed.");
    return false;
  }

  // read the file catalog
  FileMap fileMap;
  if (!readCatalog(fileMap)) {
    B2ERROR("Failed to read file catalog " << m_fileName);
    return false;
  }

  // find the entry with given LFN
  auto iEntry = fileMap.find(fileName);
  if (iEntry != fileMap.end()) {
    metaData = iEntry->second.second;
    if (!iEntry->second.first.empty()) fileName = iEntry->second.first;
    return true;
  }
  for (const auto& entry : fileMap) {
    if (fileName.compare(entry.second.first) == 0) {
      metaData = entry.second.second;
      return true;
    }
  }

  return false;
}


std::string FileCatalog::getPhysicalFileName(const std::string& lfn)
{
  std::string fileName = lfn;
  FileMetaData metaData;
  if (!getMetaData(fileName, metaData)) {
    B2DEBUG(100, "No LFN " << lfn << " found in the file catalog.");
  }
  return fileName;
}
