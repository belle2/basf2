#!/bin/bash     

# This script is used to train and to test the flavor tagger while performing vertexing at kekcc.
# It defines the whole procedure for release validation.
# After the testing all the scripts validating the vertexing and the flavor tagging performance are used.
# The script B0_GenDeltaTFit.py checks if the generated CP asymmetries are correct

BelleOrBelle2=${1} # Belle (For converted B2BII MC)  or  Belle2
trainDecayChannel=${2} # JPsiKs of nunubar
testDecayChannel=${3} # JPsiKs of nunubar
MCType=${4} # BGx0 or BGx1 (without Machine Background or with)
yourPathForWeightFiles=${5} # Set your own path to save the weight files and the sampled files
yourPathForAnalyzedFiles=${6} # Set your own path to save the output root files.

nParal=50 # Number of parallel processes

# For each training procedure, working and saving directories have to be modified. Marked with **

# ** working Directory of The Flavor Tagger
workingDirectory=${yourPathForWeightFiles}'/'${BELLE2_RELEASE}${trainDecayChannel}'Train-'${testDecayChannel}'Test'

# ** Directory to save the analysed reconstructed events while performing vertexing and/or using the trained Flavor Tagger
savingDirectory=${yourPathForAnalyzedFiles}'/'${BELLE2_RELEASE}${trainDecayChannel}'Train-'${testDecayChannel}'Test'

# In order to train for a specific release or to use an own signal side channel, The directories containing the 3 training samples and the testing sample
# have to be modified. Marked with ***
sampleFilesForAsym='noGivenFiles'

mkdir -p ${workingDirectory}
mkdir -p ${savingDirectory}

for ((level = 1 ; level < 5; level++))
   # Level 1 samples and trains the Event Level, Level 2 samples and trains the Kaon-Pion category (Event Level), Level 3 samples and trains the combiner,
   # Level 4 tests the flavorTagger
   do

   decayChannel=''

   sample='noGivenFiles'   
   processID=''
   
   sampleNumber=1
   doVertex='True' #'False'

   eventLimit=''

   if ((level > 1))
       then
       sampleNumber=$((level - 1))
       # doVertex='False'
       doVertex='True'
       fi

   if ((level < 4))
       then
       decayChannel=${trainDecayChannel}
       else
       decayChannel=${testDecayChannel}
       fi


   if [ ${BelleOrBelle2} = "Belle" ] && [ ${MCType} = "BGx1" ]
       then
       # *** The three samples of converted B2BII MC (level 0, 1 for training and level 2 for testing) are saved under 
       # sample='/gpfs/fs02/belle2/users/shekeln/dstFiles/Bd_JpsiKS_mumu-myBelle-2M-0'${sampleNumber}'/B2BII-root/'
       sampleFilesForAsym='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleMCConv_'${decayChannel}'/sample-01/'
       sample='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleMCConv_'${decayChannel}'/sample-0'${sampleNumber}'/'
       # eventLimit='-n 2500'
       processID='11'
       fi
 
 
   if [ ${BelleOrBelle2} = "Belle2" ] && [ ${MCType} = "BGx1" ]
        then
       # *** The three samples of Belle II MC with machine background (level 0, 1 for training and level 2 for testing) are saved under
     sampleFilesForAsym='/gpfs/fs02/belle2/users/abudinen/dstFiles/MC11/Bd_'${decayChannel}'_BGx1/sample-01/'
     sample='/gpfs/fs02/belle2/users/abudinen/dstFiles/MC11/Bd_'${decayChannel}'_BGx1/sample-0'${sampleNumber}'/'
       # eventLimit='-n 2500'
       processID='21'
        fi


    fileNumber=0
 
    mode='Expert' 
    if ((level < 4))
        then
        # In sampled mode only the training events are sampled into root files
        mode='Sampler' 
        fi
    
    # Check that previous teaching procedures are finished
    nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
    while [ ${nJobs} -ge 1 ]
        do
        sleep 30
        nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
        done
 
    # Samples or tests the given events in the sample
    for i in `find ${sample} -name '*.root' -printf "%f\n"`
        do
    
        fileNumber=$((fileNumber + 1)) 
        fileNumberToPrint=$(printf "%04d" ${fileNumber})
        fverb=${BelleOrBelle2}${MCType}level${level}$(echo ${i}| sed 's/.root/.out/g')
 
        nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
        while [ ${nJobs} -ge ${nParal} ]
            do
            sleep 30
            nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
            done
 
 
        echo ${sample}${i} ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} ${MCType} ${sampleNumber}${fileNumberToPrint} ${workingDirectory} ${savingDirectory} ${doVertex}
 
        nohup basf2 ${eventLimit} -l ERROR -i ${sample}${i} flavorTaggerVertexingValidation.py ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} ${MCType} ${sampleNumber}${fileNumberToPrint} ${workingDirectory} ${savingDirectory} ${doVertex}>& ${workingDirectory}'/'${fverb} &
 
        if ((level==1)) && ((fileNumber==1))
            # Just wait a bit if this is the first process in order to give time to create directories
            then
            sleep 40
            fi            
        done
    
    if ((level < 4))
    # Teaching procedure after sampling 
       then
        nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`     
        while [ ${nJobs} -ge 1 ]
            do
            sleep 30
            nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
            done
 
        echo TeacherLevel${level}   
        nohup time basf2 flavorTaggerVertexingValidation.py ${BelleOrBelle2} Teacher ${trainDecayChannel} ${testDecayChannel} ${MCType} ${fileNumber} ${workingDirectory} ${savingDirectory} ${doVertex}>& ${workingDirectory}'/'TeacherLevel${BelleOrBelle2}${MCType}${level}.out &
        fi
 
    if ((level == 4)) && [ ${BelleOrBelle2} = "Belle" ] && [ ${MCType} = "BGx1" ]
        then
        
        echo Test with Belle Data
        belleDataSample='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleDataConv_JpsiKs/'
        fileNumber=0
        doVertex='False'
        for i in `ls -1 ${belleDataSample}`
            do
 
            fileNumber=$((fileNumber + 1))
            fverb=${BelleOrBelle2}${MCType}level${level}$(echo ${i}| sed 's/.root/.out/g')
 
            nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
            while [ ${nJobs} -ge ${nParal} ]
                do
                sleep 30
                nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
                done
 
            echo ${belleDataSample}${i} ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} ${MCType} ${fileNumber} ${workingDirectory} ${savingDirectory} False BelleDataConv 
 
            nohup basf2 -l ERROR -i ${belleDataSample}${i} flavorTaggerVertexingValidation.py ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} ${MCType} ${fileNumber} ${workingDirectory} ${savingDirectory} False BelleDataConv >& ${workingDirectory}'/'${fverb}  &
 
            done
 
        fi
    done
 
 #Wait until testing is finished
 nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
 while [ ${nJobs} -ge 1 ]
     do
     sleep 30
     nJobs=`ps u | grep " basf2 " | grep -v grep | wc -l`
     done
 
mkdir -p ${workingDirectory}'/FlavorTagging/ValidationPlots'${BelleOrBelle2}${MCType} 
 
currentpath=$(pwd)

# Now all the validation plots are produced

 
cd ${workingDirectory}'/FlavorTagging/ValidationPlots'${BelleOrBelle2}${MCType}

# Check if the generated CP asymmetry and the asymmetries between positively and negatively charged particles make sense
nohup basf2 -n 2000000 -i ${sampleFilesForAsym}'*.root' ${currentpath}/B0_GenDeltaTFit.py >& B0_JpsiKs_GenDeltaTFitResults.txt &
nohup basf2 ${currentpath}/genLevelAsymmsImpactParams.py ${BelleOrBelle2} ${sampleFilesForAsym}'*.root' >& genLevelAsymmsImpactParams.txt &
 
# Make Input Variable Plots
nohup basf2 ${currentpath}/inputVariablesPlots.py ${BelleOrBelle2} ${MCType} ${trainDecayChannel} ${workingDirectory} >& inputVariablesPlots.txt &
nohup basf2 ${currentpath}/asymmetriesInVariablesPlots.py ${BelleOrBelle2} ${MCType} ${trainDecayChannel} ${workingDirectory} >& asymmetriesInVariablesPlots.py.txt &

# Calculate efficiencies and produce validation plots with root script
nohup basf2 ${currentpath}/flavorTaggerEfficiency.py ${savingDirectory}'/B2A801-FlavorTaggerExpert*'${BelleOrBelle2}${MCType}'*.root' B0tree >& validationEfficiencies.txt &

 
if [ ${BelleOrBelle2} = "Belle2" ] 
    then
    nohup basf2 ${currentpath}/deltaTVertexTagVResolution.py  ${savingDirectory}'/B2A801-FlavorTagger*'${BelleOrBelle2}${MCType}'*.root' B0tree PXD >& deltaTResPXD0.txt &
    else
    nohup basf2 ${currentpath}/deltaTVertexTagVResolution.py  ${savingDirectory}'/B2A801-FlavorTagger*'${BelleOrBelle2}${MCType}'*.root' B0tree SVD >& deltaTResSVD0.txt &
    fi

 
if [ ${BelleOrBelle2} = "Belle" ] && [ ${MCType} = "BGx1" ]
     then
     # Produce validation plots comparing Belle B2BII data with Belle B2BII MC
     nohup basf2 ${currentpath}/B2JpsiKs_mu_qrBelleDataSplot.py ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1.root' ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1BelleDataConv.root' B0tree >& qrSPlot.txt &
      echo evaluating now  ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1.root' ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1BelleDataConv.root'  
  
     fi
 
  
# Calculate efficiencies and produce validation plots with python script
nohup basf2 ${currentpath}/pythonFlavorTaggerEfficiency.py ${savingDirectory}'/B2A801-FlavorTaggerExpert*'${BelleOrBelle2}${MCType}'*.root' B0tree >& pyValidationEfficiencies.txt &

declare -a categories=(
               "Electron"
               "IntermediateElectron"
               "Muon"
               "IntermediateMuon"
               "KinLepton"
               "IntermediateKinLepton"
               "Kaon"
                "SlowPion"
                "FastHadron"
                "Lambda"
                "FSC"
                "MaximumPstar"
                "KaonPion"
                )

workingDirectoryForCatVal=${workingDirectory}'/FlavorTagging/ValidationPlots'${BelleOrBelle2}${MCType}                

CatCode='CatCode00010203040506070809101112' 

identifier="B2"${trainDecayChannel}

if [ ${trainDecayChannel} = "JpsiKs" ] 
   then
   identifier="B2JpsiKs_mu"
   fi

for iCategory in "${categories[@]}"                
    do                

      nJobs=`ps u | grep " python3 " | grep -v grep | wc -l`
      while [ ${nJobs} -ge ${nParal} ]
          do
          sleep 30
          nJobs=`ps u | grep " python3 " | grep -v grep | wc -l`
          done      
      
      
      
      mkdir -p ${workingDirectoryForCatVal}'/EventLevel'${iCategory}
      
      nohup ${currentpath}/ft_mva_evaluate.py \
        -id "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDT_1.root" \
        -train "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDTsampled*.root" \
        -data  "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDTsampled*.root" \
        -tree ${BelleOrBelle2}'_'${identifier}${MCType}'EventLevel'${iCategory}'FBDT_tree' \
        -out  ${BelleOrBelle2}'_'${identifier}${MCType}'EventLevel'${iCategory}'FBDT.pdf'  \
        -w ${workingDirectoryForCatVal}'/EventLevel'${iCategory} -b2Orb ${BelleOrBelle2} \
        >& ${workingDirectoryForCatVal}'/EventLevel'${iCategory}'/mva_evaluate.out' & 
        
    done 

mkdir -p ${workingDirectoryForCatVal}'/CombinerFBDT'    
nohup ${currentpath}/ft_mva_evaluate.py \
        -id "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FBDT_1.root" \
        -train "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
        -data  "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
        -tree ${BelleOrBelle2}'_'${identifier}${MCType}'Combiner'${CatCode}'FBDT_tree' \
        -out ${BelleOrBelle2}'_'${identifier}${MCType}'Combiner'${CatCode}'FBDT.pdf' \
        -w ${workingDirectoryForCatVal}'/CombinerFBDT' -b2Orb ${BelleOrBelle2} \
        >& ${workingDirectoryForCatVal}'/CombinerFBDT/mva_evaluate.out' & 

mkdir -p ${workingDirectoryForCatVal}'/CombinerFANN'    
nohup ${currentpath}/ft_mva_evaluate.py \
        -id "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FANN_1.root" \
        -train "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
        -data  "${workingDirectory}/FlavorTagging/TrainedMethods/${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
        -tree ${BelleOrBelle2}'_'${identifier}${MCType}'Combiner'${CatCode}'FBDT_tree' \
        -out ${BelleOrBelle2}'_'${identifier}${MCType}'Combiner'${CatCode}'FANN.pdf' \
        -w ${workingDirectoryForCatVal}'/CombinerFANN' -b2Orb ${BelleOrBelle2} \
        >& ${workingDirectoryForCatVal}'/CombinerFANN/mva_evaluate.out' & 


