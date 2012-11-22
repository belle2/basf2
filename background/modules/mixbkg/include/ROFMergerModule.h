/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROFMERGERMODULE_H
#define ROFMERGERMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>


namespace Belle2 {

  /** The ROFMerger module.
   *
   */
  class ROFMergerModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    ROFMergerModule();

    /** Destructor. */
    virtual ~ROFMergerModule() {}

    /**  */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();


  protected:

    //std::string m_particleList; /**< The name of the MCParticle collection. */
    //std::vector<bool> m_seen;   /**< Tag the particles which were already visited using their index. */
  };

} // end namespace Belle2


#endif /* ROFMERGERMODULE_H */
