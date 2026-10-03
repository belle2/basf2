/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/eventdata/segments/CDCWireHitCluster.h>

#include <TBuffer.h>

using namespace Belle2;
using namespace TrackingUtilities;

void CDCWireHitCluster::Streamer(TBuffer& buffer)
{
  // Only the cluster's own state is transferred. The inherited
  // std::vector<CDCWireHit*> is deliberately left out: the cluster borrows its wire hits from
  // the CDCWireHitVector and does not own them, so streaming the pointers would make ROOT deep
  // copy every CDCWireHit into the message and allocate a fresh one when reading it back, which
  // no destructor ever frees. On reading, the vector is therefore left empty rather than filled
  // with pointers to private copies - which is what the generated streamer used to produce, so
  // cluster to wire hit identity never survived the transfer in the first place.
  if (buffer.IsReading()) {
    UInt_t start = 0;
    UInt_t count = 0;
    buffer.ReadVersion(&start, &count, CDCWireHitCluster::Class());
    buffer >> m_iSuperCluster;
    buffer >> m_backgroundFlag;
    clear();
    buffer.CheckByteCount(start, count, CDCWireHitCluster::Class());
  } else {
    const UInt_t start = buffer.WriteVersion(CDCWireHitCluster::Class(), kTRUE);
    buffer << m_iSuperCluster;
    buffer << m_backgroundFlag;
    buffer.SetByteCount(start, kTRUE);
  }
}
