/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMPARAMLOADERMODULE_H
#define BKLMPARAMLOADERMODULE_H

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  /** This module loads BKLM simulation-control parameters from the
   *  steering script and an XML document.
   *
   * Reads some parameters from the steering script, then
   * creates a GearboxIO object to read the remaining parameters
   * from an XML file.
   */
  class BKLMParamLoaderModule : public Module {

  public:

    //! Constructor sets the description and defines the module parameters
    BKLMParamLoaderModule();

    //! Destructor
    virtual ~BKLMParamLoaderModule();

    /*! Passes the steering-script parameters to BKLMSimulationPar, then
     *  connects to the XML document (specified by a scipr parameter) so
     *  that BKLMSimulationPar can read the XML parameters.
     */
    virtual void initialize();

  protected:

  private:

    //! Flag to check (or not) the validity of each XML parameter
    bool m_ParamCheck;

    //! Random number seed for RPC strip-multiplicity algorithm
    int m_RandomSeed;

    //! Flag to do (or not) the neutron-background study
    bool m_DoBackgroundStudy;

  };

} // end of namespace Belle2

#endif // BKLMPARAMLOADERMODULE_H
