/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMELECTRONICMAPPING_DB_H
#define BKLMELECTRONICMAPPING_DB_H

#include <TObject.h>
#include <string>
#include <cmath>
//#include <framework/logging/Logger.h>
#include <vector>

#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for BKLM electronics mapping.
   */

  class BKLMElectronicMapping: public TObject {
  public:

    /**
     * Default constructor
     */

    BKLMElectronicMapping(): m_version(0.0), m_copperId(0), m_slotId(0), m_laneId(0), m_axisId(0), m_channelId(0),
      m_isForward(0), m_sector(0), m_layer(0), m_plane(0), m_strip(0) {};

    /**
     * Constructor
     */

    BKLMElectronicMapping(int version, int copperId, int slotId, int laneId, int axisId, int channelId, int isForward, int sector,
                          int layer,
                          int plane, int stripId)
    {
      m_version = version; m_copperId = copperId; m_slotId = slotId; m_laneId = laneId; m_axisId = axisId; m_channelId = channelId;
      m_isForward = isForward; m_sector = sector; m_layer = layer; m_plane = plane; m_strip = stripId;
    };

    /**
     * Destructor
     */
    ~BKLMElectronicMapping() {};

    /** Return BKLMElectronicMapping Version
     */
    int getBKLMElectronictMappingVersion() const {return m_version; }

    /** Return copperId in the map
     */
    int getCopperId() const {return m_copperId; }

    /** Return finesse slotId in the map
     */
    int getSlotId() const {return m_slotId; }

    /** Return axisId in the map
     */
    int getAxisId() const {return m_axisId; }

    /** Return laneId in the map
     */
    int getLaneId() const {return m_laneId; }

    /** Return electronics channelId in the map
     */
    int getChannelId() const {return m_channelId; }

    /** Return forward/backward info. in the map
     */
    int getIsForward() const {return m_isForward; }

    /** Return sector #No in the map
     */
    int getSector() const {return m_sector; }

    /** Return layer #No in the map
     */
    int getLayer() const {return m_layer; }

    /** Return which plane (z/phi) in the map
     */
    int getPlane() const {return m_plane; }

    /** Return physical channel in the map
     */
    int getStripId() const {return m_strip; }

  private:

    int m_version;                             /**< map version (measurement number) */
    int m_copperId;                            /**< BKLM motherboard copper id */
    int m_slotId;                              /**< BKLM motherboard fission id should be 1,2,3 or 4*/
    int m_laneId;                              /**< lane id*/
    int m_axisId;                              /**< Axis id, should be 0 or 1 */
    int m_channelId;                              /**< electronic channel id */
    int m_isForward;                       /**< forward(1) or backward(0) */
    int m_sector;                              /**< which sector */
    int m_layer;                               /**< which layer*/
    int m_plane;                               /**< phi-measuring plane or z-measuring plane*/
    int m_strip;                               /**< strip number */
    //int is_phiMeasuring;                       /**< is phi-measuring plane (1) or z-measuring plane (0)*/
    //std::vector<int> m_copperId;               /**< BKLM motherboard copper id */
    std::string m_comment;                     /**< optional comment */


    ClassDef(BKLMElectronicMapping, 3);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // end namespace Belle2

#endif
