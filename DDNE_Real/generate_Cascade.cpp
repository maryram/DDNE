#include "stdafx.h"

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Generate cascades. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
		const int TModel = Env.GetIfArgPrefixInt("-m:", 0, "Transmission model\n0:exponential, 1:power law, 2:mixture of 2 exponentials (default:0)\n");
	/// We allow for mixture of exponentials
	const double alpha = Env.GetIfArgPrefixFlt("-a:", 1.0, "Alpha for transmission model (default:1) \n");
	const double alpha2 = Env.GetIfArgPrefixFlt("-a2:", 1.0, "Second alpha for mixture of exponentials/power laws transmission model (default:1) \n");
	const double p = Env.GetIfArgPrefixFlt("-p:", 1.0, "Mixture of exponential/power laws transmission model weight (1.0 means max weight in 1st exp) (default:1) \n");
	const double beta = Env.GetIfArgPrefixFlt("-b:", 0.5, "Beta for transmission model (default:0.5) \n");
	const double std_beta = Env.GetIfArgPrefixFlt("-stdb:", 0.0, "Std for Beta in the transmission model (default:0) \n");
	const double std_waiting_time = Env.GetIfArgPrefixFlt("-stdwt:", 0.0, "Std for waiting time in the transmission model (default:0) \n");
	const double perc_rnd_nodes = Env.GetIfArgPrefixFlt("-prnd:", 0.0, "Percentage of random nodes in a cascade (default:0) \n");
	const double perc_rnd_removal = Env.GetIfArgPrefixFlt("-prmv:", 0.0, "Percentage of missing nodes in a cascade (default:0) \n");

	const int NCascades = Env.GetIfArgPrefixInt("-c:", 1000, "If positive, number of cascades, if negative, percentage of edges used at least in one cascade (default:-95) \n");
	const int NSnapshot = Env.GetIfArgPrefixInt("-ssn:", 12, "snapShot Numbers (default:100) \n");

	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/NetworkName/Network", "Network file \n");
	const TStr ReferenceNetFNm = Env.GetIfArgPrefixStr("-referenceNetwork:", "./Data/NetworkName/Network/reference.txt", "Reference nodes file \n");
	const TStr CascadeFNm  = Env.GetIfArgPrefixStr("-cascade:", "./Data/NetworkName/Cascade", "Cascade file \n");
	const TStr mappingFile  = Env.GetIfArgPrefixStr("-mappingFile:", "./Data/NetworkName/Result/mappingFile", "Mapping file \n");
	/*---------------------------------------------------------------------------*/
	printf("DDNE_RealData/generate_Cascade Running \n"); 
	DNE dne; 
	TFIn FInG(TStr::Fmt("%s", ReferenceNetFNm.CStr())); 
	dne.LoadReferenceTxt(FInG, mappingFile);

	for (int i = 1; i < NSnapshot; i++) {
		cout << "Snapshot Number: " << i << endl;	

		cout << "Load GroundTruth Network";
		TFIn FInG(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i));
		dne.LoadGroundTruthTxt(FInG); 

		cout << "Generate and Save Cascades" << endl;
		TIntPrIntH EdgesUsed;
		int last = 0;
		int cascade_Num = 0;
		for (int j=0; ((j < NCascades)); j++) {
			TCascade C(alpha);
			dne.GenNoisyCascade(C, TModel, alpha, beta, EdgesUsed, alpha2, p, std_waiting_time, std_beta, perc_rnd_nodes, perc_rnd_removal);
			dne.AddCasc(C);
			cascade_Num = j; 
		}
		printf(" ----- %d Cascades Generated \n", cascade_Num);
		dne.SaveCascades(TStr::Fmt("%s/month_%d.txt",CascadeFNm.CStr(), i));
	}
	Catch
		printf("DDNE_RealData/generate_Cascade Finished \n"); 
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}