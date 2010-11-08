/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/filesystem.hpp>

#include <framework/core/ModuleUtils.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;
using namespace boost;

//-------------------------------------------------------
//          filepath related methods
//-------------------------------------------------------

bool ModuleUtils::fileNameExists(const std::string& filename)
{
  try {
    boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(filename));
    return boost::filesystem::exists(fullPath);
  } catch (...) {
    B2ERROR("Could not check for the existence of the path: " + filename + " !");
  }
  return false;
}


bool ModuleUtils::filePathExists(const std::string& filepath)
{
  try {
    boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(filepath));
    fullPath.remove_filename();
    return boost::filesystem::exists(fullPath);
  } catch (...) {
    B2ERROR("Could not check for the existence of the path: " + filepath + " !");
  }
  return false;
}


bool ModuleUtils::isFile(const std::string& filename)
{
  try {
    boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(filename));
    return (boost::filesystem::exists(fullPath)) && (boost::filesystem::is_regular_file(fullPath));
  } catch (...) {
    B2ERROR("Could not check if the filename " + filename + " is a file !");
  }
  return false;
}


bool ModuleUtils::isDirectory(const std::string& filename)
{
  try {
    boost::filesystem::path fullPath(boost::filesystem::initial_path<boost::filesystem::path>());
    fullPath = boost::filesystem::system_complete(boost::filesystem::path(filename));
    return (boost::filesystem::exists(fullPath)) && (boost::filesystem::is_directory(fullPath));
  } catch (...) {
    B2ERROR("Could not check if the filename " + filename + " is a directory !");
  }
  return false;
}
