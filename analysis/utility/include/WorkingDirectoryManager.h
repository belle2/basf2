/*
 * WorkingDirectoryManager.h
 *
 *  Created on: May 19, 2015
 *      Author: tkeck
 */

#pragma once

#include <TSystem.h>

class WorkingDirectoryManager {
public:
  /**
   * Change ROOT working directory to given directory
   * @param workingDirectory
   */
  explicit WorkingDirectoryManager(std::string workingDirectory) : oldDirectory(gSystem->WorkingDirectory())
  {
    gSystem->ChangeDirectory(workingDirectory.c_str());
  }

  /**
   * Change ROOT working directory back to old working directory
   */
  ~WorkingDirectoryManager()
  {
    gSystem->ChangeDirectory(oldDirectory.c_str());
  }

private:
  std::string oldDirectory; /**< Old working directory */
};

