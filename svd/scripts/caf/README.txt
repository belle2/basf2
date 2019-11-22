How to run the SVD CoG calibration using CAF:

1) run basf2 SVDCoGTimeCalibrationCAF.py -i "list of input files"
2) run source CalibrationCheck_Input_Files_List_tool.sh "same of the folder with the input txt files for the check"
3) run source SVDCoGTimeCalibrationCheck.sh "exp_number" "txt_folder produced in 2)" "localDB produced in 1)" "log files output folder"
