#include "predictor.h" // tournament
#define branchHistoryWidth 16
#define percepTableHistoryLength 256
#define pcMask 7

uint16_t globalBranchHistory;
int16_t y;
int8_t historyRegister[branchHistoryWidth];
int8_t percepTable[percepTableHistoryLength][branchHistoryWidth];
int8_t perceptronPrediction;
int theta = 33;

int8_t outcomeNorm(uint8_t outcome) {
  if (outcome == 1)
    return 1;
  else if (outcome == 0)
    return (int8_t)-1;
  else {
    printf("Unknown value in outcome\n");
    return 2;
  }
}
int8_t signNo(int16_t no) {
  if (no >= 0)
    return 1;
  else
    return -1;
}
// int8_t *BranchH2History() {

//   int i;
//   uint8_t index;
//   for (i = 0; i < branchHistoryWidth; i++) {
//     index = (uint8_t)(globalBranchHistory >> (i) & 1);
//     if (index == NOTTAKEN)
//       historyRegister[i] = (int8_t)-1;
//     else if (index == TAKEN)
//       historyRegister[i] = (int8_t)1;
//     else
//       printf("Unknown value in global branch history register\n");
//   }
//   return historyRegister;
// }

void init_percep() {
  int i, j;
  // historyRegister = (int8_t *)malloc(branchHistoryWidth * sizeof(int8_t *));
  // historyRegister = CLEAR;
  for (i = 0; i < branchHistoryWidth; i++)
    historyRegister[i] = CLEAR;

  for (i = 0; i < 1 << pcMask; i++)
    for (j = 0; j < branchHistoryWidth; j++)
      percepTable[i][j] = (int8_t)-1;

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

  // BranchH2History();
  y = percepSelected[branchHistoryWidth - 1];
  for (i = 0; i < branchHistoryWidth-1; i++) {
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

  for (i = 0; i < branchHistoryWidth; i++) {
    percepSelected[i] = percepTable[percepTableIndex][i];
  }
  // BranchH2History();
  if (perceptronPrediction != outcome || abs(y) <= theta) {
    // for (i = 0; i < branchHistoryWidth; i++) {
    //   percepSelected[i] +=  outcomeNormValue;
    percepSelected[15] += outcomeNormValue;
    // }
  }

  for (i = 0; i < branchHistoryWidth; i++) {
    percepTable[percepTableIndex][i] = percepSelected[i];
  }

  globalBranchHistory =
  ((globalBranchHistory << 1) | outcome);
  globalBranchHistory = ((1<<branchHistoryWidth)-1)&globalBranchHistory;
}
