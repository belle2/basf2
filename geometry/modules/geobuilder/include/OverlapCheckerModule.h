/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OVERLAPCHECKERMODULE_H_
#define OVERLAPCHECKERMODULE_H_

#include <framework/core/Module.h>


namespace Belle2 {

  /**
   * The OverlapCheckerModule module.
   *
   * Checks the current geometry for overlaps and prints them.
  */
  class OverlapCheckerModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description and the parameters of the module.
     */
    OverlapCheckerModule();

    /** Destructor. */
    virtual ~OverlapCheckerModule();

    /**
     * Checks the validity of the module parameters.
     */
    virtual void initialize();

    /** Saves the geometry to a root file for every event. */
    virtual void event();


  protected:

    double m_tolerance;   /**< The tolerance allowed for the overlapping distance.*/
    bool m_printOverlaps; /**< Set to true to print the found overlaps.*/

  private:


  };
}

#endif /* OVERLAPCHECKERMODULE_H_ */
