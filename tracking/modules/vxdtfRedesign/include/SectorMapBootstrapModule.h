/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni, Thomas Lueck                                  *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#pragma once

//framework:
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/PayloadFile.h>

#include <vector>
#include <string>
#include <tuple>

namespace Belle2 {

  class VXDTFSecMap;
  struct SectorMapConfig;

  /** The SegmentFilterConverterModule is a module able to read the
   *  cutoff values for filtering the Segments written in the
   *  xml gear box and write it in the new redesigned format
   */
  class SectorMapBootstrapModule : public Module {

  public:

    //! Constructor
    SectorMapBootstrapModule();

    //! Destructor
    virtual ~SectorMapBootstrapModule()
    {
      if (m_ptrDBObjPtr != nullptr) delete m_ptrDBObjPtr;
    };

    virtual void initialize()   ;
    virtual void beginRun()     ;
    virtual void event()        ;
    virtual void endRun()       ;

  private:
    void bootstrapSectorMap(void);
    void bootstrapSectorMap(const SectorMapConfig& config);
    void persistSectorMap(void);
    /** retrieves SectorMap from file or from the DB **/
    void retrieveSectorMap(void);

    const std::string c_setupKeyNameTTreeName     = "Setups";
    const std::string c_setupKeyNameBranchName      = "name";

    std::string m_sectorMapsInputFile = "SectorMaps.root";
    std::string m_sectorMapsOutputFile = "SectorMaps.root";

    // if specified (non "") ONLY the setup with this name will be read. Else all setups in the root file will be read
    std::string m_setupToRead = std::string("");

    // pointer to the DBObjPtr for the payloadfile from which the sectormap is read
    DBObjPtr<PayloadFile>* m_ptrDBObjPtr = nullptr;


    // if true the sector map will be read from the DB. NOTE: this will override m_readSectorMap (read from file)
    bool m_readSecMapFromDB = false;

    bool m_readSectorMap  = true;
    bool m_writeSectorMap = false;
  };
} // Belle2 namespace
