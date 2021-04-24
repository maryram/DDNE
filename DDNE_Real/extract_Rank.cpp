#include "stdafx.h"
using namespace std;

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Load networks & cascades and compute the rank of edges in training snapshots. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
		// Parameters
		const int MxEdges  = Env.GetIfArgPrefixInt("-e:", 1, "Ratio: diffusion edges/network edges (default:1) \n");
	const int NTSnapshot = Env.GetIfArgPrefixInt("-etssn:", 1, "Number of training snapShot (default:1) \n");
	const int realCas = Env.GetIfArgPrefixInt("-realCas:", 0, "synthetic cascade: 0 real cascade: 1 (default:0) \n");

	// File Names
	const TStr CasFNm = Env.GetIfArgPrefixStr("-cascade:", "./Data/NetworkName/Cascade", "Cascade file \n");
	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/NetworkName/Network", "Network file \n");
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/NetworkName/Result", "Result file \n");
	const TStr ReferenceNetFNm = Env.GetIfArgPrefixStr("-referenceNetwork:", "./Data/NetworkName/Network/reference.txt", "Reference nodes file \n");
	const TStr mappingFile  = Env.GetIfArgPrefixStr("-mappingFile:", "./Data/NetworkName/mappingFile", "Mapping file \n");
	/*---------------------------------------------------------------------------*/  
	cout << "DDNE_RealData/extract_Rank Running" << endl;	
	DNE dne;
	int NEdge, extractedEdges = 0;	

	TFIn FInG(TStr::Fmt("%s", ReferenceNetFNm.CStr()));
	dne.LoadReferenceTxt(FInG, mappingFile);

	for (int k = 1; k <= NTSnapshot; k++) {
		cout << "Training Snapshot Number: " << k << endl;	


		cout << "Load GroundTruth Network" << endl;
		TFIn FInG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), k));
		NEdge = dne.LoadGroundTruthTxt(FInG);  

		cout << "Load Cascade" << endl;
		TFIn RefNetFIn(TStr::Fmt("%s", ReferenceNetFNm.CStr())); 
		TFIn CascadeFIn(TStr::Fmt("%s/month_%d.txt", CasFNm.CStr(), k)); 
		dne.LoadCascadesTxt(RefNetFIn, CascadeFIn, realCas); 
		dne.initialization();
		cout << "Run DNE - compute edges' rank" << endl;
		extractedEdges = MxEdges * (NEdge);
		dne.runDNE(extractedEdges,TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(), k),k);
		cout << "Saving ranks" << endl;
		dne.SaveRank(TStr::Fmt("%s/%d-ranks.txt", ResultFNm.CStr(), k));
	}
	Catch
		cout << "DDNE_RealData/extract_Rank Finished" << endl;	
	printf("\n run time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
