/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TRandom.h>
#include <TObject.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Storable object which can be filled with random chunk data of a certain size.
     * Can be used for measuring the streaming behavior of a certain multiprocessing implementation
     **/
    class TestChunkData : public TObject {
    public:
      /// Generate chunk data with (size // 8) bytes.
      TestChunkData(unsigned int size = 0)
      {
        m_chunkData.reserve(size);
        for (unsigned int i = 0; i < size; i++) {
          m_chunkData.push_back(gRandom->Rndm());
        }
      }

    private:
      /// Storage for the chunk data
      std::vector<double> m_chunkData;

      /** Making this class a ROOT class.*/
      ClassDef(TestChunkData, 1);
    };
  }
}
