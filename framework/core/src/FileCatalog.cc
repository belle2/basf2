/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2013  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
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
  const char* fileCatalog = getenv("BELLE2_FILECATALOG");
  if (fileCatalog && (strcmp(fileCatalog, "NONE") == 0)) return;

  // check for file catalog in home directory
  if (!fileCatalog) {
    const char* path = "~/Belle2FileCatalog.xml";
    if (fs::exists(path)) {
      fileCatalog = path;
    }
  }

  // use file catalog in current directory if nothing else found
  if (!fileCatalog) {
    fileCatalog = "Belle2FileCatalog.xml";
  }

  // get absolute path name
  m_fileName = fs::absolute(fileCatalog, fs::initial_path<fs::path>()).c_str();

  // check whether file exists and is readable, otherwise try to create it
  std::ifstream ifile(m_fileName.c_str());
  if (!ifile.good()) {
    B2DEBUG(100, "Creating file catalog " << m_fileName);
    FileSystem::Lock lock(m_fileName);
    if (!lock.lock()) {
      B2ERROR("Creation of file catalog " << m_fileName << " failed.");
      m_fileName = "";
    }
    std::ofstream ofile(m_fileName.c_str());
    if (!ofile.is_open()) {
      B2ERROR("Creation of file catalog " << m_fileName << " failed.");
      m_fileName = "";
    } else {
      ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      ofile << "<FileCatalog>\n";
      ofile << "</FileCatalog>\n";
    }
  }
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
  for (auto entry : fileMap) entry.second.second.write(file, entry.second.first);
  file << "</FileCatalog>\n";

  return true;
}


bool FileCatalog::registerFile(std::string fileName, FileMetaData& metaData)
{
  if (m_fileName.empty()) return false;

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

  // use absolute path as LFN if it is not set
  fileName = fs::absolute(fileName, fs::initial_path<fs::path>()).c_str();
  if (metaData.getLfn().empty()) {
    metaData.setLfn(fileName);
  }

  // check whether a file with this name is already registered
  for (auto entry : fileMap) {
    if (fileName.compare(entry.second.second.getLfn()) == 0) {
      B2WARNING("A file " << fileName << " is already registered and will be overwritten in the catalog.");
      fileMap.erase(entry.first);
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
  for (auto entry : fileMap) {
    if (fileName.compare(entry.second.first) == 0) {
      metaData = entry.second.second;
      return true;
    }
  }

  return false;
}


std::string FileCatalog::getPhysicalFileName(std::string lfn)
{
  std::string fileName = lfn;
  FileMetaData metaData;
  if (!getMetaData(fileName, metaData)) {
    B2DEBUG(100, "No LFN " << lfn << " found in the file catalog.");
  }
  return fileName;
}

