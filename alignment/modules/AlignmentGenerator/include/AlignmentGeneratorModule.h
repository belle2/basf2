/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNMENTGENERATORMODULE_H
#define ALIGNMENTGENERATORMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Generate VXD misalignment and store in database
   *
   * Generate VXD misalignment and store in database
   *
   */
  class AlignmentGeneratorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    AlignmentGeneratorModule();

    /** Generate misalignment and store in database */
    virtual void initialize();


  private:

    int m_experimentLow;  /**< Min experiment number to generate this misalignment for */
    int m_runLow;  /**< Min run number to generate this misalignment for */
    int m_experimentHigh;  /**< Max experiment number to generate this misalignment for */
    int m_runHigh;  /**< Max run number to generate this misalignment for */
    std::vector<std::string> m_data;  /**< Data for misalignment generation */
    std::string m_name;  /**< Name of generated misalignment in database */
  };
}

#endif /* ALIGNMENTGENERATORMODULE_H */
