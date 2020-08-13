How to run the SVD CoG calibration using CAF:

1) run basf2 SVDCoGTimeCalibrationCAF.py -i <list of input files>
   
   -It runs the CAF and produces the local database for the input file runs specified (ex. "/group/belle2/dataprod/Data/release-04-00-02/DB00000748/proc10/e0008/4S/r008*/skim/hlt_hadron/cdst/sub00/*.root") 
   
2) run source CalibrationCheck_Input_Files_List_tool.sh <name of the folder with the input txt files for the check>
   
   -Inside the script there is the path of the input data used, 
    it is necessary to change it if you want to run the script over a different set of data.
   -It creates a folder with one txt file for each run that contains the list of the files for that run
   -This lists of files are used by the script described in 3) to run the check of the calibration for all runs inside this folder

3) run source SVDCoGTimeCalibrationCheck.sh <exp_number> <txt_folder produced in 2)> <localDB produced in 1)> <log files output folder>
   
   -It runs the check of the calibration (in the localDB specified as third argument) for all runs contained in the folder created in 2)
