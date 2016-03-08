library(pomp)

writeSimControl.sobol = function(
  simControlNum,
	reps = 100,
	NumDays = 365 * 10,
  VaccineDay = 365 * 5,
  Vaccination = 0,
  VaccineCoverage = .9,
  VaccineAge = 9,
  Catchup = 0,
  PropInf = .5,
	OutputPath = '../simulator/data/Output',
	LocationFile = '../simulator/data/Input/locations_20150801.csv',
  VaccineProfileFile = '../simulator/data/Input/vaccine_profile_CYD15.csv',
  DemographyFile = '../simulator/data/Input/demo_rates_peru.csv',
  scalars = scalars)
{  
  ii = 1
  
  simControlFile = paste('~/Condor/SimControl_', simControlNum - 1, '.csv', sep='')

  file.connection = file(simControlFile)
	toWrite = 'SimulationName,Seed,SeedInf,NumDays,VaccineDay,Vaccination,VaccineCoverage,VaccineAge,Catchup,PropInf,OutputPath,LocationFile,TrajectoryFile,VaccineProfileFile,DeathRate,ForceOfImportation,HumanImmunityDays,EmergenceFactor,MosquitoLifespan,MosquitoInfectiousness,MosquitoMoveProbability,MosquitoBiting\n'

	TrajectoryFile = '../simulator/data/Input/trajectories.txt'

	demo_rates = read.csv(DemographyFile)
	
	toWrite =
		paste(
			toWrite,
			'sim_', simControlNum - 1, ',',
			round(scalars$Seed[ii]), ',',
			round(scalars$SeedInf[ii]), ',',
			NumDays, ',',
			VaccineDay, ',',
			Vaccination, ',',
			VaccineCoverage, ',',
			VaccineAge, ',',
			Catchup, ',',
			PropInf, ',',
			OutputPath, ',',
			LocationFile, ',',
			TrajectoryFile, ',',
      VaccineProfileFile, ',',
			demo_rates$death, ',',
			scalars$ForceOfImportation[ii], ',',
      scalars$HumanImmunityDays[ii], ',',
			scalars$EmergenceFactor[ii], ',',
			scalars$MosquitoLifespan[ii], ',',
			scalars$MosquitoInfectiousness[ii], ',',
			scalars$MosquitoMoveProbability[ii], ',',
			scalars$MosquitoRestDays[ii],
      '\n',
			sep = '')
  
	write(toWrite, file.connection)

	close(file.connection)
}
