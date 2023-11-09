/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <klm/dbobjects/KLMLikelihoodParameters.h>
#include <klm/muid/MuidElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/dataobjects/KLMHit2d.h>

#include <klm/bklm/geometry/GeometryPar.h>

#include <klm/muidgnn/MuidBuilder_fixed.h>

#include <string>

namespace Belle2 {
  /**
   * Get information from KLMMuidLikelihood
   */
  class KLMMuIDGetterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    KLMMuIDGetterModule();

    /**  */
    void initialize() override;

    /** n */
    void event() override;

    /** n */
    void getStripPosition();

  private:

    std::string  m_inputListName;

    DBObjPtr<KLMLikelihoodParameters> m_LikelihoodParameters;

    StoreArray<KLMMuidLikelihood> m_klmMuidLikelihoods;

    StoreArray<KLMHit2d> m_klmHit2ds;

    std::unique_ptr<MuidBuilder_fixed> mubuilder;
    std::unique_ptr<MuidBuilder_fixed> pibuilder;

    bklm::GeometryPar* m_bklmGeoPar;

    int get_binary(int number, int position)
    {
      if (number & (1 << position)) {
        return 1;
      } else {
        return 0;
      }
    }


  };
}
