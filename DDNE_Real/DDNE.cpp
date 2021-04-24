#include "stdafx.h"
using namespace std;

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("Dynamic DNE. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try  
		const int NSnapshot = Env.GetIfArgPrefixInt("-ssn:", 11, "Number of snapshot \n");
	const int MxEdges  = Env.GetIfArgPrefixInt("-e:", 1, "Ratio: diffusion edges/network edges (default:1) \n");
	const int prior = Env.GetIfArgPrefixInt("-prior:", 0, "0: Only network 1 is known **** 1: Network 1 - (NSnapshot -1) are known, ---  (default:0) \n");
	const int realCas = Env.GetIfArgPrefixInt("-realCas:", 0, "synthetic cascade: 0 real cascade: 1 (default:0) \n");
	const int startIndex = Env.GetIfArgPrefixInt("-startIndex:", 2, "(default:2) \n");
	const double Threshold= Env.GetIfArgPrefixFlt("-trsh:", 0.4, "(default:0.4) \n");

	const TStr CasFNm = Env.GetIfArgPrefixStr("-cascade:", "./Data/NetworkName/Cascade", "Cascade file \n"); 	
	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/NetworkName/Network", "Network file \n"); 
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/NetworkName/Result", "Result file \n");
	const TStr ReferenceNetFNm = Env.GetIfArgPrefixStr("-referenceNetwork:", "./Data/NetworkName/Network/reference.txt", "Reference nodes file \n");
	const TStr resultFile  = Env.GetIfArgPrefixStr("-fmeasureFile:", "./Data/NetworkName/Result/fmeasure", "Fmeasure file \n");	
	const TStr mappingFile  = Env.GetIfArgPrefixStr("-mappingFile:", "./Data/NetworkName/mappingFile", "Mapping file \n");
	/*---------------------------------------------------------------------------*/
	cout << "DDNE_RealData/DDNE Running" << endl;
	DNE dne;
	int NEdge, extractedEdges = 0;

	dne.fmeasureFile.open(resultFile.CStr(), ios::app);

	cout << "DDNE - Loading Reference network" << endl;	
	TFIn FInGR(TStr::Fmt("%s", ReferenceNetFNm.CStr())); 
	dne.LoadReferenceTxt(FInGR, mappingFile);

	for(int i = startIndex; i <= NSnapshot; i++){
		cout << "Snapshot: " << i << endl;

		cout << "DDNE - Loading cascade" << endl;	
		TFIn RefNetFIn(TStr::Fmt("%s", ReferenceNetFNm.CStr())); 
		TFIn CascadeFIn(TStr::Fmt("%s/month_%d.txt", CasFNm.CStr(), i));
		dne.LoadCascadesTxt(RefNetFIn, CascadeFIn, realCas); 

		cout << "DDNE - Loading Ground Truth Network" << endl;
		TFIn FInG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i));
		NEdge = dne.LoadGroundTruthTxt(FInG);
		extractedEdges = NEdge * MxEdges;  

		if (prior == 0) { 
			TFIn FInLG(TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(), i-1)); // Load last inferred network

			dne.LoadLastGroundTruthTxt(FInLG);
		}else if(prior == 1) { 
			TFIn FInLG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i-1)); // Load last ground truth network

			dne.LoadLastGroundTruthTxt(FInLG);
		}
		dne.runDNE(extractedEdges,TStr::Fmt(""),0);

		cout << "DDNE -  Extract network and compare inferred network with ground truth. Save inferred network." << endl;
		dne.compare(Threshold, TStr::Fmt("%s", ResultFNm.CStr()), TStr::Fmt("%s/eprob.txt", ResultFNm.CStr()), TStr::Fmt("%s/%d-inferredNetwork.txt", NetFNm.CStr(),i), extractedEdges, i, prior);//, address.CStr());

	}
	Catch
		cout << "DDNE_Real/DDNE Finished" << endl;	
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
