#!/bin/bash
# We generate a few files in directories that are separated by run number.
# Roughly similar to the data that is used for the prompt.

# An input_data.json file is created to use as input to the example.

if [ -d ./test_prompt_data ]; then
  rm -rf ./test_prompt_data
fi

INITIAL_RUN=1000
MAX_RUN=1032

for ((i=$INITIAL_RUN;i<=$MAX_RUN;i++)); do
  if ! ((i % 4)); then
    # Create directory to store the file
    mkdir -p test_prompt_data/$i
    basf2 ../generate_test_data.py -- test_prompt_data/$i 12 $i $i 500
  fi
done

rm -f input_data.json
printf "{\n" >> input_data.json
printf "\"physics\": [\n" >> input_data.json

for ((i=$INITIAL_RUN;i<=$MAX_RUN;i++)); do
  if ! ((i % 4)); then
    if ((i == $MAX_RUN)); then
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]]\n],\n" >> input_data.json
    else
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]],\n" >> input_data.json
    fi
  fi
done

printf "\"cosmics\": [\n" >> input_data.json
for ((i=$INITIAL_RUN;i<=$MAX_RUN;i++)); do
  if ! ((i % 4)); then
    if ((i == $MAX_RUN)); then
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]]\n],\n" >> input_data.json
    else
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]],\n" >> input_data.json
    fi
  fi
done

printf "\"Bcosmics\": [\n" >> input_data.json
for ((i=$INITIAL_RUN;i<=$MAX_RUN;i++)); do
  if ! ((i % 4)); then
    if ((i == $MAX_RUN)); then
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]]\n]\n" >> input_data.json
    else
      printf "[\"$PWD/test_prompt_data/$i\", [12,$i]],\n" >> input_data.json
    fi
  fi
done

printf "}" >> input_data.json
