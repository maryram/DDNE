#include "stdafx.h"
#include "splitData.h"

int main(int argc, char* argv[]) {
typedef map<int, int, less<int> > IDMAP;
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Split synthetic network and cascades into snapshots. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
	const TStr InCascadeFNm  = Env.GetIfArgPrefixStr("-ic:", "./example-cascades.txt", "Input cascades\n");
	const TStr InNetworkFNm = Env.GetIfArgPrefixStr("-in:", "./Alqueda/Alqueda_Network.txt", "Input ground-truth network\n");
	const TStr OutNetworkFNm  = Env.GetIfArgPrefixStr("-on:", "/home/netgroup/Desktop/MaryamTahani/InfoPath", "Output network\n");
	const TStr OutCascadeFNm  = Env.GetIfArgPrefixStr("-oc:", "/home/netgroup/Desktop/MaryamTahani/InfoPath", "Output cascade\n");

	const int NNodes  = Env.GetIfArgPrefixInt("-n:", 1024, "Number of nodes (default : 512)\n");
	const double MinAlpha = Env.GetIfArgPrefixFlt("-minAlpha:", 0.05, "if rate < minAlpha the edge does not exist (default:0.05)\n");
	const int NSnapshot = Env.GetIfArgPrefixInt("-ssn:", 10, "Number of snapshots (default:10)\n");
    const double TotalTime = Env.GetIfArgPrefixFlt("-tt:", 100.0, "Total time (default:100)\n");
	/*---------------------------------------------------------------------------*/
	printf("Infopath/split_Data_Synthetic.cpp - Running\n"); 
	
	printf("Loading Network\n"); 
	TFIn FIn(InNetworkFNm);
	TStr Line; 	TStrV NIdV;
	// skip nodes
	FIn.GetNextLn(Line);
	while (!FIn.Eof() && Line != "") { 
		FIn.GetNextLn(Line);
	}
	DynamicEdgeInfo** edgesID = new DynamicEdgeInfo*[NNodes];
	for(int i = 0; i < NNodes; i++) {
		edgesID[i] = new DynamicEdgeInfo[NNodes];
	}
	for(int i = 0 ; i < NNodes; i++) {
		for(int j = 0 ; j < NNodes; j++) {
			edgesID[i][j].init();	
		}
	}
	while (!FIn.Eof()) {
		FIn.GetNextLn(Line);
		TStrV NIdV;
		Line.SplitOnAllCh(',', NIdV);
		int i = NIdV[0].GetInt();
		int j = NIdV[1].GetInt();
		for (int k = 2; k < NIdV.Len()-1; k+=2) {
			edgesID[i][j].setValue(NIdV[k+1].GetFlt(),(int)NIdV[k].GetFlt());
			if(i == j) // Do not allow self loops
				edgesID[i][j].setValue(0.0,(int)NIdV[k].GetFlt());
		}
  	}	
	
	printf("Saving Network per Snapshot\n"); 
	int snapShot = TotalTime / NSnapshot;
	for (int t = 0; t < NSnapshot + 1; t++) {
		TFOut FOut(TStr::Fmt("%s/%d-network.txt",OutNetworkFNm.CStr(),t));
		// write the nodes 	
		for(int i = 0; i < NNodes; i++) {
			FOut.PutStr(TStr::Fmt("%d,%d\r\n", i, i));
		}
		FOut.PutStr("\r\n");
		// write the edges 	
		for(int i = 0; i < NNodes; i++){
			for(int j = 0; j < NNodes; j++){
				if (edgesID[i][j].getValue(t*snapShot) > MinAlpha) {
					FOut.PutStr(TStr::Fmt("%d,%d\r\n", i, j));
				}
			}	
		}
	}
	
	printf("Loading Cascade and Splitting them into Snapshots\n"); 	
	TFIn FIn2(InCascadeFNm);
	TStr Line2;
	// skip nodes
	FIn2.GetNextLn(Line2);
	while (!FIn2.Eof() && Line2 != "") {
		FIn2.GetNextLn(Line2);
	}
	TStr cascadeId;
	double casStartTime;
	TStr casStartTimeTStr;
	int casStartTime2;
	TStr userid;
	double* cascadeLen = new double[NSnapshot];
	double* cascadeCount = new double[NSnapshot];
	TStr* FRecords= new TStr [NSnapshot];

	for(int i = 0; i < NSnapshot; i++) {
		cascadeLen[i] = 0.00;
		cascadeCount[i]=0.00;
	}

	vector<FILE*> FOut;
	for(int i = 0; i < NSnapshot; i++) {
	stringstream sstm;
	char * first_name = "-cascades.txt";
	sstm <<OutCascadeFNm.CStr()<<"/"<< i+1 << first_name;
	string ans = sstm.str();
	char * ans2 = (char *) ans.c_str();
	FOut.push_back(fopen((ans2), "w+b"));
	}
	
	for ( int index=0;index< NSnapshot;index++){
	for(int i = 0; i < NNodes; i++) {
		fprintf(FOut[index],"%d,%d\r\n", i, i);
		}
	fprintf(FOut[index],"\n");
	}


	  
	while (!FIn2.Eof()) {
		FIn2.GetNextLn(Line2);
		TStrV NIdV, NIdV2;
		Line2.SplitOnAllCh(',', NIdV);
		if(NIdV.Len() > 1){
			NIdV[0].SplitOnAllCh(';', NIdV2);
			cascadeId = NIdV2[0].GetStr();			
			for(int i = 0; i< NSnapshot; i++) {
				FRecords[i] = cascadeId+";"; 
			}
			for(int m = 0; m < NIdV.Len()-1; m = m+2) {
				if(m == 0)
					userid = NIdV2[1].GetStr();
				else
				userid = NIdV[m].GetStr();
				casStartTimeTStr = NIdV[m+1].GetStr();
				casStartTime = NIdV[m+1].GetFlt();
				casStartTime2 = (int)casStartTime /snapShot;
				if((FRecords[casStartTime2][FRecords[casStartTime2].Len()-1]) == ';') {
					FRecords[casStartTime2]+=userid+","+casStartTimeTStr;
				}else
					FRecords[casStartTime2]+= "," + userid + "," + casStartTimeTStr;
			}
			for ( int index=0;index< NSnapshot;index++){
				if(FRecords[index].CountCh(',') != 0) {
				cascadeCount[index]++;
				cascadeLen[index]+=FRecords[index].Len()-1;
				fprintf(FOut[index],"%s\r\n", FRecords[index].CStr());
				FRecords[index].Empty();
			}
			}
		}
	}
	for(int i = 0; i< NSnapshot; i++) {
		double avg=(cascadeLen[i]/cascadeCount[i]);
		 printf("\n\r time step: %d , CascadeCount: %f , CascadeLength: %f, AvgCascadeLength: %f", i*snapShot, cascadeCount[i],cascadeLen[i],avg);
	} 

	for ( int index=0;index< NSnapshot;index++){
		fclose(FOut[index]);
	}
	 Catch
	 printf("Infopath/split_Data_Synthetic.cpp - Finished\n"); 
	 return 0;
}
