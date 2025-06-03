# FEI Training

**Instructions for a more customizable training of FEI**

## Table of Contents
- [Overview](#overview)
- [Stop/resume](#stopresume)
- [Rebasing](#rebasing)
- [Retrain](#retrain)
- [Reduced](#reduced)
- [Notes](#notes)

## Overview
Each stage of the FEI training is segmented into the following steps:
1. BDT training on an already prepared dataset from the previous stage.
2. Submission of jobs, which has three steps:
    - Application of the trained BDT to the particles from the "previous" stage: PostReconstruction (which marks the end of a stage)
    - Reconstruction of candidates for the "next" stage: PreReconstruction
    - Preparation of training_input.root data for BDT training. 
3. Merging of training_input.root files and Monitor_*.root files from all jobs to collection
4. Update of input files: basf2_output.root overwrites the previous basf2_input.root, and Summary.pickle is updated in collection
5. Clean up of the job directories (removes logs and previous monitoring files)

## Stop/resume
One can run the FEI training to any stage, by using the **-e or --end_stage** option.
- In the last specified stage of the training procedure BDTs of that stage will be trained and monitoring plots will be created for PostReconstruction.
- PreReconstruction and TrainingData for next stage are not created.
When retraining from a stage EARLIER than the last stage, one needs to pass the **-r or --retrain** option with the stage number. Also option **-x retrain** is needed.

## Rebasing
Just run till a specific stage using **-e or --end_stage** option.
After this create a new training directory and copy the following contents from the collection to a new empty collection:

`python3 copy_for_rebasing.py <source_collection> <target_collection>`

Then you can start the training from the new collection using the argument **-x rebase**.
This way you can train lower stages on less data.
And then rebase to more data, and train only the last stages on more data.
Things to keep in mind:
- BDTs have a maximum number of samples they use for training, so rebasing might not save you much time.
- when rebasing, training_input.root files will be created for the new data because particles from previous stages are also used for the following stages decay channels.
- Monitor_TrainingData.root does not overwrite previous stages as it keeps info of data used for training.

## Retrain
When retraining from a stage earlier than the last stage, one needs to pass the **-r or --retrain** option with the stage number. Also option **-x run** is needed.
- This will first remove all BDTs, Monitoring info, and particle lists in jobs' basf2_input.root NOT connected to stages before and excluding the retrain stage.
- Already trained channels not specified in the channels list will not be removed, but also will not be used during the remainder of the training.
- This will retrain the BDTs from that stage onwards.
- Payloads in localdb are not removed, so retraining channels will create additional payloads with newer revisions.

In order to retrain, the following lines of code need to present in the steering file (for cleaning up already registered particle lists from higher stages):
```python
if pickle.load(open('Summary.pickle', 'rb'))[1].roundMode == 3: 
    output.param('excludeBranchNames', feistate.excludelists)
```

## Reduced
Since FEI creates particle lists for every channel, which are then merged and filtered into a single particle list (per particle type),
there is around 40% of particles in basf2_input/output.root that become redundant after the training. 
It is possible to minimize the FEI training directory by adding these lines in the steering file:

```python
output = b2.register_module('RootOutput')

# region for reduced training
required_lists = feistate.plists + feistate.fsplists
if len(required_lists) > 0:
     mod = b2.register_module('RemoveParticlesNotInLists')
     mod.param('particleLists', required_lists)
     path.add_module(mod)
     
from ROOT import Belle2
if os.path.exists('basf2_input.root'):  
  root_file = ROOT.TFile.Open('basf2_input.root', "READ")
  tree = root_file.Get('tree')
  datastorelists = []
  for branch in tree.GetListOfBranches():
    if ":" not in branch.GetName():
      datastorelists.append(branch.GetName())

  listWithConjugates = [str(name) for name in list(Belle2.ParticleListName.addAntiParticleLists(required_lists))]
  output.param('branchNames', listWithConjugates + datastorelists)
# endregion

path.add_module(output)
```

## Notes
Things to potentially work on:
- Be careful -l (directory for large files) creates absolute not relative symlinks.
- Merging of Monitor_ModuleStastics.root might not be working the way it is intended.
- The file mcParticlesCount.root gets calculated only at the beginning of the training, so if particles with new PDGs, this file will not be updated, 
  which is why you have to start the training from scratch. This could be improved, but the situation isn't expected to happen often.
- FastBDTs have the option to include flatnessLoss, which can mitigate some unwanted correlations with the "spectator" variables. However, this option does not seem to have been implemented in FEI as the default is flatnessLoss=-1 (which means it's turned off).