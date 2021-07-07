/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Klong identifcation (KlId) datastore object to store results from KlId calculations.
   * Note that the KlId is stored as the weight between the KLMCluster and this object.
   */
  class KlId : public RelationsObject {

  public:

    /** constructor */
    KlId() {};

    /** destructor */
    virtual ~KlId() {};

    /** is this ID originally a KLM Cluster ?*/
    bool isKLM() const;

    /** is this ID originally a ECL Cluster ?*/
    bool isECL() const;

    /** get the klong classifier output  */
    double getKlId() const;

  private:

    /** ClassDef. */
    ClassDef(KlId, 2);
  };
}
