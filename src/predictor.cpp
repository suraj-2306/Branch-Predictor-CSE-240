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
//
const char *studentName = "Suraj Sathya Prakash";
const char *studentID = "A59026390";
const char *email = "ssathyaprakash@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare", "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 14; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

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
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare() { free(bht_gshare); }

void init_predictor() {
  switch (bpType) {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_tourn();
    break;
  case CUSTOM:
    init_tourn2();
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
    return tourn_predict2(pc);
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
  if (condition) {
    switch (bpType) {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tourn(pc, outcome);
    case CUSTOM:
      return train_tourn2(pc, outcome);
    default:
      break;
    }
  }
}

uint16_t *localHistTable;
uint8_t *localPredictTable;
uint32_t globalHistTable;
uint8_t *globalPredictTable;
uint8_t *globalChoiceTable;

int localHistoryBits = 14;
int globalHistoryBits = 14;
int pcSelectBits = 14;

void init_tourn() {
  int localHistTableEntries = 1 << localHistoryBits;
  localHistTable = (uint16_t *)malloc((1 << pcSelectBits) * sizeof(uint16_t));
  localPredictTable =
      (uint8_t *)malloc((1 << localHistoryBits) * sizeof(uint8_t));

  int globalHistTableEntries = 1 << globalHistoryBits;

  globalPredictTable =
      (uint8_t *)malloc((1 << globalHistoryBits) * sizeof(uint8_t));
  globalChoiceTable =
      (uint8_t *)malloc((1 << globalHistoryBits) * sizeof(uint8_t));

  for (int i = 0; i < 1 << pcSelectBits; i++) {
    localHistTable[i] = 0;
  }
  for (int i = 0; i < 1 << localHistoryBits; i++) {
    localPredictTable[i] = SN;
  }

  for (int i = 0; i < 1 << globalHistoryBits; i++) {
    globalPredictTable[i] = SN;
    globalChoiceTable[i] = WL;
  }
  globalHistTable = CLEAR;
}
// int ninjaCountGlobal=0,ninjaCountLocal=0;
uint8_t tourn_predict(uint32_t PC) {
  // uint16_t globalHistTableEntries = 1 << globalHistoryBits;
  // uint16_t localHistTableEntries = 1 << localHistoryBits;

  uint16_t pcLowerBits = PC & ((1 << pcSelectBits) - 1);
  uint16_t localPredictIndex = localHistTable[pcLowerBits];
  uint16_t localHistTableEntriesIndex =
      localPredictIndex & ((1 << localHistoryBits) - 1);
  //  printf("%d\n",(1 << localHistoryBits) - 1);
  uint8_t localPrediction = localPredictTable[localHistTableEntriesIndex];

  uint16_t globalPredictIndex =
      globalHistTable & ((1 << globalHistoryBits) - 1);
  uint8_t globalPrediction = globalPredictTable[globalPredictIndex];
  uint8_t globalChoicePrediction = globalChoiceTable[globalPredictIndex];
  uint8_t predictorChoice;

  // if(globalChoicePrediction!=0&globalChoicePrediction!=1)
  // {
  //   ninjaCountLocal+=1;
  // }
  // else {
  // ninjaCountGlobal+=1;
  // printf("%d local %d global\n",ninjaCountLocal,ninjaCountGlobal);
  // }
  switch (globalChoicePrediction) {
  case WG:
    predictorChoice = globalPredictTable[globalPredictIndex];

    break;
  case SG:
    predictorChoice = globalPredictTable[globalPredictIndex];

    break;
  case WL:
    predictorChoice = localPredictTable[localHistTableEntriesIndex];

    break;
  case SL:
    predictorChoice = localPredictTable[localHistTableEntriesIndex];

    break;
  default:
    printf("Warning: Undefined state of entry in Choice Predict Table!\n");
    printf("%d", globalChoicePrediction);
    break;
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
    break;
  }
}

void train_tourn_local(uint8_t outcome, uint16_t localHistEntry,
                       uint8_t localPrediction) {
  switch (localPrediction) {

  case WN:
    localPredictTable[localHistEntry] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    localPredictTable[localHistEntry] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    localPredictTable[localHistEntry] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    localPredictTable[localHistEntry] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Local Predict TABLE!\n");
    break;
  }
}
void train_tourn_global(uint8_t outcome, uint32_t globalPredictIndex,
                        uint8_t globalPrediction) {
  switch (globalPrediction) {

  case WN:
    globalPredictTable[globalPredictIndex] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    globalPredictTable[globalPredictIndex] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    globalPredictTable[globalPredictIndex] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    globalPredictTable[globalPredictIndex] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Global Predict TABLE!\n");
    break;
  }
}
void train_tourn_global_choice(uint8_t outcome, uint16_t globalPrediction,
                               uint16_t globalPredictIndex,
                               uint16_t localPrediction) {

  int scoreTableGlobal = 0;
  int scoreTableLocal = 0;
  uint16_t globalChoicePrediction = globalChoiceTable[globalPredictIndex];
  switch (outcome) {
  case TAKEN:
    scoreTableGlobal =
        ((globalPrediction == WT) || (globalPrediction == ST)) ? 1 : 0;
    scoreTableLocal =
        ((localPrediction == WT) || (localPrediction == ST)) ? 2 : 0;
    break;
  case NOTTAKEN:
    scoreTableGlobal =
        ((globalPrediction == WN) || (globalPrediction == SN)) ? 1 : 0;
    scoreTableLocal =
        ((localPrediction == WN) || (localPrediction == SN)) ? 2 : 0;
    break;
  default:
    scoreTableGlobal = 0;
    scoreTableLocal = 0;
    printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
    break;
  }

  int predictionTuple = scoreTableGlobal + scoreTableLocal;

  switch (predictionTuple) {
  case 0:
    break;
  case 3:
    break;
  case 2:
    switch (globalChoicePrediction) {
    case WG:
      globalChoiceTable[globalPredictIndex] = WL;
      break;
    case WL:
      globalChoiceTable[globalPredictIndex] = SL;
      break;
    case SG:
      globalChoiceTable[globalPredictIndex] = WG;
      break;
    case SL:
      break;
    default:
      printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
      break;
    }
    break;
  case 1:
    switch (globalChoicePrediction) {
    case WG:
      globalChoiceTable[globalPredictIndex] = SG;
      break;
    case WL:
      globalChoiceTable[globalPredictIndex] = WG;
      break;
    case SL:
      globalChoiceTable[globalPredictIndex] = WL;
      break;
    case SG:
      break;
    default:
      printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
      break;
    }
    break;
  default:
    printf("Warning: Undefined state of entry in CHOICE TABLE Assigner!\n");
    break;
  }
}

void train_tourn(uint32_t PC, uint8_t outcome) {
  // uint16_t globalHistTableEntries = 1 << globalHistoryBits;
  // uint16_t localHistTableEntries = 1 << localHistoryBits;

  uint16_t pcLowerBits = PC & ((1 << pcSelectBits) - 1);
  uint16_t localHistEntry = localHistTable[pcLowerBits];
  uint16_t localHistTableEntriesIndex =
      localHistEntry & ((1 << localHistoryBits) - 1);
  uint8_t localPrediction = localPredictTable[localHistTableEntriesIndex];

  uint32_t globalPredictIndex =
      globalHistTable & ((1 << globalHistoryBits) - 1);
  uint8_t globalPrediction = globalPredictTable[globalPredictIndex];

  train_tourn_global_choice(outcome, globalPrediction, globalPredictIndex,
                            localPrediction);
  train_tourn_global(outcome, globalPredictIndex, globalPrediction);
  train_tourn_local(outcome, localHistEntry, localPrediction);

  localHistTable[pcLowerBits] =
      ((1 << localHistoryBits) - 1) & ((localHistEntry << 1) | outcome);
  globalHistTable =
      ((1 << globalHistoryBits) - 1) & ((globalHistTable << 1) | outcome);
}

#define branchHistoryWidth 62
#define percepTableHistoryLength 2048
#define pcMask 11

uint64_t globalBranchHistory;
int16_t y;
int8_t historyRegister[branchHistoryWidth];
int8_t percepTable[percepTableHistoryLength][branchHistoryWidth];
int8_t perceptronPrediction;
int16_t percepSelected[branchHistoryWidth];
int theta = 0;

int8_t signNo(int16_t no) {
  if (no >= 0)
    return 1;
  else
    return -1;
}

void init_percep() {
  int i, j;

  for (i = 0; i < 1 << pcMask; i++)
    for (j = 0; j < branchHistoryWidth; j++)
      percepTable[i][j] = (int8_t)0;

  globalBranchHistory = CLEAR;
  for (i = 0; i < branchHistoryWidth; i++)
    percepSelected[i] = CLEAR;
}

uint8_t percep_predict(uint32_t PC) {
  uint8_t i;
  uint16_t pcMasked = PC & ((1 << pcMask) - 1);
  uint16_t pcMaskedGBH = (globalBranchHistory ^ pcMasked) & ((1 << pcMask) - 1);

  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[pcMaskedGBH][i];
  }

  y = percepSelected[0];
  for (i = 1; i < branchHistoryWidth; i++) {
    if ((globalBranchHistory >> i) & 1)
      y += percepSelected[i];
    else
      y -= percepSelected[i];
  }
  perceptronPrediction = (y > 0) ? 1 : 0;

  return perceptronPrediction;
}

void train_percep(uint32_t PC, uint8_t outcome) {
  int i;
  uint16_t pcMasked = PC & ((1 << pcMask) - 1);
  uint16_t pcMaskedGBH = (globalBranchHistory ^ pcMasked) & ((1 << pcMask) - 1);

  int8_t outcomeNormValue = (outcome == 1) ? 1 : -1;

  if (perceptronPrediction != outcome || abs(y) <= theta) {
    for (i = 0; i < branchHistoryWidth; i++) {
      if (((globalBranchHistory >> i) & 1) == outcomeNormValue)
        percepSelected[i] += 1;
      else
        percepSelected[i] -= 1;
    }
  }

  for (i = 0; i < branchHistoryWidth; i++) {
    percepTable[pcMaskedGBH][i] = percepSelected[i];
  }

  globalBranchHistory = ((globalBranchHistory << 1) | outcome);
  // uint64_t branchMask = ((1 << branchHistoryWidth) - 1);
  // globalBranchHistory = globalBranchHistory & branchMask;
}

uint16_t *localHistTable2;
uint8_t *localPredictTable2;
uint32_t globalHistTable2;
uint8_t *globalPredictTable2;
uint8_t *globalChoiceTable2;

int localHistoryBits2 = 11;
int globalHistoryBits2 = 12;
int pcSelectBits2 = 10;

void init_tourn2() {
  int localHistTable2Entries = 1 << localHistoryBits2;
  localHistTable2 = (uint16_t *)malloc((1 << pcSelectBits2) * sizeof(uint16_t));
  localPredictTable2 =
      (uint8_t *)malloc((1 << localHistoryBits2) * sizeof(uint8_t));

  int globalHistTable2Entries = 1 << globalHistoryBits2;

  globalPredictTable2 =
      (uint8_t *)malloc((1 << globalHistoryBits2) * sizeof(uint8_t));
  globalChoiceTable2 =
      (uint8_t *)malloc((1 << globalHistoryBits2) * sizeof(uint8_t));

  for (int i = 0; i < 1 << pcSelectBits2; i++) {
    localHistTable2[i] = 0;
  }
  for (int i = 0; i < 1 << localHistoryBits2; i++) {
    localPredictTable2[i] = SN;
  }

  for (int i = 0; i < 1 << globalHistoryBits2; i++) {
    globalPredictTable2[i] = SN;
    globalChoiceTable2[i] = WL;
  }
  globalHistTable2 = CLEAR;
}
// int ninjaCountGlobal=0,ninjaCountLocal=0;
uint8_t tourn_predict2(uint32_t PC) {
  // uint16_t globalHistTable2Entries = 1 << globalHistoryBits2;
  // uint16_t localHistTable2Entries = 1 << localHistoryBits2;

  uint16_t pcLowerBits = PC & ((1 << pcSelectBits2) - 1);
  uint16_t localPredictIndex = localHistTable2[pcLowerBits];
  uint16_t localHistTable2EntriesIndex =
      localPredictIndex & ((1 << localHistoryBits2) - 1);
  //  printf("%d\n",(1 << localHistoryBits2) - 1);
  uint8_t localPrediction = localPredictTable2[localHistTable2EntriesIndex];

  uint16_t pcLowerBitsGlobal = PC & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictIndex =
      (pcLowerBitsGlobal ^ globalHistTable2) & ((1 << globalHistoryBits2) - 1);
  // uint16_t globalPredictIndex =
  //     globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  //
  uint32_t globalPredictChoiceIndex =
      globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  uint8_t globalPrediction = globalPredictTable2[globalPredictIndex];
  uint8_t globalChoicePrediction = globalChoiceTable2[globalPredictChoiceIndex];

  uint8_t predictorChoice;

  // if(globalChoicePrediction!=0&globalChoicePrediction!=1)
  // {
  //   ninjaCountLocal+=1;
  // }
  // else {
  // ninjaCountGlobal+=1;
  // printf("%d local %d global\n",ninjaCountLocal,ninjaCountGlobal);
  // }
  switch (globalChoicePrediction) {
  case WG:
    predictorChoice = globalPredictTable2[globalPredictIndex];

    break;
  case SG:
    predictorChoice = globalPredictTable2[globalPredictIndex];

    break;
  case WL:
    predictorChoice = localPredictTable2[localHistTable2EntriesIndex];

    break;
  case SL:
    predictorChoice = localPredictTable2[localHistTable2EntriesIndex];

    break;
  default:
    printf("Warning: Undefined state of entry in Choice Predict Table!\n");
    printf("%d", globalChoicePrediction);
    break;
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
    break;
  }
}

void train_tourn_local2(uint8_t outcome, uint16_t localHistEntry,
                        uint8_t localPrediction) {
  switch (localPrediction) {

  case WN:
    localPredictTable2[localHistEntry] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    localPredictTable2[localHistEntry] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    localPredictTable2[localHistEntry] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    localPredictTable2[localHistEntry] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Local Predict TABLE!\n");
    break;
  }
}
void train_tourn_global2(uint8_t outcome, uint32_t globalPredictIndex,
                         uint8_t globalPrediction) {
  switch (globalPrediction) {

  case WN:
    globalPredictTable2[globalPredictIndex] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    globalPredictTable2[globalPredictIndex] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    globalPredictTable2[globalPredictIndex] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    globalPredictTable2[globalPredictIndex] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Global Predict TABLE!\n");
    break;
  }
}
void train_tourn_global_choice2(uint8_t outcome, uint16_t globalPrediction,
                                uint16_t globalPredictIndex,
                                uint16_t localPrediction) {
  int scoreTableGlobal = 0;
  int scoreTableLocal = 0;
  uint16_t globalChoicePrediction = globalChoiceTable2[globalPredictIndex];
  switch (outcome) {
  case TAKEN:
    scoreTableGlobal =
        ((globalPrediction == WT) || (globalPrediction == ST)) ? 1 : 0;
    scoreTableLocal =
        ((localPrediction == WT) || (localPrediction == ST)) ? 2 : 0;
    break;
  case NOTTAKEN:
    scoreTableGlobal =
        ((globalPrediction == WN) || (globalPrediction == SN)) ? 1 : 0;
    scoreTableLocal =
        ((localPrediction == WN) || (localPrediction == SN)) ? 2 : 0;
    break;
  default:
    scoreTableGlobal = 0;
    scoreTableLocal = 0;
    printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
    break;
  }

  int predictionTuple = scoreTableGlobal + scoreTableLocal;

  switch (predictionTuple) {
  case 0:
    break;
  case 3:
    break;
  case 2:
    switch (globalChoicePrediction) {
    case WG:
      globalChoiceTable2[globalPredictIndex] = WL;
      break;
    case WL:
      globalChoiceTable2[globalPredictIndex] = SL;
      break;
    case SG:
      globalChoiceTable2[globalPredictIndex] = WG;
      break;
    case SL:
      break;
    default:
      printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
      break;
    }
    break;
  case 1:
    switch (globalChoicePrediction) {
    case WG:
      globalChoiceTable2[globalPredictIndex] = SG;
      break;
    case WL:
      globalChoiceTable2[globalPredictIndex] = WG;
      break;
    case SL:
      globalChoiceTable2[globalPredictIndex] = WL;
      break;
    case SG:
      break;
    default:
      printf("Warning: Undefined state of entry in CHOICE TABLE!\n");
      break;
    }
    break;
  default:
    printf("Warning: Undefined state of entry in CHOICE TABLE Assigner!\n");
    break;
  }
}

void train_tourn2(uint32_t PC, uint8_t outcome) {
  uint16_t pcLowerBits = PC & ((1 << pcSelectBits2) - 1);
  uint16_t localHistEntry = localHistTable2[pcLowerBits];
  uint16_t localHistTable2EntriesIndex =
      localHistEntry & ((1 << localHistoryBits2) - 1);
  uint8_t localPrediction = localPredictTable2[localHistTable2EntriesIndex];

  uint16_t pcLowerBitsGlobal = PC & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictIndex =
      (pcLowerBitsGlobal ^ globalHistTable2) & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictChoiceIndex =
      globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  // uint32_t globalPredictIndex =
  // globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  uint8_t globalPrediction = globalPredictTable2[globalPredictIndex];

  train_tourn_global_choice2(outcome, globalPrediction,
                             globalPredictChoiceIndex, localPrediction);
  train_tourn_global2(outcome, globalPredictIndex, globalPrediction);
  train_tourn_local2(outcome, localHistEntry, localPrediction);

  localHistTable2[pcLowerBits] =
      ((1 << localHistoryBits2) - 1) & ((localHistEntry << 1) | outcome);
  globalHistTable2 =
      ((1 << globalHistoryBits2) - 1) & ((globalHistTable2 << 1) | outcome);
}
