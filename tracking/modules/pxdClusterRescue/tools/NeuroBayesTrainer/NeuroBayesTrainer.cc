/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <cstdio>

#include "NeuroBayesTeacher.hh"

#define NUM_VARIABLES 11

#define NB_WEIGTH_SIGNAL 1.0
#define NB_WEIGTH_BACKGROUND 1.0
#define NB_EVENT_SIGNAL 1.0
#define NB_EVENT_BACKGROUND -1.0

// get number of lines of a file
int file_get_lines(const char* filename)
{
  std::ifstream input_file(filename);
  int lines_count = 0;
  std::string line;
  while (std::getline(input_file, line)) ++lines_count;
  return lines_count;
}

// read single line of file and write NUM_VARIABLES numbers to given data array and last number to array_state
bool file_line_to_array(std::ifstream* file, float* array_data)
{
  // get line as string
  std::string line;
  std::getline(*file, line);

  // make stringstream
  std::stringstream line_stream(line.c_str());

  // split stringstream
  std::string segment;
  std::vector<std::string> line_split;
  while (std::getline(line_stream, segment, ' ')) line_split.push_back(segment);

  // check for correct number of segments
  if (line_split.size() != NUM_VARIABLES) return 0;

  // write split string as float to array
  for (int k = 0; k < NUM_VARIABLES; k++) array_data[k] = atof(line_split[k].c_str());

  return 1;
}

// setup NeuroBayes number of nodes
void init_teacher_layers(NeuroBayesTeacher* nb)
{
  nb->NB_DEF_NODE1(NUM_VARIABLES + 1); // Input layer has to be variables + 1 (bias var)
  nb->NB_DEF_NODE2(NUM_VARIABLES + 2); // Hidden layer, num is explorable
  nb->NB_DEF_NODE3(1); // Output Layer, either 1 for Classification or 20 for density
}

// setup NeuroBayes preprocessing
void init_teacher_preprocessing(NeuroBayesTeacher* nb)
{
  int g_no_decorrel = 0;
  nb->SetIndividualPreproFlag(0 , g_no_decorrel ? 0 : 14, "pixelChargeSum");
  nb->SetIndividualPreproFlag(1 , g_no_decorrel ? 0 : 14, "pixelChargeMin");
  nb->SetIndividualPreproFlag(2 , g_no_decorrel ? 0 : 14, "pixelChargeMax");
  nb->SetIndividualPreproFlag(3 , g_no_decorrel ? 0 : 14, "pixelChargeMean");
  nb->SetIndividualPreproFlag(4 , g_no_decorrel ? 0 : 14, "pixelChargeVar");
  nb->SetIndividualPreproFlag(5 , g_no_decorrel ? 0 : 14, "clusterSize");
  nb->SetIndividualPreproFlag(6 , g_no_decorrel ? 0 : 14, "clusterSizeU");
  nb->SetIndividualPreproFlag(7 , g_no_decorrel ? 0 : 14, "clusterSizeV");
  nb->SetIndividualPreproFlag(8 , g_no_decorrel ? 0 : 14, "globalCoordinateZ");
  nb->SetIndividualPreproFlag(9 , g_no_decorrel ? 0 : 14, "globalCoordinateR");
  nb->SetIndividualPreproFlag(10 , g_no_decorrel ? 0 : 14, "globalCoordinatePhi");
}

// setup NeuroBayes params for zero-iteration training
void init_teacher_params(NeuroBayesTeacher* nb)
{
  nb->NB_DEF(true);
  nb->NB_DEF_TASK("CLA");
  nb->NB_DEF_REG("REG");
  nb->NB_DEF_LOSS("ENTROPY");
  nb->NB_DEF_SPEED(1.0);
  nb->NB_DEF_SHAPE("DIAG");
  nb->NB_DEF_ITER(0);
  nb->NB_DEF_PRE(622);
  nb->NB_DEF_EPOCH(100);
  nb->NB_DEF_MAXLEARN(1.0);
}

/*

//setup NeuroBayes params for full training
void init_teacher_params(NeuroBayesTeacher* nb){
    nb->NB_DEF_TASK("CLASSIFICATION"); // set task to classification
    nb->NB_DEF_ITER(100); // number of training iterations
    nb->NB_DEF_METHOD("BFGS"); // use BFGS algorithm
}

*/

// setup NeuroBayes output
void init_teacher_output(NeuroBayesTeacher* nb, const char* filename)
{
  nb->SetOutputFile(filename);
}

int main(int argc, char** argv)
{
  /* process input args */

  char* input_training_data_signal;
  char* input_training_data_background;
  char* output_nb_expertise;
  const char* output_nb_log;

  // if not appropriate number of args is given
  if (argc != 5) {
    std::cout <<
              "Use NeuroBayesTrainer with command './NeuroBayesTrainer input_training_data_signal input_training_data_background output_neurobayes_expertise_file output_neurobayes_log'."
              << std::endl;
    return -1;
  }

  // if input and output file is given
  else {
    input_training_data_signal = argv[1];
    input_training_data_background = argv[2];
    output_nb_expertise = argv[3];
    output_nb_log = argv[4];
    std::cout << "Input training data signal: " << input_training_data_signal << std::endl;
    std::cout << "Input training data background: " << input_training_data_background << std::endl;
    std::cout << "Output NeuroBayes expertise file: " << output_nb_expertise << std::endl;
    std::cout << "Output NeuroBayes log file: " << output_nb_log << std::endl;
    std::cout << "Output histogram data: " << "ahist.txt" << std::endl;
    std::cout << "Output correl signi txt file: " << "correl_signi.txt" << std::endl;
    std::cout << "Ooutput correl signi html file: " << "correl_signi.html" << std::endl;
    std::cout << "Number training variables: " << NUM_VARIABLES << std::endl;
  }

  /* setup NeuroBayes teacher */

  NeuroBayesTeacher* nb = NeuroBayesTeacher::Instance();
  init_teacher_params(nb);
  init_teacher_layers(nb);
  init_teacher_output(nb, output_nb_expertise);
  init_teacher_preprocessing(nb);

  /* load data in teacher */

  int num_lines_signal = file_get_lines(input_training_data_signal);
  int num_lines_background = file_get_lines(input_training_data_background);
  float training_variables[NUM_VARIABLES];
  bool read_status; // status for correct read line
  std::ifstream file_training_data_signal(input_training_data_signal); // open data file for input
  std::ifstream file_training_data_background(input_training_data_background); // open data file for input

  // load signal
  for (int k = 0; k < num_lines_signal; k++) {
    read_status = file_line_to_array(&file_training_data_signal, training_variables);

    if (read_status) { // if line of file is read correctly
      // give data to teacher
      nb->SetWeight(NB_WEIGTH_SIGNAL);
      nb->SetTarget(NB_EVENT_SIGNAL);

      // set input
      nb->SetNextInput(NUM_VARIABLES, training_variables);
    }

    else { // throw error for false read line
      std::cout << "Line of input file is not read correctly." << std::endl;
      std:: cout << "File: " << input_training_data_signal << std::endl;
      std:: cout << "Line: " << k << std::endl;
      return -2;
    }
  }

  // load background
  for (int k = 0; k < num_lines_background; k++) {
    read_status = file_line_to_array(&file_training_data_background, training_variables);

    if (read_status) { // if line of file is read correctly
      // give data to teacher
      nb->SetWeight(NB_WEIGTH_BACKGROUND);
      nb->SetTarget(NB_EVENT_BACKGROUND);

      // set input
      nb->SetNextInput(NUM_VARIABLES, training_variables);
    }

    else { // throw error for false read line
      std::cout << "Line of input file is not read correctly." << std::endl;
      std:: cout << "File: " << input_training_data_background << std::endl;
      std:: cout << "Line: " << k << std::endl;
      return -2;
    }
  }

  /* train network */

  // stdout is redirected to the file output_nb_log
  int original = dup(fileno(stdout));
  fflush(stdout);
  FILE* log_stream = freopen(output_nb_log, "w", stdout);
  if (log_stream == nullptr) {
    B2FATAL("NeuroBayes output log file " << output_nb_log << " could not be opened in writing mode.");
  }

  // train neural network
  nb->TrainNet();

  fflush(stdout);
  dup2(original, fileno(stdout));
  close(original);

  /* write correlation and significances output */

  nb->nb_correl_signi("correl_signi.txt", "correl_signi.html");

  return 0;
}
