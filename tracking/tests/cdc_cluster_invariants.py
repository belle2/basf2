##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Checks the invariants of the CDC wire hit clustering.

The findlets behind TFCDC_ClusterPreparer used to verify these invariants with
assert() and B2ASSERT() on every event, which is expensive because some of the
checks are super-linear in the number of wire hits. The invariants they guard
are properties of the wire topology and of the clustering code, not of the
event data, so checking them once here is equivalent and costs nothing at run
time.

Covered:

* the wire hits are sorted by wire - this is the precondition of the
  std::equal_range lookups in the wire hit relation filters, and violating it
  would silently return the wrong neighbours rather than crash;
* the wire hits of every cluster and every super cluster are sorted;
* the super clusters partition the wire hits, and so do the clusters - a wire
  hit neighbourhood that is not symmetric would let the clusterizer assign a
  hit to two different connected components, so this covers the areSymmetric
  checks through their observable consequence;
* every cluster is contained in exactly one super cluster and the
  ISuperCluster bookkeeping on clusters and on wire hits agrees.
'''

import basf2 as b2
import cppyy
from ROOT import Belle2
from simulation import add_simulation
from tracking.path_utils import add_cdc_track_finding


def wire_key(wire_hit):
    '''The (super layer, layer, wire) triple the wire hits are ordered by.'''
    wire_id = wire_hit.getWireID()
    return (wire_id.getISuperLayer(), wire_id.getILayer(), wire_id.getIWire())


class ClusterInvariantChecker(b2.Module):
    '''Verifies the clustering invariants that the findlets used to assert.'''

    def __init__(self):
        '''Constructor.'''
        super().__init__()
        #: number of events that were checked
        self.n_checked = 0
        #: number of wire hits that were seen in total
        self.n_wire_hits = 0

    def event(self):
        '''Check every invariant on this event.'''
        wire_hits = Belle2.PyStoreObj('CDCWireHitVector').obj().get()
        clusters = Belle2.PyStoreObj('CDCWireHitClusterVector').obj().get()
        super_clusters = Belle2.PyStoreObj('CDCWireHitSuperClusterVector').obj().get()

        n_wire_hits = wire_hits.size()
        if n_wire_hits == 0:
            return

        # The wire hits must be sorted by wire, otherwise the equal_range
        # lookups in the relation filters return the wrong neighbours.
        keys = [wire_key(wire_hit) for wire_hit in wire_hits]
        if keys != sorted(keys):
            b2.B2FATAL('The wire hits are not sorted by wire')

        # Index every wire hit by its address so that the cluster contents,
        # which are pointers into this very vector, can be identified.
        index_of = {cppyy.ll.addressof(wire_hit): i for i, wire_hit in enumerate(wire_hits)}
        if len(index_of) != n_wire_hits:
            b2.B2FATAL('The wire hits are not distinct objects')

        super_cluster_of = self.check_partition(super_clusters, index_of, 'super cluster')
        cluster_of = self.check_partition(clusters, index_of, 'cluster')

        # Every cluster refines exactly one super cluster, and the book keeping
        # written onto the clusters and onto the wire hits has to agree with it.
        for i_cluster, cluster in enumerate(clusters):
            enclosing = {super_cluster_of[index_of[cppyy.ll.addressof(wire_hit)]]
                         for wire_hit in cluster}
            if len(enclosing) != 1:
                b2.B2FATAL(f'Cluster {i_cluster} spans {len(enclosing)} super clusters')
            i_super_cluster = enclosing.pop()
            if cluster.getISuperCluster() != i_super_cluster:
                b2.B2FATAL(f'Cluster {i_cluster} claims super cluster '
                           f'{cluster.getISuperCluster()} but sits in {i_super_cluster}')
            for wire_hit in cluster:
                if wire_hit.getISuperCluster() != i_super_cluster:
                    b2.B2FATAL(f'A wire hit of cluster {i_cluster} claims super cluster '
                               f'{wire_hit.getISuperCluster()} but sits in {i_super_cluster}')

        del cluster_of
        self.n_checked += 1
        self.n_wire_hits += n_wire_hits

    def check_partition(self, groups, index_of, label):
        '''Check that the groups are sorted and partition the wire hits.

        Returns the group index of every wire hit.
        '''
        owner = [None] * len(index_of)
        for i_group, group in enumerate(groups):
            indices = []
            for wire_hit in group:
                address = cppyy.ll.addressof(wire_hit)
                if address not in index_of:
                    b2.B2FATAL(f'A {label} contains a wire hit outside the wire hit vector')
                indices.append(index_of[address])
            if indices != sorted(indices):
                b2.B2FATAL(f'The wire hits of {label} {i_group} are not sorted')
            for index in indices:
                if owner[index] is not None:
                    b2.B2FATAL(f'Wire hit {index} is in {label} {owner[index]} and {i_group}')
                owner[index] = i_group
        missing = owner.count(None)
        if missing:
            b2.B2FATAL(f'{missing} wire hits are in no {label}')
        return owner

    def terminate(self):
        '''Make sure the test actually looked at something.'''
        if self.n_checked == 0 or self.n_wire_hits == 0:
            b2.B2FATAL('No wire hits were checked - the test is not testing anything')
        b2.B2INFO(f'Checked {self.n_wire_hits} wire hits in {self.n_checked} events')


b2.set_random_seed('cdc_cluster_invariants')

main = b2.Path()
main.add_module('EventInfoSetter', evtNumList=[5])
main.add_module('EvtGenInput')
add_simulation(main, components=['CDC'])

# Take the modules and their configuration from the track finding itself rather
# than repeating the settings here, so that the test keeps checking what the
# reconstruction actually runs. Everything after the clustering is dropped.
cdc_path = b2.Path()
add_cdc_track_finding(cdc_path)
for module in cdc_path.modules():
    main.add_module(module)
    if module.type() == 'TFCDC_ClusterPreparer':
        break
else:
    b2.B2FATAL('add_cdc_track_finding no longer adds a TFCDC_ClusterPreparer')

main.add_module(ClusterInvariantChecker())

b2.process(main)
print(b2.statistics)
