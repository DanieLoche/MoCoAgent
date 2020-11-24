import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas

fully=[]
fullx=[]
fullz=[]
testa=-1
sns.set_theme(style="whitegrid")
for nom in sys.argv[1:]:
    if(str(nom)!='/'):
        flag=1
        try:
            f = open(str(nom),"r")
        except IOError:
            print(str(nom),"n'existe pas")
            flag=0
        if(flag):
            lignes = f.readlines()
            i=0
            listeMot=lignes[0].split(";")
            for mot in listeMot:
                motPropre=str(mot).split(" ")
                for ff in motPropre:
                    final=ff.split('\n')
                    for lala in final:
                        if(str(lala)=='name'):
                            etiquetteNom=i
                        if(str(lala)=='duration'):
                            etiquetteValeur=i
                i=i+1            
            flagStress="Isolé"
            testa=testa*-1
            if(testa<0):
                flagStress="Stressé"
            for ligne in lignes[1:]:
                numero=str(ligne).split(";")
                fully.append(int(numero[etiquetteValeur])/1000000)# modif echelle à mettre dans la légénde
                fullx.append(str(numero[etiquetteNom])) 
                fullz.append(flagStress)                   
            f.close
datafinal = {'':fullx,'Time (ms)':fully,'Legende':fullz}
dl = pandas.DataFrame(datafinal)

check="Stressé"
checkIsole="Isolé"
StressPresent = fullz.count(check)
IsolePresent = fullz.count(checkIsole)

#ax = sns.violinplot(x=dl["Graphe"],y=dl["Valeur (1e6)"],hue=dl["Legende"],split=True,cut=0,scale="count",inner="quartile") area / count / width
#plt.show()

if StressPresent > 0 and IsolePresent > 0:
    ax = sns.violinplot(x=dl[""],y=dl["Time (ms)"],hue=dl["Legende"],split=True,scale="count",inner="quartile",cut=0) #,cut=0

if StressPresent == 0 and IsolePresent > 0:
    ax = sns.violinplot(x=dl[""],y=dl["Time (ms)"],cut=0)   
    
#fig.text(0.5, 0.04, 'Remaining Response Times', ha='center', va='center')
figName=str(sys.argv[1]).split("/")
fig=plt.gcf();
fig.canvas.set_window_title(figName[0]);
plt.show()
#python3 AffichageViolon.py FF_L_2_expe.csv FF_L_2_expe+S.csv FFi_S_2_expe.csv FFi_S_2_expe+S.csv FF_S_2_expe.csv FF_S_2_expe+S.csv
#gestion du split si il nexiste pas 
