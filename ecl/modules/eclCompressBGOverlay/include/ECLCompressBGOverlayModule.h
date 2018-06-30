/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: Alexei Sibidanov                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//Framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <ecl/digitization/ECLCompress.h>

namespace Belle2 {
  class ECLDsp;
  class ECLWaveforms;

  /** The ECLCompressBGOverlay module compresses recorded waveforms
   *  triggered by the random trigger and to use them in simulation to
   *  follow background conditions.
   */
  class ECLCompressBGOverlayModule : public Module {
  public:

    /** Constructor */
    ECLCompressBGOverlayModule();

    /** Destructor  */
    ~ECLCompressBGOverlayModule();

    /** Initialize variables  */
    void initialize();

    /** Compression happens here */
    void event();

    /** Cleanup variables  */
    void terminate();

  private:
    StoreArray<ECLDsp> m_eclDsps; /**< recorded waveforms */
    StoreObjPtr<ECLWaveforms> m_eclWaveforms; /**< compressed waveforms */

    ECL::ECLCompress* m_comp; /**< pointer to a compression object which do all work */

    /** Module parameters */
    unsigned int m_compAlgo; /**< compression algorithm for background waveforms */
    std::string m_eclWaveformsName; /**< name of background waveforms storage */
  };
}//Belle2
