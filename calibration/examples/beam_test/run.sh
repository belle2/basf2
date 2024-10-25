#b2caf-prompt-run Local configEcms_test.json  input_data_bucket37_highStat.json 
#b2caf-prompt-run Local configEcms_test.json  input_data_bucket37_lowStatTest.json

b2caf-prompt-run Local configEcms.json  input_data_bucket37_highStat.json # &> output.txt &


#nohup b2caf-prompt-run LSF --max-files-per-subjob 1 configEcms.json  input_bucket37_all_cdsts.json   &> output.txt &
#echo $! > process.txt
