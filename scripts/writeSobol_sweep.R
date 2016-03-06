source('functions_sweep.R')

reps = 50

ForceOfImportation = c(1e-7,1e-4)
EmergenceFactor = c(.01,1.99)
MosquitoLifespan = c(2,10)
MosquitoInfectiousness = c(.01,.99)
HumanImmunityDays = rep(686,2)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDays = rep(2/3,2)
Seed = c(1,as.integer(Sys.time()))
SeedInf = Seed

scalars = sobol(vars = list(
  ForceOfImportation = ForceOfImportation,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  HumanImmunityDays = HumanImmunityDays,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDays = MosquitoRestDays,
  Seed = Seed,
  SeedInf = SeedInf),
  reps)

for(ii in 1 : nrow(scalars)){
  writeSimControl.sobol(
    simControlNum = ii,
    reps = 1,
    NumDays = 365 * 2,
    VaccineDay = 0,
    Vaccination = 0,
    OutputPath = '../simulator/data/Output',
    LocationFile = '../simulator/data/Input/locations_20150801.csv',
    VaccineProfileFile = '../simulator/data/Input/vaccine_profile_age.csv',
    DemographyFile = '../simulator/data/Input/demo_rates_peru.csv',
    scalars = scalars[ii,])
}
