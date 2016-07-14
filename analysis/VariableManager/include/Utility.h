#pragma once

#include <analysis/VariableManager/Manager.h>
#include <framework/utilities/GeneralCut.h>

namespace Belle2 {

  class Particle;

  namespace Variable {
    /// A cut in the analysis package is a general cut using the analysis variable manager.
    using Cut = GeneralCut<Belle2::Variable::Manager>;
  }
}
