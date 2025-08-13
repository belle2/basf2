/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

class TObject;

namespace Belle2 {
  /** Define (de)serialization methods for TObject.
   *
   *  This code is also exported to Python, after 'from ROOT import Belle2' it is available as Belle2.Stream.
   */
  namespace Stream {

    /** Convert given TObject into encoded byte stream (for storing in XML).
     *
     * Returns base64-encoded TMessage.
     */
    std::string serializeAndEncode(const TObject* obj);

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
