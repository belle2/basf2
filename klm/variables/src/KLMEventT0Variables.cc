/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Basf2 headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <limits>
#include <optional>
#include <vector>

namespace Belle2 {
  namespace Variable {

    //! @returns the KLM entry that KLMEventT0Estimator added to EventT0, if there is one
    std::optional<EventT0::EventT0Component> getKLMEventT0Component()
    {
      StoreObjPtr<EventT0> eventT0;
      if (!eventT0.isValid())
        return std::nullopt;
      // getTemporaryEventT0s(detector) returns a copy, so the entry is returned by value
      const std::vector<EventT0::EventT0Component> klmT0s = eventT0->getTemporaryEventT0s(Const::KLM);
      if (klmT0s.empty())
        return std::nullopt;
      return klmT0s.back();
    }

    //! @returns the event T0 measured with the KLM in ns
    double klmEventT0(const Particle*)
    {
      const std::optional<EventT0::EventT0Component> klmT0 = getKLMEventT0Component();
      if (!klmT0)
        return std::numeric_limits<double>::quiet_NaN();
      return klmT0->eventT0;
    }

    //! @returns the uncertainty of the event T0 measured with the KLM in ns
    double klmEventT0Uncertainty(const Particle*)
    {
      const std::optional<EventT0::EventT0Component> klmT0 = getKLMEventT0Component();
      if (!klmT0)
        return std::numeric_limits<double>::quiet_NaN();
      return klmT0->eventT0Uncertainty;
    }

    VARIABLE_GROUP("KLM event T0 calibration");
    REGISTER_VARIABLE("klmEventT0", klmEventT0, R"DOC(
**[Calibration][Eventbased]** Returns the event time measured with the KLM: the KLM entry that ``KLMEventT0Estimator``
adds to the ``EventT0`` object, alongside the values of the other detectors.
Available on cDST only. Requires a KLM module in the path, for example ``KLMEventT0Estimator``; NaN if
``KLMEventT0Estimator`` did not run.
)DOC", "ns");
    REGISTER_VARIABLE("klmEventT0Uncertainty", klmEventT0Uncertainty, R"DOC(
**[Calibration][Eventbased]** Returns the uncertainty of :b2:var:`klmEventT0`, propagated from the calibrated per-hit
time resolution. Requires a KLM module in the path, as :b2:var:`klmEventT0` does; NaN if :b2:var:`klmEventT0` is NaN.
)DOC", "ns");

  }
}
