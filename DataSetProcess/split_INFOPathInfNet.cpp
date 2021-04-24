#include "stdafx.h"
#include "splitData.h"

int main(int argc, char* argv[]) {
	typedef map<int, int, less<int> > IDMAP;
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Split INFOPATH inferred network into snapshots. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
	const TStr InfopathNetworkFNm = Env.GetIfArgPrefixStr("-Info:", "", "INFOPATH Network\n");
	
	const double MinAlpha = Env.GetIfArgPrefixFlt("-minalpha:", 0.05, "if rate < minAlpha then no edge (default:0.05)\n");
	const int NSnapshot = Env.GetIfArgPrefixInt("-ssn:", 10, "Number of snapshot (default:10)\n");
	/*---------------------------------------------------------------------------*/
	printf("Infopath/split_INFOPATHInfNet.cpp - Running\n"); 
	
	printf("Load Infopath's inferred network \n"); 
	cout<<"man:\t"<<InfopathNetworkFNm.CStr()<<endl;
	
	TFIn FIn(TStr::Fmt("%s/net.txt", InfopathNetworkFNm.CStr()));
	TStr Line; TStrV NIdV;
	int NNodes = 0;
	maps ma;	
	// Add Nodes
	FIn.GetNextLn(Line);
	TFOut FOut(TStr::Fmt("%s/Nodes.txt", InfopathNetworkFNm.CStr()));  
	while (!FIn.Eof() && Line != ""){ 
		Line.SplitOnAllCh(',', NIdV);
		ma.mp.insert(IDMAP::value_type(NIdV[0].GetInt(),NNodes));
		NNodes ++;
		FIn.GetNextLn(Line);
		FOut.PutStr(TStr::Fmt("%d\n", NIdV[0].GetInt()));
	}
	// Add Edges
	DynamicEdgeInfo** edgesID = new DynamicEdgeInfo*[NNodes];
	for(int i = 0; i < NNodes; i++){
		edgesID[i] = new DynamicEdgeInfo[NNodes];
	}
	for(int i = 0 ; i < NNodes; i++) {
		for(int j = 0 ; j < NNodes; j++) {
			edgesID[i][j].init();	
		}
	}	
	IDMAP::const_iterator it;
	int ii, jj, i=0, j = 0;
	while (!FIn.Eof()) {
		FIn.GetNextLn(Line);
		TStrV NIdV;
		Line.SplitOnAllCh(',', NIdV);
		i = NIdV[0].GetInt();
		j = NIdV[1].GetInt();
		ii = ma.mp.find(i)->second;
		jj = ma.mp.find(j)->second;
		for (int k=2; k<NIdV.Len()-1; k+=2) {
			edgesID[ii][jj].setValue(NIdV[k+1].GetFlt(),(int)NIdV[k].GetFlt());
			if(i == j)
				edgesID[ii][jj].setValue(0.0,(int)NIdV[k].GetFlt());
		}
  	}
	
	printf("Map Infopath's inferred network into snapshots\n"); 
	for (int t = 1; t <= NSnapshot; t++) {
		TFOut FOut(TStr::Fmt("%s/%d-network.txt", InfopathNetworkFNm.CStr(),t));  
		for(int i = 0 ; i < NNodes; i++) {
			for(int j = 0 ; j < NNodes; j++) {
				if(edgesID[i][j].getValue(t*100) > MinAlpha) {
					for (it = ma.mp.begin(); it != ma.mp.end(); ++it) {
						if (it->second == i)
							ii = it->first;
						if (it->second == j)
							jj = it->first;
					}
					FOut.PutStr(TStr::Fmt("%d,%d,%f\r\n", ii, jj, edgesID[i][j].getValue(t*100)));
				}
			}
		}
	}
	 Catch
	 printf("Infopath/split_INFOPATHInfNet.cpp - Finished\n"); 
	 return 0;
}