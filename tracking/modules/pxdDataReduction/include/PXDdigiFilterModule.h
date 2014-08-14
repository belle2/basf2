/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni, Giulia Casarosa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDIGIFILTERMODULE_H
#define PXDDIGIFILTERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDDigit inside the ROIs.
   *
   *    *
   */
  class PXDdigiFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDdigiFilterModule();

    /**  */
    virtual ~PXDdigiFilterModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /**  */
    virtual void event();

    /**  */
    virtual void endRun();

    /**  */
    virtual void terminate();


  private:

    std::string m_PXDDigitsName;  /**< The name of the StoreArray of PXDDigits to be filtered */
    std::string m_PXDFilteredDigitsName;  /**< The name of the StoreArray of Filtered PXDDigits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */
  };
}

#endif /* PXDDIGIFILTERMODULE_H */
