
// class to replace POD 2D array to save stack usage
// since it just allocates memory dynamically.

namespace Belle2 {
  namespace ECL {
    template <typename T>
    class WrapArray2D {
    public:
      WrapArray2D(int rows,  int cols) :
        m_data(new T[rows * cols]), m_ncols(cols)
      {}

      WrapArray2D(const WrapArray2D& obj)
      {
        m_data = new T;
        *m_data = *obj.m_data;
        m_ncols = obj.m_ncols;
      }

      ~WrapArray2D()
      {  delete [] m_data;  }

      T* operator[](int irow)
      {
        return m_data + irow * m_ncols;
      }

      T& operator=(const T& x)
      {
        *m_data = x.m_data;
        return *this;
      }

      operator T* () { return m_data; }
    private:

      T* m_data;
      int m_ncols;
    };
  }
}
