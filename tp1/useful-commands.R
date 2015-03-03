pairs(pro)

mylm <- lm(formula = lpsa ~ lcavol + lweight + age + lbph + svi + lcp + factor(gleason) + pgg45, data = pro)
summary(mylm)

# avec factor(svi)
mylm <- lm(formula = lpsa ~ lcavol + lweight + age + lbph + factor(svi) + lcp + factor(gleason) + pgg45, data = pro)
summary(mylm)

# intervalle de confiance
confint(mylm, level=0.99)
# => lcp                              -0.52254821 0.17406981

# sans lcavol et svi
mylm2 <- lm(formula = lpsa ~ lweight + age + lbph + lcp + factor(gleason) + pgg45, data = pro)

confint(mylm2, level=0.99)
# => lcp                               0.1049965 0.7526673


# correlation entre lcavol, svi, lcp
pairs(~ lcp + lcavol + svi, data = pro, main = "Matrix")

# x = (question 2.c)
qt(0.999, 86) # 3.187722

X <- mylm$residuals
sum(X*X)
# 41.81094

# 4)a)
# overfitting, plus on rajoute de données, plus on fit les points mais ce n'est pas satisfaisant car on prédit mal de nouvelles données
lm(lpsa~1,data=pro) # approxime par la moyenne

lm(lpsa~.,data=pro[,c(1,4,9)]) # équivalent à lm(lpsa~lcavol + lbph,data=pro) => approxime en fonction de lcavol et lbph 

lm(lpsa~.,data=pro[,c(2,7,9)]) # équivalent à lm(lpsa~lweight + gleason,data=pro) => approxime en fonction de lweigth et gleason

X <- mylm$residuals
sum(X*X)

A <- combn(8, 2)
for(i in 1:length(A[1,])) {
    mylm <- lm(lpsa~., data = pro[,c(A[1,i], A[2,i], 9)])
    res <- mylm$residuals
    print(sum(res*res))
}

best.rss <- function(k){
  if(k == 0){
    reg= lm(lpsa~1,data=pro)
    res = reg$residuals
    rss = sum(res*res)
    return(c(rss,c()))
  }
  
  min.rss = -1
  pred.id = combn(1:8,k)
  n = length(pred.id[1,])
  for(i in 1:n){
    var.id = 9
    reg = lm(lpsa~.,data=pro[,c(pred.id[,i],var.id)])
    res = reg$residuals
    rss = sum(res*res)
    if(i == 1){
      min.rss = rss
      best.pred = pred.id[,i]
    } else {
      if(rss < min.rss){
        min.rss = rss
        best.pred = pred.id[,i]
      }
    }
  }
  return(c(min.rss, c(best.pred)))
}

disp.best.rss <- function(){
  rss <- seq(0,0,length.out=9)
  for(k in 0:8){
    rss[k+1] = best.rss(k)[1]
  }
    plot(0:8,rss,type="h")
}

disp.names <- function(){
  for(k in 1:8){
    print(names(pro)[best.rss(k)[2:(k+1)]])
  }
}

get.names <- function(){
  best.subsets = matrix("", 8, 8)
  for(k in 1:8)
  best.subsets[k,1:k] = names(pro[best.rss(k)[2:(k+1)]])
  return(best.subsets)
}

# 5)a) 
# garder des donnees pour valider le modele (pour eviter l'overfitting)

# 5)b)
valid <- seq(from = 1, to = length(pro[,1]), by = 2) 

# 5)c)
split.validation <- function(){
  best.subsets = get.names()
  err.min = -1
  for(k in 0:8){
    # i = best.subsets[2,1]
    # j = best.subsets[2,2]
    if(k == 0){
      preds = c()
      my.lm = lm(lpsa~1, data=pro[-valid,])
    } else {
      preds = best.subsets[k,1:k]
      my.lm = lm(lpsa~.,data=pro[-valid,c(preds,"lpsa")]) # utilise les colonnes i et j pour predire lpsa en retirant les lignes dont les indices donc dans valid   
    }
    y.pred = predict.lm(my.lm,pro[valid,])
    err.pred = y.pred - pro[valid,"lpsa"]
    err.pred.moy = mean(err.pred*err.pred)
    if(k==0){
      err.min = err.pred.moy
      best.preds = preds
    } else if(err.pred.moy < err.min) {
      err.min = err.pred.moy
      best.preds = preds
    }
  }
  return(c(err.min,best.preds))
}


# RSS

# 5)d)