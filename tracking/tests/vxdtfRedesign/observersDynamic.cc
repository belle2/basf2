/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include "tracking/trackFindingVXD/observingTools/general/ObservableState.h"

using namespace std;
using namespace Belle2;


namespace VXDTFDynamicObserversTest {


  /** Test class demonstrating the behavior of ObservableState.
   *
   * ObservableState allows the user to observe and un-observe a given state.
   * */
  class ObserversDynamicTest : public ::testing::Test {
  protected:

    /** SetUp environment */
    virtual void SetUp() {}

    /** TearDown environment */
    virtual void TearDown() {}
  };



  /** classic function matching the std::function-pattern of ObservableState<double>.
   *
   * further infos, see test below.
   */
  void giveMeUpdate(double update)
  {
    B2INFO("giveMeUpdate was updated! new value: " << update);
  }


  /** small class which wants to be kept informed about changes of a value.
   *
   * The observed value may be stored in another class which is completely unknown to our mini class here.
   * No inheritance and similar dependencies needed.
   * Only drawback: we have to do the bookkeeping via storing an identifier
   * */
  template<typename ValueType>
  class FancyValueCarrier {

  protected:

    /** data member to count nChanges of Value */
    unsigned int m_countedChanges;


    /** value which shall be automatically updated */
    ValueType m_myVal;


    /** identifier - needed for bookkeeping.
     *
     * this value has to be stored so the State to be observed can be unsubsrcibed again.
     * */
    unsigned int m_identifier;


    /** protector so that the indentifier can not be rewritten */
    bool m_lockIdentifier;


  public:

    /** constructor */
    FancyValueCarrier() :
      m_countedChanges(0),
      m_myVal(0),
      m_identifier(std::numeric_limits<unsigned int>::max()),
      m_lockIdentifier(false) {}


    /** this is the member function matching the std::function-pattern of ObservableState<ValueType>.
     *
     * futher infos, see test below.
     * */
    void updateMyVal(ValueType newVal)
    {
      m_countedChanges++;
      B2INFO("ValueType::updateMyVal was updated (change revision " << m_countedChanges << ")! old value: " << get() << ", new value: " <<
             newVal);
      m_myVal = newVal;
      B2INFO("ValueType::updateMyVal: new value with get again: " << get());
    }


    /** simple getter to maintain encapsulation */
    ValueType get() const { return m_myVal; }


    /** how often our value was changed */
    ValueType getNChanges() const { return m_countedChanges; }


    /** WARNING dangerous! can only be set once! */
    void setIdentifier(unsigned int identifier)
    {
      if (m_lockIdentifier == true) {
        B2ERROR("FancyValueCarrier: identifier is locked, you can not reset it!");
        return;
      }
      m_lockIdentifier = true;
      m_identifier = identifier;
    }


    /** returns identifier to be able to delete subscription of connected value */
    unsigned int getIdentifier() const
    {
      if (m_lockIdentifier == false) {
        B2ERROR("FancyValueCarrier: fancy value is not yet locked to a ObservableState!");
      }
      return m_identifier;
    }
  };



  /** tests if it is possible to simply store the function-pointers to my observers instead of a direct pointer to them */
  TEST_F(ObserversDynamicTest, TestObserverPatternWithSTDbind)
  {
    // assigning value directly works
    ObservableState<double> testState = 42.;
    EXPECT_DOUBLE_EQ(42., testState.get());


    // adding a classic function as an observing function
    testState.addObserver(giveMeUpdate);
    testState.set(23.);
    EXPECT_DOUBLE_EQ(23., testState.get());
    EXPECT_EQ(1, testState.getNObservers());
    EXPECT_EQ(1, testState.getNObserversTotal());


    // creating instance of a class which wants to be kept informed:
    FancyValueCarrier<double> meSoFancy;
    EXPECT_DOUBLE_EQ(0., meSoFancy.get());


    // adding member of given instance to the observers and securing identifier to be able to delete subscription afterwards
    std::function<void(double)> boundMember = std::bind(
                                                &FancyValueCarrier<double>::updateMyVal,
                                                &meSoFancy,
                                                std::placeholders::_1);
    unsigned int identifier = testState.addObserver(boundMember);
    meSoFancy.setIdentifier(identifier);
    // WARNING std::bind takes pointer to given instance. Someone has to take care that the object does not get invalid during lifetime of testState!


    // meSoFancy-value was automatically synched with testState
    EXPECT_DOUBLE_EQ(testState.get(), meSoFancy.get());
    EXPECT_EQ(1, meSoFancy.getNChanges());


    testState.set(5.);
    EXPECT_DOUBLE_EQ(5., testState.get());
    EXPECT_DOUBLE_EQ(testState.get(), meSoFancy.get());
    EXPECT_EQ(2, testState.getNObservers());
    EXPECT_EQ(2, testState.getNObserversTotal());
    EXPECT_EQ(2, meSoFancy.getNChanges());


    //remove observer
    testState.removeObserver(meSoFancy.getIdentifier());
    EXPECT_EQ(1, testState.getNObservers());
    EXPECT_EQ(2, testState.getNObserversTotal());
    // classic function can not be removed if the connected identifier is not stored somehow


    //now FancyValueCarrier-instance is not updated any more:
    testState.set(3.14);
    EXPECT_EQ(2, meSoFancy.getNChanges());
    EXPECT_NE(testState.get(), meSoFancy.get());
  }

}
