/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>

#include <boost/shared_ptr.hpp>

class TObject;

namespace Belle2 {
  typedef boost::shared_ptr<TObject> TObjectPtr;

  /** Define XML (de)serialization methods for TObject.
   *
   *  Prototype, might be moved later on.
   */
  namespace Stream {
    /** Convert given TObject into an XML string.
     */
    std::string serialize(const TObject* obj);

    /** Convert given serialized XML string back into TObject.
     *
     *  Returns a boost::shared_ptr<TObject> to the deserialized object, might be NULL if conversion was impossible.
     */
    TObjectPtr deserialize(const std::string& data);
  }
}
