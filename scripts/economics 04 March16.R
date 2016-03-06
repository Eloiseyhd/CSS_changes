#Cost-effectiveness calculations for WHO dengue vaccine modelling consortium
#(c) Mark Jit and Stefan Flasche 2016
#mark.jit@lshtm.ac.uk; February 2016; version 3
#changelog v2
#*Added societal perspective scenarios for reference case
#*Changed main outcome from ICER to net monetary benefit
#*Catchup now compared to reference rather than no vacc
#*Threshold for CE set to 2000, 4000, ..., 10000
#changelog v3
#*Set base case to 0% discounting for health effects (sensitivity analysis 3%)
#*Set thresholds to 500, 1000, ..., 2500 at 0% health discounting and 2000, 4000, ..., 10000 at 3% health discounting
#*Lower PHP costs
#changelog v4
#*Revert back to 3% discounting in the base case (sensitivity analysis 0% health effects)
#* Set thresholds to 0, 2000, ..., 10000 for everything

rm(list = ls(all = TRUE))	
 setwd("~/")
 setwd("..")
 setwd(".\\Dropbox\\CMDVI\\Phase II analysis\\Code\\Economics")
#setwd("C:/Users/EIDESFLA/Dropbox/CMDVI/Phase II analysis/Code/Economics")
#setwd("C:\\Users\\Stefan Flasche\\Dropbox\\CMDVI\\Phase II analysis\\Code\\Economics")
source("economics_functions 04 March16.r")
require(tidyr)
require(dplyr)

#############################################
# input from modellers
#############################################
  n.group="A"

  #mock model epi outputs for the non-optional scenarios and the optional (except intro at 16) age of introductions 
  #Maximum scenario number, model year, year of age and posterior samples 
  #This number of categories less 1 since counting starts at 0)
  scenarios=paste(rep(c("noVaccine","reference","catchUp",paste("routineAT",10:18,sep=""),"coverageAT50%"),each=5),c(10,30,50,70,90),sep="-")
  scenarios.ref=grep("reference",scenarios,value=T)
  year.max=29
  age.max=99
  
  npost=2 #number of samples from posterior distribution

  #Mock model mortality data
  mort=rep(0,age.max+1)	#all-cause mortality by age
  mort[1]=0.1; mort[2:50]=0.01; mort[51:age.max]=0.1; mort[age.max+1]=1  
  
  # create data structure to hold mock data
  data=matrix(0, nrow=(length(scenarios))*(year.max+1)*(age.max+1), ncol=7)
  colnames(data)=c("scen","year","age","vac","amb","hosp","death")
  #scen = scenario (here 0=no vaccination, 1=vaccination scenario)
  #year = years since start of vaccination (0 = pre-vaccination)
  #age = age group
  #vac = absolute number of individuals vaccinated (with all 3 doses)
  #amb = absolute number of ambulatory (symptomatic) cases
  #hosp = absolute number of hospitalised cases
  #death = absolute number of deaths
  for(s in scenarios){
    s_num=which(scenarios==s)-1
    for(y in 0:year.max){
      for(a in 0:age.max){
        r=s_num*(year.max+1)*(age.max+1)+y*(age.max+1)+a+1
        data[r,"scen"]=s
        data[r,"year"]=y
        data[r,"age"]=a
        if(!(s %in% paste("noVaccine",c(10,30,50,70,90),sep="-")) & a==9) data[r,"vac"]=170000 else data[r,"vac"]=0
        if(!(s %in% paste("noVaccine",c(10,30,50,70,90),sep="-"))) data[r,"amb"]=200-y*3 else data[r,"amb"]=200
        if(!(s %in% paste("noVaccine",c(10,30,50,70,90),sep="-"))) data[r,"hosp"]=100-y*2 else data[r,"hosp"]=100
        if(!(s %in% paste("noVaccine",c(10,30,50,70,90),sep="-"))) data[r,"death"]=1-y*0.01 else data[r,"death"]=1
      }}}
  
  # representing 100 posterior model samples (which are all the same here)
  # number of scenarios x number of indicators x number of posterior model samples 
  data <- array( rep( data , npost) , dim = c( dim(data) , npost), dimnames=list(RowNumber=1:dim(data)[1], Colnames=colnames(data), PosteriorNumber=1:npost))
  
#############################################
# Economic parameters and scenarios
#############################################

  econ.scenarios=c("PHL","nodisc","society")

  disc.cost = 0.03	#discount rate for costs
  disc.daly = 0.03	#discount rate for dalys
  persp = 0			#perspective 0=health care provider, 1=society
  thresholds=(0:5)*2000 #threshold for cost-effectiveness

  econ.para.bra=list(
    c.vac = 30.50,	#HCP cost of vaccination
    c.amb = 60,		#HCP cost of treating VCD case
    c.hosp = 200,		#HCP cost of treating hospitalised VCD case
    c.death= 0,		#HCP cost of dengue death

    sc.vac = 30.50,	#societal cost of vaccination
    sc.amb = 200,		#societal cost of treating VCD case
    sc.hosp = 500,	#societal cost of treating hospitalised VCD case
    sc.death = 11000,	#societal cost of dengue death

    d.vac = 0,			#dalys due to vaccination
    d.amb = 0.545*4/365,	#dalys due to ambulatory case
    d.hosp = 0.545*14/365	#dalys due to hospitalised case
  )

  econ.para.phl=list(
    c.vac = 30.50,	#HCP cost of vaccination
    c.amb = 20,		#HCP cost of treating VCD case
    c.hosp = 400,		#HCP cost of treating hospitalised VCD case
    c.death = 0,		#HCP cost of dengue death
    
    sc.vac = 30.50,	#societal cost of vaccination
    sc.amb = 100,		#societal cost of treating VCD case
    sc.hosp = 700,	#societal cost of treating hospitalised VCD case
    sc.death = 3000,	#societal cost of treating hospitalised VCD case

    d.vac = 0,		#dalys due to vaccination
    d.amb = 0.545*4/365,	#dalys due to ambulatory case
    d.hosp = 0.545*14/365	#dalys due to hospitalised case
  )

############################################# 
# calcuate outcomes
#############################################  
  
  # calc life expectancies
  out.le=calcLifExp(age.max, mort, disc.daly)
  disc.surv=out.le[,"survival"]
  disc.life.exp=out.le[,"lifeExp"]
  
  #calc NMB and threshold cost for epi scenarios
  df_out=matrix(NA, nrow=0, ncol=10)
  colnames(df_out)=c("group","transmission_setting","scenario","outcome","outcome_denominator","age","year","value","CI_low","CI_high")

  for(scenario_num in 1:length(scenarios)){
  for(thresh_num in 1:length(thresholds)){
  	
    print(scenario<-scenarios[scenario_num])
    Transmission_intensity=unlist(strsplit(scenario, "-"))[2]
    scenario_name=unlist(strsplit(scenario, "-"))[1]
    if(scenario_name!="catchUp"){
      reference_scenario=paste("noVaccine",Transmission_intensity,sep="-")
      incrm=""
    }
    if(scenario_name=="catchUp") {
      reference_scenario=paste("reference",Transmission_intensity,sep="-")
      incrm="-incremental"
    }
    scenario_name_econ=paste(scenario_name,"BRA",sep="-")
    # for all posterior samples
    icer.sample=rep(NA,dim(data)[3])
    threshold.sample=icer.sample
    pb <- txtProgressBar(min = 0, max = dim(data)[3], style = 3)
    for(post in 1:dim(data)[3]){
      data_post=as.data.frame(data[,,post])
      for(i in 2:dim(data)[2]) data_post[,i]=as.numeric(as.character((data_post[,i])))
	out=calcEcoOut(
		scenario=scenario, reference_scenario=reference_scenario, data=data_post,econ=econ.para.bra,
		persp=persp,thresh=thresholds[thresh_num],disc.cost=disc.cost,disc.daly=disc.daly,year.max=year.max)
      icer.sample[post]=out[,"NMBpp"]
	threshold.sample[post]=out[,"ThresholdCosts"]
      setTxtProgressBar(pb, post)
    }
    close(pb)
    res_icer=c(mean(icer.sample),quantile(icer.sample,probs=c(0.025,0.975)))
    df_out = rbind(df_out,c(
	    n.group,Transmission_intensity,scenario_name_econ,paste("NMBpp-",thresholds[thresh_num],incrm,sep=""),NA,"overall",
	    paste0("cum",year.max+1),
	    res_icer))
    res_thres=c(mean(threshold.sample),quantile(threshold.sample,probs=c(0.025,0.975)))
    df_out = rbind(df_out,c(
	    n.group,Transmission_intensity,scenario_name_econ,paste("ThresholdCost-",thresholds[thresh_num],incrm,sep=""),NA,"overall",
	    paste0("cum",year.max+1),
	    res_thres))
}}


#Economic scenarios
for(Transmission_intensity in c(10,30,50,70,90)){
for(thresh_num in 1:length(thresholds)){
for(econ.scen.num in 1:length(econ.scenarios)){

	scenario_name="reference"
	scenario=paste(scenario_name,Transmission_intensity,sep="-")
	scenario_name_econ<-paste(scenario_name,econ.scenarios[econ.scen.num],sep="-")
	reference_scenario=paste("noVaccine",Transmission_intensity,sep="-")
  	incrm=""
	if(econ.scenarios[econ.scen.num]=="discdaly0.03"){
		threshold.value=thresholds[thresh_num]*4			#threshold goes from $500...$2500 to $2000...$10000 if health effects discounted
	} else {
		threshold.value=thresholds[thresh_num]			
	}

	icer.sample=rep(NA,dim(data)[3])
	threshold.sample=icer.sample
	print(paste(scenario_name_econ,Transmission_intensity,threshold.value,sep=" "))
	pb <- txtProgressBar(min = 0, max = dim(data)[3], style = 3)
 	for(post in 1:dim(data)[3]){
      	data_post=as.data.frame(data[,,post])
      	for(i in 2:dim(data)[2])
        	data_post[,i]=as.numeric(as.character((data_post[,i])))
			
		if(econ.scenarios[econ.scen.num]=="PHL")
			out=calcEcoOut(
				scenario=scenario, reference_scenario=reference_scenario, data=data_post,econ=econ.para.phl,
				persp=persp,thresh=threshold.value,disc.cost=disc.cost,disc.daly=disc.daly,year.max=year.max)
			
		if(econ.scenarios[econ.scen.num]=="nodisc"){
				
				#Recalculate discounted life expectancies
  				out.le=calcLifExp(age.max, mort, 0)
  				disc.surv=out.le[,"survival"]
				disc.life.exp=out.le[,"lifeExp"]

      	  		out=calcEcoOut(
					scenario=scenario, reference_scenario=reference_scenario, data=data_post,econ=econ.para.bra,
					persp=persp,thresh=threshold.value,disc.cost=disc.cost,disc.daly=0,year.max=year.max)

				#Recalculate discounted life expectancies
  				out.le=calcLifExp(age.max, mort, disc.daly)
  				disc.surv=out.le[,"survival"]
				disc.life.exp=out.le[,"lifeExp"]

			}
		
		if(econ.scenarios[econ.scen.num]=="society")
      		out=calcEcoOut(
				scenario=scenario, reference_scenario=reference_scenario, data=data_post,econ=econ.para.bra,
				persp=1,thresh=threshold.value,disc.cost=disc.cost,disc.daly=disc.daly,year.max=year.max)
	  		
		icer.sample[post]=out[,"NMBpp"]
	  	threshold.sample[post]=out[,"ThresholdCosts"]
 	    	setTxtProgressBar(pb, post)

	}

	close(pb)
	res_icer=c(mean(icer.sample),quantile(icer.sample,probs=c(0.025,0.975)))
	df_out = rbind(df_out,c(
  			n.group,Transmission_intensity,scenario_name_econ,paste("NMBpp-",threshold.value,incrm,sep=""),NA,"overall",
  			paste0("cum",year.max+1),
  			res_icer))
	res_thres=c(mean(threshold.sample),quantile(threshold.sample,probs=c(0.025,0.975))) 
    	df_out = rbind(df_out,c(
	  		n.group,Transmission_intensity,scenario_name_econ,paste("ThresholdCost-",threshold.value,incrm,sep=""),NA,"overall",
	  		paste0("cum",year.max+1),
	  		res_thres))
}}}

df_out=as.data.frame(df_out)
  
#List all scenarios with threshold $2000 and 50% seroprevalence
df_out[df_out$transmission_setting==50 & df_out$outcome=="NMBpp-2000","scenario"]
  
  
  