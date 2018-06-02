/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <vector>

namespace Belle2 {

  /**
    * Class representing a cluster candidate for DATCON during simple clustering of the SVD
    */
  class DATCONSVDSimpleClusterCandidate {

  public:

    /** Constructor to create an empty Cluster */
    DATCONSVDSimpleClusterCandidate();

    /** Constructor indicating vxdid and u-side information */
    DATCONSVDSimpleClusterCandidate(VxdID vxdID, bool isUside);

    /** Constructor indicating vxdid, u-side information in case of a maximum cluster size */
    DATCONSVDSimpleClusterCandidate(VxdID vxdID, bool isUside, unsigned short maxClusterSize);

    /**
     * Add a Strip to the current cluster.
     * Update the cluster seed seed.
     * @param vxdID: VxdID of the strip that is to be added to the cluster
     * @param isUSide: Is the strip a u-side strip?
     * @param index: Index of the strip inside the DATCONSVDDigit StoreArray
     * @param charge: Charge of the strip that is to be added
     * @param cellID: CellID of the strip that is to be added
     * @return true if the strip is on the expected side and sensor and it's next to the last strip added to the cluster candidate
     */
    bool add(VxdID vxdID, bool isUside, unsigned short index, unsigned short charge, unsigned short cellID);

    /**
     * compute the position of the cluster
     */
    void finalizeCluster();

    /**
     * compute the simple cluster position as in phase 2 FPGA implementation
     */
    void finalizeSimpleCluster();

    /**
     * return true if the cluster candidate can be promoted to cluster
     */
    bool isGoodCluster();

    /**
     * return the VxdID of the cluster sensor
     */
    VxdID getSensorID() {return m_vxdID;}

    /**
     * return true if the cluster is on the U/P side
     */
    bool isUSide() {return m_isUside;}

    /**
     * Return the charge of the cluster.
     * I.e. charge of the central strip in case of simple cluster.
     */
    unsigned short getCharge() const { return m_charge; }

    /**
     * Return the seed charge of the cluster.
     * I.e. charge of the central strip in case of simple cluster.
     */
    unsigned short getSeedCharge() const { return m_seedCharge; }

    /**
     * return the position of the cluster
     */
    float getPosition() const { return m_position; }

    /** TODO */
    unsigned short getSeedStripIndex() const { return m_seedStripIndex; }

    /**
     * Return the cluster size (number of strips of the cluster).
     */
    unsigned short size() const { return m_size; }

    /** Get vector of strips of this cluster candidate */
    std::vector<unsigned short> getStripVector() const  { return m_strips; }

    /** Get vector of the charges of the stips of this cluster candidate */
    std::vector<unsigned short> getChargeVector() const { return m_charges; }

    /** Get vector of indices of the DATCONSVDDigits in the DATCONSVDDigit StoreArray */
    std::vector<unsigned short> getIndexVector() const  { return m_digitIndices; }

  protected:

    /** VxdID of the cluster */
    VxdID m_vxdID;

    /** side of the cluster */
    bool m_isUside;

    /** Maximum cluster size (mainly for the simple clusterizer) */
    unsigned short m_maxClusterSize;

    /** Charge of the cluster */
    unsigned short m_charge;

    /** Seed Charge of the cluster */
    unsigned short m_seedCharge;

    /** Position of the cluster */
    float m_position;

    /** SVD strip (i.e. 0...511 or 0...767) that is considered the seed strip of the cluster */
    unsigned short m_seedStrip;

    /** Size of the cluster */
    unsigned short m_size;

    /** Index of the seed strip of the cluster (0...m_Size) */
    unsigned short m_seedStripIndex;

    /** Vector containing strips (DATCONSVDDigits) that are added */
    std::vector<unsigned short> m_strips;

    /** Vector containing the charges of the corresponding strips that are added */
    std::vector<unsigned short> m_charges;

    /** Vector of the indices in the DATCONSVDDigit StoreArray of the added strips */
    std::vector<unsigned short> m_digitIndices;

  }; // end class definition

} // end namespace Belle2
