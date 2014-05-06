/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <genfit/TrackCand.h>


namespace Belle2 {

  class PXDTrueHit;
  class PXDCluster;
  class SVDTrueHit;
  class SVDCluster;
//  class GFTrackCand;
  class ExporterEventInfo;
  class PassData;

  /** Bundles information for all events to be stored by NonRootDataExportModule and writes that info into text files */
  class HitExporter {

  public:

    typedef std::map<int, ExporterEventInfo*> EventMap;
    typedef std::pair<int, ExporterEventInfo*> EventMapEntry;
    typedef std::pair<double, std::string> TcHitEntry;

    /** Empty constructor. */
    HitExporter():
      m_eventNumber(0),
      m_attachedPass(NULL) {}

    /** Destructor. */
    ~HitExporter();


    /** a call per run expected: takes given sensorInfoBase and stores relevant information about it */
    void storeSensorInfo(const VXD::SensorInfoBase& aSensorInfo);


    /** event wise call expected: sets internal Event number needed for identifying hits */
    void prepareEvent(int n);


    /** set type of outputFormat you wish. currently supported formats can be found in the nonRootDataExportModule */
    void setOutputFormat(std::string oFormat) { m_outputFormat = oFormat; }


    /** attach pass to hitExporter */
    void setPass(PassData* myPass) { m_attachedPass = myPass; }


    /** set the sector-cuts for the u-coordinate */
    void setUCuts(std::vector<double> uCuts) { m_uCuts = uCuts; }


    /** set the sector-cuts for the u-coordinate */
    void setVCuts(std::vector<double> vCuts) { m_vCuts = vCuts; }


    /** returns outputFormat set by user. If value is empty, it has not been set yet. */
    std::string getOutputFormat() { return m_outputFormat; }


    /** returns event number of last event executed */
    int getCurrentEventNumber();


    /** returns event total number of hits stored during last event executed */
    int getNumberOfHits();


    /** returns event total number of PXDTruehits stored during last event executed */
    int getNumberOfPXDTrueHits();


    /** returns event total number of SVDTruehits stored during last event executed */
    int getNumberOfSVDTrueHits();


    /** event wise call expected: Expects pointer to the PXDTrueHit to be stored. It stores its entries to hits. Returns string containing error message if something went wrong */
    std::string storePXDTrueHit(VXD::GeoCache& geometry, const PXDTrueHit* aHit, int storeArrayID, bool doSmear, int isPrimaryBackgroundOrGhost = -1, int particleID = -1, int pdg = -1); // default arguments are only allowed for .h-files, not in the .cc- version of the memberfunction


    /** event wise call expected: Expects pointer to the SVDTrueHit to be stored. It stores its entries to hits. Returns string containing error message if something went wrong */
    std::string storeSVDTrueHit(VXD::GeoCache& geometry, const SVDTrueHit* aHit, int storeArrayID, bool doSmear, int isPrimaryBackgroundOrGhost = -1, int particleID = -1, int pdg = -1);


    /** event wise call expected: It stores GFTrackCands and its containing TrueHits(as McPoints) */
    std::string storeGFTC(VXD::GeoCache& geometry, const genfit::TrackCand* aTC, int tcFileIndex, int tcSimIndex, std::vector<const PXDTrueHit*> pxdHits, std::vector<const SVDTrueHit*> svdHits, std::vector<int> hitIDs);


    /** expected to be called at the end of a run: exports every TC and every hit stored.
     * the naming convention for standard output will be:
     * run[X]_event[X]_hits for hits and rX_eX_tcs for track candidates
     * individual output will be event[X]_[name]
     */
    std::string exportGsi(int runNumber, float bz = 1.5);


    /** expected to be called at the end of a run: every hit stored.
     */
    std::string exportSimpleMatlab(int runNumber);


  protected:

    /** feeded by storePXDTrueHit or storeSVDTrueHit and does all the calculations and storing */
    std::string storeTrueHit(VXD::SensorInfoBase aSensorInfo, double u, double v, int type, int storeArrayID, int isPrimaryBackgroundOrGhost, int aLayerID, int particleID, int pdg, bool doSmear);


    int m_eventNumber; /**< stores current event number */
    EventMap m_storedOutput; /**< stores everything which has to be stored */
    ExporterEventInfo* m_thisEvent; /**< pointer for current event for easier code */
    std::string m_exportFileName; /**< name used  for exporting data */
    std::string m_outputFormat; /**< value set by module and defines ouputFormat */
    std::vector<std::string> m_geometry; /**< stores info about layers */
    PassData* m_attachedPass; /**< direct access to pass connected to current hits */
    std::vector<double> m_uCuts; /**< carries sector-cuts for the u-coordinate of the hits */
    std::vector<double> m_vCuts; /**< carries sector-cuts for the v-coordinate of the hits */
//    VXD::GeoCache& m_geometry; /**< stores info about geometry */

  }; //end class HitExporter
} //end namespace Belle2

