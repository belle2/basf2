/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
