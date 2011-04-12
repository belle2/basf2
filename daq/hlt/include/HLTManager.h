/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTMANAGER_H
#define HLTMANAGER_H

#include <iostream>
#include <vector>

#include <daq/hlt/XMLParser.h>
#include <daq/hlt/UnitManager.h>

namespace Belle2 {

  //! HLTManager class
  /*! This class is a manager for the entire HLT farm
  */
  class HLTManager {
  public:
    //! Constructor
    HLTManager(void);
    HLTManager(std::string& input);
    HLTManager(char* input);
    //! Destructor
    ~HLTManager(void);

    //! Initializing the HLT manager
    EStatus init(void);

    EStatus broadCasting(void);
    EStatus broadCasting(int unitNo);

    //! Print information of the HLT farm (only for internal testing)
    void Print(void);

  protected:
    //! Initializing the HLT manager (protected)
    void initHLT(void);
    //! Initializing the units inside the HLT
    EStatus initUnit(void);

  private:
    std::vector<UnitManager> m_units;       /*!< vector container to pointers to units inside HLT */
    XMLParser* m_XMLParser;                 /*!< Pointer to XML parser */
    char* m_inputXML;                       /*!< Input XML file of the entire information */

    int m_expNo;                            /*!< Experiment number that is assigned from XML file */
    int m_runStart;                         /*!< Start run number that is assigned from XML file */
    int m_runEnd;                           /*!< End run number that is assigned from XML file */
    char* m_inputName;                      /*!< Input name (human readable) from XML file */
    char* m_inputDescription;               /*!< Description (human readable) of this assignment from XML file */
  };
}

#endif
