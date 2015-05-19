/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// fw:
#include <framework/logging/Logger.h>

// stl:
#include <string>


namespace Belle2 {

  /** Base class (functor) for storing an algorithm determining the data one wants to have */
  template <class DataType, class TCInfoType, class VectorType , typename ... otherTypes>
  class AnalyzingAlgorithmBase {
  protected:

    /** carries unique ID */
    std::string m_iD;

    /** stores the origin used for some calculations, can be set here */
    static VectorType m_origin;

    /** constructor used for inheriting classes */
    AnalyzingAlgorithmBase(std::string newID) : m_iD(newID)/*, m_origin(0,0,0)*/ {}

  public:

    /** constructor */
    AnalyzingAlgorithmBase() : m_iD("AnalyzingAlgorithmBase")/*, m_origin(0,0,0)*/ {}

    /** operator for comparison. */
    inline bool operator == (const AnalyzingAlgorithmBase& b) const { return m_iD == b.getID(); }

    /** returns unique ID */
    std::string getID() const { return m_iD; }

    /** returns current value for the origin */
    VectorType getOrigin() const { return m_origin; }

    /** set origin for all inherited classes */
    void setOrigin(VectorType newOrigin) const { m_origin = newOrigin ; }

    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(otherTypes ...)
    {
      B2ERROR(" AnalyzingAlgorithmBase::calcData: if you can see this, the code tried to return the actual baseClass instead of the inherited ones - this is unintended behavior! The TCs had the types: ")

      return DataType();
    }
  };


  /** setting the static origin to a standard value */
  template<class DataType, class TCInfoType, class VectorType, typename ... otherTypes>
  VectorType AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType, otherTypes ... >::m_origin = VectorType(0, 0, 0);


  /** non-memberfunction Comparison for equality with a std::string */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& a, std::string b)
  {
    return (a.getID() == b);
  }

  /** non-memberfunction Comparison for equality with a std::string */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (std::string a, const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& b)
  {
    return (a == b.getID());
  }

}
