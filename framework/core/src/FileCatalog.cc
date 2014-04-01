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
#include <framework/logging/Logger.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <chrono>
#include <random>
#include <sys/file.h>


using namespace Belle2;
namespace fs = boost::filesystem;


FileCatalog::Lock::Lock(std::string fileName)
{
  m_file = open(fileName.c_str(), O_RDWR | O_CREAT);
}

FileCatalog::Lock::~Lock()
{
  if (m_file >= 0) close(m_file);
}

bool FileCatalog::Lock::lock(int timeout)
{
  if (m_file < 0) return false;

  auto const maxtime = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
  std::default_random_engine random;
  std::uniform_int_distribution<int> uniform(1, 100);

  while (std::chrono::steady_clock::now() < maxtime) {
    int lock = flock(m_file, LOCK_EX | LOCK_NB);
    if (lock == 0) return true;
    auto next = std::chrono::steady_clock::now() + std::chrono::milliseconds(uniform(random));
    while (std::chrono::steady_clock::now() < next);
  }

  return false;
}


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
    B2INFO("Creating file catalog " << m_fileName);
    Lock lock(m_fileName);
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

  FileMetaData entry;

  while (!file.eof()) {
    file >> entry;
    if (entry.getId() != 0) fileMap.insert(FileMap::value_type(entry.getId(), entry));
  }

  return true;
}


bool FileCatalog::writeCatalog(const FileCatalog::FileMap& fileMap)
{
  std::ofstream file(m_fileName.c_str());
  if (!file.is_open()) return false;

  file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  file << "<FileCatalog>\n";
  for (auto entry : fileMap) file << entry.second;
  file << "</FileCatalog>\n";

  return true;
}


bool FileCatalog::registerFile(std::string fileName, FileMetaData& metaData)
{
  if (m_fileName.empty()) return false;

  // get lock for write access to file catalog
  Lock lock(m_fileName);
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

  // use absolute path and next negative integer number as ID for the meta data
  fileName = fs::absolute(fileName, fs::initial_path<fs::path>()).c_str();
  int id = fileMap.begin()->first - 1;
  if (metaData.getId() <= 0) {
    metaData.setIds(id, "", fileName);
  } else {
    metaData.setIds(metaData.getId(), metaData.getGuid(), fileName);
  }

  // check whether a file with this name is already registered
  for (auto entry : fileMap) {
    if (fileName.compare(entry.second.getLfn()) == 0) {
      if (metaData.getId() <= 0) B2WARNING("A file " << fileName << " is already registered in with ID " << entry.first << " and will be overwritten in the catalog.");
      fileMap.erase(entry.first);
      break;
    }
  }

  // add the new entry and write the file catalog
  fileMap.insert(FileMap::value_type(id, metaData));
  if (!writeCatalog(fileMap)) {
    B2ERROR("Failed to write file catalog " << m_fileName);
    return false;
  }

  return true;
}


bool FileCatalog::getMetaData(int id, FileMetaData& metaData)
{
  new(&metaData) FileMetaData;
  if (m_fileName.empty()) return false;

  // get lock for read access to file catalog
  Lock lock(m_fileName);
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

  // find the entry with given ID
  auto iterator = fileMap.find(id);
  if (iterator == fileMap.end()) return false;
  metaData = iterator->second;

  return true;
}


bool FileCatalog::getMetaData(std::string lfn, FileMetaData& metaData)
{
  new(&metaData) FileMetaData;
  if (m_fileName.empty()) return false;

  // get lock for read access to file catalog
  Lock lock(m_fileName);
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
  for (auto entry : fileMap) {
    if (lfn.compare(entry.second.getLfn()) == 0) {
      metaData = entry.second;
      return true;
    }
  }

  return false;
}


bool FileCatalog::getParents(const FileMap& fileMap, int level, const FileMetaData& metaData, ParentMetaData& parentMetaData)
{
  // loop over parents and get their meatdata from the catalog
  for (int iParent = 0; iParent < metaData.getNParents(); iParent++) {
    int parentID = metaData.getParent(iParent);
    auto iterator = fileMap.find(parentID);
    if (iterator == fileMap.end()) {
      B2ERROR("Parent file with ID " << parentID << " not found in the file catalog.");
      return false;
    }

    // fill the parent meta data information and recursively call getParents to get the grandparents meatdata
    parentMetaData.resize(parentMetaData.size() + 1);
    ParentMetaDataEntry& entry = parentMetaData.back();
    entry.metaData = iterator->second;
    if (level > 0) {
      if (!getParents(fileMap, level - 1, iterator->second, entry.parents)) return false;
    }
  }

  return true;
}

bool FileCatalog::getParentMetaData(int level, int id, FileMetaData& metaData, ParentMetaData& parentMetaData)
{
  parentMetaData.resize(0);
  if (level == 0) return true;

  // get lock for read access to file catalog
  Lock lock(m_fileName);
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

  // if no metadata given search for entry in catalog
  if ((metaData.getNParents() == 0) && (metaData.getId() == 0)) {
    auto iterator = fileMap.find(id);
    if (iterator == fileMap.end()) return false;
    metaData = iterator->second;
  }

  // now get the parent metadata information
  return getParents(fileMap, level, metaData, parentMetaData);
}
