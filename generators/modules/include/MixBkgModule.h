/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MIXBKGMODULE_H
#define MIXBKGMODULE_H

#include <framework/core/Module.h>
#include <generators/mixbkg/MixBackground.h>

#include <string>
#include <vector>


namespace Belle2 {

  /** The MixBkgModule module.
   * This module mixes pre-simulated background to the current event.
   * The background is given as a list of readout frames for each subdetector, where each
   * readout frame consists of a list of SimHits. Optional, MonteCarlo information can be
   * stored with each readout frame. If the mixing module finds MonteCarlo information, only
   * the MCParticle which created the SimHit is added to the standard MCParticle collection.
   * If the analysis mode is turned on, all MCParticles of the background and the information
   * about the background generator that created a specific MCParticle are stored into a
   * separate collection.
   */
  class MixBkgModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    MixBkgModule();

    /** Destructor. */
    virtual ~MixBkgModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Terminates the module. */
    virtual void terminate();


  protected:

    //Parameters
    std::vector<std::string> m_fileNames; /**< List of the ROF Root background files.*/
    bool m_analysisMode; /**< Activates the analysis mode which stores all background MCParticles (+mothers) and background info in a separate collection. */

    //Variables
    generators::MixBackground m_mixBackground; /**< The background mixing library.*/

  };

} // end namespace Belle2

#endif /* MIXBKGMODULE_H */
