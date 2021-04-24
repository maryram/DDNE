#ifndef snap_dne_h
#define snap_dne_h
#include <map>
#include "Snap.h"

// DNE algorithm class
class DNE {
public:

	typedef map<int, int, less<int> > IDMAP;
	IDMAP mp;
	TVec<TCascade> CascV;
	THash<TInt, TNodeInfo> NodeNmH;
	THash<TIntPr, TEdgeInfo> EdgeInfoH;

	THash<TIntPr, TIntV> CascPerEdge; // To implement localized update
	PNGraph Graph, GroundTruth, LastGroundTruth, ReferenceNetwork, inferredNetwork;
	TFltPrV PrecisionRecall;
	bool BoundOn, CompareGroundTruth;

	double** numCascades;
	double** ranks;	
	int** linkRanks;
	double** emissionProbability;
	double** emission;

	int nodes;	
	int edges;

	int realData;

	int** past;	
	int** present;
	double tr_01;
	double tr_11;	
	double** transition_00;	
	double** transition_01;
	double** transition_10;	
	double** transition_11;		
	int transition_00_num;
	int transition_01_num;
	int transition_10_num;
	int transition_11_num;

	const int sFactor = 99999;
	const int NBin = 300;

	int potentialEdges;
	//**************Files***********
	ofstream fmeasureFile;

public:
	DNE() { BoundOn = false; }
	DNE(bool bo, bool cgt) { BoundOn=bo; CompareGroundTruth=cgt; }

	DNE(TSIn& SIn) : CascV(SIn), NodeNmH(SIn) { }
	void Save(TSOut& SOut) const { CascV.Save(SOut); NodeNmH.Save(SOut); }

	void LoadCascadesTxt(TSIn& SInNet, TSIn& SInCas, int realCas);
	int LoadGroundTruthTxt(TSIn& SIn);
	void LoadLastGroundTruthTxt(TSIn& SIn);

	void AddGroundTruth(PNGraph& gt) { GroundTruth = gt; }


	void AddCasc(const TStr& CascStr, const double& alpha, int realData);
	void AddCasc(const TCascade& Cascade) { CascV.Add(Cascade); }
	void GenCascade(TCascade& Cascade, const int& TModel, const double& alpha, const double &beta, TIntPrIntH& EdgesUsed, const double &alpha2=0.0, const double& p=1.0, const double& std_waiting_time=0, const double& std_beta=0);
	void GenNoisyCascade(TCascade& Cascade, const int& TModel, const double& alpha, const double& beta, TIntPrIntH& EdgesUsed,
		const double& alpha2=0.0, const double& p=1.0, const double& std_waiting_time=0, const double& std_beta=0, const double& PercRndNodes=0,
		const double& PercRndRemoval=0);
	TCascade & GetCasc(int c) { return CascV[c]; }
	int GetCascs() { return CascV.Len(); }

	int GetNodes() { return Graph->GetNodes(); }
	void AddNodeNm(const int& NId, const TNodeInfo& Info) { NodeNmH.AddDat(NId, Info); }
	TStr GetNodeNm(const int& NId) const { return NodeNmH.GetDat(NId).Name; }
	TNodeInfo GetNodeInfo(const int& NId) const { return NodeNmH.GetDat(NId); }
	bool IsNodeNm(const int& NId) const { return NodeNmH.IsKey(NId); }

	void Init();
	double GetAllCascProb(const int& EdgeN1, const int& EdgeN2);
	void SaveHistory(const TStr& OutFNm);
	void SavePajek(const TStr& OutFNm);
	void SavePlaneTextNet(const TStr& OutFNm);
	void SaveEdgeInfo(const TStr& OutFNm);
	void SaveObjInfo(const TStr& OutFNm);

	void SaveGroundTruth(const TStr& OutFNm);
	void SaveCascades(const TStr& OutFNm);

	void initialization();
	void InitializeVariables(const bool mode, const int nodes);

	int  LoadReferenceTxt(TSIn& SIn, const TStr& mappingFNm);
	int  checkReferenceTxt(int node1, int node2);
	int findKey(const int i);


	void ExtractParameter_Transition(TSIn& pastFile, TSIn& presentFile, const int nodes, int snapShot);
	void SaveParameter_Transition(const TStr& OutFNm, const int nodes);
	void CalculateParameter_Emission(const TStr& rankFNm, const TStr& networkFNm, const int NSnapshot);
	void SaveRank(const TStr& OutFNm);

	void UpdateEmissionProbability(const TStr& rankFNm, const TStr& networkFNm, const int snapShot);
	void UpdateTransitionProbability(const TStr& t_probFNm, const TStr& t_newProbFNm, int i);
	void loadEmissionProbability(const TStr& e_probFNm);
	void loadTransitionProbabilty(const TStr& t_probFNm);

	void compare(double Threshold, const TStr& ResultFNm, const TStr& ResultFNm2, const TStr& InferredFNm,  const int MxEdges, const int snapShot, int Unknown);
	void runDNE(const int& MxEdges,const TStr& InferredFNm,int snapshot);
};

#endif
