/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <map>
#include <vector>
#include <algorithm>

#include <tracking/trackFindingVXD/displayInterfaceTF/SectorTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/ClusterTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/HitTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/CellTFInfo.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/TrackCandidateTFInfo.h>

#include <string>

namespace Belle2 {
  /** Analysis TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Important Methodes:

   *
   *  silentKill = Checks if there are not connected Clusters, Cells and Hits and deactivates them
   *  safeInformation = Stores the Informations of an Event in StoreArrays and RelationArrays
   *
   */

  class AnalizerCollectorTFInfo:  public TObject {

    //friend class AnalizerCollectorTFInfoTest;

  public:

    typedef std::pair<unsigned int, unsigned int> KeyClusters;  /**< Key of Cluster */
    typedef std::pair<unsigned int, unsigned int> KeySectors;     /**< Key to Sectors */

    /** Default constructor for the ROOT IO. */
    AnalizerCollectorTFInfo();
    virtual ~AnalizerCollectorTFInfo();

    /** Stores All Hit Information in a file */
    virtual void storeAllHitInformation(std::string filename);

    /** Stores Hit Information of 1 Particle in a file */
    virtual void storeHitInformation(std::string filename, int particleIdFilter);


    /** Stores All Cell Information in a file */
    virtual void storeAllCellInformation(std::string filename);

    /** Stores Cell Information of 1 Particle in a file */
    virtual void storeCellInformation(std::string filename, int particleIdFilter);


    /** Stores All TCand Information in a file */
    virtual void storeAllTCInformation(std::string filename);

    /** Stores Hit Information of 1 Particle in a file */
    virtual void storeTCInformation(std::string filename, int particleIdFilter);


    /** Stores All Cluster Information in a file */
    virtual void storeClustersInformation(std::string filename);

    /** Stores All Sector Information in a file */
    virtual void storeSectorInformation(std::string filename, bool withFriends);

    /** Sets all Particle IDs and real-Information from all clusters */
    virtual void setAllParticleIDs(double boarder);

    /** getter of the TCand-ID to a given gfIndex */
    virtual int getTcIDFromGfIndex(int gfIndex);



  private:

    const static std::string m_fileSeparator; /**< Seperator for output-File */

    const static int m_idAlive;     /**< Id for is Alive of Objects */
    const static int m_idAlive_output;  /**< Id for is Alive of Objects for output-File */


    ClassDef(AnalizerCollectorTFInfo, 1)
  };
}
