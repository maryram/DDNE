#!/bin/bash

codePath="../DDNE_Synthetic"
dataPath="../Data/Synthetic"
process_exe="../DataSetProcess"

#**************************************
Graph="ForFire" #ForFire Hierarchical  Periphery
CasType="exponential" #chainsaw exponential	square
Scenario_num=1
#************************************

datafile=$dataPath/$Graph/$CasType
NNode=1024
NSnapshot=10 
NBin=90
MxEdge=1
maxRank=100
ttssn=1
etssn=1
prior=0
trsh=0.04	
minAlpha=0.1

$process_exe/split_Data_Synthetic -n:$NNode -minAlpha:$minAlpha -ssn:$NSnapshot -tt:$tt -h:$h -dt:0  -ic:"$datafile/$Scenario_num/Cascade/cascade.txt" -on:"$datafile/$Scenario_num/Network" -oc:"$datafile/$Scenario_num/Cascade" -in:"$datafile/$Scenario_num/Network/network.txt"
					echo "Split Networks and Cascades done"
				

# Running DDNE - Step 1: Loading Networks and Cascades and Computing Edges' Ranks 
	echo "Running DDNE - Step 1: Load Networks and Cascades and Save Ranks"
	$codePath/extract_Rank -etssn:$etssn -e:$MxEdge -network:"$datafile/$Scenario_num/Network" -cascade:"$datafile/$Scenario_num/Cascade" -result:"$datafile/$Scenario_num/Result" -maxRankFile:"$datafile/$Scenario_num/Result/maxRank" -fmeasureFile:"$datafile/$Scenario_num/Result/fmeasure"
	echo "Running DDNE - Loading Networks and Cascades and Computing Edges' Ranks done"

# Running DDNE - Step 2: Extracting Transition & Emission Probability Parameters 
	echo "Running DDNE - Step 2: Extracting Transition & Emission Probability Parameters"
	$codePath/extract_ProbabilityParams -n:$NNode -ttssn:$ttssn -etssn:$etssn -prior:$prior -b:$NBin -maxRank:$maxRank -network:"$datafile/$Scenario_num/Network" -result:"$datafile/$Scenario_num/Result" -cascade:"$datafile/$Scenario_num/Cascade"
	echo "Running DDNE - Extracting Transition & Emission Probability Parameters done"

# Running DDNE - Step 3: Extracting Network
	echo "Running DDNE - Step 3: Extracting Network"
	$codePath/DDNE  -ad:"$datafile/$Scenario_num" -ssn:$NSnapshot -b:$NBin -e:$MxEdge -maxRank:$maxRank -prior:0 -startIndex:2 -trsh:$trsh -network:"$datafile/$Scenario_num/Network" -result:"$datafile/$Scenario_num/Result" -cascade:"$datafile/$Scenario_num/Cascade" -fmeasureFile:"$datafile/$Scenario_num/Result/fmeasure" 
	echo "Running DDNE - Extracting Network"