/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#pragma once

//framework:
#include <framework/core/Module.h>

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
    virtual ~SectorMapBootstrapModule() { };

    virtual void initialize()   ;
    virtual void beginRun()     ;
    virtual void event()        ;
    virtual void endRun()       ;

  private:
    void bootstrapSectorMap(void);
    void bootstrapSectorMap(const SectorMapConfig& config);
    void persistSectorMap(void);
    void retrieveSectorMap(void);

    const std::string c_setupKeyNameTTreeName     = "Setups";
    const std::string c_setupKeyNameBranchName      = "name";

    std::string m_sectorMapsInputFile = "SectorMaps.root";
    std::string m_sectorMapsOutputFile = "SectorMaps.root";
    bool m_readSectorMap  = true;
    bool m_writeSectorMap = false;
  };
} // Belle2 namespace
