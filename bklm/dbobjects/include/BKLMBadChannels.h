/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   *   Contains bad(dead/hot) channels
   */

  class BKLMBadChannels: public TObject {

  public:

    /**
     * Default constructor
     */
    BKLMBadChannels(): m_DeadChannels(), m_HotChannels() {};

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
    void setDeadChannel(std::vector<int> channels) {m_DeadChannels = channels; }

    /**
     * Set the list of hot channels
     */
    void setHotChannel(std::vector<int> channels) { m_HotChannels = channels; }

    /**
     * Add a channel number to the list of dead channels
     */
    void appendDeadChannel(int channel) {m_DeadChannels.push_back(channel); }

    /**
     * Add a channel number to the list of hot channels
     */
    void appendHotChannel(int channel) {m_HotChannels.push_back(channel); }

    //! Get comment
    std::string getComment() const {return m_comment; }

    //! Set comment
    void setComment(const std::string& comment) {m_comment = comment;}


  private:

    std::vector<int> m_DeadChannels;           /**< List of dead channels >*/
    std::vector<int> m_HotChannels;            /**< List of hot channels >*/
    std::string m_comment;                     /**< optional comment > */

    ClassDef(BKLMBadChannels, 1); /**< ClassDef, must be the last term before the closing {}*/
  };

}// end namespace Belle2
