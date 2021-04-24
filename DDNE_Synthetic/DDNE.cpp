#include "stdafx.h"

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("Dynamic DNE. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try  
	const int NSnapshot = Env.GetIfArgPrefixInt("-ssn:", 11, "Number of snapshot \n");
  	const int MxEdges  = Env.GetIfArgPrefixInt("-e:", 1, "Ratio: diffusion edges/network edges (default:1)\n");
	const int prior = Env.GetIfArgPrefixInt("-prior:", 0, "0: Only network 1 is known **** 1: Network 1 - (NSnapshot -1) are known, ---  (default:0)\n");
	const int startIndex = Env.GetIfArgPrefixInt("-startIndex:", 2, "(default:2)\n");
	const double Threshold= Env.GetIfArgPrefixFlt("-trsh:", 0.4, "(default:0.4)\n");

	const TStr CasFNm = Env.GetIfArgPrefixStr("-cascade:", "./Data/Scenario_num/Cascade", "Name of the data folder (default:example)\n"); 	
	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/Scenario_num/Network", "Name of the data folder (default:example)\n"); 
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/Scenario_num/Result", "Result file name(s) prefix\n");
	const TStr resultFile  = Env.GetIfArgPrefixStr("-fmeasureFile:", "./Data/Scenario_num/Result/fmeasure", "max weight file name(s) prefix\n");
	
	/*---------------------------------------------------------------------------*/
	cout << "DDNE_Synthetic/DDNE Running" << endl;
	DNE dne;
	int NEdge, NNode, extractedEdges = 0;
	
	dne.fmeasureFile.open(resultFile.CStr(), ios::app);
	
	for (int i = startIndex; i <= NSnapshot; i++) {
		cout << "Snapshot: " << i << endl;
		
		cout << "DDNE - Loading cascade" << endl;	
		TFIn CascadeFIn(TStr::Fmt("%s/%d-cascades.txt", CasFNm.CStr(), i));
		dne.LoadCascadesTxt(CascadeFIn); 		

		cout << "DDNE - Loading ground truth network" << endl;
		TFIn FInG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i));

		NEdge = dne.LoadGroundTruthTxt(FInG);
		extractedEdges = NEdge * MxEdges;  

		if (prior == 0){ 
			TFIn FInLG(TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(), i-1)); // Load last inferred network
			dne.LoadLastGroundTruthTxt(FInLG);
		}else if (prior == 1){ 
			TFIn FInLG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i-1)); // Load last ground truth network
			dne.LoadLastGroundTruthTxt(FInLG);
		}
		
		dne.runDNE(extractedEdges,TStr::Fmt(""),0);
		
		cout << "DDNE - Extract network and compare inferred network with ground truth. Save inferred network" << endl;
		dne.compare(Threshold,TStr::Fmt("%s", ResultFNm.CStr()), TStr::Fmt("%s/eprob.txt", ResultFNm.CStr()), TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(),i), extractedEdges, i, prior);//, address.CStr());
	}
	Catch
	cout << "DDNE_Synthetic/DDNE Finished" << endl;	
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
