#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/TestHelpers.h>

#include <analysis/utility/ParticleCopy.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::ParticleCopy;

namespace {
  /** Test fixture. */
  class KalmanTest : public ::testing::Test {
  protected:
    /** setup datastore if needed */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

}  // namespace
