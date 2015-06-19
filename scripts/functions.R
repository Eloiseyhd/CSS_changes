initialInfections = function(
  simControlNum = 1,
	num.1 = 0,
	num.2 = 0,
	num.3 = 1,
	zone = 'MY',
	neighborhood = NA,
	block = NA,
	reps = 1)
{
	loc = read.csv('../simulator/data/Input/locations_20140807.csv')

	for(ii in 1:reps){
		houses = character()

		num.total = num.1 + num.2 + num.3

		if(!is.na(zone)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$zone %in% zone],
				size = num.total,
				replace = F)))
		}

		if(!is.na(neighborhood)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$neighborhood %in% neighborhood],
				size = num.total,
				replace = F)))
		}

		if(!is.na(block)){
			houses = c(houses, as.character(sample(
				loc$code[loc$landuse == 'HOUSE' & loc$block %in% block],
				size = num.total,
				replace = F)))
		}

		write(
			paste(
				houses,
				rep(1,num.total),
				c(rep(1,num.1),rep(2,num.2),rep(3,num.3)),
				sep=',',collapse='\n'),
			file = paste('../simulator/data/Input/initialInfections_', simControlNum, '_', ii, '.csv', sep=''))	
	}
}



# writeSimControl.reps = function(
# 	reps = 10,
# 	NumDays = 30,
# 	OutputPath = '../simulator/data/Output',
# 	LocationFile = '../simulator/data/Input/locations_20140807.csv',
# 	NeighboorhoodFile = '../simulator/data/Input/neighborhoods.csv',
# 	HumanLatencyLow = 4,
# 	HumanLatencyHigh = 7,
# 	HumanInfectionDays = 9,
# 	HumanImmunityDays = 686,
# 	BiteProbability = 0.5,
# 	MosquitoLifespan_Low = 10,
# 	MosquitoLifespan_High = 18,
# 	MosquitoInfectiousness = 0.5,
# 	MosquitoLatency_Low = 8,
# 	MosquitoLatency_High = 10,
# 	MosquitoMoveProbability = 0.3,
# 	MosquitoRestDays_Low = 2,
# 	MosquitoRestDays_High = 3,
# 	num.1 = 0,
# 	num.2 = 0,
# 	num.3 = 1,
# 	zone = 'MY',
# 	neighborhood = NA,
# 	block = NA,
# 	sameMosquitoes = FALSE,
# 	sameTrajectories = FALSE,
# 	sameInfections = FALSE)
# {
# 	file.connection = file('../simulator/SimControl.csv')
# 	digits = ceiling(log(reps,10))
# 	toWrite = 'SimulationName,Seed,NumDays,OutputPath,MosquitoEmergenceFile,LocationFile,NeighboorhoodFile,TrajectoryFile,InitialInfectionsFile,HumanLatencyLow,HumanLatencyHigh,HumanInfectionDays,HumanImmunityDays,BiteProbability,MosquitoLifespan_Low,MosquitoLifespan_High,MosquitoInfectiousness,MosquitoLatency_Low,MosquitoLatency_High,MosquitoMoveProbability,MosquitoRestDays_Low,MosquitoRestDays_High\n'
# 
# 	MosquitoEmergenceFile = list.files('../simulator/data/Input/')[which(substr(list.files('../simulator/data/Input/'),0,4) == 'Mosq')]
# 	if(sameMosquitoes){
# 		MosquitoEmergenceFile = rep(paste('../simulator/data/Input/', MosquitoEmergenceFile, sep='')[1],reps)
# 	}
# 	if(!sameMosquitoes){
# 		MosquitoEmergenceFile = paste('../simulator/data/Input/', MosquitoEmergenceFile, sep='')[sample(length(MosquitoEmergenceFile),reps,replace=TRUE)]	
# 	}
# 
# 	TrajectoryFile = list.files('../simulator/data/Input/')[which(substr(list.files('../simulator/data/Input/'),0,4) == 'traj')]
# 	if(sameTrajectories){
# 		TrajectoryFile = rep(paste('../simulator/data/Input/', TrajectoryFile, sep='')[1],reps)
# 	}
# 	if(!sameTrajectories){
# 		TrajectoryFile = paste('../simulator/data/Input/', TrajectoryFile, sep='')[sample(length(TrajectoryFile),reps,replace=TRUE)]	
# 	}
# 
# 	if(sameInfections){
# 		initialInfections(num.1, num.2, num.3, zone, neighborhood, block, 1)
# 		InitialInfectionsFile = rep('../simulator/data/Input/initialInfections_1.csv', reps)
# 	}
# 	if(!sameInfections){
# 		initialInfections(num.1, num.2, num.3, zone, neighborhood, block, reps)
# 		InitialInfectionsFile = paste('../simulator/data/Input/initialInfections_', formatC(1:reps,digits,format='d',flag=0), '.csv', sep='')
# 	}
# 
# 	for(ii in 1:reps){
# 		toWrite =
# 			paste(
# 				toWrite,
# 				'temp', formatC(ii,digits,format='d',flag=0), ',',
# 				as.integer(Sys.time()), formatC(ii,digits,format='d',flag=0), ',',
# 				NumDays, ',',
# 				OutputPath, ',',
# 				MosquitoEmergenceFile[ii], ',',
# 				LocationFile, ',',
# 				NeighboorhoodFile, ',',
# 				TrajectoryFile[ii], ',',
# 				InitialInfectionsFile[ii], ',',
# 				HumanLatencyLow, ',',
# 				HumanLatencyHigh, ',',
# 				HumanInfectionDays, ',',
# 				HumanImmunityDays, ',',
# 				BiteProbability, ',',
# 				MosquitoLifespan_Low, ',',
# 				MosquitoLifespan_High, ',',
# 				MosquitoInfectiousness, ',',
# 				MosquitoLatency_Low, ',',
# 				MosquitoLatency_High, ',',
# 				MosquitoMoveProbability, ',',
# 				MosquitoRestDays_Low, ',',
# 				MosquitoRestDays_High,
# 				'\n',
# 				sep = '')
# 	}
# 
# 	write(toWrite, file.connection)
# 
# 	close(file.connection)
# }



library(pomp)

writeSimControl.sobol = function(
  simControlNum = 1,
  simControlFile = '../simulator/SimControl.csv',
	reps = 10,
	NumDays = 30,
	OutputPath = '../simulator/data/Output',
	LocationFile = '../simulator/data/Input/locations_20140807.csv',
	NeighboorhoodFile = '../simulator/data/Input/neighborhoods.csv',
	HumanLatencyLow = rep(4,2),
	HumanLatencyHigh = rep(7,2),
	HumanInfectionDays = rep(9,2),
	HumanImmunityDays = rep(686,2),
	BiteProbability = rep(1,2),
	MosquitoLifespan_Low = rep(10,2),
	MosquitoLifespan_High = rep(18,2),
	MosquitoInfectiousness = rep(0.5,2),
	MosquitoLatency_Low = rep(8,2),
	MosquitoLatency_High = rep(10,2),
	MosquitoMoveProbability = rep(0.3,2),
	MosquitoRestDays_Low = rep(2,2),
	MosquitoRestDays_High = rep(3,2),
	num.1 = 0,
	num.2 = 0,
	num.3 = 0,
	zone = 'MY',
	neighborhood = NA,
	block = NA,
	sameMosquitoes = FALSE,
	sameTrajectories = FALSE,
	sameInfections = FALSE)
{
	file.connection = file(simControlFile)
	digits = ceiling(log(reps,10))
	toWrite = 'SimulationName,Seed,NumDays,OutputPath,MosquitoEmergenceFile,LocationFile,NeighboorhoodFile,TrajectoryFile,InitialInfectionsFile,HumanLatencyLow,HumanLatencyHigh,HumanInfectionDays,HumanImmunityDays,BiteProbability,MosquitoLifespan_Low,MosquitoLifespan_High,MosquitoInfectiousness,MosquitoLatency_Low,MosquitoLatency_High,MosquitoMoveProbability,MosquitoRestDays_Low,MosquitoRestDays_High\n'

	MosquitoEmergenceFile = list.files('../simulator/data/Input/')[which(substr(list.files('../simulator/data/Input/'),0,4) == 'Mosq')]
	if(sameMosquitoes){
		MosquitoEmergenceFile = rep(paste('../simulator/data/Input/', MosquitoEmergenceFile, sep='')[1],reps)
	}
	if(!sameMosquitoes){
		MosquitoEmergenceFile = paste('../simulator/data/Input/', MosquitoEmergenceFile, sep='')[sample(length(MosquitoEmergenceFile),reps,replace=TRUE)]	
	}

	TrajectoryFile = list.files('../simulator/data/Input/')[which(substr(list.files('../simulator/data/Input/'),0,4) == 'traj')]
	if(sameTrajectories){
		TrajectoryFile = rep(paste('../simulator/data/Input/', TrajectoryFile, sep='')[1],reps)
	}
	if(!sameTrajectories){
		TrajectoryFile = paste('../simulator/data/Input/', TrajectoryFile, sep='')[sample(length(TrajectoryFile),reps,replace=TRUE)]	
	}

	if(sameInfections){
		initialInfections(simControlNum, 1, num.1, num.2, num.3, zone, neighborhood, block, 1)
		InitialInfectionsFile = paste('../simulator/data/Input/initialInfections_', simControlNum, '_', rep(1,reps), '.csv', sep='')
	}
	if(!sameInfections){
		initialInfections(simControlNum, num.1, num.2, num.3, zone, neighborhood, block, reps)
		InitialInfectionsFile = paste('../simulator/data/Input/initialInfections_', simControlNum, '_', 1:reps, '.csv', sep='')
	}

	scalars = sobol(vars = list(
		HumanLatencyLow = HumanLatencyLow,
		HumanLatencyHigh = HumanLatencyHigh,
		HumanInfectionDays = HumanInfectionDays,
		HumanImmunityDays = HumanImmunityDays,
		BiteProbability = BiteProbability,
		MosquitoLifespan_Low = MosquitoLifespan_Low,
		MosquitoLifespan_High = MosquitoLifespan_High,
		MosquitoInfectiousness = MosquitoInfectiousness,
		MosquitoLatency_Low = MosquitoLatency_Low,
		MosquitoLatency_High = MosquitoLatency_High,
		MosquitoMoveProbability = MosquitoMoveProbability,
		MosquitoRestDays_Low = MosquitoRestDays_Low,
		MosquitoRestDays_High = MosquitoRestDays_High),
		simControlNum * reps)

	for(ii in 1:reps){
		toWrite =
			paste(
				toWrite,
				'temp_', simControlNum, '_', ii, ',',
				as.integer(Sys.time()), formatC(ii,digits,format='d',flag=0), ',',
				NumDays, ',',
				OutputPath, ',',
				MosquitoEmergenceFile[ii], ',',
				LocationFile, ',',
				NeighboorhoodFile, ',',
				TrajectoryFile[ii], ',',
				InitialInfectionsFile[ii], ',',
				tail(scalars$HumanLatencyLow,reps)[ii], ',',
				tail(scalars$HumanLatencyHigh,reps)[ii], ',',
				tail(scalars$HumanInfectionDays,reps)[ii], ',',
				tail(scalars$HumanImmunityDays,reps)[ii], ',',
				tail(scalars$BiteProbability,reps)[ii], ',',
				tail(scalars$MosquitoLifespan_Low,reps)[ii], ',',
				tail(scalars$MosquitoLifespan_High,reps)[ii], ',',
				tail(scalars$MosquitoInfectiousness,reps)[ii], ',',
				tail(scalars$MosquitoLatency_Low,reps)[ii], ',',
				tail(scalars$MosquitoLatency_High,reps)[ii], ',',
				tail(scalars$MosquitoMoveProbability,reps)[ii], ',',
				tail(scalars$MosquitoRestDays_Low,reps)[ii], ',',
				tail(scalars$MosquitoRestDays_High,reps)[ii],
				'\n',
				sep = '')
	}
  
	write(toWrite, file.connection)

	close(file.connection)
}
