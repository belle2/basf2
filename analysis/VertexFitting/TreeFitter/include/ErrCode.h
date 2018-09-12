/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace TreeFitter {

  /** abstract errorocode be aware that the default is succes */
  class ErrCode {
  public:
    /** some enums to store errors */
    enum Status {success = 0,
                 pocafailure = 1,
                 baddistance = 2,
                 inversionerror = 4,
                 badsetup = 8,
                 divergingconstraint = 16,
                 slowdivergingfit = 32,
                 fastdivergingfit = 64,
                 filtererror = 128,
                 photondimerror = 256,
                 klongdimerror = 512
                } ;

    /** default constructor */
    ErrCode() : m_flag(success) {}

    /** constructor */
    explicit ErrCode(Status flag) : m_flag(flag) {}

    /** operator */
    const ErrCode& operator|=(const ErrCode& rhs)
    {
      m_flag |= rhs.m_flag ; return *this ;
    }

    /** operator */
    bool operator==(const ErrCode& rhs) const
    {
      return m_flag == rhs.m_flag ;
    }

    /** operator */
    bool operator==(const ErrCode::Status& rhs) const
    {
      return *this == ErrCode(rhs) ;
    }

    /** reset the errorcode to default (success!) */
    void reset() { m_flag = success ; }

    /** returns true if errorcode is error */
    bool failure() const { return m_flag != success ; }

    /** get errorcode */
    unsigned int flag() const { return m_flag ; }
  private:

    /** storing the errorcode */
    unsigned int m_flag ;

  } ;
}
