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
#ifndef KlId_H
#define KlId_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** KlId datastore object to store results from KlId calculations.
   * Note that the klid id is stored as the weight between the cluster and this object */
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

    ClassDef(KlId,
             2); /**< K_L0 ID object. The actual ID is stored as the weight of a relation to this object. The klongID is a classfier ouput. This means it is a frequency relative to the Klong fraction of the sample the id is trained on and the performance of the classification. */
  };
}
#endif
