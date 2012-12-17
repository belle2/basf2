/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

using namespace Belle2;

ClassImp(Belle2::EKLMDigit);

EKLMDigit::EKLMDigit(const EKLMSimHit* hit)
  : EKLMHitBase((EKLMHitBase)(*hit)),
    m_Plane(hit->getPlane()),
    m_Strip(hit->getStrip()),
    m_NumberPhotoElectrons(-1),
    m_MCtime(hit->getTime()),
    m_volid(hit->getVolumeID())
{
}

double EKLMDigit::getNumberPhotoElectrons() const
{
  return m_NumberPhotoElectrons;
}

void EKLMDigit::setNumberPhotoElectrons(double npe)
{
  m_NumberPhotoElectrons = npe;
}

bool EKLMDigit::isGood() const
{
  return m_good;
}

void EKLMDigit::isGood(bool status)
{
  m_good = status;
}

int EKLMDigit::getPlane() const
{
  return m_Plane;
}

void EKLMDigit::setPlane(int plane)
{
  m_Plane = plane;
}

int EKLMDigit::getStrip() const
{
  return m_Strip;
}

void EKLMDigit::setStrip(int strip)
{
  m_Strip = strip;
}

const TFitResult* EKLMDigit::getFitResults() const
{
  return &m_fitResults;
}

void EKLMDigit::setFitResults(TFitResult& res)
{
  m_fitResults = res;
}

void EKLMDigit::setFitResults(TFitResultPtr resPtr)
{
  m_fitResults = *resPtr;
}

void EKLMDigit::setFitStatus(int s)
{
  m_fitStatus = s;
}

int EKLMDigit::getFitStatus()
{
  return m_fitStatus;
}

int EKLMDigit::getVolumeID() const
{
  return m_volid;
}

void EKLMDigit::setVolumeID(int id)
{
  m_volid = id;
}

void EKLMDigit::setMCTS(double ts)
{
  m_timeshift = ts;
}

double EKLMDigit::getMCTS() const
{
  return m_timeshift;
}

void EKLMDigit::Print() const
{
  printf("Endcap: %d Layer: %d Sector: %d Plane: %d Strip: %d # Time: %f\n",
         getEndcap(), getLayer(), getSector(), getPlane(), getStrip(), m_Time);
}
