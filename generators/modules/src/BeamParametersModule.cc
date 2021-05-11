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
  addParam("angleXHER", m_angleXHER, "angle of the High Energy Ring in the x-z plane. "
           "Negative values are interpreted as pi - value", 0.0415);
  addParam("angleYHER", m_angleYHER, "angle of the High Energy Ring in the y-z plane. "
           "Negative values are interpreted as pi - value", 0.0);
  addParam("covHER", m_covHER, "covariance matrix of the High Energy Ring. This "
           "can either be one value which is the squared energy spread, three "
           "values corresponding to the variances of energy and the horizontal "
           "and vertial spread (E, theta_x, theta_y) or a full covariance "
           "matrix with 9 elements", std::vector<double>(1, 0));
  addParam("energyLER", m_energyLER, "Energy of the Low Energy Ring", 4.0);
  addParam("angleXLER", m_angleXLER, "angle of the Low Energy Ring in the x-z plane. "
           "Negative values are interpreted as pi - value", -0.0415);
  addParam("angleYLER", m_angleYLER, "angle of the Low Energy Ring in the y-z plane. "
           "Negative values are interpreted as pi - value", 0.0);
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
  addParam("createPayload", m_createPayload, "if true create a database payload "
           "with the iov given with the payloadIov parameter", false);
  addParam("payloadIov", m_payloadIov, "iov of the payload to be created. List "
           "of four numbers: first experiment, first run, last experiment, "
           "last run", m_payloadIov);
}

void BeamParametersModule::initialize()
{
  m_beamParams.setHER(m_energyHER, m_angleXHER, m_angleYHER, m_covHER);
  m_beamParams.setLER(m_energyLER, m_angleXLER, m_angleYLER, m_covLER);
  TVector3 vertex;
  if (m_vertex.size() == 3) {
    vertex.SetXYZ(m_vertex[0], m_vertex[1], m_vertex[2]);
  } else {
    B2ERROR("Vertex position needs to have 3 entries");
  }
  m_beamParams.setVertex(vertex, m_covVertex);
  int flags = 0;
  if (m_generateCMS) flags |= BeamParameters::c_generateCMS;
  if (m_smearEnergy) flags |= BeamParameters::c_smearBeamEnergy;
  if (m_smearDirection) flags |= BeamParameters::c_smearBeamDirection;
  if (m_smearVertex) flags |= BeamParameters::c_smearVertex;
  m_beamParams.setGenerationFlags(flags);

  // Now make sure the database interface always returns the values we just set
  DBStore::Instance().addConstantOverride("BeamParameters", new BeamParameters(m_beamParams));

  if (m_createPayload) {
    IntervalOfValidity iov(m_payloadIov[0], m_payloadIov[1], m_payloadIov[2], m_payloadIov[3]);
    Database::Instance().storeData(&m_beamParams, iov);
  }
}
