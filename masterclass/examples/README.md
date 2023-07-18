
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


basf2 -n10 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py 
basf2 -n10000000 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py 
bsub -q s 'basf2 -n10000000 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py'

for test and validation
/group/belle2/dataprod/MC/MC15ri/charged/sub00/mdst_000587_prod00024816_task10020000589.root



basf2 -n10 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py 



gb2_ds_search collection --list_datasets /belle/Data/release-06-00-08/DB00000498/PromptExp26/ > LPN_masterclass.list

# full exp26 dataset
gbasf2 -p masterclass2 -s release-06-01-12 masterclass.py -i /belle/Data/release-06-00-08/DB00000498/PromptExp26/prod00029657/e0026/4S/r00000/mdst/sub00 -f="libmasterclass_modules.so, libmasterclass_modules.b2modmap" --platform EL7 --basf2opt='-n 10'

# one file
gbasf2 -p masterclass2 -s release-06-01-12 masterclass.py --input_dslist LPN_masterclass.list -f="libmasterclass_modules.so, libmasterclass_modules.b2modmap" --platform EL7 --basf2opt='-n 10'