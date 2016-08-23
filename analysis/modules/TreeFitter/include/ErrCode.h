#ifndef ERRCODE_H
#define ERRCODE_H

#include <iostream>
//#include <framework/logging/Logger.h> //FT: included only for the sake of debugging

namespace TreeFitter {

  class ErrCode {
  public:
    enum Status {success = 0,
                 pocafailure = 1,
                 baddistance = 2,
                 inversionerror = 4,
                 badsetup = 8,
                 divergingconstraint = 16,
                 slowdivergingfit = 32,
                 fastdivergingfit = 64,
                 filtererror = 128
                } ;

    ErrCode() : m_flag(success) {}

    ErrCode(Status flag) : m_flag(flag) {}

    const ErrCode& operator|=(const ErrCode& rhs)
    {
      m_flag |= rhs.m_flag ; return *this ;
    }

    bool operator==(const ErrCode& rhs) const
    {
      return m_flag == rhs.m_flag ;
    }

    bool operator==(const ErrCode::Status& rhs) const
    {
      return *this == ErrCode(rhs) ;
    }

    void reset() { m_flag = success ; }
    bool failure() const { return m_flag != success ; }

    unsigned int flag() const { return m_flag ; }
  private:
    unsigned int m_flag ;
  } ;

  std::ostream& operator<<(std::ostream& os, const ErrCode& code) ;

}

#endif //ERRCODE_H
