import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas
tag=0
origine=0
maximum=0
resultx=[]
resulty=[]
NomLegend=[]
fully=[]
fullx=[]
sns.set_theme(style="whitegrid")

etiquetteNom=99
etiquetteValeur=99


couleur=['#800000','#808080','#ff0000','#ffa000','#ffff00','#808000','#800080','#ff00ff','#00ff00','#008000','#000080','#0000ff','#00ffff','#008080','#000000'] # faire un modulo 15
for nom in sys.argv[1:]:
    if(str(nom)=='/'):
        origine=maximum+6*1000000000
        maximum=maximum+6*1000000000
    else:
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
            print(str(lignes[0]))
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
        #travail sur le nom
        flagStress=0
        ListeNom=str(nom).split("+")
        if(len(ListeNom)>1):
            flagStress=1
        print(flagStress)









