#!/bin/tcsh -m

set MCcampaign='MC9'
set release='release-00-09-01'
set DBtag='DB00000276'
set DBtag2='DB00000265'

foreach DType (mixed charged ccbar uubar ddbar ssbar taupair) 

echo $DType
if ($DType == "mixed") then
 set prodN1 = 2288
 set prodN2 = 2166
else if ($DType == "charged") then 
 set prodN1 = 2289
 set prodN2 = 2167
else if ($DType == "uubar") then
 set prodN1=2290
 set prodN2=2168
else if ($DType == "ddbar") then
 set prodN1=2311
 set prodN2=2169
else if ($DType == "ssbar") then
 set prodN1=2312
 set prodN2=2170
else if ($DType == "ccbar") then
 set prodN1=2321
 set prodN2=2171
else if ($DType == "taupair") then
 set prodN1=2322
 set prodN2=2172
endif


set filename1='inputFiles/'$MCcampaign'/'$DType'BGx1.txt'

set filename2='inputFiles/'$MCcampaign'/'$DType'BGx0.txt'


set output1='/ghi/fs01/belle2/bdata/MC/'$release'/'$DBtag'/'$MCcampaign'/prod0000'$prodN1'/e0000/4S/r00000/'$DType'/'

set input1='/belle/MC/release-00-09-01/DB00000276/MC9/prod0000'$prodN1'/e0000/4S/r00000/'$DType'/sub00/mdst_000001_prod0000'$prodN1'_task00000001.root'

echo 'getting file from '$input1' and downloading to '$output1

set inputFile1=$output1'sub00/mdst_000001_prod0000'$prodN1'_task00000001.root'
echo $inputFile1 > $filename1
cd $output1
gb2_ds_get $input1



set output2='/ghi/fs01/belle2/bdata/MC/'$release'/'$DBtag2'/'$MCcampaign'/prod0000'$prodN2'/e0000/4S/r00000/'$DType'/'
set input2='/belle/MC/release-00-09-01/DB00000276/MC9/prod0000'$prodN1'/e0000/4S/r00000/'$DType'/sub00/mdst_000001_prod0000'$prodN1'_task00000001.root'

echo 'getting file from '$input2' and downloading to '$output2

set inputFile2=$output2'sub00/mdst_000001_prod0000'$prodN2'_task00000001.root'
echo $inputFile2 > $filename2
cd $output2
gb2_ds_get $input2
end
end
