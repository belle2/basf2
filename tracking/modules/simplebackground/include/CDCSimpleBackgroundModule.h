/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSIMPLEBACKGROUNDMODULE_H_
#define CDCSIMPLEBACKGROUNDMODULE_H_


#include <framework/core/Module.h>


namespace Belle2 {

  /** Module to create some random background hits in the CDC.
   *  The module should be used after the CDCDigitizer but before tracking.
   *  The module generates an 1/r background, e.g. the number of hits per layer is constant.
   *  In a loop over all layers a random wireId is chosen, other needed variables (driftTime, charge) are also assigned, but it has to be checked how realistic they are.
   *  There is a possibility to generate single random hits, or clusters of hits.
   *  In this case cluster means that there are some additional hits generated around the random hit (neighboring layers/wires). In total a cluster will have 5-7 Hits.
   *  The steering parameters give the amount of wires hit by background in %.
   *  (The CDC has 14336 wires, e.g. if you choose 1% for single hits you will get 143 background hits, if you choose 1% for clusters you will get ~24 cluster with ~6 hits each).
   *
   *  @todo Check the values for drift time and charge, maybe change the random distribution of background hits to some more realistic distribution. Add also SVD and PXD random hits.
   */
  class CDCSimpleBackgroundModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    CDCSimpleBackgroundModule();

    /** Destructor of the module.
     */
    virtual ~CDCSimpleBackgroundModule();

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


  private:

    std::string m_cdcHitsColName;                    /**< Input digitized hits collection name (output of CDCDigitizer module) */
    double m_hits;                                   /**< Percentage of wires hit by background through single hits. Should be between 0 and 100. */
    double m_clusters;                               /**< Percentage of wires hit by background through hit clusters. Should be between 0 and 100. */

  };
} // end namespace Belle2

#endif /* CDCSIMPLEBACKGROUNDMODULE_H_ */

