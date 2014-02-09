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
  /** Define (de)serialization methods for TObject.
   *
   *  For XML, legibility of output depends on the dictionary, enabling schema evolution with '+'
   *  after the class name adds variable names. (likely to be better for compatibility, too)
   *
   *  This code is also exported to Python, after 'from ROOT import Belle2' it is available as Belle2.Stream.
   */
  namespace Stream {
    /** Convert given TObject into an XML string.
     *
     *  Note that the returned string cannot be embedded into a normal XML file
     *  that can be used with Gearbox. Please pass it through escapeXML() first.
     */
    std::string serializeXML(const TObject* obj);

    /** Convert given TObject into encoded byte stream (for storing in XML).
     *
     * Returns base64-encoded TMessage.
     */
    std::string serializeAndEncode(const TObject* obj);


    /** Escape given XML string as CDATA sequence.
     *
     * This format is suitable for storing in an XML file, wrap it in a tag
     * and use Gearbox::getInstance().getTObject(".../MyTag") to retrieve the
     * object again.
     *
     * You don't need to call this on the output of serializeAndEncode(),
     * which is already safe.
     */
    std::string escapeXML(const std::string& xmlString);

    /** Convert given serialized XML string back into TObject.
     *
     *  Returns a pointer to the deserialized object, might be NULL if conversion was impossible.
     *  User is responsible for deletion.
     */
    TObject* deserializeXML(const std::string& data);

    /** Convert given serialized raw data back into TObject.
     *
     *  Returns a pointer to the deserialized object, might be NULL if conversion was impossible.
     *  User is responsible for deletion.
     *
     *  If input is not well-formed base64-encoded data, this might crash.
     */
    TObject* deserializeEncodedRawData(const std::string& base64Data);
  }
}
