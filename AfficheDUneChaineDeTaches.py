


import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas
tag=0
resultx=[]
resulty=[]
NomLegend=[]
fully=[]
fullx=[]
fullz=[]
fulltt=[]
fullR=[]
fullRSTRESS=[]
fullRx=[]
NmbreRwcet=0

sns.set_theme(style="whitegrid")
for nom in sys.argv[1:]:
    if(str(nom)!='/'):
        NomLegend.append(str(nom))
        flag=1
        flagpremier=1
        try:
            f = open(str(nom),"r")
        except IOError:
            print(str(nom),"n'existe pas")
            flag=0
        if(flag):
            lignes = f.readlines()
            #gerer les etiquettes et leur place
            i=0
            NmbreRwcet=0
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
                        MotTestRwcet=str(lala).split('_t')
                        for AutreMot in MotTestRwcet:
                            if(str(AutreMot)=='rWCETs'):
                                NmbreRwcet=NmbreRwcet+1
                i=i+1 
            FlagArretWhile=0                  
            while(FlagArretWhile < NmbreRwcet):
                tempo=[]
                tempo2=[]
                tempo3=[]
                fullR.append(tempo)
                fullRSTRESS.append(tempo2)
                fullRx.append(tempo3)
                FlagArretWhile=FlagArretWhile+1
            flagStress="Isolé"
            ListeNom=str(nom).split("+")
            if(len(ListeNom)>1):
                flagStress="Stressé"
            for ligne in lignes[1:]:
                numero=str(ligne).split(";")
                fully.append(int(numero[etiquetteValeur])/1000000) # à comparer sur le nom durée
                #fullx.append(str(numero[etiquetteNom])) # modifier pour end to end if choix flag
                fullx.append("End to end")
                fullz.append(flagStress)
#gestion
                FlagArretWhile=0                
                while(FlagArretWhile < NmbreRwcet):
                    fullR[FlagArretWhile].append(int(numero[etiquetteValeur+FlagArretWhile+1])/1000000)
                    fullRSTRESS[FlagArretWhile].append(flagStress) 
                    alle="From t"+str(FlagArretWhile+1)
                    fullRx[FlagArretWhile].append(str(alle))   
                    FlagArretWhile=FlagArretWhile+1 
#fin gestion          
            f.close
            tag+=1
FinalLegend=[]
NomDonnees=''
datafinal = {NomDonnees:fullx,'Time (ms)':fully,'Legende':fullz}
dl = pandas.DataFrame(datafinal)
fig, axs = plt.subplots(1, NmbreRwcet+1, figsize=(15, 5), sharey=True)
check="Stressé"
checkIsole="Isolé"
StressPresent = fullz.count(check)
IsolePresent = fullz.count(checkIsole)

if StressPresent > 0 and IsolePresent > 0:
    sns.violinplot(ax=axs[0],x=dl[""],y=dl["Time (ms)"],hue=dl["Legende"],split=True,cut=0)
    FlagArretWhile=0                
    while(FlagArretWhile < NmbreRwcet):   
        datafinal = {"":fullRx[FlagArretWhile],' ':fullR[FlagArretWhile],'Legende':fullRSTRESS[FlagArretWhile]}
        dl = pandas.DataFrame(datafinal)
        sns.violinplot(ax=axs[FlagArretWhile+1],x=dl[""],y=dl[" "],hue=dl["Legende"],split=True,dodge=True,cut=0) 
        handles, labels = axs[FlagArretWhile+1].get_legend_handles_labels()
        axs[FlagArretWhile+1].legend(handles="", labels="")
        axs[FlagArretWhile+1].get_legend().remove()
        FlagArretWhile=FlagArretWhile+1 
if StressPresent == 0 and IsolePresent > 0:
    sns.violinplot(ax=axs[0],x=dl[""],y=dl["Time (ms)"],cut=0)
    FlagArretWhile=0                
    while(FlagArretWhile < NmbreRwcet):   
        datafinal = {"":fullRx[FlagArretWhile],' ':fullR[FlagArretWhile],'Legende':fullRSTRESS[FlagArretWhile]}
        dl = pandas.DataFrame(datafinal)
        sns.violinplot(ax=axs[FlagArretWhile+1],x=dl[""],y=dl[" "],cut=0) 
        FlagArretWhile=FlagArretWhile+1     
if StressPresent > 0 and IsolePresent == 0:
    sns.violinplot(ax=axs[0],x=dl[""],y=dl["Time (ms)"],cut=0)
    FlagArretWhile=0                
    while(FlagArretWhile < NmbreRwcet):
        datafinal = {"":fullRx[FlagArretWhile],' ':fullR[FlagArretWhile],'Legende':fullRSTRESS[FlagArretWhile]}
        dl = pandas.DataFrame(datafinal)
        sns.violinplot(ax=axs[FlagArretWhile+1],x=dl[""],y=dl[" "],cut=0) 
        FlagArretWhile=FlagArretWhile+1     
fig.text(0.5, 0.04, 'Remaining Response Times', ha='center', va='center')
plt.show()
