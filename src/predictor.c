//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include "predictor.h"

#include <math.h>
#include <stdio.h>

//
// TODO:Student Information
//
const char *studentName = "Suraj Sathya Prakash";
const char *studentID = "";
const char *email = "ssathyaprakash@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare", "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 14;  // Number of bits used for Global History
int bpType;             // Branch Prediction Type
int verbose;

// tournament specific declarations
int localHistoryBits = 10;
int globalHistoryBits = 12;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint16_t ghistory;

// tournament
uint32_t *localHistTable;
uint8_t *localPredictTable;
uint8_t *globalHistTable;
uint8_t *globalPredictTable;
uint8_t *globalChoiceTable;
uint16_t *globalHistory;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

// tournnament functions
void init_tourn() {
  int localHistTableEntries = 1 << localHistoryBits;
  localHistTable = (uint32_t *)malloc(localHistTableEntries * sizeof(uint32_t));
  localPredictTable =
      (uint8_t *)malloc(localHistTableEntries * sizeof(uint8_t));

  int globalHistTableEntries = 1 << globalHistoryBits;
  globalHistTable = (uint8_t *)malloc(globalHistTableEntries * sizeof(uint8_t));
  globalPredictTable =
      (uint8_t *)malloc(globalHistTableEntries * sizeof(uint8_t));
  globalPredictTable =
      (uint8_t *)malloc(globalHistTableEntries * sizeof(uint8_t));

  for (int i = 0; i < localHistTableEntries; i++) {
    localHistTable[i] = CLEAR;
    localPredictTable[i] = WN;
  }

  for (int i = 0; i < globalHistTableEntries; i++) {
    globalPredictTable[i] = WN;
    globalChoiceTable[i] = WN;
  }
  globalHistTable = CLEAR;
}

// tournament functions
uint8_t tourn_predict(uint32_t PC) {
  uint16_t pcLowerBits = PC & (localHistoryBits - 1);
  uint16_t localPredictIndex = localHistTable[pcLowerBits];
  uint8_t localPrediction = localPredictTable[localPredictIndex];
  uint16_t globalPredictIndex = globalHistory & (globalHistoryBits - 1);
  uint8_t globalPrediction = globalPredictTable[globalPredictIndex];
  uint8_t globalChoicePrediction = globalPredictTable[globalPredictIndex];
  uint8_t predictorChoice;

  switch (globalChoicePrediction) {
    case WG:
      predictorChoice = globalPrediction;
    case SG:
      predictorChoice = globalPrediction;
    case WL:
      predictorChoice = localPrediction;
    case SL:
      predictorChoice = localPrediction;
    default:
      printf("Warning: Undefined state of entry in Choice Predict Table!\n");
  }

  switch (predictorChoice) {
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in Local/Global Predictor!\n");
      return NOTTAKEN;
  }
}

// gshare functions
void init_gshare() {
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++) {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc) {
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index]) {
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void train_tourn(uint32_t pc, uint8_t outcome) {

}
void train_gshare(uint32_t pc, uint8_t outcome) {
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index]) {
    case WN:
      bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
      break;
    case SN:
      bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
      break;
    case WT:
      bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare() { free(bht_gshare); }

void init_predictor() {
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc) {
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_predictor(uint32_t pc, uint8_t outcome) {
  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}
