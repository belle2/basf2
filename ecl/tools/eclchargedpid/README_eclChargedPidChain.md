Contributors: Caitlin MacQueen, Marco Milesi
Contact: cmq.centaurus@gmail.com, marco.milesi@unimelb.edu.au
Last Modified: June 2018

Description of the steps needed to extract charge-dependent PDFs for ECL charged PID, for different charged particle hypotheses.

Step 1: Generation of MC with Particle Gun in DST format:
	Using `./ecl/examples/MakeMC.py` MC can be generated for the user's choice of PDG Code (including antiparticle codes).
	Here the user can set the beam background hypothesis to be overlayed.

Step 2: Processing of DST file to NTup:
	Using `./ecl/examples/EclChargedPidAnalysis.py` the DST file is read in and an NTuple is created (ROOT TTree).
	This is done by means of the `./ecl/modules/eclChargedPIDDataAnalysisExpert` module.
	Herein, separate trees are created for every possible Shower/Cluster Hypothesis.
	It is the recommendation of this author that ONLY the N1 hypothesis (HypoID == 5) is used until further hypotheses become available and are understood.

Step 3: Saving histograms for the fitter, in bins of P and Theta:
	Run `eclChargedPidReader.py`, which executes `eclChargedPidSelector.C`, to loop over all particle and antiparticle types.
	This reads the NTuple produced at Step 2 and creates simple histograms of observables for the PDFs fit (e.g., E/P).
	The user should not need to alter the selector code, unless different momentum and theta ranges are desired.
	Basic selection criteria are also included here:
		1) at least one shower
		2) that shower has an energy of at least 30MeV

Step 5: Fitting distributions and creating PDFs as DB payloads:
	Distributions obtained at Step 3 are fit using `eclChargedPidDatabaseImporter.py`. The script then automatically creates a local DB with the payload.
	The payload, described via the class `.ecl/dbobjects/include/ECLChargedPidPDFs.h`, contains a map with all the PDFs as ROOT TF1 functions for different particle/antiparticle hypotheses.
	This payload will be later retrieved by basf2 in the `./ecl/modules/eclChargedPID` module using the same DB class interface, to create the likelihood data object.
	The definition of the PDFs to be fitted is contained in the various `electrons_pdf.py, muons_pdf.py`... modules.

