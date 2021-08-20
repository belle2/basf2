This directory contain a number of alternative scripts for running the ecl timing calibrations and validations.

The methods used for the timing calibration are outlined in the following internal note:
https://docs.belle2.org/record/1276


!!!!!!!!!!!!!!!!
The main calibration script, which is executed as part of the prompt calibration in airflow, is
   calibration/scripts/prompt/calibrations/caf_ecl_time_crystalCrate.py
!!!!!!!!!!!!!!!!


The above main script performs the crystal and crate calibrations, executes the hadronic selection validation
and the bhabha self-consistency check "validation", and plots the crate times jumps.

Should a user wish to execute only the calibrations or only the validations etc, they can use the scripts provided in this directory.
This might be useful because modifying the main script to skip certain steps might be quite difficult as many of the steps
have to be executed in a specific order.


The below scripts execute the various calibrations and validations using the airflow framework,
which executes both the collector and the algorithms.  To be used they have to be put in the 
   calibration/scripts/prompt/calibrations/
diretory; however, we don't want to keep them there permanently as the automated calibration system
will execute all scripts in that directory, which we don't want to do.  These scripts are used with
the "b2caf-prompt-run" command (or whatever the latest version is).  They have to be copied into
the calibration/scripts/prompt/calibrations/ and basf2 recompiled for them to be recognized
and able to be used.
   caf_ecl_time_crate.py ------------------------------------ performs just the crate calibration
   caf_ecl_time_crystal.py ---------------------------------- performs just the crystal calibration
   caf_ecl_time_shifts.py ----------------------------------- plots the crate time jumps
   caf_ecl_time_validate_bhabha_and_hadronic.py ------------- performs the bhabha and hadronic validations
   caf_ecl_time_validate_bhabha.py -------------------------- performs just the bhabha validation
   caf_ecl_time_validate_hadronic.py ------------------------ performs just the hadronic validation

The below scripts execute the various calibrations and validations collector and algorithm
codes individually.  Used with the "basf2" command.
   run_eclBhabhaT_collector.py ------------------------------ executes the collector code for the crystal or crate calibration
   run_eclBhabhaT_algorithm.py ------------------------------ executes the algorithm code for the crystal or crate calibration
   run_eclBhabhaTimeCalibrationValidation_collector.py ------ executes the collector code for the bhabha validation
   run_eclBhabhaTimeCalibrationValidation_algorithm.py ------ executes the algorithm code for the bhabha validation
   run_eclHadronTimeCalibrationValidation_collector.py ------ executes the collector code for the hadronic validation
   run_eclHadronTimeCalibrationValidation_algorithm.py ------ executes the algorithm code for the hadronic validation

These codes may not be identical to the main caf_ecl_time_crystalCrate.py script.  Eventually these scripts may be removed should
they prove to be no longer useful since the main script, caf_ecl_time_crystalCrate.py, is the ones that will be executed most of
the time.   If the main script can be executed on just the individual components with all possible combinations desired, then some
or these scripts will become redundant; however, some of these scripts also provide examples on how to pass specific parameters
to the collectors and algorithms.  One important one is "run_eclBhabhaT_algorithm.py", which has an example of how to pass
reference crystal numbers to the algorithm so that it can make a new payload for you.  

Note that you can still use calibration/scripts/prompt/calibrations/caf_ecl_time_crystalCrate.py to run the individual
parts of the calibration (e.g. just the bhabha validation or just iteration 2 of the crystal calibration algorithm); however,
it may require modifying the state of the execution, which is a hack (see b2caf-status).  This is useful if you are running
the calibration yourself.  Sometimes a collector job fails for unknown reasons but will work if you just rerun it.  For
instructions on how to hack CAF in this way, speak with the person in charge of CAF.


If being run locally on kekcc with the LSF batch system, this is an excample of how to execute the caf scripts:
 nohup b2caf-prompt-run   LSF    caf_eclBhabhaT_config.json  input_data_bucket_16.json   > bucket16.out 2> bucket16.err < /dev/null   &

   where
      * caf_eclBhabhaT_config.json is the configuration file telling b2caf to execute the one of the caf scripts,
        like the main one we are interested in here, caf_ecl_time_crystalCrate.py
      * input_data_bucket_16.json gives the list of input cdst files with which to run over
      * bucket16.out is the highest level log file for outputs to the screen
      * bucket16.err is the highest level errors reporting file

It is best to see the manual on how to execute calibrations using the "b2caf-prompt-run" command or whatever the latest
command is that should be used.

Running with the unix commands "screen" and "nohup" is useful as the full caf scripts can take hours/days to execute if you are
doing so by yourself and not via the calibrations.belle2.org website.


A single full calibration performs:
1) a crate calibration
2) a crystal calibration
3) steps 1 and 2 potentially multiple times if more than one iteration is requested, e.g.
   If you execute 5 iterations: 
   crate_0, crystal_0,     crate_1, crystal_1,    crate_2, crystal_2,    crate_3, crystal_3,    crate_4, crystal_4
4) the hadron validation, bhabha validation, and crate time jumps plotting all in parallel

The crystal and crate calibrations have to iterate to converge towards the correct calibrations.  More iterations may be required
if the crystals calibrations are expected to changed by a larger amount.  To perform a crystal calibration, a set of crate calibration 
constants are required and to perform a crate calibration a set of crystal calibration constants are required.  So the better the 
starting crytal and crate calibration constants are, the fewer the iterations that will be required to converge towards the final values.


Note that the crystal calibration, crate calibration, crystal calibration constant merger, bhabha validation, hadron validation, and crate time
plotting ALL execute a collector module and then an algorithm.  In the case of the crate time plotting and the crystal calibration constant merging,
the code doesn't need to analyse the individual events, it just needs to access the database information about the calibration constants
so the collector jobs execute very quickly.  The crate time plotting code has a separate feature that allows the user to instead have the algorithm
directly access the payloads run by run but this is much slower.  The benefit of the alternate method is that it only has to execute the collector
on the first and last runs, and then the algorithm will look at every single run in between them.  The normal method of running the collector over
each run has the drawback that it won't show the crate times for any run for which you don't have an input file.  So the alternate method may be
useful if you need to make the crate time jumps plots locally where you only have access to a small number of input files.  In general, the 
crate time jumping code requires all the runs to be from the same experiment but this is easily addressed by using "payload_boundaries", described
in more detail below.


===================================
Here is a description of how to use calibration/scripts/prompt/calibrations/caf_ecl_time_crystalCrate.py

The input parameters of the script have default values and can be changed via the calibrations website when submitting the jobs.
* input_data_formats=["cdst"] : require cdst files.
* input_data_names=["bhabha_all_calib", "hadron_calib"] : require bhabha and hadron skimmed runs defined by input_data_filters 
* input_data_filters=... : parameters for picking out bhabha events and hadronic events
   * Access the "bhabha_all_calib" and "hadron_calib" skims.
   * The data quality currently requires "good" runs as the default but the user can test to see if "recoverable" runs could also be used.  
     This can be changed by the user via the website interface through these input parameters
   * Require any energies for the beams (4S, Continuum, Scan)
   * The magnets should be on
   The default values should be sufficient to pick out the correct files, unless "good" runs don't give enough bhabha events for the crystal 
   calibration, in which case the "recoverable" may need to be added.
 * expert_config={"numCrysCrateIterations": 2, "payload_boundaries": [], "t0_bhabhaToHadron_correction": 0}
   * numCrysCrateIterations
      * The crystal and crate calibrations have to iterate to converge towards the best values.  The default value is 2 but it is not yet 
        known just how many is required.  The number of iterations require probably will be smaller for buckets of data within the same 
        experiment but buckets of data started after a shutdown may require more iterations as the calibration constants are more likely 
        to have changed after things have been turned off/on and physical interventions made.  If 2 iterations is not enough, you can 
        either just resubmit with a larger number (and waste CPU time by repeating the first 2 iterations) or maybe the Calibrations convenors 
        will let you upload the crystal and crate payloads to the GT as a new starting point for more iterations.  The policies haven't yet 
        been established.  You can set the numebr of iterations through this paramter, an integer.  The user can look at the convergence 
        histograms to determine if the calibrations have converged properly.  There are several different convergence histograms produced 
        by the different calibration and valiadtion algorithms.

        Note that a second mode exists for if numCrysCrateIterations <= 0.  In this case the code does a crate calibration, then a
        crystal calibration, and finally a second crate calibration before doing the validations.  Perhaps this is not needed in the
        future and can be removed but there is the possibility that the a second crystal calibration is not required, in which case
        this option could be faster.
   * payload_boundaries
      * The payload_boundaries variable allows users to define run ranges for which one crystal calibration is performed.  This is useful 
        if multiple buckets or experiments are being calibrated at the same time.  See the example configuration file below to see the 
        format required.  By default, the code assumes that all of the data will be used in a single crystal calibration.  These boundaries 
        don't directly affect the crate calibrations, which are performed run-by-run, but do indirectly affect the crate calibrations by 
        determining which crystal calibration constants are used.
   * t0_bhabhaToHadron_correction
      * This allows the user to insert an overall offset to the calibration constants.  This should only be changed after discussion with the 
        other experts!!!  The default is 0ns but could be changed in the future after some studies are done to understand why 4 GeV bhabhas 
        have different times than 4 GeV photons in hadronic events with 4 tracks.  At the time of writing this README.txt, there was still 
        low-energy and high-energy time talks that needed correcting for photons in the hadronic validation.  Really, if the ECL / neutrals 
        groups etc decide that an offset should be used, that offset should perhaps be coded in as the new default value but until that code 
        change is done, the user can still process data by setting that value manually through the input parameters.




Example configuration file: 
     {"caf_script": "caf_ecl_time_crystalCrate.py", "database_chain": ["Reco_master_patch_rel5", "ECL_testingNewPayload_RefCrystalPerCrate", "AIRFLOW_online_snapshot_20201207-094707", "data_reprocessing_prompt", "AIRFLOW_TEST_data_staging_calibration_prompt", "ECL_pre_bucket9", "ECL_testing_timingCalibs_forIteratingTowardsConvergence", "ECL_crystalTimeMergingTests"] , "requested_iov": [14, 694, 14, 7000], "backend_args": {"queue": "s"}, "expert_config": {"numCrysCrateIterations": 2, "payload_boundaries": [[14, 694], [14, 7000]]}}

* You will have to modify the list of global tags
* You will have to modify the requested_iov to the experiment and run range you care about.  
  This is generally done automatically through the website.  
* You will have to modify the expert_config variable payload_boundaries.  The example here produces a single crystal calibration 
  constant covering exp 14 run 694 through to experiment 14 run 7000, which is the same as the requested_iov.  If you changed this to 
    "payload_boundaries": [[14, 694], [14, 786], [14, 791], [14, 795], [14, 7000]]
  then there would be 4 different run ranges where the crystal calibration constants would be separately calculated.  This is an 
  artificial example, as there would not be enough bhabha events in only a few runs but it is useful to see that listing 5 (exp,run) pairs
  gives 4 crystal ranges.  
* You will have to change the batch queue name as appropriate for your batch jobs.
* You will have to modify the expert_config variable numCrysCrateIterations as is appropriate for the data you are calibrating.
 


==================
Important parameters passed to the collectors and algorithms by 
   calibration/scripts/prompt/calibrations/caf_ecl_time_crystalCrate.py


There are several parameters that are passed to the individual collector and algorithm codes from the CAF script.  These cannot
be changed through the CAF input parameters, they require edits to the CAF script itself.  Most probably should NOT be changed.
There are several other parameters defined in the script that the user may need to eventually change should the cdst files
be modified, thus changing the number of input events.




    max_files_per_run_calibration = 26 : The maximum number of files per run to use for the crate calibrations and for
                                         the bhabha validation, which looks at the crystal times.  This may need
                                         changing if the number of events in a bhabha skim cdst file decreases.
                                         The crystal calibration uses all the files available.
    max_files_per_run_validation = 4   : The maximum number of files per run to use for the hadron validation.  This
                                         may need changing if the number of event in a hadron skim cdst file decreases.
                                         The bhabha validation uses max_files_per_run_calibration.
    max_events_per_run_plotting = 1    : The maximum number of files per run to use for plotting the crate time shifts.
                                         This won't blikely need changing.



    bhabha calibration collector: timeAbsMax = 250   : The range of times accepted.  This needs to be quite wide for crates but could be smaller
                                                       for the crystals because ts(reference crystal) = 0.  This forces tcrate to contain most of
                                                       the magnitude of (ts+tcrate).
    bhabha calibration collector: minCrystal = 1     : This the lowest crystal number to look at for the calibrations.  It should be 1 for
                                                       a crystal calibration.  When doing a crate calibration, it should be set to a large number
                                                       so that the code is told not to do a crystal calibration.
    bhabha calibration collector: maxCrystal = 8736  : This the highest crystal number to look at for the calibrations.  It should be 8736 for
                                                       a crystal calibration.  When doing a crate calibration, it should be set to a number smaller 
                                                       than minCrystal so that the code is told not to do a crystal calibration.

    bhabha calibration collector: saveTree = False   : This tells the bhabha calibration collector whether or not to save information to an ntuple.
                                                       It should usually be False because that information is currently only used for debugging.


    bhabha calibration algorithm: cellIDLo = 3     : This the lowest crystal number to look at for the calibrations.  It should be 1 for
                                                       a crystal calibration.  When doing a crate calibration, it should be set to a number
                                                       larger than cellIDHi so that the code is told not to do a crystal calibration.
    bhabha calibration algorithm: cellIDHi = 2     : This the highest crystal number to look at for the calibrations.  It should be 8736 for
                                                       a crystal calibration.  When doing a crate calibration, it should be set to a number 
                                                       smaller than cellIDLo so that the code is told not to do a crystal calibration.


    bhabha calibration algorithm: crateIDLo = 3     : This the lowest crate number to look at for the calibrations.  It should be 1 for
                                                        a crate calibration.  When doing a crystal calibration, it should be set to a number
                                                        larger than crateIDHi so that the code is told not to do a crate calibration.
    bhabha calibration algorithm: crateIDHi = 2     : This the highest crate number to look at for the calibrations.  It should be 52 for
                                                        a crate calibration.  When doing a crystal calibration, it should be set to a number 
                                                        smaller than crateIDLo so that the code is told not to do a crate calibration.


    bhabha calibration algorithm: meanCleanRebinFactor = 3      : This tells the algorithm to rebin the timing information with a factor of X
                                                                  so that there are more entries per bin.  This then allows for a more accurate
                                                                  determination of where the peak is and how tall it is compared to the
                                                                  other times nearby.  The final fit is NOT done on the rebinned data though.
                                                                  The same variable is applicable to both crystal and crate times plots for
                                                                  that one execution of the algorithm, which should only be for one of
                                                                  crystals or crates, not both crystals and crates.
                                                                  This variable should probably not be changed.
    bhabha calibration algorithm: meanCleanCutMinFactor = 0.3   : This tells the algorithm to remove the tails of the time distribution for
                                                                  the fit where the threshold is the bins where the number of entries is less 
                                                                  than meanCleanCutMinFactor times the maximum number of entries in the peak
                                                                  as counted in the rebinned histogram.  Once these bins are determined in the
                                                                  rebinned histogram, they are used to determine the time ranges to be applied
                                                                  similarly in the non-rebinned histogram where the fit is done.  
                                                                  The same variable is applicable to both crystal and crate times plots for
                                                                  that one execution of the algorithm, which should only be for one of
                                                                  crystals or crates, not both crystals and crates.  
                                                                  This probably should not be changed.


    bhabha calibration algorithm: savePrevCrysPayload  : This tells the algorithm whether or not to save the previous crystal calibration 
                                                         constants to a temporary payload (only used internally to this code) for keeping
                                                         track of how the calibration constants have change from one iteration to the next.
                                                         The user should not change these values.
    bhabha calibration algorithm: readPrevCrysPayload  : This tells the algorithm whether or not to try to read one of the temporary
                                                         payloads that keeps track of the previous values of the crystal calibration constants.
                                                         If you try to read a payload that doesn't yet exist, the code will crash.
                                                         The user should not change these values.



    validation algorithm: readPrevCrysPayload  : This tells the algorithm whether or not to save the previous crystal calibration 
                                                 constants to a temporary payload (only used internally to this code) for keeping
                                                 track of how the calibration constants have change from one iteration to the next.
                                                 The user should not change these values.



    validation algorithm:meanCleanRebinFactor = 3      : See the equivalent variable for the calibrations algorithm
    validation algorithm:meanCleanCutMinFactor = 0.4   : See the equivalent variable for the calibrations algorithm

    validation algorithm: debugFilenameBase   : This tells the code what to label the output files with.  Otherwise, the code itself
                                                does not know the difference between the different physics processes and particles
                                                it is analysing.


    validation algorithm: clusterTimesFractionWindow_maxtime
        = 1.5 for bhabhas 
        = 8 for photons from hadronic event selection               : this tells the algorithm what size window to use for calculating
                                                                      the fraction of ECL cluster times that lie within the window.
                                                                      The code does not use the exact size given by the user but rounds
                                                                      to the appropriate bin edge since it looks at histograms.  The 
                                                                      difference in sizes for bhabhas and photons from a hadronic event
                                                                      selection only has to do with the energy ranges of the particles
                                                                      and not the particle types.


    crate time jumps plotting algorithm: crysCrateShift_min = -30  : the lower limit of the crate time plots for seeing the crate time jumps.
    crate time jumps plotting algorithm: crysCrateShift_max = 30   : the upper limit of the crate time plots for seeing the crate time jumps.



===================

Validations

To look at the valiation plots, you'll have to get access to the outputs of the "algorithm" codes.  During routine operation, 
     the calibration will be run remotely, maybe on a server to which you don't have access, so you may have to use your grid 
     certificate to download the files.  Currently, the jira tickets automatically have a post saying how to download 
     those files using the grid.


To check that the calibrations worked, look at the output root files from the various algorithms:
   * crate calibrations (at least the last iteration) : eclBhabhaTAlgorithm_cratesAllRuns.root
      * root files also exist for individual runs if you find problems you want to further investigate
   * crystal calibrations (at least the last iteration) : eclBhabhaTAlgorithm_*.root
      * There will be one root file for each run range encumpassing a single crystal calibration, set by expert_config payload_boundaries
   * crystal merger (at least the last iteration) : eclTimeShiftsAlgorithm_*.root
      * There will be one root file for each run range encumpassing a single crystal calibration, set by expert_config payload_boundaries
   * bhabha validations : eclBhabhaTValidationAlgorithm_*.root
      * There will be one root file for each run range encumpassing a single crystal calibration, set by expert_config payload_boundaries
   * hadronic validations : eclHadronTValidationAlgorithm_*.root
      * There will be one root file for each run range encumpassing a single crystal calibration, set by expert_config payload_boundaries
   * crate time shifts : eclTimeShiftsAlgorithm_*.root
      * There will be one root file for each run range encumpassing a single crystal calibration, set by expert_config payload_boundaries


The plots here will help the user to regularly (just simple ideas, not rigorous):
   * check if the crystal calibration constants have converged.  Plots of the calibration constants found in the latest iteration minus
     those from the previous iteration should ideally show only zeros.  In practice, maybe there will be a small number of small differences.
   * check if the crate calibration constants have converged.  Differences in the calibration constants found in the latest iteration
     minus those from the previous iteration should ideally show only zeros.  In practice, maybe there will be a small number of
     small differences.
   * check if the times of the electrons in the bhabha validation peak at the right values (generally 0ns)
   * check if the times of the photons in the hadron validation peak at the right values (generally 0ns)
   * check if the times of the electrons/photons in the bhabha/hadron validations have reasonable resolutions based on the window fractions.  
     We want these to be relatively large for all crystals.  Compared to previous runs, we don't want crystals to have their fractions
     reduce in size by a significant amount.
   * look for crate time jumps.  Jumps aren't bad but should be reported to the ECL group.  Definitely report if ~8ns in size but jumps of 
     roughly 1ns are probably from the CDC updating their constants or powercycling etc.  Report to the ECL group any jump larger than ~1ns. 
     These could be compared with the DQM plots that monitor for crate time jumps.  See the ECL DQM confluence page:
     https://confluence.desy.de/pages/viewpage.action?pageId=87337319
   * look at the size of the time biases as a function of energy, crystal, and physics process (bhabhas vs photons from hadronic selection).  
     New detector regions (large groups of crystals) with sizable biases (>~2ns) should be reported to the ECL group.
   * See how the calibration constants have changed compared to the initial values, which if they were taken from a previous bucket show 
     how much the calibration constants have changed from one bucket to the next.  Changes on the order of a few nanoseconds may be expected
     if systems were turned off and on.  Changes on the order of 8ns could result from FTSW 8ns clocks somehow slipping and this should be
     communicated to the ECL TRG group.  Changes of tens of nanoseconds are very much unexpected and should be reported to the ECL group.

Other plots also exist for deeper analysis of problems:
   * fits for individual crystals, energy bins, etc. : the user can see which crystals may be producing bad fits, bad calibration 
     constants, weird shapes, have low stats etc.
   * histogram of hits to each crystal : The user can see which crystals rarely get hits.
   * histogram of good fits to the crates to know which runs had good statistics for the fits.
   * cutflow, to diagnose problems
   * values of the crystal calibration constants from the different calibraitons before and after merger : to understand the merger
   * differences in the times of the two highest energy electrons/photons, which cancels out the event t0: understand how 
     self-consistent the ECL times are.


=====================
Break down of some of the more important plot names


These plot names apply to the crystal calibration:
(The names are similar to those in the crate calibration because the same algorithm is run)

   TH2F TimevsCrysPrevCrateCalibNoCrystCalib       --- important !  used by the algorithm as the input.  
                                                       Times vs crystal where the previous crate calibration is included in the time calculation
                                                       but no crystal calibration constants were applied
   TH2F TimevsCrateNoCrateCalibPrevCrystCalib      --- important !  used by the algorithm as the input.  
                                                       Times vs crate where the previous crystal calibration is included in the time calculation
                                                       but no crate calibration constants were applied

   TH1D h_crysHits                                 --- number of hits to each crystal

   TH1D h_time_psi1                                --- histogram of the timing information for crystal 1.  A similar plot exists for each crystal.  
                                                       The histogram stat box mean from this histogram may be used to calculate the crystal calibration 
                                                       constant if the fit doesn't work.  important !

   TH1D h_time_psi_masked1                         --- histogram of the timing information for crystal 1 after cutting off the tails and doing a fit.  
                                                       A similar plot exists for each crystal.  The mean value from this fit is used to calculate the 
                                                       crystal calibration constant if the fit is good.  IMPORTANT !
   
   TH1F tsNew_MINUS_ts*     --- plots and histograms showing the change in the crystal calibration constants.  Pay attention to if the older ts values
                                are from the previous iteration or from before the first iteration.  Also pay attention to which ts values are used 
                                (only those from this bhabha calibration or the merged ts values? etc).  Some of these are useful to see if the crystal
                                calibration constants have converged.






These plots names apply ot the crate calibration root files for individual runs, e.g.   eclBhabhaTAlgorithm_14_948-14_948.root
(The names are similar to those in the crate calibration because the same algorithm is run)

   TH2F TimevsCrysPrevCrateCalibNoCrystCalib       --- important !  used by the algorithm as the input.  
                                                       Times vs crystal where the previous crate calibration is included in the time calculation
                                                       but no crystal calibration constants were applied
   TH2F TimevsCrateNoCrateCalibPrevCrystCalib      --- important !  used by the algorithm as the input.  
                                                       Times vs crate where the previous crystal calibration is included in the time calculation
                                                       but no crate calibration constants were applied

   TH1D h_crysHits                                  --- number of hits to each crystal.  Note required for the crate calibration but
                                                        might help the user determine which individual runs are best for doing studies 
                                                        based on statistics per crystal.

   TH1D h_time_psi_crate1                           --- histogram of the timing information for crate 1.  A similar plot exists for each crate.  
                                                        The histogram stat box mean from this histogram may be used to calculate the crate
                                                        calibration constant if the fit doesn't work.  important !
   TH1D h_time_psi_crate_masked1                    --- histogram of the timing information for crate 1 after cutting off the tails and 
                                                        doing a fit.  A similar plot exists for each crate.  The mean value from this 
                                                        fit is used to calculate the crystal calibration constant if the fit is good.  IMPORTANT !
   TH1D h_time_psi_crate_rebinned1                  --- histogram of the timing information for crate 1 that has been rebinned.  This helps to show
                                                        how the position of the tails to be remove are found.
   TH1F tcrateNew_MINUS_tcrateOld*                  --- plots and histograms showing the change in the crate calibration constants.  Pay attention to 
                                                        if the older tcrate values are from the previous iteration or from before the first iteration.
                                                        Some of these are useful to see if the crate calibration constants have converged.




These plot names apply to the crate calibration summary root file:  eclBhabhaTAlgorithm_cratesAllRuns.root

  TH1F tcrateNew_MINUS_tcrateOld_allRuns                 --- Histogram of the change in the crate time calibration constants between the current
                                                             iteration and the previous iteration.  This histogram only includes the crates for which 
                                                             the uncertainty on the calibration constant is relatively small so that runs with low 
                                                             statistics aren't shown
  TH1F tcrateNew_MINUS_tcrateOld_allRuns_allCrates       --- Histogram of the change in the crate time calibration constants the current iteration and 
                                                             the previous iteration. This histogram includes all the crates from all runs, 
                                                             regardless of the statistics.
  TH1I num_tcrates_perRun                                --- Plot of the number of good quality crate time calibration constants (small uncertainty)
                                                             found in each run.
  TH2F tcrateNew_MINUS_tcrateOld__vs__runNum             --- 2D histogram of the change in the crate time calibration constants as a function of
                                                             the run number.  The histogram is only filled for crates that have a small uncertainty 
                                                             on the latest iteration of the crate time calibration constant.  The change plotted is 
                                                             the difference between the most recent iteration of the crate time calibration constant
                                                             and the previous iteration.


These plot names apply to both the bhabha validations and the hadron validations because the same algorithm is run:
   TH1F clusterTime                 --- histogram of all cluster times
   TH2F clusterTime_cid             --- 2D histogram of cluster times vs crystal id
   TH2F clusterTime_run             --- 2D histogram of cluster times vs run number
   TH2F clusterTimeClusterE         --- 2D histogram of cluster times vs cluster energy
   TH2F dt99_clusterE               --- 2D histogram of dt99 vs cluster energy
   TH1F eventT0                     --- histogram of the event t0s
   TH1F clusterTimeE0E1diff         --- histogram of the difference in times of the two highest energy particles
   
   TH1D h_time_psi1                 --- histogram of the timing information for crystal 1.  A similar plot exists for each crystal
   TH1D h_time_psi_masked1          --- histogram of the timing information for crystal 1 after cutting off the tails and doing a fit.  
                                        A similar plot exists for each crystal


   TH1F peakClusterTime_cid               --- plots the mean value from the Guassians fitted to the time histograms above vs cell id.  
                                              Includes all crystals.
   TH1F peakClusterTimes                  --- histogram of the above Guassian means
   TH1F peakClusterTimesGoodFit__cid      --- plots the mean value from the Guassians fitted to the time histograms above vs cell id.  
                                              Only crystals with decent fits. 
   TH1F peakClusterTimesGoodFit           --- histogram of the above Guassian means 

   TGraphAsymmErrors  g_clusterTimeFractionInWindow__cid                 --- plot the fraction of cluster times within the window, for all crystals
   TH1F clusterTimeFractionInWindow                                      --- histogram of the above fractions
   TGraphAsymmErrors  g_clusterTimeFractionInWindowInGoodECLRings__cid   --- plot the fraction of cluster times within the window, but ignoring
                                                                             rings near gaps etc.

   
   TH1D h_time_E_slice0.025000             --- histogram of the timing information for particles of approximately 0.025 GeV. 
                                               A similar plot exists for each energy bin
   TH1D h_time_E_slice_masked0.025000      --- histogram of the timing information for particles of approximately 0.025 GeV after cutting off
                                               the tails and doing a fit.  A similar plot exists for each energy bin

    
   TH1F clusterTimePeak_ClusterEnergy_varBin              --- plot of the above Guassian fit means to the energy binned time 
                                                              histograms, plotted vs energy
   TH1F clusterTimePeakWidth_ClusterEnergy_varBin         --- plot of the above Guassian fit sigmas to the energy binned time
                                                               histograms, plotted vs energy
   TH1F tsNew_MINUS_tsCustomPrev__cid                     --- a crystal convergence plot: new minus pre-calibration values.  
                                                              the ts values are from after the ts merger for both
   TH1F tsNew_MINUS_tsCustomPrev                          --- a histogram of the above convergence plot.






These plot names apply to the crate time jumps plotting:   e.g. eclTimeShiftsAlgorithm_14_694-14_1772.root


   TGraphErrors crateTimeVSrunNum__14_694-14_1772_crate01               --- a graph of the crate time calibration constants themselves vs run number
                                                                            for crate 1.  A similar plot exists for the other crates.
   TGraphErrors crystalCrateTimeVSrunNum__14_694-14_1772_crate01        --- a graph of the t_zeroed = (ts+tcrate+userOffset)-mean vs run number
                                                                            for crate 1.  The mean value is the mean of (ts+tcrate) over all runs.
                                                                            By default the userOffset is 0 but the user can change this through
                                                                            input parameters.  A similar plot exists for the other crates.
   TGraphErrors crystalCrateTimeVSrunCounter__14_694-14_1772_crate01    --- a graph of t_zeroed vs run counter for crate 1.  See above for the 
                                                                            definition of t_zeroed.  The run counter is a re-numbering of the 
                                                                            run numbers to remove gaps.  A similar plot exists for the other crates.




====================
Talks about the code (lots of useful backup slides):
https://indico.belle2.org/event/4338/contributions/21152/attachments/10778/16544/EwanHill_2021_05_May_06__ECLtimingCalibNewCodeSummary_v03.pdf
https://indico.belle2.org/event/4490/contributions/23235/attachments/11455/17475/EwanHill_2021_06_June_14__B2GM_ECL_ECLtiming_v01.pdf

