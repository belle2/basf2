/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/HistModule.h>
#include <framework/pcore/ProcHandler.h>

#include <TFile.h>
#include <TFileMerger.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include <sstream>


namespace Belle2 {

  std::vector<std::string> HistModule::m_filenames;

  void HistModule::setFilename(std::string filename)
  {
    m_filenames.push_back(filename);
    m_filename = filename;
  }

  void HistModule::saveHists()
  {

    TFile* rootFile = nullptr;
    int procid = ProcHandler::EvtProcID();

    if (procid >= 0) {
      std::stringstream ss;
      ss << m_filename << "." << procid;
      rootFile = new TFile(ss.str().c_str(), "update");
    } else {
      rootFile = new TFile(m_filename.c_str(), "recreate");
    }

    if (rootFile != nullptr) {
      rootFile->cd();
      this->writeHists();
      rootFile->Write();
      rootFile->Close();
      delete rootFile;
    }

  }

  void HistModule::mergeFiles()
  {

    B2INFO("Begin merge inside");

    for (auto filename : m_filenames) {

      B2INFO("Merge all files with filename " << filename << ".*")

      // Set up merger with output file
      TFileMerger merger(false, false);
      if (! merger.OutputFile(filename.c_str())) {
        B2ERROR("HistModule: Error while open output file " << filename);
        return;
      }

      // Open current directory
      std::string dir = ".";
      DIR* dp;
      struct dirent* dirp;
      if ((dp = opendir(dir.c_str())) == NULL) {
        B2ERROR("HistModule: Error while open directory" << dir);
        return;
      }

      std::vector<std::string> pid_filenames;
      // Scan the directory and register all histogram files
      std::string compfile = filename + ".";
      while ((dirp = readdir(dp)) != NULL) {
        std::string curfile = std::string(dirp->d_name);
        if (curfile.compare(0, compfile.size(), compfile) == 0) {
          merger.AddFile(curfile.c_str());
          pid_filenames.push_back(curfile);
        }
      }
      closedir(dp);

      // Do Merge
      if (!merger.Merge()) {
        B2ERROR("HistModule: Error while merge files");
        return;
      }

      // Delete temporary files
      for (auto pid_filename : pid_filenames) {
        B2INFO("Unlink " << pid_filename)
        unlink(pid_filename.c_str());
      }
    }
  }
}
