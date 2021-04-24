#include "stdafx.h"
using namespace std;

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Extract probability distribution parameters: P(E(t)|E(t-1)) and P(E(t)|(R(t))). build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try 
		const int TTSnapshot = Env.GetIfArgPrefixInt("-ttssn:", 1, "transition training snapShot Numbers (default:1) \n");
	const int ETSnapshot = Env.GetIfArgPrefixInt("-etssn:", 1, "emission training snapShot Numbers (default:1) \n");

	const TStr NetFNm = Env.GetIfArgPrefixStr("-network:", "./Data/NetworkName/Network", "Network file \n"); 
	const TStr ResultFNm  = Env.GetIfArgPrefixStr("-result:", "./Data/NetworkName/Result", "Result file \n");
	const TStr ReferenceNetFNm = Env.GetIfArgPrefixStr("-referenceNetwork:", "./Data/NetworkName/Network/reference.txt", "Reference nodes file \n");
	const TStr mappingFile  = Env.GetIfArgPrefixStr("-mappingFile:", "./Data/NetworkName/mappingFile", "Mapping file \n");
	/*---------------------------------------------------------------------------*/
	cout << "DDNE_RealData/extract_ProbabilityParams Running" << endl;	
	DNE dne;
	int NNode;

	cout << "Loading Reference File" << endl;
	TFIn FInG(TStr::Fmt("%s", ReferenceNetFNm.CStr())); 
	NNode = dne.LoadReferenceTxt(FInG, mappingFile);

	cout << "Extracting the parameter the transition probability : P(E(t)|E(t-1)) " << endl;
	dne.InitializeVariables(1, NNode); // 1: Reset whole transition array
	for(int i = 1; i <= TTSnapshot; i++){ // Month_0 does not exist
		TFIn pastFIn(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i));		
		TFIn presentFIn(TStr::Fmt("%s/%d-network.txt", NetFNm.CStr(), i+1));
		dne.InitializeVariables(0, NNode); // 0: Reset past and present edge information  
		dne.ExtractParameter_Transition(pastFIn, presentFIn, NNode, i);
	}
	dne.SaveParameter_Transition(TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), 1), NNode);
	cout << "Extracting the parameter of the emission probability : P(E(t)|R(t))" << endl;
	dne.CalculateParameter_Emission(ResultFNm.CStr(), NetFNm.CStr(), ETSnapshot);

	Catch
		cout << "DDNE_RealData/extract_ProbabilityParams Finished" << endl;	
	printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}
