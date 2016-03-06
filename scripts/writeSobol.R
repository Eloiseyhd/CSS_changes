source('functions.R')

reps = 200

HumanImmunityDays = rep(686,2)
EmergenceFactor = c(.01,1.99)
MosquitoLifespan = c(2,6)
MosquitoInfectiousness = c(.01,.4)
MosquitoMoveProbability = rep(0.3,2)
MosquitoRestDays = rep(2/3,2)
Seed = c(1,as.integer(Sys.time()))
SeedInf = Seed

scalars = sobol(vars = list(
  HumanImmunityDays = HumanImmunityDays,
  EmergenceFactor = EmergenceFactor,
  MosquitoLifespan = MosquitoLifespan,
  MosquitoInfectiousness = MosquitoInfectiousness,
  MosquitoMoveProbability = MosquitoMoveProbability,
  MosquitoRestDays = MosquitoRestDays,
  Seed = Seed,
  SeedInf = SeedInf),
  reps)

scalars = rbind(scalars,scalars,scalars,scalars,scalars)
scalars$ForceOfImportation = c(
  rep(.000025,reps),
  rep(.00011,reps),
  rep(.0002,reps),
  rep(.00036,reps),
  rep(.00069,reps)
)

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
    VaccineDay = 365 * 20,
    Vaccination = 1,
    VaccineCoverage = .8,
    VaccineAge = 9,
    Catchup = 0,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
  
  writeSimControl.sobol(
    simControlNum = 2 * nrow(scalars) + ii,
    reps = 1,
    NumDays = 365 * 50,
    VaccineDay = 365 * 20,
    Vaccination = 1,
    VaccineCoverage = .8,
    VaccineAge = 9,
    Catchup = 1,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
  
  writeSimControl.sobol(
    simControlNum = 3 * nrow(scalars) + ii,
    reps = 1,
    NumDays = 365 * 50,
    VaccineDay = 365 * 20,
    Vaccination = 1,
    VaccineCoverage = .5,
    VaccineAge = 9,
    Catchup = 0,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
  
  writeSimControl.sobol(
    simControlNum = 4 * nrow(scalars) + ii,
    reps = 1,
    NumDays = 365 * 50,
    VaccineDay = 365 * 20,
    Vaccination = 1,
    VaccineCoverage = .8,
    VaccineAge = 16,
    Catchup = 0,
    OutputPath = '.',
    LocationFile = 'locations_20150801.csv',
    VaccineProfileFile = 'vaccine_profile_placebo.csv',
    DemographyFile = 'demo_rates_peru.csv',
    scalars = scalars[ii,])
}
