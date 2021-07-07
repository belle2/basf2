/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TRandom.h>
#include <TObject.h>

namespace Belle2 {
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
