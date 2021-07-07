Contributor: Christopher Hearty (hearty@physics.ubc.ca)
Last Modified: May 2020

The ecl_edge calibration is defined in collector/scripts/prompt/calibrations/caf_ecl_edge.py. It is run in AirFlow as part of the prompt calibration program, but new payloads are needed only when the ECL geometry changes (payload ECLCrystalsShapeAndPosition). 

It creates the payloads ECLCrystalThetaEdge and ECLCrystalPhiEdge, which specify the lower edge of each ECL crystal, defined as the angles at which beam energy photons have the maximum energy leakage. From these payloads it then finds the payloads ECLCrystalThetaWidth and ECLCrystalPhiWidth, which are the dimensions of the crystals (i.e. distance between lower edges) in the two directions.

Prior to running the calibration, modify input_data.json to point to a file of the appropriate experiment and run, and modify caf_config.json to request that experiment and run. The actual data is not used in obtaining the payloads. 

To run the calibration at KEKCC:  b2caf-prompt-run LSF caf_config.json input_data.json

The new payloads will appear in the subdirectory caf_outputdb_results/ecl_edge/
Adjust the iov's in caf_outputdb_results/ecl_edge/database.txt before uploading to a global tag. 

Some diagnostic output is included in
calibration_results/ecl_edge/0/collector_output/default/0/stdout
calibration_results/ecl_edge/0/algorithm_output/eclEdgeAlgorithm.root 

