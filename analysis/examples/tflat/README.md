# TFlaT

This README describes the training process of the transformer based flavortagger TFlaT.\
The provided scripts cover all steps that are required to get from the mdst samples to a weightfile that can be uploaded into the conditions database.

---

## Training Samples

The Training of TFlaT requires $B^0 \rightarrow \nu \overline{\nu}$ samples to be produced. The baseline performance was achieved with a dataset of 50M MC16 run dependent samples.

---

## Hardware Requirements
The training process requires a CUDA capable GPU.\
The time needed to complete a training depends on the specific GPU. For a NVIDIA A100 GPU the expected time to completion with 50M samples is ~4 days.\
Depending on the hardware used for the training some of the parameters found in the basf2/analysis/scrips/tflat/config.py file might need to be adjusted.\
If the VRAM of the used GPU is not sufficient, reduce the value of the 'batch_size' parameter.\
If the system memory is not sufficient, reduce the value of the 'chunk_size' parameter. Not that for optimal efficiency the 'chunk_size' should be an integer multiple of the 'batch_size'.\
If the GPU utilization is less than ~70% it might be possible to speed up the training by increasing the 'batch_size' parameter.

---

## Step-by-Step Training Guide

1. **Configuration**
   - Create new .yaml config file in analysis/data with parameters for your training (start with the latest version as a baseline).\
      The name of the file will also be the uniqueIdentifier of the final weightfile.
   - Possible update 'VersionBeamBackgroundMVA' and 'VersionFakePhotonMVA' in the config file if newer versions are available.

2. **Sample training data**
   - Run sampler script on grid
    ```bash
    gbasf2 sampler.py -p {name} -i {collection_name} -s {basf2_release} --basf2opt='' -n 3
    ```
   - Download the resulting ntuples

3. **Prepare training data**
   - The training samples need to be split into a training and validation dataset.
   The datasets also need to be shuffled and split into chunks to allow them to be loaded into memory piece by piece.
   - All of this is done by running the `merge_samples_to_parquet.py` script:
    ```bash
    python3 merge_samples_to_parquet.py ----root_dir /path/to/input_root/ --parquet_dir /path/to/output_parquet/ --uniqueIdentifier {uniqueIdentifier}
    ```
    - `/path/to/input_root/` is the path to the directory containing the root outputs produced by the sampler script.
    - The resulting training and validation files are saved to the `/path/to/output_parquet/` directory

4. **Training**
    - To launch the training use the `trainer.py` script:
    ```bash
   python3 trainer.py --train_input /path/to/output_parquet/tflat_training_samples.parquet --val_input /path/to/output_parquet/tflat_validation_samples.parquet --uniqueIdentifier {uniqueIdentifier}
    ```
    - Should the training crash at any point it can be restarted from the latest checkpoint like this:
    ```bash
   python3 trainer.py --train_input /path/to/output_parquet/tflat_training_samples.parquet --val_input /path/to/output_parquet/tflat_validation_samples.parquet --uniqueIdentifier {uniqueIdentifier} --warmstart
    ```
   - Once the training is done a `localdb` that contains the packaged onnx weightfile is produced.
