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
    virtual ~ROISenderModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:


    std::string  m_messageQueueName;
    const char* m_messageQueueNameCstring;
    std::string  m_ROIpayloadName;

    int          m_messageQueueDepth;
    int          m_messageQueueMsgSize;

  private:
    mqd_t        m_messageQueue;


    void openMessageQueue(const char* log_string);
    void closeMessageQueue(const char* log_string);
    void unlinkMessageQueue(const char* log_string);
  };
}
#endif
