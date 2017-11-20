/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <cstddef>
#include <cstdlib>

namespace Belle2 {

  /**
   * Class to provide a constant access time memory pool for one kind of objects.
   *
   * It is similar to TClonesArray but uses Templates and is also suited for non
   * TObject classes. The Memory will be allocated in chunks, each large enough
   * to hold chunkSize objects of type T.
   *
   * This class will not initialize the memory, it is the responsibility of the
   * user to call new(pool.add()) T(...) when adding an element to initialize the
   * memory. This is required to make the Pool usable for classes without default
   * constructor
   */
  template < class T, int chunkSize = 128 > class MemoryPool {

  public:
    /**
     * Constructor.
     * @param n Reserve space for n elements on construction
     */
    explicit MemoryPool(int n = 0): m_entries(0) { reserve(n); }

    /** Free allocated memory. */
    ~MemoryPool() { release_memory(); }

    /**
     * Return number of elements currently stored.
     * @return The number of elements currently stored
     */
    size_t size() const { return m_entries; }

    /**
     * Return pointer to memory segment for element i, no range check.
     * @param i The index of the memory segment.
     * @return Pointer to the memory segment.
     */
    T* operator[](size_t i) { return m_chunks[i / chunkSize] + i % chunkSize; }

    /**
     * Return pointer to memory segment for element i, including range check.
     * @param i The index of the memory segment.
     * @return Pointer to the memory segment.
     */
    T* at(size_t i) { return (i >= m_entries) ? 0 : (*this)[i]; }

    /**
     * Clear number of entries, does not free memory or call destructors.
     */
    void clear() { m_entries = 0; }

    /**
     * Returns an pointer to the next free memory segment, allocating additional memory if necessary.
     * @return Pointer to the next free memory segment.
     */
    T* add()
    {
      if (m_entries / chunkSize >= m_chunks.size()) {
        m_chunks.push_back(reinterpret_cast<T*>(malloc(chunkSize * sizeof(T))));
      }
      return (*this)[m_entries++];
    }

    /**
     * Make sure there is enough memory allocated for n elements.
     * @param n The number of elements for which memory should be made available.
     */
    void reserve(size_t n)
    {
      size_t needed_chunks = n / chunkSize + 1;
      m_chunks.reserve(needed_chunks);
      while (m_chunks.size() < needed_chunks) {
        m_chunks.push_back(reinterpret_cast<T*>(malloc(chunkSize * sizeof(T))));
      }
    }

    /**
     * Release all allocated memory, called automatically upon destruction.
     */
    void release_memory()
    {
      for (T* ptr : m_chunks) {
        free(reinterpret_cast<void*>(ptr));
      }
      m_chunks.clear();
    }

  protected:

    size_t m_entries; /**< Number of occupied entries. */
    std::vector<T*> m_chunks; /**< Pointers to all allocated memory chunks. */
  };

} //end namespace Belle2
