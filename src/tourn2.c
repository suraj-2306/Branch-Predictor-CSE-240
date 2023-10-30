#include "predictor.h"

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
