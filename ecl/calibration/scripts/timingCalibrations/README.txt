This directory contain a number of alternative scripts for running the ecl timing calibrations and validations.

The methods used for the timing calibration are outlined in the following internal note:
https://docs.belle2.org/record/1276


!!!!!!!!!!!!!!!!
The main calibration script that should be used is
   calibration/scripts/prompt/calibrations/caf_ecl_time_crystalCrate.py
!!!!!!!!!!!!!!!!


The above main script performs the crystal and crate calibrations, executes the hadronic selection validation
and the bhabha self-consistency check "validation", and plots the crate times jump.

Should a user wish to execute only the calibrations or only the validations etc, they can use the scripts provided in this directory.
This might be useful because modifying the main script to skip certain steps might be quite difficult as many of the steps
have to be executed in a specific order.


The below scripts execute the various calibrations and validations using the airflow framework,
which executes both the collector and the algorithms.  To be used they have to be put in the 
   calibration/scripts/prompt/calibrations/
diretory; however, we don't want to keep them there permanently as the automated calibration system
will execute all scripts in that directory, which we don't want to do.  These scripts are used with
the "b2caf-prompt-run" command (or whatever the latest version is):
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
the time.   If the main script can be executed on just the individual components it all possible combinations desired, then some
or these scripts will become redundant; however, some of these scripts also provide examples on how to pass specific parameters
to the collectors and algorithms.  One important one is "run_eclBhabhaT_algorithm.py", which has an example of how to pass
reference crystal numbers to the algorithm so that it can make a new payload for you.


If being run locally on kekcc with the LSF batch system, this is an excample of how to execute the caf scripts:
 nohup b2caf-prompt-run   LSF    caf_eclBhabhaT_config_customBucket16_v17.json  input_data_bucket_16_custom_removedFailing20Runs_v01.json   > bucket16_2021_05_May_02_v02.2.out 2> bucket16_2021_05_May_02_v02.2.err < /dev/null   &

   where
      * caf_eclBhabhaT_config_customBucket16_v17.json is the configuration file telling b2caf to execute the one of the caf scripts,
        like the main one caf_ecl_time_crystalCrate.py
      * input_data_bucket_16_custom_removedFailing20Runs_v01.json gives the list of input cdst files with which to run over
      * bucket16_2021_05_May_02_v02.2.out is the highest level log file for outputs to the screen
      * bucket16_2021_05_May_02_v02.2.err is the highest level errors reporting file

It is best to see the manually on how to execute calibrations using the "b2caf-prompt-run" command.

Running with the unix commands "screen" and "nohup" is useful as the full caf scripts can take hours to execute if you are
doing so by yourself and not via the calibrations.belle2.org website.


A single full calibration performs:
1) a crate calibration
2) a crystal calibration
3) steps 1 and 2 potentially multiple times if more than one iteration is requested
4) the hadron validation, bhabha validation, and crate time jumps plotting all in parallel

The crystal and crate calibrations have to iterate to converge towards the correct calibrations.  More iterations may be required
if the crystals calibrations are expected to changed by a larger amount.


