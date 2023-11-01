#include "predictor.h"

#define branchHistoryWidth 31
#define percepTableHistoryLength 2048
#define pcMask 11

uint64_t globalBranchHistory;
int16_t y;
int8_t historyRegister[branchHistoryWidth];
int8_t percepTable[percepTableHistoryLength][branchHistoryWidth];
int8_t perceptronPrediction;
int16_t percepSelected[branchHistoryWidth];
int theta = 0;

uint16_t *localHistTable2;
uint8_t *localPredictTable2;
uint32_t globalHistTable2;
uint8_t *globalPredictTable2;
uint8_t *globalChoiceTable2;

// int localHistoryBits2 = 11;
int globalHistoryBits2 = 14;
// int pcSelectBits2 = 10;

void init_tourn2() {

  int i, j;

  for (i = 0; i < 1 << pcMask; i++)
    for (j = 0; j < branchHistoryWidth; j++)
      percepTable[i][j] = (int8_t)-1;

  globalBranchHistory = CLEAR;
  for (i = 0; i < branchHistoryWidth; i++)
    percepSelected[i] = CLEAR;

  int globalHistTable2Entries = 1 << globalHistoryBits2;

  globalPredictTable2 =
      (uint8_t *)malloc((1 << globalHistoryBits2) * sizeof(uint8_t));
  globalChoiceTable2 =
      (uint8_t *)malloc((1 << globalHistoryBits2) * sizeof(uint8_t));

  for (int i = 0; i < 1 << globalHistoryBits2; i++) {
    globalPredictTable2[i] = SN;
    globalChoiceTable2[i] = WL;
  }
  globalHistTable2 = CLEAR;
}
uint8_t tourn_predict2(uint32_t PC) {

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
  perceptronPrediction = (y >= 0) ? 1 : 0;

  uint16_t pcLowerBitsGlobal = PC & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictIndex =
      (pcLowerBitsGlobal ^ globalHistTable2) & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictChoiceIndex =
      globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  uint8_t globalPrediction = globalPredictTable2[globalPredictIndex];
  uint8_t globalChoicePrediction = globalChoiceTable2[globalPredictChoiceIndex];

  uint8_t predictorChoice;

  switch (globalChoicePrediction) {
  case WG:
  case SG:
    predictorChoice = globalPredictTable2[globalPredictIndex];

    break;
  case WL:
  case SL:
    predictorChoice = perceptronPrediction;

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
 
  uint16_t pcLowerBitsGlobal = PC & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictIndex =
      (globalHistTable2 ^ pcLowerBitsGlobal) & ((1 << globalHistoryBits2) - 1);
  uint32_t globalPredictChoiceIndex =
      globalHistTable2 & ((1 << globalHistoryBits2) - 1);
  uint8_t globalPrediction = globalPredictTable2[globalPredictIndex];

  // train the choice predictor first
  train_tourn_global_choice2(outcome, globalPrediction,
                             globalPredictChoiceIndex, perceptronPrediction);

  // start perceptron
  int i;
  uint16_t pcMasked = PC & ((1 << pcMask) - 1);
  uint16_t pcMaskedGBH = (globalBranchHistory ^ pcMasked) & ((1 << pcMask) - 1);
  int8_t outcomeNormValue = (outcome == 1) ? 1 : -1;

  if (perceptronPrediction != outcome || abs(y) <= theta) {
    if (outcome)
      percepSelected[0] += 1;
    else
      percepSelected[0] -= 1;

    for (i = 1; i < branchHistoryWidth; i++) {
      if (((globalBranchHistory >> i) & 1) == outcome)
        percepSelected[i] += 1;
      else
        percepSelected[i] -= 1; 
    }
  }

  for (i = 0; i < branchHistoryWidth; i++) {
    percepTable[pcMaskedGBH][i] = percepSelected[i];
  }

  globalBranchHistory = ((globalBranchHistory << 1) | outcome);

  train_tourn_global2(outcome, globalPredictIndex, globalPrediction);

  // end perceptron

  globalHistTable2 =
      ((1 << globalHistoryBits2) - 1) & ((globalHistTable2 << 1) | outcome);
}
