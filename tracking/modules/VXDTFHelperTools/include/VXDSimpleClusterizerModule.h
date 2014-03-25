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

  /** Module to convert TrueHits into Clusters using a simplified process. Supports PXD (2D) and SVD (1D)
   * usefull for tracking purposes and crosschecks with the PXD/SVDDigitizer and Clusterizer modules.
   * so far only the coordinates (smeared) and the timestamp (sharp) are useful values
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
    double m_energyThresholdU; /**< set energy threshold for SVDClusters in u-direction in GeV (standard is 17.4E-6) */
    double m_energyThresholdV; /**< set energy threshold for SVDClusters in v-direction in GeV (standard is 28.6E-6) */
    double m_energyThreshold; /**< set energy threshold for PXDClusters in GeV (standard is 7E-6) */
    bool m_onlyPrimaries; /**< set True if you do not want to have hits by secondary particles */
    double m_uniSigma;  /**< you can define the sigma of the smearing. Standard value is the sigma of the unifom distribution for 0-1: 1/sqrt(12) */
    double m_setMeasSigma; /**< if positive value (in cm) is given it will be used as the sigma to smear the Clusters otherwise pitch/uniSigma will be used */
    int m_weakSVDHitCtr; /**< counts SVDHits whose energy deposit is lower than energyThreshold */
    int m_weakPXDHitCtr; /**< counts PXDHits whose energy deposit is lower than energyThreshold */

  };
} // end namespace Belle2

#endif /* VXDSIMPLECLUSTERIZERMODULE_H_ */

