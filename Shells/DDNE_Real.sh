#!/bin/bash

process_exe="../DataSetProcess"
codePath="../DDNE_Real"
dataPath="../Data/Real"

#---------------------  Parameter Setting: Scenario  ---------------------------#
for NetworkName in "LinkedIn" "Alqueda" "NBA" "World" "jihad" "earthquake" "unemployment" "steve" "baseball" "fukushima" 
do

	######################### Process Gephi Files #########################
#	echo "Processing Gephi Files"
#	echo "Step 1: Extract edges from gephi files"
#	./ProcessGephiFile -inputFile:"$dataPath/$NetworkName/NetworkName-gephi.gexf" -outputFile:"$dataPath/$NetworkName/$NetworkName-gephi-modified.txt" -operation:1
#	echo "Step 2: Extract edges per month"
#	./ProcessGephiFile -inputFile:"$dataPath/$NetworkName/$NetworkName-gephi-modified.txt" -outputFile:"$dataPath/$NetworkName/Network" -operation:2
#	echo "Processing Gephi Files done"	

	########################## Generate Cascades ##########################
	#echo "Generating Cascades"
	#$codePath/generate_Cascade -c:1000 -ssn:12 -network:"$dataPath/$NetworkName/Network" -referenceNetwork:"$dataPath/$NetworkName/Network/reference.txt" -mappingFile:"$dataPath/$NetworkName/mappingFile" -cascade:"$dataPath/$NetworkName/Cascade"
	#echo "Generating Cascades done"	
		
	#-----------------------  Parameter Setting: DDNE  -----------------------------
	NSnapshot=11
	NBin=490
	MxEdge=1
	maxRank=500
	ttssn=1 
	etssn=1
	prior=0
    trsh=0.001

	#######################################################################
	#                       Running DDNE 
	#######################################################################
	
	#-------------- Loading Networks and Cascades and Computing Edges' Ranks -------
		echo "Loading Networks and Cascades and Computing Edges' Ranks"
		$codePath/extract_Rank -etssn:$etssn -e:$MxEdge -realCas:0 -network:"$dataPath/$NetworkName/Network" -referenceNetwork:"$dataPath/$NetworkName/Network/reference.txt" -result:"$dataPath/$NetworkName/Result" -mappingFile:"$dataPath/$NetworkName/mappingFile" -cascade:"$dataPath/$NetworkName/Cascade"  -fmeasureFile:"$dataPath/$NetworkName/Result/fmeasure" -maxRankFile:"$dataPath/$NetworkName/Result/maxRank"
		echo "Loading Networks and Cascades and Computing Edges' Ranks done"
	
	#-------------- Extracting Transition & Emission Probability Parameters --------
		echo "Extracting Transition & Emission Probability Parameters"
		$codePath/extract_ProbabilityParams -ttssn:$ttssn -etssn:$etssn -b:$NBin -maxRank:$maxRank -network:"$dataPath/$NetworkName/Network" -referenceNetwork:"$dataPath/$NetworkName/Network/reference.txt" -result:"$dataPath/$NetworkName/Result" -mappingFile:"$dataPath/$NetworkName/mappingFile" -cascade:"$dataPath/$NetworkName/Cascade"
		echo "Extracting Transition & Emission Probability Parameters done"
	
	#-------------------------- Extracting Network ---------------------------------
		echo "Extracting Network"
		$codePath/DDNE  -ad:"$dataPath/$NetworkName" -ssn:$NSnapshot -b:$NBin -e:$MxEdge -maxRank:$maxRank -realCas:0 -prior:0 -trsh:$trsh -startIndex:2 -network:"$dataPath/$NetworkName/Network" -referenceNetwork:"$dataPath/$NetworkName/Network/reference.txt" -result:"$dataPath/$NetworkName/Result" -mappingFile:"$dataPath/$NetworkName/mappingFile" -cascade:"$dataPath/$NetworkName/Cascade" -fmeasureFile:"$dataPath/$NetworkName/Result/fmeasure"
		echo "Extracting Network done"
	done


