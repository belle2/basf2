/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include "tracking/vxdCaTracking/HitExporter.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/vxdCaTracking/VXDSector.h"
// #include <fstream>
#include <string>
#include <vector>


namespace Belle2 {

  /** NonRootDataExportModule
   *
   * this module can be used for text-file-output of data (in the VXD) generated during events. It supports export of trueHits, clusterHits (converted to 2D hits in the SVD) and GFTrackCands
   *
   */
  class NonRootDataExportModule : public Module {

  public:

    typedef std::vector<PassData*> PassSetupVector; /**< contains all passes used for track reconstruction */
    typedef std::pair<unsigned int, VXDSector* > secMapEntry; /**< represents an entry of the MapOfSectors */

    /**
     * Constructor of the module.
     */
    NonRootDataExportModule();

    /** Destructor of the module. */
    ~NonRootDataExportModule();

    /** Initializes the Module.
     */
    void initialize() override;

    /**
     * Prints a header for each new run.
     */
    void beginRun() override;

    /** Prints the full information about the event, run and experiment number. */
    void event() override;

    /**
     * Prints a footer for each run which ended.
     */
    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

    /** imports all sectorMaps listed in the vector */
    void loadSecMap(std::vector<std::string> secMaps);

    /** prints all sectorMaps to a file with the same name */
    void printSecMap();

    /** importing whole geometry information.
    *
    * we don't need the whole geometry information, but only a guess for each layer, therefore we only store one sensor per layer.
    * Since the info is stored within a set which needs a key to reveal its info,
    * I have to loop over all entries until I find the right one.
    * */
    void importGeometry(HitExporter* hitExporter);

  protected:

    std::string
    m_PARAMExportTrueHits; /**< allows you to export true hits. here, various values will be set at once, first part will determine whether you want to export PXD, SVD or VXD hits, second part says whether you want to export 'real' hits or 'background' hits, full example: 'SVDreal'. Wrong input values will cause fatal error. If you don't want to export trueHits, use 'none' */
    bool m_PARAMExportGFTCs; /**< allows you to export mcInformation about whole tracks, set true for tcOutput */
    std::string
    m_PARAMDetectorType; /**< set detectorype. Please choose between 'PXD', 'SVD' (standard) or 'VXD'. Wrong input values will set to SVD with an error. */
    int m_PARAMeventCounter; /**< adds this number to the m_eventCounter (useful for looping shell scripts) */
    int m_PARAMminTCLength;  /**< tracks with less than minTCLength hits will be neglected */
    bool m_PARAMsmearTrueHits; /**< when using trueHits, hits and mcPoints have got exactly the same position. If you activate the smearing, the hits will be smeared using the sensor resolution */
    std::string
    m_PARAMoutputFormat; /**< this module can produce output-files with different styles, currently supported are 'gsi' and 'simpleMatlab'. */
    HitExporter m_exportContainer; /**< container storing storing hit infos */
    int m_eventCounter; /**< knows current event number */
    int m_runCounter; /**< knows current run number */
    std::vector<std::string>
    m_PARAMwriteSecMaps; /**< if you want to output an extra file for secMaps (sector-relations) simply write the names of the sectorMaps into that list of names here (same coding as for VXDTF) */
    PassSetupVector m_passSetupVector; /**< contains information for each pass */
    std::vector<HitExporter>
    m_exportContainerVector; /**< vector of HitExporters (has to be same amount as passes stored in the PassSetupVector) */

  private:

  };
}
