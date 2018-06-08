/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMBADCHANNELS_DB_H
#define BKLMBADCHANNELS_DB_H

#include <TObject.h>
#include <vector>
#include <algorithm>
#include <bklm/dataobjects/BKLMStatus.h>

namespace Belle2 {

  /**
   *   Contains bad(dead/hot) channels
   *   The channel ID is defined according to the geometry position on the detector
   *   isForward[0,1], 0 is backward, 1 is forward
   *   sector[1-8], 1 is on +x axis, 2 is on +y axis
   *   layer[1-15], 1 is the innemost
   *   plane[0, 1], 0 zReadout plane, not always the inner plane according to the real detector geometry
   *   strip[1-48]
   */

  class BKLMBadChannels: public TObject {

  public:

    /**
     * Default constructor
     */
    BKLMBadChannels(): m_DeadChannels(), m_HotChannels(), m_comment() {};

    /**
     * Default destructor
     */
    ~BKLMBadChannels() {};

    /**
     * Return a list of dead channels
     */
    std::vector<int> getDeadChannels() const {return m_DeadChannels; }

    /**
     * Return a list of hot channels
     */
    std::vector<int> getHotChannels() const {return m_HotChannels; }

    /**
     * Set the list of dead channels
     */
    void setDeadChannel(std::vector<int>& channels) {m_DeadChannels = channels; }

    /**
     * Set the list of hot channels
     */
    void setHotChannel(std::vector<int>& channels) { m_HotChannels = channels; }

    /**
     * Add a channel number to the list of dead channels
     */
    void appendDeadChannel(int channel);

    /**
     * Add a channel number to the list of dead channels
     */
    void appendDeadChannel(int isForward, int sector, int layer, int plane, int strip);

    /**
     * Add a channel number to the list of hot channels
     */
    void appendHotChannel(int channel);

    /**
     * Add a channel number to the list of hot channels
     */
    void appendHotChannel(int isForward, int sector, int layer, int plane, int strip);

    //! retrun if the channel is hot or not
    bool isHotChannel(int channel) const;

    //! retrun if the channel is dead or not
    bool isDeadChannel(int channel) const;

    //! retrun if the channel is hot or not
    bool isHotChannel(int isForward, int sector, int layer, int plane, int strip) const;

    //! retrun if the channel is dead or not
    bool isDeadChannel(int isForward, int sector, int layer, int plane, int strip) const;

    //! Get comment
    std::string getComment() const {return m_comment; }

    //! Set comment
    void setComment(const std::string& comment) {m_comment = comment;}

    //! convert geometry position to a channel ID
    int geometryToChannelId(int isForward, int sector, int layer, int plane, int strip) const;

    //! print hot channels
    void printHotChannels() const;

    //! print dead channels
    void printDeadChannels() const;


  private:

    std::vector<int> m_DeadChannels;           /**< List of dead channels >*/
    std::vector<int> m_HotChannels;            /**< List of hot channels >*/
    std::string m_comment;                     /**< optional comment > */

    ClassDef(BKLMBadChannels, 1); /**< ClassDef, must be the last term before the closing {}*/
  };

}// end namespace Belle2

#endif
