/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCheckDigitsModule_H
#define SVDCheckDigitsModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>

namespace Belle2 {
  class RelationArray;
  class RelationElement;

  namespace SVD {

    /** The SVDCheckDigits module.
     *
     * This module compares a set of SVDShaperDigits generated from SVDDigits to check
     * they are equivalent. Only useful for diagnostic purposes.
     * Fields that are incompatible between the two types of digits are ignored in the comparison
     * (position in SVDDigits, time and time error in SVDShaperDigits)
     * @see SVDDigitMergerModule
     */
    class SVDCheckDigitsModule : public Module {

    public:
      /** Constructor defining the parameters */
      SVDCheckDigitsModule();
      /** Initialize the module */
      virtual void initialize() override;
      /** do the clustering */
      virtual void event() override;

    protected:

      // Data members
      /** Name of the collection to use for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;

    };//end module declaration

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDCheckDigitsModule_H
