/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <mqueue.h>

namespace Belle2 {
  class ROIpayload;
  class EventMetaData;

  /** The ROI to ONSEN Module
   *
   * this module is used to send out the payload to ONSEN
   *
   */

  class ROISenderModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROISenderModule();


  private:

    /** Input ptr for RoiPayload. */
    StoreObjPtr<ROIpayload> m_roiPayload;
    /** Input ptr for EventMetaData. */
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /** poor mans histogramming in a vector */
    std::vector <int> m_histo;

    /**
     *Initializes the Module.
     */
    void initialize() override final;

    void event() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

    std::string  m_messageQueueName; /**< message queue name*/
    const char* m_messageQueueNameCstring; /**< message queue name c string */
    std::string  m_ROIpayloadName; /**< ROI payload name*/

    int          m_messageQueueDepth; /**< message queue depth*/
    int          m_messageQueueMsgSize; /**< message queue message size*/

    mqd_t        m_messageQueue; /**< message queue*/


    void openMessageQueue(const char* log_string); /**< open message queue*/
    void closeMessageQueue(const char* log_string); /**< close message queue*/
    /* cppcheck-suppress unusedPrivateFunction */
    void unlinkMessageQueue(const char* log_string); /**< unlink message queue*/
  };
}
