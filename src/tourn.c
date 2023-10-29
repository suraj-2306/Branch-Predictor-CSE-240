#include "predictor.h"
// tournament
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
