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
#include <framework/core/FrameworkExceptions.h>

// tracking:
#include<tracking/trackFindingVXD/analyzingTools/TCType.h>
#include<tracking/trackFindingVXD/analyzingTools/AlgoritmType.h>

// stl:
#include <string>


namespace Belle2 {


  /** Base class for storing an algorithm determining the data one wants to have */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmBase {
  protected:

    /** minimal struct for keeping track which tc is which */
    struct TcPair {
      /** standard constructor sets nullptr-ptrs. */
      TcPair() : refTC(nullptr), testTC(nullptr) {}

      /** constructor sets valid values */
      TcPair(const TCInfoType& aRefTC, const TCInfoType& aTestTC) : refTC(&aRefTC), testTC(&aTestTC) {}

      /** here the reference TC will be stored */
      const TCInfoType* refTC;

      /** here the TC to be tested will be stored */
      const TCInfoType* testTC;
    };


    /** carries unique ID */
    AlgoritmType::Type m_iD;


    /** stores the origin used for some calculations, can be set here */
    static VectorType s_origin;


    /** if true, for testTC the values of attached refTC will be stored instead of own values.
     *
     * - why are there values of the mcTC stored?
     * we want to know the real data, not the guesses of the reconstructed data.
     * Deviations of reference values to guesses of the reconstructed data will be stored in resiudals anyway.
     */
    static bool m_storeRefTCDataForTestTC;


    /** constructor used for inheriting classes */
    explicit AnalyzingAlgorithmBase(const AlgoritmType::Type& newID) : m_iD(newID) {}

    /** constructor used for inheriting classes */
    explicit AnalyzingAlgorithmBase(const std::string& newID) : m_iD(AlgoritmType::getTypeEnum(newID)) {}

    /** copy constructor should not be used so delete it*/
    AnalyzingAlgorithmBase(const AnalyzingAlgorithmBase& algo) = delete;
    /** also assignement constructor should not be used */
    AnalyzingAlgorithmBase& operator = (const AnalyzingAlgorithmBase& algo) = delete;


    /** virtual class to determine the correct TC to be used for algorithm calculation.
    *
    * - throws exeption if there are problems.
    * More explanations @ m_storeRefTCDataForTestTC.
    */
    virtual const TCInfoType& chooseCorrectTC(const TCInfoType& aTC) const
    {
      // capture cases of aTC == referenceTC first:
      if (TCType::isReference(aTC.tcType)) { return aTC; }

      // is no reference TC and own data usage is allowed:
      if (m_storeRefTCDataForTestTC == false) { return aTC; }

      // handle cases when attached reference TC has to be used instead of own data:
      if (aTC.assignedTC != nullptr) { return *aTC.assignedTC; }

      throw AnalyzingAlgorithmBase::No_refTC_Attached();
    }


    /** makes sure that TcPair.refTC and .testTC are correctly set - throws exeption if there are problems */
    virtual const TcPair chooseCorrectPairOfTCs(const TCInfoType& aTC) const
    {
      // capture bad case, where second TC is missing:
      if (aTC.assignedTC == nullptr) { throw AnalyzingAlgorithmBase::No_refTC_Attached(); }

      if (aTC.tcType == TCType::Reference or aTC.tcType == TCType::Lost) {
        return TcPair(aTC, *aTC.assignedTC);
      }
      return TcPair(*aTC.assignedTC, aTC);
    }

  public:

    /** this exception is thrown if m_storeRefTCDataForTestTC is true and no refTC could be found */
    BELLE2_DEFINE_EXCEPTION(No_refTC_Attached,
                            "To given testTC no refTC was attached, could not provide valid data for algorithm - no value returned!");


    /** constructor */
    AnalyzingAlgorithmBase() : m_iD(AlgoritmType::AnalyzingAlgorithmBase) {}


    /** virtual destructor - derived classes need to write their own destructors if any other data members are added. */
    virtual ~AnalyzingAlgorithmBase() {}


    /** operator for comparison. */
    inline bool operator == (const AnalyzingAlgorithmBase& b) const { return m_iD == b.getID(); }


    /** returns unique ID */
    AlgoritmType::Type getID() const { return m_iD; }

    /** returns unique ID as a string */
    std::string getIDName() const { return AlgoritmType::getTypeName(m_iD); }

    /** returns current value for the origin */
    static VectorType& getOrigin() { return s_origin; }


    /** set origin for all inherited classes */
    static void setOrigin(VectorType newOrigin) { s_origin = newOrigin; }


    /** returns current choice for behavior of algorithms in terms of storing reference or testData for successfully matched TCs */
    static bool willRefTCdataBeUsed4TestTCs() { return m_storeRefTCDataForTestTC; }


    /** set behavior of algorithms in terms of storing reference or testData for successfully matched TCs */
    static void setWillRefTCdataBeUsed4TestTCs(bool newBehavior) { m_storeRefTCDataForTestTC = newBehavior; }


    /** virtual class to calculate data. takes two TCInfos */
    virtual DataType calcData(const TCInfoType&)
    {
      B2ERROR(" AnalyzingAlgorithmBase::calcData: if you can see this, the code tried to return the actual baseClass instead of the inherited ones - this is unintended behavior!");
      return DataType();
    }
  };


  /** setting the static origin to a standard value */
  template<class DataType, class TCInfoType, class VectorType>
  VectorType AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::s_origin = VectorType(0, 0, 0);


  /** setting the static storeRefTCDataForTestTC to a standard value */
  template<class DataType, class TCInfoType, class VectorType>
  bool AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::m_storeRefTCDataForTestTC = false;


  /** non-memberfunction Comparison for equality with a std::string */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& a, const std::string& b)
  { return (a.getIDName() == b); }


  /** non-memberfunction Comparison for equality with a std::string */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (const std::string& a, const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& b)
  { return (a == b.getIDName()); }


  /** non-memberfunction Comparison for equality with a AlgoritmType::Type */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& a, const AlgoritmType::Type& b)
  { return (a.getID() == b); }


  /** non-memberfunction Comparison for equality with a AlgoritmType::Type */
  template <class DataType, class TCInfoType, class VectorType>
  inline bool operator == (const AlgoritmType::Type& a, const AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>& b)
  { return (a == b.getID()); }

}
