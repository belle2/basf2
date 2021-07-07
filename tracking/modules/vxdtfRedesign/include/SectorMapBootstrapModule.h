/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//framework:
#include <framework/core/Module.h>
#include <framework/database/PayloadFile.h>

#include <vector>
#include <string>
#include <tuple>

namespace Belle2 {

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
    ~SectorMapBootstrapModule()
    {
      if (m_ptrDBObjPtr != nullptr) delete m_ptrDBObjPtr;
    };

    void initialize() override   ;
    void beginRun() override     ;
    void event() override        ;
    void endRun() override       ;

  private:

    /// puts several empty sectormaps into the framework
    void bootstrapSectorMap(void);
    /** puts one empty sectormap into the framework:
      @param config: the configuration used to generate the empty sectormap */
    void bootstrapSectorMap(const SectorMapConfig& config);
    /// writes a sectormap to a root file
    void persistSectorMap(void);
    /// retrieves SectorMap from file or from the DB
    void retrieveSectorMap(void);

    /// the name of the tree the setups are stored in in the root file
    const std::string c_setupKeyNameTTreeName     = "Setups";
    /// the name of the branch the setupt are stored in the tree
    const std::string c_setupKeyNameBranchName      = "name";

    /// the name of the input root file the sectormaps are read from
    std::string m_sectorMapsInputFile = "SectorMaps.root";
    /// the name of the ouput root file the sectormaps are written to
    std::string m_sectorMapsOutputFile = "SectorMaps.root";

    /// if specified (non "") ONLY the setup with this name will be read. Else all setups in the root file will be read
    std::string m_setupToRead = std::string("");

    /// pointer to the DBObjPtr for the payloadfile from which the sectormap is read
    DBObjPtr<PayloadFile>* m_ptrDBObjPtr = nullptr;


    /** vector of tuple<int, string> specifying how 2-hit filters are altered.
      The int entry of the tuple contains the index of the cut value to be changed
      (see Filter::getNameAndReference function) and the string entry of the tuple contains a regex for a TF1 */
    std::vector< std::tuple<int, std::string> > m_twoHitFilterAdjustFunctions = {};

    /** vector of tuple<int, string>  specifying how 3-hit filters are altered.
      The int entry of the tuple contains the index of the cut value to be changed
      (see Filter::getNameAndReference function) and the string entry of the tuple contains a regex for a TF1 */
    std::vector< std::tuple<int, std::string> > m_threeHitFilterAdjustFunctions = {};

    /// if true the sector map will be read from the DB. NOTE: this will override m_readSectorMap (read from file)
    bool m_readSecMapFromDB = false;

    /// if true a sectormap will be read from a file. NOTE: this will be overridden by m_readSecMapFromDB!
    bool m_readSectorMap  = true;
    /// if true the sectormap will be written to an output file
    bool m_writeSectorMap = false;
  };
} // Belle2 namespace
