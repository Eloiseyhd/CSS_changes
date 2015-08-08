source('functions.R')

reps = 50

HumanLatencyLow = rep(4,2)
HumanLatencyHigh = rep(7,2)
HumanInfectionDays = rep(9,2)
HumanImmunityDays = rep(686,2)
ForceOfInfection = rep(.052,2)
EmergenceFactor = c(.5,2)
MosquitoLifespan = c(10,20)
MosquitoInfectiousness = rep(0.5,2)
MosquitoLatencyLow = rep(6,2)
MosquitoLatencyHigh = rep(9,2)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDaysLow = rep(2,2)
MosquitoRestDaysHigh = rep(3,2)

scalars = sobol(vars = list(
  HumanLatencyLow = HumanLatencyLow,
  HumanLatencyHigh = HumanLatencyHigh,
  HumanInfectionDays = HumanInfectionDays,
  HumanImmunityDays = HumanImmunityDays,
  ForceOfInfection = ForceOfInfection,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  MosquitoLatencyLow = MosquitoLatencyLow,
  MosquitoLatencyHigh = MosquitoLatencyHigh,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDaysLow = MosquitoRestDaysLow,
  MosquitoRestDaysHigh = MosquitoRestDaysHigh),
  reps)

for(ii in 1 : reps){
  writeSimControl.sobol(
    simControlNum = ii,
    reps = reps,
    NumDays = 365 * 10,
    OutputPath = '../simulator/data/Output',
    LocationFile = '../simulator/data/Input/locations_20150801.csv',
    NeighborhoodFile = '../simulator/data/Input/neighborhoods.csv',
    MortalityFile = '../simulator/data/Input/age_specific_mortality.csv',
    VaccineProfileFile = '../simulator/data/Input/vaccine_profile_CYD15.csv',
    num.1 = 10,
    num.2 = 10,
    num.3 = 10,
    num.4 = 10,
    zone = NA,
    neighborhood = NA,
    block = NA,
    scalars = scalars)
}
