/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef VXDSIMPLECLUSTERIZERMODULE_H_
#define VXDSIMPLECLUSTERIZERMODULE_H_


#include <framework/core/Module.h>


namespace Belle2 {

  /** Module to create some random background hits and
   * measrument outliers in the Si detectors
   */
  class VXDSimpleClusterizerModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    VXDSimpleClusterizerModule();

    /** Destructor of the module.
     */
    virtual ~VXDSimpleClusterizerModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** End of the event processing.
     */
    virtual void terminate();

  protected:
    double m_energyThresholdU;
    double m_energyThresholdV;
    double m_energyThreshold;
    bool m_onlyPrimaries;
    bool m_writeTruthToFile;
    double m_setMeasSigma;

  };
} // end namespace Belle2

#endif /* VXDSIMPLECLUSTERIZERMODULE_H_ */

