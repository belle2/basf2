/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/OrcaKinFitVariables.h>

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
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double OrcaKinFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitProb")) return part->getExtraInfo("OrcaKinFitProb");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitProb' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double ErrorCode(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitErrorCode")) return part->getExtraInfo("OrcaKinFitErrorCode");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitErrorCode' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }


    // For 1C fit ------------------------------------------------------------
    double UnmeasuredTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredTheta' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double UnmeasuredPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredPhi' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double UnmeasuredE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredE")) return part->getExtraInfo("OrcaKinFitUnmeasuredE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredE' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double UnmeasuredErrorTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorTheta' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double UnmeasuredErrorPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorPhi' not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double UnmeasuredErrorE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorE")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorE' not found!");
        return std::numeric_limits<double>::quiet_NaN();
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
    // TODO: explan the actual error codes
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
        )DOC");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredPhi", UnmeasuredPhi, R"DOC(
The azimuthal angle of the "unmeasured photon" returned by Orca kinematic fitter for specific 1C fits with the ``addUnmeasuredPhoton`` parameter set.
I.e. for fits with sufficient constraints remaining to constrain a missing 3-vector.
Returns NaN if Orca was not run or if the ``addUnmeasuredPhoton`` parameter was not set.
        
.. seealso:: :ref:`kinfit`
        )DOC");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredE", UnmeasuredE, R"DOC(
The energy of the "unmeasured photon" returned by Orca kinematic fitter for specific 1C fits with the ``addUnmeasuredPhoton`` parameter set.
I.e. for fits with sufficient constraints remaining to constrain a missing 3-vector.
Returns NaN if Orca was not run or if the ``addUnmeasuredPhoton`` parameter was not set.
        
.. seealso:: :ref:`kinfit`
        )DOC");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorTheta", UnmeasuredErrorTheta,
                      "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredTheta`.");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorPhi", UnmeasuredErrorPhi, "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredPhi`.");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorE", UnmeasuredErrorE, "The uncertainty on :b2:var:`OrcaKinFitUnmeasuredE`.");

  }
}
