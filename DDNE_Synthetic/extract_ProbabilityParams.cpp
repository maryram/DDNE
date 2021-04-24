#include "stdafx.h"

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Extract Probability Distribution Parameters. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try 
	const int NNode  = Env.GetIfArgPrefixInt("-n:", 512,"number of nodes (default:512)\n");
	const int TTSnapshot = Env.GetIfArgPrefixInt("-ttssn:", 1, "transition training snapShot Numbers (default:1)\n");
	const int ETSnapshot = Env.GetIfArgPrefixInt("-etssn:", 1, "emission training snapShot Numbers (default:1)\n");
	
	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/Scenario_num/Network", "Name of the data folder (default:example)\n"); 
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/Scenario_num/Result/***", "Result file name(s) prefix\n");
	/*---------------------------------------------------------------------------*/  
	cout << "DDNE_SyntheticData/extract_ProbabilityParams Running" << endl;	
	DNE dne;
		
	cout << "Extracting the parameter the transition probability : P(E(t)|E(t-1)) " << endl;
	dne.InitializeVariables(1, NNode); // 1 = reset transition array
	for(int i = 1; i <= TTSnapshot; i++) {
		cout << "SnapShot: " << i << endl;
		TFIn pastFIn(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i));		
		TFIn presentFIn(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i+1));
		dne.InitializeVariables(0, NNode); // 0 = only reset the information array of past and present network 
		dne.ExtractParameter_Transition(pastFIn, presentFIn, NNode, i);
	}
	dne.SaveParameter_Transition(TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), 1), NNode,ResultFNm.CStr());	

	cout << "Extracting the parameter of the emission probability : P(E(t)|R(t))" << endl;
	dne.CalculateParameter_Emission(ResultFNm.CStr(), NetFNm.CStr(), ETSnapshot);
	
	Catch
	cout << "DDNE_SyntheticData/extract_ProbabilityParams Finished" << endl;	
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
