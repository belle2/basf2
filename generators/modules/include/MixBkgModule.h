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
   *
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

    //Variables
    generators::MixBackground m_mixBackground; /**< The background mixing library.*/

  };

} // end namespace Belle2

#endif /* MIXBKGMODULE_H */
