/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/BeamParametersModule.h>
#include <framework/database/Database.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamParameters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamParametersModule::BeamParametersModule() : Module()
{
  // Set module properties
  setDescription("Setting of beam parameters. This module allows to set the "
                 "beamparameters to be used by generators and analyis. One can "
                 "either select from a list of predefined parameter sets or "
                 "manually set the parameters to use.");

  // Parameter definitions
  addParam("energyHER", m_energyHER, "Energy of the High Energy Ring", 7.0);
  addParam("angleHER", m_angleHER, "angle of the High Energy Ring. "
           "Negative values are interpreted as pi - value", 0.0415);
  addParam("covHER", m_covHER, "covariance matrix of the High Energy Ring. This "
           "can either be one value which is the squared energy spread, three "
           "values corresponding to the variances of energy and the horizontal "
           "and vertial spread (E, theta_x, theta_y) or a full covariance "
           "matrix with 9 elements", std::vector<double>(1, 0));
  addParam("energyLER", m_energyLER, "Energy of the Low Energy Ring", 4.0);
  addParam("angleLER", m_angleLER, "angle of the Low Energy Ring. "
           "Negative values are interpreted as pi - value", -0.0415);
  addParam("covLER", m_covLER, "covariance matrix of the Low Energy Ring. This "
           "can either be one value which is the squared energy spread, three "
           "values corresponding to the variances of energy and the horizontal "
           "and vertial spread (E, theta_x, theta_y) or a full covariance "
           "matrix with 9 elements", std::vector<double>(1, 0));
  addParam("vertex", m_vertex, "nominal vertex position", std::vector<double>(3, 0));
  addParam("covVertex", m_covVertex, "covariance matrix of the nominal vertex "
           "position. Can be either one value which is the common variance for "
           "all directions, three values for the variance of x, y and z or a "
           "full covariance matrix", std::vector<double>(1, 0));
  addParam("smearEnergy", m_smearEnergy, "if true, smear energy when generating "
           "initial events", true);
  addParam("smearDirection", m_smearDirection, "if true, smear beam direction "
           "when generating initial events", true);
  addParam("smearVertex", m_smearVertex, "if true, smear vertex position when "
           "generating initial events", true);
  addParam("generateCMS", m_generateCMS, "if true, generate events in CMS, not "
           "lab system", false);
}

void BeamParametersModule::initialize()
{
  StoreObjPtr<BeamParameters> beamParams("", DataStore::c_Persistent);
  beamParams.registerInDataStore();
  beamParams.create();
  beamParams->setHER(m_energyHER, m_angleHER, m_covHER);
  beamParams->setLER(m_energyLER, m_angleLER, m_covLER);
  TVector3 vertex;
  if (m_vertex.size() == 3) {
    vertex.SetXYZ(m_vertex[0], m_vertex[1], m_vertex[2]);
  } else {
    B2ERROR("Vertex position needs to have 3 entries");
  }
  beamParams->setVertex(vertex, m_covVertex);
  int flags = 0;
  if (m_generateCMS) flags |= BeamParameters::c_generateCMS;
  if (m_smearEnergy) flags |= BeamParameters::c_smearBeamEnergy;
  if (m_smearDirection) flags |= BeamParameters::c_smearBeamDirection;
  if (m_smearVertex) flags |= BeamParameters::c_smearVertex;
  beamParams->setGenerationFlags(flags);

  //BeamParameter params;
  //IntervalOfValidity iov(1, 1);
  //Database::Instance().storeData("BeamParameter", &params, iov);
}
