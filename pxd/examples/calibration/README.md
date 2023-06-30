PXD Cluster shape corrections

The sequence of steps to train a new set of cluster shape corrections for PXD from scratch is 
as follows. 

$ basf2 cluster_position_collector.py -n 2000000 -- --clusterkind=0
$ basf2 cluster_position_collector.py -n 2000000 -- --clusterkind=1
$ basf2 cluster_position_collector.py -n 2000000 -- --clusterkind=2
$ basf2 cluster_position_collector.py -n 2000000 -- --clusterkind=3

This produces four training files: 

PXDClusterPositionCollectorOutput_kind_0.root
PXDClusterPositionCollectorOutput_kind_1.root
PXDClusterPositionCollectorOutput_kind_2.root
PXDClusterPositionCollectorOutput_kind_3.root

$ basf2 cluster_position_algo.py

This produces a localdb containing cluster shape payloads. The content are three files: 

database.txt
dbstore_PXDClusterShapeIndexPar_rev_4e623c.root
dbstore_PXDClusterPositionEstimatorPar_rev_be220e.root

$ basf2 plotPXDPositionEstimator.py -- --resultdir=results --dbfile=./localdb/database.txt

Visualizes the cluster shape payloads from localdb. Creates a folder results containing plots. 


$ basf2 test_cluster_position_estimator.py -- --dbfile=./localdb/database.txt

Produces a root file (VTXPositionEstimation.root) with all sorts of histograms about residuals and pulls.


$ b2conditionsdb tag create DEV   GT_NAME_HERE “Development cluster Shape payloads”
$ b2conditionsdb upload GT_NAME_HERE  localdb/database.txt

Uploads localdb into a new GT called GT_NAME_HERE on the condDB server

The final step is to activate cluster shapes in the PXD reconstruction. This can be done in the function 
add_pxd_reconstruction(...,usePXDClusterShapes=True) in pxd/scripts/pxd/__init__.py. There will be an 
error when cluster shape payloads are not found in the condDB. 
