#!/bin/bash     

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This script is used to train and to test the flavor tagger while performing vertexing at kekcc.
# It defines the whole procedure for release validation.
# The jobs are submitted to the batch system at kekcc to produce official weight files.
# After the testing all the scripts validating the vertexing and the flavor tagging performance are used.
# The script B0_GenDeltaTFit.py checks if the generated CP asymmetries are correct.

BelleOrBelle2=${1} # Belle (For converted B2BII MC)  or  Belle2
trainDecayChannel=${2} # JPsiKs of nunubar
testDecayChannel=${3} # JPsiKs of nunubar
MCType=${4} # BGx0 or BGx1 (without Machine Background or with)
yourPathForWeightFiles=${5} # Set your own path to save the weight files and the sampled files
yourPathForAnalyzedFiles=${6} # Set your own path to save the output root files.

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

# Queue where we will run the jobs
jobQueue="l"

# Ids needed to identify the jobs 
trainID=''
if [ ${trainDecayChannel} = "JPsiKs" ]
    then 
    trainID='J' 
    elif [ ${trainDecayChannel} = "nunubar" ]
    then
    trainID='N'
    fi

testID=''
if [ ${testDecayChannel} = "JPsiKs" ]
    then 
    testID='J' 
    elif [ ${testDecayChannel} = "nunubar" ]
    then
    testID='N'
    fi

processID=''
TeachingProcessID='zampano'

for ((level = 1 ; level < 5; level++))
    # Level 1 samples and trains the Event Level, 
    # Level 2 samples and trains the Kaon-Pion category (Event Level), 
    # Level 3 samples and trains the combiner,
    # Level 4 tests the flavorTagger.
    do

    decayChannel=''

    sample='noGivenFiles'   
    
    sampleNumber=1
    doVertex='True' #'False'

    eventLimit=''

    fileEnding='.root'

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
        jobQueue="s"
        fileEnding=".mdst"
        sampleFilesForAsym='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleMC_'${decayChannel}'/sample-01/'
        sample='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleMC_'${decayChannel}'/sample-0'${sampleNumber}'/'
        # eventLimit='-n 2500'
        processID='11'
        fi
 
 
    if [ ${BelleOrBelle2} = "Belle2" ] && [ ${MCType} = "BGx1" ]
        then
        # *** The three samples of Belle II MC with machine background (level 0, 1 for training and level 2 for testing) are saved under
        sampleFilesForAsym='/gpfs/fs02/belle2/users/abudinen/dstFiles/MC13a/Bd_'${decayChannel}'_BGx1/sample-01/'
        sample='/gpfs/fs02/belle2/users/abudinen/dstFiles/MC13a/Bd_'${decayChannel}'_BGx1/sample-0'${sampleNumber}'/'
        # eventLimit='-n 2500'
        processID='21'
        fi

    mode='Expert' 
    if ((level < 4))
        then
        # In sampled mode only the training events are sampled into root files
        mode='Sampler' 
        fi
    
    # Check that previous processes in the batch system are finished
    nJobs=`bjobs | grep ${trainID}${testID}${processID} | grep -v grep | wc -l`
    while [ ${nJobs} -ge 1 ]
        do
        sleep 30
        nJobs=`bjobs | grep ${trainID}${testID}${processID} | grep -v grep | wc -l`
        done

    # Check that previous teaching procedures are finished
    nJobs=`ps | grep ${TeachingProcessID} | wc -l`
    while [ ${nJobs} -ge 1 ]
        do
        echo `ps | grep ${TeachingProcessID} | grep -v grep`
        sleep 30
        nJobs=`ps | grep ${TeachingProcessID} | wc -l`
        done

    fileNumber=0

    echo ${sample}
 
    # Samples or tests the given events in the sample
    for i in `find ${sample} -name '*'${fileEnding} -printf "%f\n"`
        do
    
        fileNumber=$((fileNumber + 1)) 
        fileNumberToPrint=$(printf "%04d" ${fileNumber})
        fverb=${BelleOrBelle2}${MCType}level${level}$(echo ${i}| sed 's/.root/.out/g')
 
        echo ${sample}${i} ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} ${MCType} \
             ${sampleNumber}${fileNumberToPrint} ${workingDirectory} ${savingDirectory} ${doVertex}
 
        bsub -q ${jobQueue} -J ${trainID}${testID}${processID}${level}M${fileNumber} -o ${workingDirectory}'/'${fverb} \
             basf2 ${eventLimit} -l ERROR -i ${sample}${i} flavorTaggerVertexingValidation.py -- -bob2 ${BelleOrBelle2} \
             -m ${mode} -trc ${trainDecayChannel} -dc ${testDecayChannel} -mct ${MCType} \
             -fn ${sampleNumber}${fileNumberToPrint} -wd ${workingDirectory} -sd ${savingDirectory} -dv ${doVertex}
 
        if ((level==1)) && ((fileNumber==1))
            # Just wait a bit if this is the first process in order to give time to create directories
            then
            sleep 40
            fi            
        done
    
    if ((level < 4))
        # Teaching procedure after sampling 
        then
        nJobs=`bjobs | grep ${trainID}${testID}${processID} | wc -l`
        while [ ${nJobs} -ge 1 ]
            do
            echo 'Found jobs in queue = '${nJobs}
            sleep 30
            nJobs=`bjobs | grep ${trainID}${testID}${processID} | wc -l`
            done
 
         echo TeacherLevel${level}   
         # bsub -q h -J ${trainID}${testID}${processID}T${level} -o ${workingDirectory}'/'TeacherLevel${BelleOrBelle2}${MCType}${level}.out \
         #      basf2 flavorTaggerVertexingValidation.py -- -bob2 ${BelleOrBelle2} -m Teacher -trc ${trainDecayChannel} \
         #      -dc ${testDecayChannel} -mct ${MCType} -wd ${workingDirectory} -sd ${savingDirectory} 
         # A training job in the queues takes about one day (no paralellization in training). 
         # Running on a cx0? machine at kekcc, it takes about 4 hours.
         # A single nohup job per machine is allowed. 
         nohup basf2 flavorTaggerVertexingValidation.py -- -bob2 ${BelleOrBelle2} -m Teacher -trc ${trainDecayChannel} \
               -dc ${testDecayChannel} -mct ${MCType} -wd ${workingDirectory} -sd ${savingDirectory} >& \
                ${workingDirectory}'/'TeacherLevel${BelleOrBelle2}${MCType}${level}.out &
         TeachingProcessID=$!
         echo 'TeachingProcessID = '${TeachingProcessID}

         fi
 
    if ((level == 4)) && [ ${BelleOrBelle2} = "Belle" ] && [ ${MCType} = "BGx1" ] && [ ${testDecayChannel} = "JPsiKs" ]
        then
        
        echo Test with Belle Data
        belleDataSample='/gpfs/fs02/belle2/users/abudinen/dstFiles/BelleData_JpsiKs/'
        fileNumber=0
        doVertex='False'
        for i in `ls -1 ${belleDataSample}`
            do
 
            fileNumber=$((fileNumber + 1))
            fverb=${BelleOrBelle2}${MCType}level${level}$(echo ${i}| sed 's/.root/.out/g')
 
            echo ${belleDataSample}${i} ${BelleOrBelle2} ${mode} ${trainDecayChannel} ${testDecayChannel} \
                 ${MCType} ${fileNumber} ${workingDirectory} ${savingDirectory} False BelleDataConv 
 
            bsub -q ${jobQueue} -J ${trainID}${testID}${processID}${level}D${fileNumber} -o ${workingDirectory}'/'${fverb} \
                 basf2 -l ERROR -i ${belleDataSample}${i} flavorTaggerVertexingValidation.py -- -bob2 ${BelleOrBelle2} \
                 -m ${mode} -trc ${trainDecayChannel} -dc ${testDecayChannel} -mct ${MCType} -fn ${fileNumber} \
                 -wd ${workingDirectory} -sd ${savingDirectory} -dv ${doVertex} -bd BelleDataConv 
 
            done
 
        fi
    done
 
#Wait until testing is finished
nJobs=`bjobs | grep ${trainID}${testID}${processID} | wc -l`
while [ ${nJobs} -ge 1 ]
    do
    echo 'Found jobs in queue = '${nJobs}
    sleep 30
    nJobs=`bjobs | grep ${trainID}${testID}${processID} | wc -l`
    done
 
mkdir -p ${workingDirectory}'/FlavorTagging/ValidationPlots'${BelleOrBelle2}${MCType} 
 
currentpath=$(pwd)

# Now all the validation plots are produced

 
cd ${workingDirectory}'/FlavorTagging/ValidationPlots'${BelleOrBelle2}${MCType}

# Check if the generated CP asymmetry and the asymmetries between positively and negatively charged particles make sense
bsub -q l -J ${trainID}${testID}${processID}GDT -o B0_JpsiKs_GenDeltaTFitResults.txt \
     "basf2 -n 2000000 -i '${sampleFilesForAsym}*.root' ${currentpath}/B0_GenDeltaTFit.py" 
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}gLI -o genLevelAsymmsImpactParams.txt \
     "basf2 ${currentpath}/genLevelAsymmsImpactParams.py ${BelleOrBelle2} '${sampleFilesForAsym}*.root'" 
 
# Make Input Variable Plots
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}iVP -o inputVariablesPlots.txt \
     basf2 ${currentpath}/inputVariablesPlots.py ${BelleOrBelle2} ${MCType} ${trainDecayChannel} ${workingDirectory}
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}aIV -o asymmetriesInVariablesPlots.py.txt \
     basf2 ${currentpath}/asymmetriesInVariablesPlots.py ${BelleOrBelle2} ${MCType} ${trainDecayChannel} ${workingDirectory} 

# Calculate efficiencies and produce validation plots with root script
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}vE -o validationEfficiencies.txt \
     "basf2 ${currentpath}/flavorTaggerEfficiency.py '${savingDirectory}/B2A801-FlavorTaggerExpert*${BelleOrBelle2}${MCType}*.root' B0tree" 

 
if [ ${BelleOrBelle2} = "Belle2" ] 
    then
    bsub -q l -J ${trainID}${testID}${processID}dTR -o deltaTResPXD0.txt \
         "basf2 ${currentpath}/deltaTVertexTagVResolution.py \
         '${savingDirectory}/B2A801-FlavorTagger*${BelleOrBelle2}${MCType}*.root' B0tree PXD" 
    else
    bsub -q l -J ${trainID}${testID}${processID}dTR -o deltaTResSVD0.txt \
         "basf2 ${currentpath}/deltaTVertexTagVResolution.py \
         '${savingDirectory}/B2A801-FlavorTagger*${BelleOrBelle2}${MCType}*.root' B0tree SVD"
    fi

 
if [ ${BelleOrBelle2} = "Belle" ] && [ ${MCType} = "BGx1" ] && [ ${testDecayChannel} = "JPsiKs" ]
     then
     # Produce validation plots comparing Belle B2BII data with Belle B2BII MC
     echo evaluating now  ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1.root' \
           ${savingDirectory}'/B2A801-FlavorTaggerExpert*BelleBGx1BelleDataConv.root'  
     bsub -q ${jobQueue} -J ${trainID}${testID}${processID}sP -o qrSPlot.txt \
          "basf2 ${currentpath}/B2JpsiKs_mu_qrBelleDataSplot.py \
          '${savingDirectory}/B2A801-FlavorTaggerExpert*BelleBGx1.root' \
          '${savingDirectory}/B2A801-FlavorTaggerExpert*BelleBGx1BelleDataConv.root'" B0tree
     fi
 
  
# Calculate efficiencies and produce validation plots with python script
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}pVal -o pyValidationEfficiencies.txt \
     "basf2 ${currentpath}/pythonFlavorTaggerEfficiency.py \
     '${savingDirectory}/B2A801-FlavorTaggerExpert*${BelleOrBelle2}${MCType}*.root' B0tree"

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

if [ ${trainDecayChannel} = "JPsiKs" ] 
    then
    identifier="B2JpsiKs_mu"
    fi

for iCategory in "${categories[@]}"                
    do                

    mkdir -p ${workingDirectoryForCatVal}'/EventLevel'${iCategory}
    
    bsub -q ${jobQueue} -J ${trainID}${testID}${processID}${iCategory} \
         -o ${workingDirectoryForCatVal}'/EventLevel'${iCategory}'/mva_evaluate.out' \
         ${currentpath}/ft_mva_evaluate.py \
         -id    "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDT_1.root" \
         -train "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDTsampled*.root" \
         -data  "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDTsampled*.root" \
         -tree "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDT_tree" \
         -out  "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}EventLevel${iCategory}FBDT.pdf"  \
         -w ${workingDirectoryForCatVal}'/EventLevel'${iCategory} -b2Orb ${BelleOrBelle2} 
      
    done 

mkdir -p ${workingDirectoryForCatVal}'/CombinerFBDT'    
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}FBDT -o ${workingDirectoryForCatVal}'/CombinerFBDT/mva_evaluate.out' \
      ${currentpath}/ft_mva_evaluate.py \
     -id    "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FBDT_1.root" \
     -train "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
     -data  "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
     -tree "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FBDT_tree" \
     -out  "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FBDT.pdf" \
     -w ${workingDirectoryForCatVal}'/CombinerFBDT' -b2Orb ${BelleOrBelle2} 

mkdir -p ${workingDirectoryForCatVal}'/CombinerFANN'    
bsub -q ${jobQueue} -J ${trainID}${testID}${processID}FANN -o ${workingDirectoryForCatVal}'/CombinerFANN/mva_evaluate.out' \
     ${currentpath}/ft_mva_evaluate.py \
     -id    "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FANN_1.root" \
     -train "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
     -data  "${workingDirectory}/FlavorTagging/TrainedMethods/FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}sampled*.root" \
     -tree "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FBDT_tree" \
     -out  "FlavorTagger_${BelleOrBelle2}_${identifier}${MCType}Combiner${CatCode}FANN.pdf" \
     -w ${workingDirectoryForCatVal}'/CombinerFANN' -b2Orb ${BelleOrBelle2} 


