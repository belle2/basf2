

# Scripts for validation of CP-violation analysis tools

Authors: Fernando Abudinen, Vladimir Chekelian, Moritz Gelb, Thomas Keck, Luigi Li Gioi.

The scripts contained in this folder serve for validating the tools used for CP-violation analyses once 
a new basf2 release comes out. We train and test the flavor tagger and perform vertex reconstruction 
of the signal side using Rave (in future also treeFitter) and of the tag-side using the tagV module, 
which is based on the adaptive vertex fitter method of Rave.

Afterward, we check that the generated CP asymmetries are correct for the benchmark channels
B0-> JPsiKs and B0-> nu anti-nu. This is followed by an evaluation of the performances of the 
flavor tagger and the vertexing tools. The input information of the flavor tagger is plotted and analyzed for 
strange asymmetries. 

In the case of Belle data/MC, we compare the performance on data with the performance on MC.

Finally, we perform tests to understand the correlations between the input variables of the flavor tagger
and we test if the MVA methods are overtrained.

To understand the evaluation of the flavor tagger performance you can reed further in BELLE2-PTHESIS-2018-003.

## Procedure

The whole procedure is defined in the script:

`CPVToolsValidatorInParalell.sh`

This script runs all other scripts in this folder and is designed to be used at kekcc. 
It creates the folders to save the output files and the validation plots 
as well as the log files. 

First, it runs the validation script

`flavorTaggerVertexingValidation.py`

six times with the specific arguments needed to train and to test the flavor tagging while performing vertexing. 

The script `flavorTaggerVertexingValidation.py` is a basf2 steering file defining the reconstruction and the 
analysis procedure of the B0 anti-B0 events, where one of the mesons decayed into one of the signal channels
B0-> JPsiKs of B0-> nu anti-nu.

When the training and testing procedure is finished, the evaluation scripts are called. These are:

`B0_GenDeltaTFit.py` : Determines which are the simulated CP-violation parameters A and S by fitting the known
quantum mechanical pdfs to the generated decay-time distributions.

`genLevelAsymmsImpactParams.py` : Plots the distributions of the impact parameters for positive and for negative
charged particles. This serves to understand the simulated asymmetries. 

`inputVariablesPlots.py` : Plots the distributions of all input variables of the flavor tagger for target (signal)
and for all other particles (background) of each flavor tagging category. 

`asymmetriesInVariablesPlots.py` : Plots the distributions of all input variables of the flavor tagger for 
positively and for negatively charged target particles. This serves to understand the simulated asymmetries.

`flavorTaggerEfficiency.py` : Evaluates the performance of the flavor tagger. It calculates the total effective efficiency and
the effective efficiencies of the individual flavor tagging categories. It reproduces the tables that are published in the b2tip.
It produces also evaluation plots for the global performance and for the single categories.

`pythonFlavorTaggerEfficiency.py` : It evaluates the performance of the flavor tagger in a similar way as in the previous script
but produces nicer plots using matplotlib. Additionally, it produces plots evaluating the performance depending on the number 
of categories to which an event can be attributed. It evaluates also the performance of the individual categories
for events to which the respective category cannot be attributed, and for events to which no category can be attributed. 

`deltaTVertexTagVResolution.py` : Evaluates the resolution and the bias of deltaT and of 
the reconstructed signal and tag-side vertices. It calculates also the total reconstruction efficiency and the PXD efficiency.

`B2JpsiKs_mu_qrBelleDataSplot.py` : Evaluates the performance of the flavor tagger on Belle MC and on Belle data. The 
performance on Belle data and on Belle MC is evaluated by applying the sPlot technique. The sPlot uses mbc as 
discriminating variable.

`ft_mva_evaluate.py` : Plots the correlations between the flavor tagging input variables and between the outputs of the 
categories. It performs a ranking of variables. This script performs also tests to evaluate if the multivariate methods
on which the flavorTagger depends are overtrained. 

`ntupleUpsilon4SGenInfo.py` : saves MC variables for the generated Upsilon(4S). This is needed for studies of the generated MC information.
This file is written for the case when the Upsilon(4S), has two daughters (i.e. BBbar) and four granddaughters (e.g. B0->Jpsi+K0S).
The saved variables are mcPDG,M,nDaughters,mcDecayTime,mcLifeTime,mcX (a.k.a. mcDecayVertexX),mcY (a.k.a. mcDecayVertexY),mcZ (a.k.a. mcDecayVertexZ),mcE,mcPX,mcPY,mcPZ. 
