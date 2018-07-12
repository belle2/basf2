#pragma once
/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*               Simon Wehle                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <TFile.h>

#include <string>

namespace Belle2 {


  /** This sinlge instance class takes track of all open Ntuple files.
   * The files are created and destroyed by this class.
   *
   */
  class VariablesToNtupleFileManager {

  public:
    /** Interface for the FileManager */
    static VariablesToNtupleFileManager& getInstance()
    {
      static VariablesToNtupleFileManager instance;
      return instance;
    }
    /**
     * Get a file with a specific name, if is does not exist it will be created
     */
    std::shared_ptr<TFile> getFile(std::string);

  private:
    /** Constructor is private. */
    VariablesToNtupleFileManager() {}
    // C++8
    //VariablesToNtupleFileManager(VariablesToNtupleFileManager const&);
    //void operator=(VariablesToNtupleFileManager const&);

    /** creat a new file*/
    std::shared_ptr<TFile> createRootFile(std::string);
    /** store for the open files */
    std::map<std::string, std::shared_ptr<TFile>> m_files;

  public:
    /** We so not want copy construction */
    VariablesToNtupleFileManager(VariablesToNtupleFileManager const&) = delete;
    /** We do not need this interface */
    void operator=(VariablesToNtupleFileManager const&) = delete;

  };
}
