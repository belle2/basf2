/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDIGITFILTERMODULE_H
#define SVDDIGITFILTERMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**

  /**
   * Performs the easiest possible preselection of SVD digits
   *
   * This module is currently primary intended for masking a "second beamspot" in SVD6. It also erases edges of SVDs, where there are many noisy strips.   *
   */
  class SVDDigitFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDDigitFilterModule();

    /** module destructor */
    virtual ~SVDDigitFilterModule();

    /** module init */
    virtual void initialize();

    /** begin run */
    virtual void beginRun();

    /** In each event transfer all SVDDigits to a new collection, but filtered out (set to 0) */
    virtual void event();

    /** end of run */
    virtual void endRun();

    /** module termination */
    virtual void terminate();


  private:

    int m_maskUupTo;  /**< All U-digits bellow this will not pass through the filter */
    int m_maskUfrom;  /**< All U-digits after this will not pass through the filter */
    int m_maskVupTo;  /**< All V-digits bellow this will not pass through the filter */
    int m_maskVfrom;  /**< All V-digits after this will not pass through the filter */
    std::vector<int> m_maskStripsU;  /**< Stated V-digit strips will not pass through the filter */
    std::vector<int> m_maskStripsV;  /**< Stated V-digit strips will not pass through the filter */
    std::string m_inputDigits; /**< Name of collection with digits entering the filter */
    std::string m_outputDigits; /**< Name of collection with digits leaving the filter */
    std::string m_sensorID; /**< Sensor VxdID to mask */
  };
}

#endif /* SVDDIGITFILTERMODULE_H */
