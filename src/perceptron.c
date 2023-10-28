#include "predictor.h"
// tournament
#define branchHistoryWidth 16
#define percepTableHistoryLength 256
uint8_t pcMask=8;

uint16_t globalBranchHistory, y;
int8_t historyRegister[branchHistoryWidth];
int8_t percepTable[percepTableHistoryLength][branchHistoryWidth];
int theta = 33;

int8_t outcomeNorm(uint8_t outcome) {
  if (outcome == 1)
    return 1;
  else if (outcome == 0)
    return -1;
  else {
    printf("Unknown value in outcome\n");
    return 2;
  }
}
int8_t *BranchH2History() {

  int i;
  uint8_t index;
  for (i = 1; i < branchHistoryWidth; i++) {
    index = (uint8_t)(globalBranchHistory >> (i - 1) & 1);
    if (index == NOTTAKEN)
      historyRegister[i] = -1;
    else if (index == TAKEN)
      historyRegister[i] = 1;
    else
      printf("Unknown value in global branch history register\n");
  }
  return historyRegister;
}

void init_percep() {
  int i, j;
  // historyRegister = (int8_t *)malloc(branchHistoryWidth * sizeof(int8_t *));
  // historyRegister = CLEAR;
  for (i = 0; i < branchHistoryWidth; i++)
    historyRegister[i] = CLEAR;

  for (i = 0; i < 1 << pcMask; i++)
    for (j = 0; j < branchHistoryWidth; j++)
      percepTable[i][j] = (int8_t)0;

  globalBranchHistory = CLEAR;
}

uint8_t percep_predict(uint32_t PC) {
  int i;
  uint16_t percepTableIndex = PC & (1 >> (pcMask - 1));
  int ninja=0;
  if(percepTableIndex!=0)
  ninja=1;
  int16_t percepSelected[branchHistoryWidth];

  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[percepTableIndex][i];
  }

  BranchH2History();
  for (i = 0; i < branchHistoryWidth; i++) {
    y += historyRegister[i] * percepSelected[i];
  }

  if (y >= 0)
    return TAKEN;
  else if (y < 0)
    return NOTTAKEN;
  else {
    printf("Warning: Undefined predicted state!\n");
    return NOTTAKEN;
  }
}

void train_percep(uint32_t PC, uint8_t outcome) {
  int i;
  uint16_t percepTableIndex = PC & (1 >> (pcMask- 1));
  int16_t percepSelected[branchHistoryWidth], y;
  int8_t outcomeNormValue = outcomeNorm(outcome);

  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[percepTableIndex][i];
  }
  if (y != outcome || abs(y) <= theta) {
    for (i = 0; i < branchHistoryWidth; i++) {
      percepSelected[i] += historyRegister[i] * outcomeNormValue;
    }
  }
  for (i = 0; i < branchHistoryWidth; i++) {
    percepTable[percepTableIndex][i] = percepSelected[i];
  }

  globalBranchHistory = ((globalBranchHistory << 1) | outcome);
}
