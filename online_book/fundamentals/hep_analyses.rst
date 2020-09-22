.. _onlinebook_hep_analyses:

HEP Analyses
============

.. sidebar:: Overview
    :class: overview

    **Teaching**: 2 hrs

    **Prerequisites**: 
    	
    	* What's a B factory
    **Objectives**:

        * Learn everything you need in order to undertsand
          what the software is supposed to do.

In this section you will learn the basic concepts underlying an analysis at BelleII, 
starting from how the data aquisition works and ending to the description of 
the most common analysis concepts.

The workflow that goes from the data taking to the publication of a measurement at 
an HEP experiment is quite complex, and involves multiple steps that can take months 
or even years. 
While the detail of this procedure can be extremely complex and tedious the overall 
picture is simple enough to be fitted in a human-readable scheme:

.. figure:: grand_scheme_of_hep.png
  :width: 40em
  :align: center

Starting from the very end, you can see that the input to the analysis are 
reconstructed, skimmed events, coming either from the actual data taking or from the 
generation of simulated events. The skmming is necessary to reduce the size of the 
dataset and significantly simplify and speed-up the analysis.  The reconstruction 
step is the same for both real and simulated data to minimize the differences 
between the two, except that the data need to be first calibrated.

The rest of this section will quickly go through each of the four blocks in which 
the workflow is split, covering the very basic concepts and deferring most of the 
technical expalations about how the software works to the other chapters.




Intro: Cut and count
--------------------

Almost regardless of the quantity you are going to measure in your analysis, you 
will have to face some basic problems: select events you want to study (the 
signal) over similar events that mimic them (the background), estimate efficiency 
of such  selection and, possibly, estimate the intrinsic resolution on the 
quantities will measure, and finally cound how many signal events you observe.

The most basic way to select a signal is to apply what in jargon are called "cuts". 
A cut is nothing but selection, usually binary, over one quantity that has some 
separation power between signal and background. Of course multiples cuts can be 
applied in sequence, leading to quite effective background reactions. 
Before deciding on the selection criteria however, one must define the variable 
that will be used to count of many signal events are left. A good variable has a 
very peaking distribution for signal, and a smooth, uniform distribution for the 
background.

Example Here. D*? Pi0? Need to run some simulation.




Data taking: The detector
-------------------------

If you are reading this manual, you are probably already at least partially 
familiar with the general layout of the BelleII experiment. However, before 
moving on, let's very quicly review its structure.
Belle II has several sub-system, each one dedicate to a specific task: 
reconstructiong the trajectory of charged track, reconstruct the energy of photons, 
identify the particle type, identify muons and reconstruct long-living hadrons. 
Of course some systems can be used for multiple purposes: the ECL is mainly 
intended as a device to recontruct photons, but is also used to identify 
electrons and hadrons.

* Beam Pipe
  The beam pipe itself is not an anctive part of the detector, but plays the crucial 
  role of separating the detector from the interaction region, which is located in 
  the low-pressure vacuum of the superKEKB rings. It is a cilindrical pipe designed 
  to be as think as possible in order to minimize the particle's energy loss in it,
  but it also assolves the scope of absorbing part of the soft X-rays emitted by 
  the beams, that rapresent a major source of noise for the innermost detector, the PXD. 

* PXD
  The first active system met by the particles that are emerging form the IP is the PiXel Detector (PXD)

* SVD






Data taking: on resonance, continuum, comics
--------------------------------------------





Data taking: Triggers and filters
---------------------------------
 
During the data taking, each sub-detector constantly acquires data according the modes and specifications of its front-end electronics. This mass of data, however, cannot be written directly to disk as a constrant stream to be later sorted out, because it would require a comical amount of resources and bandwidth from the detector to the offline disks.  
For this reason the data are aquired only when a potentially interesting even is seen in the detector, and several level of filtering are applied during the data processing before the end-users, the analysist, can run their analysis jobs on them. 
What follows is a very simplified explanation of the process the leads from the physic event to the data you can analyze.
 
The systems that are involved in the data taking are the Data AQuisition (DAQ), the TRiGger (TRG, also known as L1) and the High Level Trigger (HLT). Collectively, they are often referred as the Online system.

During the data taking [...]


Simulation: the Montecarlo
--------------------------

Descibe here:
* What a generator is
* What the simulation is, what's Geant
* can we trust the MC 100%? Performance studies
* How can you have your MC being generated?



Processing: the reconstruction
------------------------------

Descibe here:
* What is the reconstruction
* Example 1: tracking (short)
* Example2: clustering (?)
* Why do we need to run the reconstruction separately from teh analysis? 
  Mention that resources are very not infinite



Processing: Data formats
------------------------

When an e+e- collision happens, the resulting products will leave signal in the BelleII subdetectors that are acquired, matched in time as each subsystem have a different delay and response time (event building), and saved to disk in a packed, binary format. Several steps have to be performed in order to produce a physics result of these hardly-intelligible raw data. These steps are unpacking, calibration, reconstruction and finally analysis. Each of these steps reads and writes different objects, and produces files in different formats. 

.. note::
   All the Belle II data files are root files, where the relevant objects are stored in the branches of a tree. When we say "different formats", we refer simply to the different branches contained in those trees.

Let's start form the data objects we save. There are four groups of them: raw, low-level,reconstruction-level and analysis-level. The raw objects are the output of the single subsystems: digitized  PMT signals from the TOP, digitized ADC signals form the CDC, and so on. Without any further process, these objects cannot be used. The low-level objects com from the very first step of the data processing, the unpacking. The RAW signals are turned into more abstract and understandable objects: the CDC ACD signals are converted in CDChits, the TOP PMT signals are turned into TOPDigits, and so on.  The low-level objects are foundamentalto understand the detector performance, but they cannot yet be directly used to perform an analysis. The last step is called reconstruction, and consistin in running algorithm on the collection of digits to produce analysis-friendly quantities. The ouput of the reconstruction is are high-level variables like ECL clusters, resulign from running cluster algorithms on the ECLDigits,  tracks resulting from runnign the trackign algoriths over the collections of CDC, SVD and PXD hits,  PID likelihood resulting from the analysis of the TOP signals. In teh process of recontruction the calibrations are applied, correcting for the fluctuations in the detector response. These hgh-level objects are finally read by the analysis software, and turned into analysis-level objects: charged particles, photons, missing energies and all teh quantities used to present a physics result.


In BelleII there are four different data formats, reflecting which data objects are stored in a file:

* RAW. This is the most basic format. It contains the un-processed, un-calibrated output of the detector. Analysis cannot be run on these data, but they serve as base for the production of the subsequent data format
* cDST (calibration Data Summary Table). This format contains the same objects as the RAW (so a full reconstruction could be performed starting from it), plus the results of the tracking, which is the most demanding part of the reconstruction. The scope of this format is to perform low-level detectro studies and calculate calibration constants.
* mDST (mini Data Summary Table). This is the basic data-analysis format. It contains only the high level information that can be directly used to perfrom a physics analysis. However, it is not the suggested format to perform analysis.
* uDST (micro Data Summary Table). This is the main format for data analysis. It's the result of the analysis skim procedure, that selects from the mDST only the few events that can be useful for a certain type of analysis (events with a well recontructed J/psi per example). The content of this ormat is the same as the mDST, with the addition of the recontructed particles used in the skimming selection (if you look at the J/psi skim, you will also find a list of J/psi already reconstructed for you in the file).


.. note::
   If you are simply running an analysis, you will mostly use uDST, if you are also involed in performance studies you will probably use cDST as well and if your core activity will be hardware opertions, you will be mostly dealing with the RAW and cDST formats.




Analysis: what do we measure?
-----------------------------

The Belle II detectors can provide three kind of information: momentum, energy and PID probability. Of course not all of them are available for every particle, infact in most cases only two of them are, and however only for a very limited number of particles.  


Charged particles are seen in the detector if they live long enough to cross its active colume, for example the SVD layers, and leave a ionization signal in it.



However, tracking only emasures the 3-momentum of these particles. In order to get form this the 4-momentum one has to either assign a mass hypothesis, which is done using the information form the particle
identification systems.


Neutral particles such photons, neutrons or KL do not leave any ionization in the tracking system, and can only  be detected when they interact with the dense material of the ECL or the KLM. In these cases we will have a measurement of their enery and, from the analysis of the shape of the energy deposition, an intecation about their nature.


All the other particles that are either short lived and decay nearby the interaction point (such as the J/psi), or are long-lived but neautra and decay inside the active volumen of the detector (such the Ks or Lambda), have to be reconstructed combingin the 4-momenta of their decay products, or of the decay prducts at the end of their decay chain. We define as final state particles all the particles that are directly reconstructed from the signal they leave in the detector. The particles obtained combining other particles are usually referred as combined particles.


Analysis: the skimming
----------------------


Describe here:
* what skiming is
* why it's needed


