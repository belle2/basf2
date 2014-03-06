#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** Test fixture. */
  class ParticleTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp() {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent();
      StoreArray<Particle>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown() {
      DataStore::Instance().reset();
    }
  };



  /** test some basics for extra information. */
  TEST_F(ParticleTest, ExtraInfo)
  {
    Particle p;
    //doesn't exist
    EXPECT_THROW(p.getExtraInfo("htns"), std::runtime_error);

    p.addExtraInfo("htns", 32.0);

    //cannot add it again
    EXPECT_THROW(p.addExtraInfo("htns", 1234.0), std::runtime_error);

    EXPECT_DOUBLE_EQ(32.0, p.getExtraInfo("htns"));

  }

  /** test string -> index mapping. */
  TEST_F(ParticleTest, ExtraInfoMap)
  {
    StoreObjPtr<ParticleExtraInfoMap> map;

    Particle p;
    p.print();
    p.addExtraInfo("htns", 1.0);
    EXPECT_EQ(1u, map->getNMaps());

    //adding another variable should just extend the current map
    p.addExtraInfo("somethingelse", 2.0);
    EXPECT_EQ(1u, map->getNMaps());

    //storing same things in another particle shouldn't create a new map
    Particle q;
    q.addExtraInfo("htns", 11.0);
    EXPECT_EQ(1u, map->getNMaps());
    q.addExtraInfo("somethingelse", 12.0);
    EXPECT_EQ(1u, map->getNMaps());

    //lets store something different in p and q
    p.addExtraInfo("thirdvar_p", 3.0);
    EXPECT_EQ(1u, map->getNMaps());
    //different var in 3rd place, needs new map
    q.addExtraInfo("thirdvar_q", 13.0);
    EXPECT_EQ(2u, map->getNMaps());


    //set same vars as with p and q again, one of them should switch maps
    Particle s;
    s.addExtraInfo("htns", 1.0);
    s.addExtraInfo("somethingelse", 2.0);
    s.addExtraInfo("thirdvar_p", 3.0);
    Particle t;
    t.addExtraInfo("htns", 1.0);
    t.addExtraInfo("somethingelse", 2.0);
    t.addExtraInfo("thirdvar_q", 3.0);
    //if switching works, there should still be the same number of maps
    EXPECT_EQ(2u, map->getNMaps());
    //verify switching worked
    EXPECT_THROW(s.getExtraInfo("thirdvar_q"), std::runtime_error);
    EXPECT_THROW(t.getExtraInfo("thirdvar_p"), std::runtime_error);

    //reusing maps shouldn't cause it to find unset vars (e.g. when one value is set, but map has more entries)
    Particle u;
    EXPECT_THROW(u.getExtraInfo("htns"), std::runtime_error);
    u.addExtraInfo("htns", 1.0);
    EXPECT_THROW(u.getExtraInfo("somethingelse"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_p"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_q"), std::runtime_error);
    u.addExtraInfo("somethingelse", 2.0);
    EXPECT_THROW(u.getExtraInfo("thirdvar_p"), std::runtime_error);
    EXPECT_THROW(u.getExtraInfo("thirdvar_q"), std::runtime_error);


    //add something else first, followed by variables we already used
    Particle v;
    v.addExtraInfo("first", 0.0);
    v.addExtraInfo("htns", 1.0);
    v.addExtraInfo("somethingelse", 2.0);
    v.addExtraInfo("thirdvar_q", 3.0);


    //verify the values we set
    EXPECT_DOUBLE_EQ(1.0, p.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, p.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, p.getExtraInfo("thirdvar_p"));

    EXPECT_DOUBLE_EQ(11.0, q.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(12.0, q.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(13.0, q.getExtraInfo("thirdvar_q"));

    EXPECT_DOUBLE_EQ(1.0, s.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, s.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, s.getExtraInfo("thirdvar_p"));

    EXPECT_DOUBLE_EQ(1.0, t.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, t.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, t.getExtraInfo("thirdvar_q"));

    EXPECT_DOUBLE_EQ(0.0, v.getExtraInfo("first"));
    EXPECT_DOUBLE_EQ(1.0, v.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, v.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, v.getExtraInfo("thirdvar_q"));

    //check data is copied with Particle
    Particle tCopy = t;
    EXPECT_DOUBLE_EQ(1.0, tCopy.getExtraInfo("htns"));
    EXPECT_DOUBLE_EQ(2.0, tCopy.getExtraInfo("somethingelse"));
    EXPECT_DOUBLE_EQ(3.0, tCopy.getExtraInfo("thirdvar_q"));
    tCopy.print();
  }

}  // namespace
