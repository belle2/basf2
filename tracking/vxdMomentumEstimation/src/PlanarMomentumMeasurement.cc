#include <tracking/vxdMomentumEstimation/PlanarMomentumMeasurement.h>

#include <genfit/Exception.h>
#include <genfit/RKTrackRep.h>
#include <tracking/vxdMomentumEstimation/HMatrixQP.h>
#include <cassert>

using namespace genfit;

namespace Belle2 {

  const AbsHMatrix* PlanarMomentumMeasurement::constructHMatrix(const AbsTrackRep* rep) const
  {

    if (dynamic_cast<const RKTrackRep*>(rep) == NULL) {
      Exception exc("SpacepointMeasurement default implementation can only handle state vectors of type RKTrackRep!", __LINE__, __FILE__);
      throw exc;
    }

    return new HMatrixQP();
  }

} /* End of namespace genfit */
