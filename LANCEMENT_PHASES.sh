#!/usr/bin/env bash

DURATION=30

#### PHASE 3 - Chain + Tasks BE - Monitoring
sudo ./MoCoAgent.out -i input_chain_stress.txt -OTHER  -e 1 -d $DURATION -o L_PHASE3

#### PHASE 5 - Chain            - Monitoring
sudo ./MoCoAgent.out -i input_chaine.txt -OTHER  -e 1 -d $DURATION -o L_PHASE5

#### PHASE 6 - Chain            - Control
sudo ./MoCoAgent.out -i input_chaine.txt -OTHER  -e 2 -d $DURATION -o L_PHASE6

#### PHASE 7 - Chain + Tasks BE - Control
sudo ./MoCoAgent.out -i input_chain_stress.txt -OTHER -e 2 -d $DURATION -o L_PHASE7
