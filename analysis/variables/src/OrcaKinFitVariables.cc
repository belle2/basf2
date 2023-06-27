/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/OrcaKinFitVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    // Chi2 and Prob -------------------------------------------
    double OrcaKinFitChi2(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitChi2")) return part->getExtraInfo("OrcaKinFitChi2");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitChi2' not found!");
        return Const::doubleNaN;
      }
    }

    double OrcaKinFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitProb")) return part->getExtraInfo("OrcaKinFitProb");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitProb' not found!");
        return Const::doubleNaN;
      }
    }

    double ErrorCode(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitErrorCode")) return part->getExtraInfo("OrcaKinFitErrorCode");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitErrorCode' not found!");
        return Const::doubleNaN;
      }
    }


    // For 1C fit ------------------------------------------------------------
    double UnmeasuredTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredTheta' not found!");
        return Const::doubleNaN;
      }
    }

    double UnmeasuredPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredPhi' not found!");
        return Const::doubleNaN;
      }
    }

    double UnmeasuredE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredE")) return part->getExtraInfo("OrcaKinFitUnmeasuredE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredE' not found!");
        return Const::doubleNaN;
      }
    }

    double UnmeasuredErrorTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorTheta' not found!");
        return Const::doubleNaN;
      }
    }

    double UnmeasuredErrorPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorPhi' not found!");
        return Const::doubleNaN;
      }
    }

    double UnmeasuredErrorE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorE")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorE' not found!");
        return Const::doubleNaN;
      }
    }


    VARIABLE_GROUP("OrcaKinFit");
    REGISTER_VARIABLE("OrcaKinFitChi2", OrcaKinFitChi2, R"DOC(
The :math:`\chi^2` returned by the Orca kinematic fitter.
Returns NaN if Orca was not run.

.. warning:: this is the raw :math:`\chi^2`, not over ndf.

.. seealso:: :ref:`kinfit`
        )DOC");
    REGISTER_VARIABLE("OrcaKinFitProb", OrcaKinFitProb, R"DOC(
The p-value returned by the Orca kinematic fitter.
Returns NaN if Orca was not run.
        
.. seealso:: :ref:`kinfit`
        )DOC");
    // TODO: explain the actual error codes
    REGISTER_VARIABLE("OrcaKinFitErrorCode", ErrorCode, R"DOC(
The error code returned by the Orca kinematic fitter.
Returns NaN if Orca was not run.
        
.. seealso:: :ref:`kinfit`
        )DOC");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredTheta", UnmeasuredTheta, R"DOC(
The polar angle of the "unmeasured photon" returned by Orca kinematic fitter for specific 1C fits with the ``addUnmeasuredPhoton`` parameter set.
I.e. for fits with sufficient constraints remaining to constrain a missing 3-vector.
Returns NaN if Orca was not run or if the ``addUnmeasuredPhoton`` parameter was not set.
        
.. seealso:: :ref:`kinfit`

)DOC", "rad");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredPhi", UnmeasuredPhi, R"DOC(
The azimuthal angle of the "unmeasured photon" returned by Orca kinematic fitter for specific 1C fits with the ``addUnmeasuredPhoton`` parameter set.
I.e. for fits with sufficient constraints remaining to constrain a missing 3-vector.
Returns NaN if Orca was not run or if the ``addUnmeasuredPhoton`` parameter was not set.
        
.. seealso:: :ref:`kinfit`

)DOC", "rad");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredE", UnmeasuredE, R"DOC(
The energy of the "unmeasured photon" returned by Orca kinematic fitter for specific 1C fits with the ``addUnmeasuredPhoton`` parameter set.
I.e. for fits with sufficient constraints remaining to constrain a missing 3-vector.
Returns NaN if Orca was not run or if the ``addUnmeasuredPhoton`` parameter was not set.
        
.. seealso:: :ref:`kinfit`

)DOC", "GeV");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorTheta", UnmeasuredErrorTheta,
                      "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredTheta`.\n\n", "rad");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorPhi", UnmeasuredErrorPhi, "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredPhi`.\n\n",
                      "rad");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorE", UnmeasuredErrorE, "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredE`.\n\n", "GeV");
  }
}
