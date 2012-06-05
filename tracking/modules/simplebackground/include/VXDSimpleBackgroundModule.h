/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef VXDSIMPLEBACKGROUNDMODULE_H_
#define VXDSIMPLEBACKGROUNDMODULE_H_


#include <framework/core/Module.h>


namespace Belle2 {

  /** Module to create some random background hits and
   * measrument outliers in the Si detectors
   */
  class VXDSimpleBackgroundModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    VXDSimpleBackgroundModule();

    /** Destructor of the module.
     */
    virtual ~VXDSimpleBackgroundModule();

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
    double m_outlierRatio;
    double m_sigmaFactorU;
    double m_sigmaFactorV;
    double m_energyThreshold;
    bool m_onlyPrimaries;
    std::vector<bool> m_outlierLayers;
    std::vector<bool> m_backroundLayers;
    double m_backgroundRatio;
    double m_backgroundRatio2;
    double m_semiAxisFactorU;
    double m_semiAxisFactorV;
    bool m_writeTruthToFile;
    bool m_randomRadius;
    double m_backgroundAngle;
    double m_setMeasSigma;
    bool m_only6;
    int m_notPerfectCounter;
    double m_outlierShift;
    int m_outliersPerTrack;

  };
} // end namespace Belle2

#endif /* VXDSIMPLEBACKGROUNDMODULE_H_ */

