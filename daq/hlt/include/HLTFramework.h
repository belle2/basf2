/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTFRAMEWORK_H
#define HLTFRAMEWORK_H

#include <iostream>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <daq/hlt/HLTManager.h>
#include <daq/hlt/HLTProcess.h>
#include <daq/hlt/XMLParser.h>

namespace Belle2 {

  //! HLTFramework class
  /*! This class is the highest level component for HLT system
  */
  class HLTFramework {
  public:
    //! Constructor
    HLTFramework(int type);
    //! Destructor
    ~HLTFramework();

    //! Initialize the framework
    EHLTStatus init(char* xmlFileName);

  private:
    EHLTNodeType m_type;        /**< Type of the node */

    HLTManager* m_hltManager;   /**< Pointer to HLTManager for manager node */
    HLTProcess* m_hltProcess;   /**< Pointer to HLTProcess for process node */
  };
}

#endif
