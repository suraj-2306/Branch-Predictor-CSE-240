//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Suraj Sathya Prakash";
const char *studentID = "A59026390";
const char *email = "ssathyaprakash@ucsd.edu";

//-------o-----------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare", "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 14; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

// tournament specific declarations
// int localHistoryBits = 10;
// int globalHistoryBits = 12;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint16_t ghistory;

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
// void cleanup_tourn() {

//   free(localHistTable);
//   free(localPredictTable);
//   free(globalPredictTable);
//   free(globalChoiceTable);
//   free(localHistTable);
// }

void init_predictor() {
  switch (bpType) {
  case STATIC:
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_tourn();
    break;
  case CUSTOM:
    init_percep();
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct) {
  // Make a prediction based on the bpType
  switch (bpType) {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return tourn_predict(pc);
  case CUSTOM:
    return percep_predict(pc);
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

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome,
                     uint32_t condition, uint32_t call, uint32_t ret,
                     uint32_t direct) {
  switch (bpType) {
  case STATIC:
  case GSHARE:
    return train_gshare(pc, outcome);
  case TOURNAMENT:
    return train_tourn(pc, outcome);
  case CUSTOM:
    return train_percep(pc, outcome);
  default:
    break;
  }
}

