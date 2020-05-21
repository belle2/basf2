Contributor: Christopher Hearty (hearty@physics.ubc.ca)
Last Modified: May 2020

The ecl_edge calibration is defined in collector/scripts/caf_ecl_edge.py

It creates the payloads ECLCrystalThetaEdge and ECLCrystalPhiEdge, which specify the lower edge of each ECL crystal, defined as the angles at which beam energy photons have the maximum energy leakage. 

Prior to running the calibration, modify input_data.json to point to a file of the appropriate experiment and run, and modify caf_config.json to request that experiment and run. The actual data is not used in obtaining the payloads. 

To run the calibration:  b2caf-prompt-run caf_config.json input_data.json

The new payloads will appear in the subdirectory caf_outputdb_results/ecl_edge/
Adjust the iov's in caf_outputdb_results/ecl_edge/database.txt before uploading to a global tag. 

Some diagnostic output is included in
calibration_results/ecl_edge/0/collector_output/default/0/stdout
calibration_results/ecl_edge/0/algorithm_output/eclEdgeAlgorithm.root 

