/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2::Conditions {
  /** Simple struct to group all information necessary for a single payload */
  struct PayloadMetadata {
    /** Constructor to request a payload with a given name and indicate whether non-existence is ok */
    explicit PayloadMetadata(const std::string& _name, bool _required = true): name{_name}, required{_required} {}
    /** Constructor for all the information which is usually obtained from providers. "required" and "filename" are not part of this as they get filled in later steps or from other parties */
    PayloadMetadata(const std::string& _name, const std::string& _globaltag, const std::string& _payloadUrl,
                    const std::string& _baseUrl, const std::string& _checksum, int firstExp, int firstRun, int finalExp, int finalRun,
                    int _revision): name(_name), globaltag(_globaltag), payloadUrl(_payloadUrl), baseUrl(_baseUrl), checksum(_checksum),
      iov{firstExp, firstRun, finalExp, finalRun}, revision(_revision) {}
    /** Update information from another instance but keep the "required" flag untouched */
    void update(const PayloadMetadata& other)
    {
      bool r{required};
      (*this) = other;
      required = r;
    }
    /** Name of the Payload */
    std::string name;
    /** Name of the globaltag this payload was taken from */
    std::string globaltag{""};
    /** full filename to the payload file */
    std::string filename{""};
    /** logical filename to the payload */
    std::string payloadUrl{""};
    /** base url if download is necessary */
    std::string baseUrl{""};
    /** digest (checksum) of the payload */
    std::string checksum{""};
    /** the interval of validity */
    IntervalOfValidity iov;
    /** the revision of the payload */
    size_t revision{0};
    /** if true a missing payload will not trigger an error */
    bool required{true};
  };
} // Belle2::Conditions namespace
