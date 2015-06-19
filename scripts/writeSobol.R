source('functions.R')


for(ii in 1 : 100){
  writeSimControl.sobol(
    simControlNum = ii,
    simControlFile = paste('../simulator/SimControl', ii, '.csv', sep=''),
    reps = 100,
    NumDays = 365 * 4,
    MosquitoInfectiousness = c(.1,1),
    MosquitoMoveProbability = c(0.2,0.4),
    num.3 = 50)
}