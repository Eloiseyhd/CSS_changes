source('functions.R')

reps = 1000

HumanImmunityDays = rep(686,2)
ForceOfInfection = c(.001,.10)
EmergenceFactor = c(.5,1.5)
MosquitoLifespan = c(10,20)
MosquitoInfectiousness = c(0,1)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDaysLow = rep(1,2)
MosquitoRestDaysHigh = rep(2,2)

scalars = sobol(vars = list(
  HumanImmunityDays = HumanImmunityDays,
  ForceOfInfection = ForceOfInfection,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDaysLow = MosquitoRestDaysLow,
  MosquitoRestDaysHigh = MosquitoRestDaysHigh),
  reps)

for(ii in 1 : reps){
  writeSimControl.sobol(
    simControlNum = ii,
    reps = 1, # reps,
    NumDays = 365 * 30,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    MortalityFile = 'age_specific_mortality.csv',
    VaccineProfileFile = 'vaccine_profile_CYD15.csv',
    num.1 = 25,
    num.2 = 25,
    num.3 = 25,
    num.4 = 25,
    zone = NA,
    neighborhood = NA,
    block = NA,
    scalars = scalars[ii,])
}
