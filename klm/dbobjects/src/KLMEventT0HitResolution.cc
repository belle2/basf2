/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)
 * Author: The Belle II Collaboration
 *
 * See git log for contributors and copyright holders.
 * This file is licensed under LGPL-3.0, see LICENSE.md.
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMEventT0HitResolution.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMEventT0HitResolution::KLMEventT0HitResolution()
{
}

KLMEventT0HitResolution::~KLMEventT0HitResolution()
{
}

void KLMEventT0HitResolution::setSigmaEKLMScint(float sigma, float sigmaErr)
{
  m_SigmaEKLMScint = sigma;
  m_SigmaEKLMScintErr = sigmaErr;
}

void KLMEventT0HitResolution::setSigmaBKLMScint(float sigma, float sigmaErr)
{
  m_SigmaBKLMScint = sigma;
  m_SigmaBKLMScintErr = sigmaErr;
}

void KLMEventT0HitResolution::setSigmaRPC(float sigma, float sigmaErr)
{
  m_SigmaRPC = sigma;
  m_SigmaRPCErr = sigmaErr;
}

void KLMEventT0HitResolution::setSigmaRPCPhi(float sigma, float sigmaErr)
{
  m_SigmaRPCPhi = sigma;
  m_SigmaRPCPhiErr = sigmaErr;
}

void KLMEventT0HitResolution::setSigmaRPCZ(float sigma, float sigmaErr)
{
  m_SigmaRPCZ = sigma;
  m_SigmaRPCZErr = sigmaErr;
}

void KLMEventT0HitResolution::setSigma(float sigma, float sigmaErr, int category)
{
  switch (category) {
    case c_EKLMScint:
      setSigmaEKLMScint(sigma, sigmaErr);
      return;
    case c_BKLMScint:
      setSigmaBKLMScint(sigma, sigmaErr);
      return;
    case c_RPC:
      setSigmaRPC(sigma, sigmaErr);
      return;
    case c_RPCPhi:
      setSigmaRPCPhi(sigma, sigmaErr);
      return;
    case c_RPCZ:
      setSigmaRPCZ(sigma, sigmaErr);
      return;
  }
  B2FATAL("KLMEventT0HitResolution::setSigma: incorrect category: " << category);
}

float KLMEventT0HitResolution::getSigmaEKLMScint() const
{
  return m_SigmaEKLMScint;
}

float KLMEventT0HitResolution::getSigmaEKLMScintErr() const
{
  return m_SigmaEKLMScintErr;
}

float KLMEventT0HitResolution::getSigmaBKLMScint() const
{
  return m_SigmaBKLMScint;
}

float KLMEventT0HitResolution::getSigmaBKLMScintErr() const
{
  return m_SigmaBKLMScintErr;
}

float KLMEventT0HitResolution::getSigmaRPC() const
{
  return m_SigmaRPC;
}

float KLMEventT0HitResolution::getSigmaRPCErr() const
{
  return m_SigmaRPCErr;
}

float KLMEventT0HitResolution::getSigmaRPCPhi() const
{
  return m_SigmaRPCPhi;
}

float KLMEventT0HitResolution::getSigmaRPCPhiErr() const
{
  return m_SigmaRPCPhiErr;
}

float KLMEventT0HitResolution::getSigmaRPCZ() const
{
  return m_SigmaRPCZ;
}

float KLMEventT0HitResolution::getSigmaRPCZErr() const
{
  return m_SigmaRPCZErr;
}

float KLMEventT0HitResolution::getSigma(int category) const
{
  switch (category) {
    case c_EKLMScint:
      return m_SigmaEKLMScint;
    case c_BKLMScint:
      return m_SigmaBKLMScint;
    case c_RPC:
      return m_SigmaRPC;
    case c_RPCPhi:
      return m_SigmaRPCPhi;
    case c_RPCZ:
      return m_SigmaRPCZ;
  }
  B2FATAL("KLMEventT0HitResolution::getSigma: incorrect category: " << category);
}

float KLMEventT0HitResolution::getSigmaErr(int category) const
{
  switch (category) {
    case c_EKLMScint:
      return m_SigmaEKLMScintErr;
    case c_BKLMScint:
      return m_SigmaBKLMScintErr;
    case c_RPC:
      return m_SigmaRPCErr;
    case c_RPCPhi:
      return m_SigmaRPCPhiErr;
    case c_RPCZ:
      return m_SigmaRPCZErr;
  }
  B2FATAL("KLMEventT0HitResolution::getSigmaErr: incorrect category: " << category);
}
