#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {


  /** Test class for the KlId object.
   */
  class KlIdTest : public ::testing::Test {
  protected:
  };

  /** Test setter and getter. */
  TEST_F(KlIdTest, SettersAndGetters)
  {

    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclClusters;
    eclClusters.registerInDataStore();
    StoreArray<KLMCluster> klmClusters;
    klmClusters.registerInDataStore();
    StoreArray<KlId> klids;
    klids.registerInDataStore();
    klmClusters.registerRelationTo(klids);
    eclClusters.registerRelationTo(klids);


    const KlId* klid = klids.appendNew();
    const KLMCluster* klmCluster = klmClusters.appendNew();
    const ECLCluster* eclCluster = eclClusters.appendNew();

    EXPECT_FALSE(klid->isECL());
    EXPECT_FALSE(klid->isKLM());
    EXPECT_TRUE(std::isnan(klid->getKlId()));

    klmCluster->addRelationTo(klid, 0.5);
    eclCluster->addRelationTo(klid, 0.5);

    EXPECT_TRUE(klid->isECL());
    EXPECT_TRUE(klid->isKLM());
    EXPECT_EQ(0.5, klid->getKlId());

  }
}  // namespace
