
#############################################
#Calculate discounted life expectancy
#############################################
calcLifExp = function(age.max, mort, disc.daly){
  surv=rep(0,age.max+1)		#survival
  disc.surv=rep(0,age.max+1)	#discounted survival
  life.exp=rep(0,age.max+1)	#life expectancy
  disc.life.exp=rep(0,age.max+1)#discounted life expectancy
  surv[1]=1
  disc.surv[1]=1
  for(a in 1:age.max){
    surv[a+1] = surv[a] * (1-mort[a])
    disc.surv[a+1] = disc.surv[a] * (1-mort[a])/(1+disc.daly)
  }
  for(a in 0:age.max){
    life.exp[a+1]=sum(surv[(a+1):(age.max+1)])/surv[a+1]
    disc.life.exp[a+1]=sum(disc.surv[(a+1):(age.max+1)])/disc.surv[a+1]
  }
  return(cbind(survival=disc.surv, lifeExp=disc.life.exp))
}


#############################################
#Calculate costs and DALYs
#############################################

calcEcoOut = function(scenario,reference_scenario,data,econ,persp,thresh,disc.cost,disc.daly,year.max){

  data_tmp=subset(data, scen %in% c(scenario,reference_scenario))
  if(scenario==reference_scenario){
	data_tmp2=data_tmp
	data_tmp2$scen=paste(data_tmp2$scen,"notref",sep="-")
	data_tmp=rbind(data_tmp,data_tmp2)
	}
  
  disc.cost.vec=rep(0,year.max+1)
  disc.daly.vec=rep(0,year.max+1)
  for(y in 0:year.max) disc.cost.vec[y+1]=1/(1+disc.cost)^y
  for(y in 0:year.max) disc.daly.vec[y+1]=1/(1+disc.daly)^y

  if(persp==0) {c.vac=econ$c.vac; c.amb=econ$c.amb; c.hosp=econ$c.hosp; c.death=econ$c.death}
  if(persp==1) {c.vac=econ$sc.vac; c.amb=econ$sc.amb; c.hosp=econ$sc.hosp; c.death=econ$sc.death}

  y=data_tmp[,"year"]
  a=data_tmp[,"age"]
  data_tmp$n.vac=data_tmp[,"vac"] * disc.cost.vec[y+1]
  data_tmp$cost.vac=data_tmp[,"vac"] * c.vac * disc.cost.vec[y+1]
  data_tmp$cost.treat=(data_tmp[,"amb"] * c.amb + data_tmp[,"hosp"] * c.hosp + data_tmp[,"death"] * c.death) * disc.cost.vec[y+1]
  data_tmp$daly.vac=data_tmp[,"vac"] * econ$d.vac * disc.daly.vec[y+1]
  data_tmp$daly.treat=(data_tmp[,"amb"] * econ$d.amb+ data_tmp[,"hosp"] * econ$d.hosp) * disc.daly.vec[y+1]
  data_tmp$daly.death=data_tmp[,"death"] * disc.life.exp[a+1] * disc.daly.vec[y+1]
  
  #Calculate final outcomes
  #Note this assumes only two scenarios (vaccine and no vaccine)
  outs=c("n.vac","cost.vac","cost.treat","daly.vac","daly.treat","daly.death")
  data_tmp[data_tmp$scen==reference_scenario,outs]=data_tmp[data_tmp$scen==reference_scenario,outs]*(-1)
  
  net.cost=sum(data_tmp[,c("cost.vac","cost.treat")])
  net.daly=sum(data_tmp[,c("daly.vac","daly.treat","daly.death")])
  net.cost.treat=sum(data_tmp[,c("cost.treat")])
  net.n.vac=sum(data_tmp[,c("n.vac")])
  
  icer = -net.cost/net.daly	#Incremental cost per DALY averted
  nmb = -(net.cost+net.daly*thresh) 
  nmbpp = nmb/net.n.vac
  threshold.cost = -(net.cost.treat + net.daly * thresh)/net.n.vac	#Threshold cost per person vaccinated
 
   # manually correct for rounding errors
  if(abs(net.cost)<.1) icer=0
  if(abs(net.cost)<.1 & abs(net.daly)<.1) threshold.cost=0

  return(cbind(Cost=net.cost, DALY=net.daly, ICER=icer, ThresholdCosts=threshold.cost, NMB=nmb, NMBpp=nmbpp))
}