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

class TObject;

namespace Belle2 {
  /** Define XML (de)serialization methods for TObject.
   *
   *  Legibility of output depends on the dictionary, enabling schema evolution with '+'
   *  after the class name adds variable names. (likely to be better for compatibility, too)
   *
   *  Prototype, might be moved later on.
   */
  namespace Stream {
    /** Convert given TObject into an XML string.
     *
     *  Note that the returned string cannot be embedded into a normal XML file
     *  that can be used with Gearbox. Please pass it through escapeXML() first.
     */
    std::string serialize(const TObject* obj);

    /** Escape given XML string as CDATA sequence.
     *
     * This format is suitable for storing in an XML file, wrap it in a tag
     * and use Gearbox::getInstance().getObject(".../MyTag") to retrieve the
     * object again.
     *
     * */
    std::string escapeXML(const std::string& xmlString);

    /** Convert given serialized XML string back into TObject.
     *
     *  Returns a pointer to the deserialized object, might be NULL if conversion was impossible. User is responsible for deletion.
     */
    TObject* deserialize(const std::string& data);
  }
}
