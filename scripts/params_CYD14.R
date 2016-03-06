d = read.csv('../simulator/data/Input/data_CYD14.csv')


ages.2_5 = seq(2,5.99,.01)
ages.6_11 = seq(6,11.99,.01)
ages.12_14 = seq(12,14.99,.01)


ssq.foi = function(FOI){
  (mean(dpois(0,FOI*ages.2_5)) - d$mean[d$Strata=='2-5y'&d$Arm=='Seronegative']) ^ 2 +
  (mean(dpois(0,FOI*ages.6_11)) - d$mean[d$Strata=='6-11y'&d$Arm=='Seronegative']) ^ 2 +
  (mean(dpois(0,FOI*ages.12_14)) - d$mean[d$Strata=='12-14y'&d$Arm=='Seronegative']) ^ 2
}
foi.mean = optimize(ssq.foi,interval=c(0,1))$minimum

ssq.foi = function(FOI){
  (mean(dpois(0,FOI*ages.2_5)) - d$lower[d$Strata=='2-5y'&d$Arm=='Seronegative']) ^ 2 +
    (mean(dpois(0,FOI*ages.6_11)) - d$lower[d$Strata=='6-11y'&d$Arm=='Seronegative']) ^ 2 +
    (mean(dpois(0,FOI*ages.12_14)) - d$lower[d$Strata=='12-14y'&d$Arm=='Seronegative']) ^ 2
}
foi.lower = optimize(ssq.foi,interval=c(0,1))$minimum

ssq.foi = function(FOI){
  (mean(dpois(0,FOI*ages.2_5)) - d$upper[d$Strata=='2-5y'&d$Arm=='Seronegative']) ^ 2 +
    (mean(dpois(0,FOI*ages.6_11)) - d$upper[d$Strata=='6-11y'&d$Arm=='Seronegative']) ^ 2 +
    (mean(dpois(0,FOI*ages.12_14)) - d$upper[d$Strata=='12-14y'&d$Arm=='Seronegative']) ^ 2
}
foi.upper = optimize(ssq.foi,interval=c(0,1))$minimum
