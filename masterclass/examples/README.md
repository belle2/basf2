
/belle/Data/release-06-00-08/DB00000498/PromptExp22/prod00029655/e0022/4S/r00000/mdst
/belle/Data/release-06-00-08/DB00000498/PromptExp22/prod00029662/e0022/4S/r00000/mdst

gbasf2 /home/belle2/kspenko/basf2/masterclass/examples/masterclass.py \
        -n 10000000 \
        -p masterclassNewDataset \
        -s (my development version) \
        -i /belle/Data/release-06-00-08/DB00000498/PromptExp22/prod00029662/e0022/4S/r00000/mdst/sub00 \
       
basf2 -n10000000 -i "/group/belle2/dataprod/MC/MC15ri/charged/sub00/*.root" masterclass.py 
basf2 -n10000000 -i "/group/belle2/dataprod/Data/PromptSkim/e0022/4S/r00469/skim/hadron/raw/sub00/*.root" masterclass.py

gb2_ds_list /belle/Data/release-06-00-08/DB00000498/PromptExp22/prod00029662/e0022/4S/r00000/mdst/sub00
gb2_ds_get /belle/Data/release-06-00-08/DB00000498/PromptExp22/prod00029655/e0022/4S/r00000/mdst/sub00/mdst_000001_prod00029655_task210000001.root


basf2 -n10000000 -i "/group/belle/users/kspenko/data/masterclass/sub00/*.root" masterclass.py 
bsub -q s 'basf2 -n10000000 -i "/group/belle/users/kspenko/data/masterclass/sub00/*.root" masterclass.py'