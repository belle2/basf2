
// class to replace POD 2D array to save stack usage
// since it just allocates memory dynamically.

namespace Belle2 {
  namespace ECL {
    template <typename T>
    class WrapArray2D {
    public:
      /** replace POD 2D array  */
      WrapArray2D(int rows,  int cols) :
        m_data(new T[rows * cols]), m_ncols(cols)
      {}

      /** copy */
      WrapArray2D(const WrapArray2D& obj)
      {
        m_data = new T;
        *m_data = *obj.m_data;
        m_ncols = obj.m_ncols;
      }

      /** destructor */
      ~WrapArray2D()
      {  delete [] m_data;  }

      /** return row*/
      T* operator[](int irow)
      {
        return m_data + irow * m_ncols;
      }

      /** = assign operator */
      T& operator=(const T& x)
      {
        *m_data = x.m_data;
        return *this;
      }

      /** return */
      operator T* () { return m_data; }
    private:

      T* m_data; /**< content of the 2d array */
      int m_ncols; /**< number of columns */
    };
  }
}
