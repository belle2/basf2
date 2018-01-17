/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_SENDER_H_
#define ROI_SENDER_H_

#include <framework/core/Module.h>
#include <string>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

namespace Belle2 {

  /** The ROI to ONSEN Module
   *
   * this module is used to check the payload produced by the ROItoOnsen Module
   *
   */

  class ROISenderModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROISenderModule();

    /**
     * Destructor of the module.
     */
    ~ROISenderModule();

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  protected:


    std::string  m_messageQueueName; /**< message queue name*/
    const char* m_messageQueueNameCstring; /**< message queue name c string */
    std::string  m_ROIpayloadName; /**< ROI payload name*/

    int          m_messageQueueDepth; /**< message queue depth*/
    int          m_messageQueueMsgSize; /**< message queue message size*/

  private:
    mqd_t        m_messageQueue; /**< message queue*/


    void openMessageQueue(const char* log_string); /**< open message queue*/
    void closeMessageQueue(const char* log_string); /**< close message queue*/
    void unlinkMessageQueue(const char* log_string); /**< unlink message queue*/
  };
}
#endif
