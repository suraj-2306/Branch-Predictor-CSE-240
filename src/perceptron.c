#include "predictor.h" // tournament
#define branchHistoryWidth 62
#define percepTableHistoryLength 2048
#define pcMask 11

uint64_t globalBranchHistory;
int16_t y;
int8_t historyRegister[branchHistoryWidth];
int8_t percepTable[percepTableHistoryLength][branchHistoryWidth];
int8_t perceptronPrediction;
int theta = 134;

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
}

uint8_t percep_predict(uint32_t PC) {

  int i;

  int pcMaskBits = (1 << pcMask) - 1;
  uint16_t pcMasked = PC & pcMaskBits;
  uint16_t percepTableIndex = pcMasked;

  int16_t percepSelected[branchHistoryWidth];

  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[percepTableIndex][i];
  }

  y = percepSelected[0];
  for (i = 1; i < branchHistoryWidth; i++) {
    if ((globalBranchHistory >> i) & 1)
      y += percepSelected[i];
    else
      y -= percepSelected[i];
  }
  perceptronPrediction = (y >= 0) ? 1 : 0;

  return perceptronPrediction;
}

void train_percep(uint32_t PC, uint8_t outcome) {
  int i;
  int pcMaskBits = (1 << pcMask) - 1;
  uint16_t pcMasked = PC & pcMaskBits;
  uint16_t percepTableIndex = (pcMasked);
  int16_t percepSelected[branchHistoryWidth];
  int8_t outcomeNormValue = (outcome == 1) ? 1 : -1;
  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[percepTableIndex][i];
  }

  if (perceptronPrediction != outcome || abs(y) <= theta) {
    for (i = 0; i < branchHistoryWidth; i++) {
      if (((globalBranchHistory >> i) & 1) == outcomeNormValue)
        percepSelected[i] += 1;
      else
        percepSelected[i] -= 1;
    }
  }

  for (i = 0; i < branchHistoryWidth; i++) {
    percepTable[percepTableIndex][i] = percepSelected[i];
  }

  globalBranchHistory = ((globalBranchHistory << 1) | outcome);
  uint64_t branchMask = ((1 << branchHistoryWidth) - 1);
  globalBranchHistory = globalBranchHistory & branchMask;
}
