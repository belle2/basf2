# @cond
import json
skimfiles = json.loads(open(snakemake.input.skim_dirs, "r").read())
outputfiles = snakemake.output
BatchesPerSkim = snakemake.params.BatchesPerSkim

binwidth = int(len(skimfiles)/BatchesPerSkim)
if(binwidth < 1.):
    raise ValueError("Attempting to batching with binwidth smaller 1. Decrease the number of batches!")

batches = {}
for batch in range(BatchesPerSkim):
    if(batch == BatchesPerSkim - 1):
        batches.update({outputfiles[batch]: list(skimfiles[binwidth*batch:])})
    else:
        batches.update({outputfiles[batch]: list(skimfiles[binwidth*batch:binwidth*(batch+1)])})

for key, file_list in batches.items():
    with open(key, "w") as f:
        f.write(json.dumps(file_list))
# @endcond
