/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  /// Type of the process used for storing and mapping the child processes in the process handler
  enum class ProcType {
    c_Input = 'i', /**< Input Process */
    c_Worker = 'w', /**< Worker/Reconstruction Process */
    c_Output = 'o', /**< Output Process */
    c_Proxy = 'p', /**< Multicast Proxy Process */
    c_Monitor = 'm', /**< Monitoring Process */
    c_Init = '0', /**< Before the forks, the process is in init state */
    c_Stopped = 's' /**< The process is stopped/killed */
  };
}