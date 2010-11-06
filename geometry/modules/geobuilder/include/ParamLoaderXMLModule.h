/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARAMLOADERXML_H_
#define PARAMLOADERXML_H_

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  /** The ParamLoaderXML module.
   *
   * This module loads the parameters from a XML document.
   *
   * Creates a GearboxIO object which reads the parameters
   * for the Belle II detector geometry from XML files.
   * Starting from the basic XML file, which includes all other
   * XML files (e.g. for each subdetector one file), this method
   * allows accessing all parameters from locally stored XML documents.
   */
  class ParamLoaderXMLModule : public Module {

  public:

    /* The constructor of the module.
     * Sets the description and the parameters of the module.
     */
    ParamLoaderXMLModule();

    /** Destructor. */
    virtual ~ParamLoaderXMLModule();

    /** Checks the validity of the module parameters and creates the Gearbox instance.
     *
     * During the module initialization the Gearbox XML backend is created and the
     * XML file, specified by a module parameter, is opened.
     * Then the Gearbox singleton frontend is connected to the newly created XML backend.
     */
    virtual void initialize();


  protected:


  private:

    std::string m_filenameXML;   /**< The filename of the XML file. */
    bool m_paramCheck;           /**< If set to true, performs a check of the path/parameter each time it is accessed. */

  };
}

#endif /* PARAMLOADERXML_H_ */
