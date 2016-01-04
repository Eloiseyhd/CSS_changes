library(pomp)

writeSimControl.sobol = function(
  simControlNum,
	reps = 100,
	NumDays = 365 * 10,
	OutputPath = '../simulator/data/Output',
	LocationFile = '../simulator/data/Input/locations_20150801.csv',
  VaccineProfileFile = '../simulator/data/Input/vaccine_profile_CYD15.csv',
  DemographyFile = '../simulator/data/Input/demo_rates_peru.csv',
  scalars = scalars)
{  
  ii = 1
  
  simControlFile = paste('/tmp/tperkin1/cmdvi/SimControl_calibrate_', simControlNum - 1, '.csv', sep='')

  file.connection = file(simControlFile)
	toWrite = 'SimulationName,Seed,NumDays,OutputPath,LocationFile,TrajectoryFile,VaccineProfileFile,DeathRate,ForceOfImportation,HumanImmunityDays,EmergenceFactor,MosquitoLifespan,MosquitoInfectiousness,MosquitoMoveProbability,MosquitoRestDays\n'

	TrajectoryFile = 'trajectories.txt'

	demo_rates = read.csv(paste('../simulator/data/Input/',DemographyFile,sep=''))
	
	toWrite =
		paste(
			toWrite,
			'calibrate_', simControlNum - 1, ',',
			round(scalars$Seed[ii]), ',',
			NumDays, ',',
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
