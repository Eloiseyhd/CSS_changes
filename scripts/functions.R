initialInfections = function(
  simControlNum = 1,
	num.1 = 0,
	num.2 = 0,
	num.3 = 0,
  num.4 = 0,
	zone = NA,
	neighborhood = NA,
	block = NA,
	reps = 1)
{
	loc = read.csv('../simulator/data/Input/locations_20150801.csv')

	for(ii in 1:reps){
		houses = character()

		num.total = num.1 + num.2 + num.3 + num.4

		if(!is.na(zone)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$zone %in% zone],
				size = num.total,
				replace = F)))
		}else if(!is.na(neighborhood)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$neighborhood %in% neighborhood],
				size = num.total,
				replace = F)))
		}else if(!is.na(block)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$block %in% block],
				size = num.total,
				replace = F)))
		}else{
		  houses = c(houses, as.character(sample(
		    loc$code[loc$landuse == 'HOUSE'],
		    size = num.total,
		    replace = F)))
		}

		write(
			paste(
				houses,
				rep(1,num.total),
				c(rep(1,num.1),rep(2,num.2),rep(3,num.3),rep(4,num.4)),
				sep=',',collapse='\n'),
			file = paste('initialInfections_', simControlNum - 1, '.csv', sep=''))	
	}
}



library(pomp)

writeSimControl.sobol = function(
  simControlNum,
	reps = 100,
	NumDays = 365 * 10,
	OutputPath = '../simulator/data/Output',
	LocationFile = '../simulator/data/Input/locations_20150801.csv',
  MortalityFile = '../simulator/data/Input/age_specific_mortality.csv',
  VaccineProfileFile = '../simulator/data/Input/vaccine_profile_CYD15.csv',
  num.1 = 0,
	num.2 = 0,
	num.3 = 0,
  num.4 = 0,
	zone = NA,
	neighborhood = NA,
	block = NA,
  scalars = scalars)
{  
  ii = 1 # simControlNum
  
  simControlFile = paste('SimControl_baseline_', simControlNum - 1, '.csv', sep='')

  file.connection = file(simControlFile)
	toWrite = 'SimulationName,Seed,NumDays,OutputPath,LocationFile,MortalityFile,TrajectoryFile,InitialInfectionsFile,VaccineProfileFile,ForceOfInfection,HumanImmunityDays,EmergenceFactor,MosquitoLifespan,MosquitoInfectiousness,MosquitoMoveProbability,MosquitoRestDaysLow,MosquitoRestDaysHigh\n'

	TrajectoryFile = 'trajectories.txt'

	initialInfections(simControlNum, num.1, num.2, num.3, num.4, zone, neighborhood, block, 1)
	InitialInfectionsFile = paste('initialInfections_', simControlNum - 1, '.csv', sep='')

	toWrite =
		paste(
			toWrite,
			'baseline_', simControlNum - 1, ',',
			as.integer(Sys.time()), ',',
			NumDays, ',',
			OutputPath, ',',
			LocationFile, ',',
      MortalityFile,',',
			TrajectoryFile, ',',
			InitialInfectionsFile, ',',
      VaccineProfileFile, ',',
			scalars$ForceOfInfection[ii], ',',
			scalars$HumanImmunityDays[ii], ',',
			scalars$EmergenceFactor[ii], ',',
			scalars$MosquitoLifespan[ii], ',',
			scalars$MosquitoInfectiousness[ii], ',',
			scalars$MosquitoMoveProbability[ii], ',',
			scalars$MosquitoRestDaysLow[ii], ',',
			scalars$MosquitoRestDaysHigh[ii],
      '\n',
			sep = '')
  
	write(toWrite, file.connection)

	close(file.connection)
}
