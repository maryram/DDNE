#include "stdafx.h"
/* Computing the rank of edges in training snapshots */

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Load synthetic networks & cascades and compute the rank of edges in trainig snapshots. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
	// Parameters
	const int MxEdges  = Env.GetIfArgPrefixInt("-e:", 1, "Ratio: diffusion edges/network edges (default:1)\n");
	const int NTSnapshot = Env.GetIfArgPrefixInt("-ttssn:", 1, "Number of training snapShot (default:1)\n");
	
	// File Names
	const TStr CasFNm = Env.GetIfArgPrefixStr("-cascade:", "./Data/Scenario_num/Cascade", "Cascade file\n");
	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/Scenario_num/Network", "Network file\n");
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/Scenario_num/Result", "Result file");
	/*---------------------------------------------------------------------------*/  
	cout << "DDNE_Synthetic/generate-data Running" << endl;	
	DNE dne;
	int NEdge, extractedEdges = 0;

	for (int k = 1; k <= NTSnapshot; k++){ // The last snapShot is going to be predicted
		cout << "Training Snapshot Number: " << k << endl;	

		cout << "Loading GroundTruth Network " << endl;
		TFIn NetworkFIn(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), k));
		NEdge = dne.LoadGroundTruthTxt(NetworkFIn);  

		cout << "Loading Cascade" << endl;
		TFIn CascadeFIn(TStr::Fmt("%s/%d-cascades.txt", CasFNm.CStr(), k)); 
		dne.LoadCascadesTxt(CascadeFIn); 

		cout << "Computing edges' rank" << endl;	
		extractedEdges = MxEdges * (NEdge);

		dne.runDNE(extractedEdges,TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(), k),k);
		
		cout << "Saving ranks" << endl;
		dne.SaveRank(TStr::Fmt("%s/%d-ranks.txt", ResultFNm.CStr(), k));
	}
	Catch
	cout << "DDNE_Synthetic/extract_Rank Finished" << endl;	
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
