source('functions.R')

reps = 1000

HumanImmunityDays = rep(686,2)
ForceOfImportation = c(1e-7,1e-5)
EmergenceFactor = c(.5,5)
MosquitoLifespan = c(2,20)
MosquitoInfectiousness = c(0,1)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDays = rep(2/3,2)
Seed = c(1,as.integer(Sys.time()))

scalars = sobol(vars = list(
  HumanImmunityDays = HumanImmunityDays,
  ForceOfImportation = ForceOfImportation,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDays = MosquitoRestDays,
  Seed = Seed),
  reps)

for(ii in 1 : reps){
  writeSimControl.sobol(
    simControlNum = ii,
    reps = 1, # reps,
    NumDays = 365 * 100,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
}
