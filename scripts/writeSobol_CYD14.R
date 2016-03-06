source('functions.R')

reps = 100

ForceOfImportation = c(foi.upper,foi.lower)
HumanImmunityDays = rep(686,2)
EmergenceFactor = c(.001,.001)
MosquitoLifespan = c(.001,.001)
MosquitoInfectiousness = c(.00001,.00001)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDays = rep(2/3,2)
Seed = c(1,as.integer(Sys.time()))
SeedInf = Seed

scalars = sobol(vars = list(
  ForceOfImportation = ForceOfImportation,
  HumanImmunityDays = HumanImmunityDays,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDays = MosquitoRestDays,
  Seed = Seed,
  SeedInf = SeedInf),
  reps)

for(ii in 1 : nrow(scalars)){
  writeSimControl.sobol(
    simControlNum = ii,
    reps = 1,
    NumDays = 365 * 50,
    VaccineDay = 0,
    Vaccination = 0,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
  
  writeSimControl.sobol(
    simControlNum = nrow(scalars) + ii,
    reps = 1,
    NumDays = 365 * 50,
    VaccineDay = 0,
    Vaccination = 1,
    VaccineCoverage = 1,
    VaccineAge = 9,
    Catchup = 0,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
}
