/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


namespace Belle2 {
  namespace ECL {
    template <typename T>
    /**
     * class to replace POD 2D array to save stack usage
     * since it just allocates memory dynamically.
     */
    class WrapArray2D {
    public:
      /** replace POD 2D array  */
      WrapArray2D(int rows,  int cols) :
        m_data(new T[rows * cols]), m_ncols(cols)
      {}

      /** no copy */
      WrapArray2D(const WrapArray2D&) = delete;

      /** no assignment */
      WrapArray2D& operator=(const WrapArray2D&) = delete;

      /** destructor */
      ~WrapArray2D()
      {  delete [] m_data;  }

      /** return row*/
      T* operator[](int irow)
      {
        return m_data + irow * m_ncols;
      }

      /** return */
      operator T* () { return m_data; }
    private:

      T* m_data; /**< content of the 2d array */
      int m_ncols; /**< number of columns */
    };
  }
}
