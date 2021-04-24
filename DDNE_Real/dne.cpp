#include "stdafx.h"
#include <string.h>
#include <math.h>


void DNE::runDNE(const int& MxEdges,const TStr& InferredFNm,int snapshot) {
	cout << "Running DNE..." << endl;
	double rankCoef1 = 10.0;
	double rankCoef2 = 10.0;
	potentialEdges = 0;
	int src, dst, srcIndex, dstIndex;

	initialization();

	// Computing edges' ranks
	for (int c = 0; c < CascV.Len(); c++) {		
		for (int i = CascV[c].Len() - 1; i >= 0 ; i--) {	
			for(int j = 0 ; j < i; j++) {
				src = CascV[c].GetNode(i);
				dst = CascV[c].GetNode(j);	
				if (checkReferenceTxt(src, dst)) { 	// Check weither the nodes both exist in network
					if (CascV[c].GetTm(src) < CascV[c].GetTm(dst)) {

						srcIndex = mp.find(src)->second;
						dstIndex = mp.find(dst)->second;
						double makhraj=((double)(CascV[c].Len() - 1 - j) * (i - j));
						ranks[srcIndex][dstIndex] += rankCoef1/makhraj;
						numCascades[srcIndex][dstIndex] ++;
						potentialEdges ++;
					}	
				}
				else
					cout << "DNE - RunDNE Node " << src << " or (and) " << dst << "doesn't (don't) exist" << endl;						

			}
		}
	}

	if (snapshot == 1) 
	{

		Edge *weights = new Edge[potentialEdges];
		int count = 0;
		for(int i = 0; i < nodes; i++){
			for(int j = 0; j < nodes; j++){
				if(ranks[i][j] > 0){
					weights[count].start = i;
					weights[count].end = j;
					weights[count].value =  pow(ranks[i][j],rankCoef2) * numCascades[i][j];;
					count++;
				}
			}
		}
		//***************************** Sort and Save **********************
		nth_element(weights, weights + count - MxEdges, weights + count);

		int ch=MxEdges;
		if(count<MxEdges) ch=count;


		// Extracting inferred network
		inferredNetwork = TNGraph::New();
		// Inferred Network - Add Nodes
		for (TNGraph::TNodeI NI = ReferenceNetwork->BegNI(); NI < ReferenceNetwork->EndNI(); NI++) {
			inferredNetwork->AddNode(NI.GetId());
		}

		FILE* inferredFOut=fopen(InferredFNm.CStr(), "w+b");


		for(int y=0;y<ch;y++)
		{
			int ii = findKey(weights[y].start);
			int jj = findKey(weights[y].end);

			if(!inferredNetwork->IsEdge(ii,jj,true)){

				inferredNetwork->AddEdge(ii,jj);
			}
		}

		// write edges to file (not allowing self loops in the network)
		for (TNGraph::TEdgeI EI = inferredNetwork->BegEI(); EI < inferredNetwork->EndEI(); EI++) {

			// not allowing self loops
			if (EI.GetSrcNId() != EI.GetDstNId())
				fprintf(inferredFOut,"%d,%d\r\n", EI.GetSrcNId(), EI.GetDstNId());
		}

		fclose(inferredFOut);
	}

	cout << "runDNE Finished \n";
}

void DNE::initialization(){
	Graph = TNGraph::New();
	PrecisionRecall.Clr();
	nodes = GroundTruth->GetNodes();
	edges = GroundTruth->GetEdges();
	cout<<nodes<<endl;
	numCascades = new double*[nodes];
	ranks = new double*[nodes];
	for (int i = 0; i < nodes; i++) {
		numCascades[i] = new double[nodes];
		ranks[i] = new double[nodes];
	}	
	for(int i = 0 ; i < nodes; i++) {
		for(int j = 0 ; j < nodes; j++) {
			ranks[i][j] = 0.0;
			numCascades[i][j] = 0;
		}
	}
}

void DNE::InitializeVariables(const bool mode, const int nodes){
	if(mode){
		past = new int*[nodes];
		present = new int*[nodes];	
		transition_00 = new double*[nodes];
		transition_01 = new double*[nodes];
		transition_10 = new double*[nodes];
		transition_11 = new double*[nodes];
		tr_01 = 0.00;	tr_11 = 0.00;
		transition_00_num = 0;
		transition_01_num = 0;
		transition_10_num = 0;
		transition_11_num = 0;

		for (int i = 0; i < nodes; i++) {
			past[i] = new int[nodes];
			present[i] = new int[nodes];
			transition_00[i] = new double[nodes];
			transition_01[i] = new double[nodes];
			transition_10[i] = new double[nodes];
			transition_11[i] = new double[nodes];
		}	
		for(int i = 0 ; i < nodes; i++) {
			for(int j = 0 ; j < nodes; j++) {
				past[i][j] = 0;
				present[i][j] = 0;
				transition_00[i][j] = 0.0;
				transition_01[i][j] = 0.0;
				transition_10[i][j] = 0.0;
				transition_11[i][j] = 0.0;
			}
		}
	}else if(!mode) {
		for(int i = 0 ; i < nodes; i++) {
			for(int j = 0 ; j < nodes; j++) {
				past[i][j] = 0;
				present[i][j] = 0;				
			}
		}
	}
}

int DNE::LoadReferenceTxt(TSIn& SIn, const TStr& mappingFNm) {
	TFOut FOut(mappingFNm);
	FOut.PutStr(TStr::Fmt("real id		index id\n")); // nodes		
	mp.clear();
	ReferenceNetwork = TNGraph::New();
	TStr Line;
	int count = 0;
	// add nodes
	while (!SIn.Eof()){
		SIn.GetNextLn(Line);
		ReferenceNetwork->AddNode(Line.GetInt());		
		mp.insert(IDMAP::value_type(Line.GetInt(),count));
		FOut.PutStr(TStr::Fmt("%d	%d	\n", Line.GetInt(), count));	
		count++;
	}
	printf("DNE - Loaded Reference Net: Nodes:%d\n", ReferenceNetwork->GetNodes());
	return ReferenceNetwork->GetNodes();
}

int DNE::checkReferenceTxt(int node1, int node2) {

	if (ReferenceNetwork->IsNode(node1) && ReferenceNetwork->IsNode(node2))
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}


void DNE::LoadCascadesTxt(TSIn& SInNet, TSIn& SInCas, int realCas) {
	CascV.Clr(); 	
	TStr Line;
	int cascadeNumber = 0;
	SInNet.GetNextLn(Line);
	while (!SInNet.Eof() && Line != "") {
		AddNodeNm(Line.GetInt(), TNodeInfo(Line, 0));	
		SInNet.GetNextLn(Line);
	}
	// Add the last line
	AddNodeNm(Line.GetInt(), TNodeInfo(Line, 0));	
	while (!SInCas.Eof()) {
		SInCas.GetNextLn(Line);
		AddCasc(Line,1.0 ,realCas);
		cascadeNumber++;
	}
	printf("DNE - LoadCascadesTxt -All cascades read - number of cascades:%d\n", cascadeNumber);
}

void DNE::LoadLastGroundTruthTxt(TSIn& SIn){
	LastGroundTruth = TNGraph::New();
	TStr Line;
	// add nodes
	for (TNGraph::TNodeI NI = ReferenceNetwork->BegNI(); NI < ReferenceNetwork->EndNI(); NI++) {
		LastGroundTruth->AddNode(NI.GetId());
	}
	SIn.GetNextLn(Line);
	// add edges
	while (!SIn.Eof()) {
		SIn.GetNextLn(Line);
		TStrV NIdV;
		Line.SplitOnAllCh(',', NIdV);
		LastGroundTruth->AddEdge(NIdV[0].GetInt(), NIdV[1].GetInt());
	}
	printf("Loaded LastGroundTruthNet: Nodes:%d Edges:%d\n", LastGroundTruth->GetNodes(), LastGroundTruth->GetEdges());
}

int DNE::LoadGroundTruthTxt(TSIn& SIn) {
	GroundTruth = TNGraph::New();
	TStr Line;
	// add nodes
	for (TNGraph::TNodeI NI = ReferenceNetwork->BegNI(); NI < ReferenceNetwork->EndNI(); NI++) {
		GroundTruth->AddNode(NI.GetId());
	}
	SIn.GetNextLn(Line);
	// add edges
	while (!SIn.Eof()) {
		TStrV NIdV;
		Line.SplitOnAllCh(',', NIdV);
		GroundTruth->AddEdge(NIdV[0].GetInt(), NIdV[1].GetInt());
		SIn.GetNextLn(Line);
	}
	printf("Loaded GroundTruthNet: Nodes:%d Edges:%d\n", GroundTruth->GetNodes(), GroundTruth->GetEdges());
	return GroundTruth->GetEdges();
}

void DNE::SaveGroundTruth(const TStr& OutFNm) {
	TFOut FOut(OutFNm);
	// write nodes to file
	for (TNGraph::TNodeI NI = GroundTruth->BegNI(); NI < GroundTruth->EndNI(); NI++) {
		FOut.PutStr(TStr::Fmt("%d,%d\r\n", NI.GetId(), NI.GetId())); // nodes
	}
	FOut.PutStr("\r\n");
	// write edges to file (not allowing self loops in the network)
	for (TNGraph::TEdgeI EI = GroundTruth->BegEI(); EI < GroundTruth->EndEI(); EI++) {
		if (EI.GetSrcNId() != EI.GetDstNId())
			FOut.PutStr(TStr::Fmt("%d,%d\r\n", EI.GetSrcNId(), EI.GetDstNId()));
	}
}

void DNE::SaveCascades(const TStr& OutFNm) {
	TFOut FOut(OutFNm);
	// write nodes to file
	for (TNGraph::TNodeI NI = GroundTruth->BegNI(); NI < GroundTruth->EndNI(); NI++) {
		FOut.PutStr(TStr::Fmt("%d,%d\r\n", NI.GetId(), NI.GetId())); // nodes
	}
	FOut.PutStr("\r\n");

	// write cascades to file
	for (int i=0; i<CascV.Len(); i++) {
		TCascade &C = CascV[i];
		int j = 0;
		for (THash<TInt, THitInfo>::TIter NI = C.NIdHitH.BegI(); NI < C.NIdHitH.EndI(); NI++, j++) {
			if (j > 0)
				FOut.PutStr(TStr::Fmt(";%d,%f", NI.GetDat().NId.Val, NI.GetDat().Tm.Val));
			else
				FOut.PutStr(TStr::Fmt("%d,%f", NI.GetDat().NId.Val, NI.GetDat().Tm.Val));
		}
		if (C.Len() >= 1)
			FOut.PutStr(TStr::Fmt("\r\n"));
	}
	CascV.Clr();
	printf("SaveCascades \n");
}


void DNE::AddCasc(const TStr& CascStr, const double& alpha, int realCas) {
	TCascade C;
	if(realCas == 0){
		TStrV NIdV; CascStr.SplitOnAllCh(';', NIdV);
		for (int i = 0; i < NIdV.Len(); i++) {
			TStr NId, Tm; NIdV[i].SplitOnCh(NId, ',', Tm);
			IAssert( IsNodeNm(NId.GetInt()) );
			GetNodeInfo(NId.GetInt()).Vol = GetNodeInfo(NId.GetInt()).Vol + 1;
			C.Add(NId.GetInt(), Tm.GetFlt());
		}
	}else if (realCas == 1){
		TStrV NIdV;CascStr.SplitOnAllCh(';', NIdV);
		TStrV NId;
		NIdV[1].SplitOnAllCh(',',NId); // NIdV[0] is the cascade ID
		for (int i = 0; i < NId.Len();i = i + 2) {
			if(IsNodeNm(NId[i].GetInt())) {
				IAssert( IsNodeNm(NId[i].GetInt()));
				GetNodeInfo(NId[i].GetInt()).Vol = GetNodeInfo(NId[i].GetInt()).Vol + 1;
				C.Add(NId[i].GetInt(), NId[i + 1].GetFlt());
			}		
		}
	}    
	C.Sort();
	CascV.Add(C);
}

void DNE::GenCascade(TCascade& C, const int& TModel, const double& alpha, const double &beta, TIntPrIntH& EdgesUsed, const double& alpha2, const double& p, const double& std_waiting_time, const double& std_beta) {
	TIntFltH InfectedNIdH; TIntH InfectedBy;
	double GlobalTime; int StartNId;

	if (GroundTruth->GetNodes() == 0)
		return;
	while (C.Len() < 2) {
		C.Clr();
		InfectedNIdH.Clr();
		InfectedBy.Clr();
		GlobalTime = 0;
		StartNId = GroundTruth->GetRndNId();
		InfectedNIdH.AddDat(StartNId) = GlobalTime;
		while (true) {
			// sort by time & get the oldest node that did not run infection
			InfectedNIdH.SortByDat(true);
			const int& NId = InfectedNIdH.BegI().GetKey();
			GlobalTime = InfectedNIdH.BegI().GetDat();

			// all the nodes has run infection
			if (GlobalTime == 1e12)
				break;

			// add current oldest node to the network and set its time
			C.Add(NId, GlobalTime);

			// run infection from the current oldest node
			const TNGraph::TNodeI NI = GroundTruth->GetNI(NId);
			for (int e = 0; e < NI.GetOutDeg(); e++) {
				// flip biased coin (set by beta)
				if (TInt::Rnd.GetUniDev() > beta+std_beta*TFlt::Rnd.GetNrmDev())
					continue;

				const int DstNId = NI.GetOutNId(e);

				// not infecting the parent
				if (InfectedBy.IsKey(NId) && InfectedBy.GetDat(NId).Val == DstNId)
					continue;

				double sigmaT;
				switch (TModel) {
				case 0:
					// exponential with alpha parameter
					sigmaT = (p*alpha + (1-p)*alpha2)*TInt::Rnd.GetExpDev();
					while (! (sigmaT >= 1 && sigmaT < 100)) { sigmaT = (p*alpha + (1-p)*alpha2)*TInt::Rnd.GetExpDev(); }
					break;
					// power-law
				case 1:
					// power-law with alpha parameter
					sigmaT = sigmaT = p*TInt::Rnd.GetPowerDev(alpha) + (1-p)*TInt::Rnd.GetPowerDev(alpha2);
					while (! (sigmaT >= 1 && sigmaT < 100)) { sigmaT = p*TInt::Rnd.GetPowerDev(alpha) + (1-p)*TInt::Rnd.GetPowerDev(alpha2); } // skip too large time difference (>100 days)
					break;
				default:
					sigmaT = 1;
					break;
				}

				if (std_waiting_time > 0) sigmaT = TFlt::GetMx(0.0, sigmaT + std_waiting_time*TFlt::Rnd.GetNrmDev());

				double t1 = GlobalTime + sigmaT;

				if (InfectedNIdH.IsKey(DstNId)) {
					double t2 = InfectedNIdH.GetDat(DstNId);
					if (t2 > t1 && t2 != 1e12) {
						InfectedNIdH.GetDat(DstNId) = t1;
						InfectedBy.GetDat(DstNId) = NId;
					}
				} else {
					InfectedNIdH.AddDat(DstNId) = t1;
					InfectedBy.AddDat(DstNId) = NId;
				}
			}

			// we cannot delete key (otherwise, we cannot sort), so we assign a very big time
			InfectedNIdH.GetDat(NId) = 1e12;
		}

	}
	C.Sort();
	for (TIntH::TIter EI = InfectedBy.BegI(); EI < InfectedBy.EndI(); EI++) {
		TIntPr Edge(EI.GetDat().Val, EI.GetKey().Val);

		if (!EdgesUsed.IsKey(Edge)) EdgesUsed.AddDat(Edge) = 0;

		EdgesUsed.GetDat(Edge) += 1;
	}
}

void DNE::GenNoisyCascade(TCascade& C, const int& TModel, const double& alpha, const double& beta, TIntPrIntH& EdgesUsed,
	const double& alpha2, const double& p, const double& std_waiting_time, const double& std_beta, const double& PercRndNodes,
	const double& PercRndRemoval) {
		TIntPrIntH EdgesUsedC; // list of used edges for a single cascade
		GenCascade(C, TModel, alpha, beta, EdgesUsedC, alpha2, p, std_waiting_time, std_beta);

		// store keys
		TIntV KeyV;
		C.NIdHitH.GetKeyV(KeyV);

		// store first and last time
		double tbeg = TFlt::Mx, tend = TFlt::Mn;
		for (int i=0; i < KeyV.Len(); i++) {
			if (tbeg > C.NIdHitH.GetDat(KeyV[i]).Tm) tbeg = C.NIdHitH.GetDat(KeyV[i]).Tm;
			if (tend < C.NIdHitH.GetDat(KeyV[i]).Tm) tend = C.NIdHitH.GetDat(KeyV[i]).Tm;
		}

		// remove PercRndRemoval% of the nodes of the cascades
		if (PercRndRemoval > 0) {
			for (int i=KeyV.Len()-1; i >= 0; i--) {
				if (TFlt::Rnd.GetUniDev() < PercRndRemoval) {
					// remove from the EdgesUsedC the ones affected by the removal
					TIntPrV EdgesToRemove;
					for (TIntPrIntH::TIter EI = EdgesUsedC.BegI(); EI < EdgesUsedC.EndI(); EI++) {
						if ( (KeyV[i]==EI.GetKey().Val1 && C.IsNode(EI.GetKey().Val2) && C.GetTm(KeyV[i]) < C.GetTm(EI.GetKey().Val2)) ||
							(KeyV[i]==EI.GetKey().Val2 && C.IsNode(EI.GetKey().Val1) && C.GetTm(KeyV[i]) > C.GetTm(EI.GetKey().Val1)) ) {
								EI.GetDat() = EI.GetDat()-1;

								if (EI.GetDat()==0)
									EdgesToRemove.Add(EI.GetKey());
						}
					}

					for (int er=0; er<EdgesToRemove.Len(); er++)
						EdgesUsedC.DelKey(EdgesToRemove[er]);

					C.Del(KeyV[i]);
				}
			}
			printf("defraging");
			// defrag the hash table, otherwise other functions can crash
			C.NIdHitH.Defrag();
		}

		// Substitute PercRndNodes% of the nodes for a random node at a random time
		if (PercRndNodes > 0) {
			for (int i=KeyV.Len()-1; i >= 0; i--) {
				if (TFlt::Rnd.GetUniDev() < PercRndNodes) {
					// remove from the EdgesUsedC the ones affected by the change
					TIntPrV EdgesToRemove;
					for (TIntPrIntH::TIter EI = EdgesUsedC.BegI(); EI < EdgesUsedC.EndI(); EI++) {
						if ( (KeyV[i]==EI.GetKey().Val1 && C.IsNode(EI.GetKey().Val2) && C.GetTm(KeyV[i]) < C.GetTm(EI.GetKey().Val2)) ||
							(KeyV[i]==EI.GetKey().Val2 && C.IsNode(EI.GetKey().Val1) && C.GetTm(KeyV[i]) > C.GetTm(EI.GetKey().Val1)) ) {
								EI.GetDat() = EI.GetDat()-1;

								if (EI.GetDat()==0)
									EdgesToRemove.Add(EI.GetKey());
						}
					}

					for (int er=0; er<EdgesToRemove.Len(); er++)
						EdgesUsedC.DelKey(EdgesToRemove[er]);

					printf("Old node n:%d t:%f --", KeyV[i].Val, C.GetTm(KeyV[i]));
					C.Del(KeyV[i]);

					// not repeating a label
					double tnew = 0;
					int keynew = -1;
					do {
						tnew = tbeg + TFlt::Rnd.GetUniDev()*(tend-tbeg);
						keynew = Graph->GetRndNId();
					} while (KeyV.IsIn(keynew));

					printf("New node n:%d t:%f\n", keynew, tnew);

					C.Add(keynew, tnew);
					KeyV.Add(keynew);
				}
			}
		}

		// add to the aggregate list (EdgesUsed)
		EdgesUsedC.Defrag();

		for (int i=0; i<EdgesUsedC.Len(); i++) {
			if (!EdgesUsed.IsKey(EdgesUsedC.GetKey(i))) EdgesUsed.AddDat(EdgesUsedC.GetKey(i)) = 0;

			EdgesUsed.GetDat(EdgesUsedC.GetKey(i)) += 1;
		}
}

void DNE::ExtractParameter_Transition(TSIn& pastFile, TSIn& presentFile, const int nodes, int snapShot) {
	TStr pastLine, presentLine;
	int ii, jj, i_prime, j_prime;
	int check_past = 0;	int check_present = 0;
	pastFile.GetNextLn(pastLine);	
	while (!pastFile.Eof()) { // Load Past Network
		TStrV pastNIdV;
		pastLine.SplitOnAllCh(',', pastNIdV);
		ii = mp.find(pastNIdV[0].GetInt())->second; 
		jj = mp.find(pastNIdV[1].GetInt())->second;
		past[ii][jj] = 1;
		pastFile.GetNextLn(pastLine);	
	}
	presentFile.GetNextLn(presentLine);
	while(!presentFile.Eof()) { // Load Present Network
		TStrV presentNIdV;
		presentLine.SplitOnAllCh(',', presentNIdV);
		i_prime = mp.find(presentNIdV[0].GetInt())->second; 
		j_prime = mp.find(presentNIdV[1].GetInt())->second;
		present[i_prime][j_prime] = 1;
		presentFile.GetNextLn(presentLine);
	}		

	// Compute number of different transitions for each edge
	int pastValue, presentValue;
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			pastValue = past[i][j];
			presentValue = present[i][j];
			if((pastValue == 0) && (presentValue == 0)) {
				transition_00[i][j] = transition_00[i][j] + 1;			
			}
			else if((pastValue == 0) && (presentValue == 1)) {
				transition_01[i][j] = transition_01[i][j] + 1;			
			}
			else if((pastValue == 1) && (presentValue == 0)) {
				transition_10[i][j] = transition_10[i][j] + 1;			
			}
			else if((pastValue == 1) && (presentValue == 1)) {
				transition_11[i][j] = transition_11[i][j] + 1;			
			}		
		}
	}
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			if((past[i][j] == 0) && (present[i][j] == 1)) {
				transition_01[i][j] = transition_01[i][j] + sFactor;											
			}
		}
	}
	// Compute total number of different transitions
	for (int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			transition_00_num = transition_00_num + transition_00[i][j];
			transition_01_num = transition_01_num + transition_01[i][j];
			transition_10_num = transition_10_num + transition_10[i][j];
			transition_11_num = transition_11_num + transition_11[i][j];
		}
	}
}



void DNE::CalculateParameter_Emission(const TStr& rankFNm, const TStr& networkFNm, const int NSnapshot) { //, const int NBin, const int maxRank) {
	/* Emission stores the number of times each state occurs: E(t)=0 R(t)=0, E(t)=0 R(t)=1, ..., E(t)=1 R(t)= NBin */
	double** emission= new double*[2];
	for(int i = 0; i < 2; i++){
		emission[i] = new double[NBin];
		for(int j = 0; j < NBin; j++)
			emission[i][j] = 0.0;
	}
	cout<<"Nbin\t"<<NBin<<endl;

	// Learn on Training Data
	int num_one, num_zero;
	for(int ts = 1; ts <= NSnapshot; ts++) {
		cout << "Emission Probability - Training Snapshot :" << ts << endl;

		// Load network
		TFIn FInG(TStr::Fmt("%s/%d-network.txt", networkFNm.CStr(), ts));
		LoadGroundTruthTxt(FInG);

		// Load ranks
		TFIn FInG2(TStr::Fmt("%s/%d-ranks.txt", rankFNm.CStr(), ts));
		TStr Line; FInG2.GetNextLn(Line);
		TStrV NIdV; Line.SplitOnAllCh(' ', NIdV);
		int nodes = NIdV[0].GetInt();
		linkRanks = new int*[nodes];
		for(int i=0; i < nodes; i++)
			linkRanks[i] = new int[nodes];
		for(int i = 0; i < nodes; i++)
			for(int j = 0; j < nodes; j++)
				linkRanks[i][j] = 0;
		int a,b,c;
		while(!FInG2.Eof()) {
			TStr Line2;	
			FInG2.GetNextLn(Line2);
			TStrV NIdV;
			Line2.SplitOnAllCh(',', NIdV);
			a = NIdV[0].GetInt();
			b = NIdV[1].GetInt();
			FInG2.GetNextLn(Line2);
			Line2.SplitOnAllCh(' ', NIdV);
			c = NIdV[0].GetInt();
			linkRanks[a][b] = c;
		}


		// Calculate emission (Input: network edges, edge ranks)

		num_one = 0; num_zero = 0;
		for(int i = 0; i < nodes; i++) {
			for(int j = 0; j < nodes; j++) {
				int ii = findKey(i);
				int jj = findKey(j);

				if(GroundTruth->IsEdge(ii,jj)) {

					emission[1][linkRanks[i][j]]++;
					num_one++;

				}
				else if(!GroundTruth->IsEdge(ii,jj)) {

					emission[0][linkRanks[i][j]]++;
					num_zero++;

				}
			}
		}
	}

	// Saving emission array
	TFOut FOut(TStr::Fmt("%s/eprob.txt", rankFNm.CStr()));
	FOut.PutStr(TStr::Fmt("%d \n", num_zero));
	FOut.PutStr(TStr::Fmt("%d \n", num_one));
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < NBin; j++) {
			FOut.PutStr(TStr::Fmt("%f \n", emission[i][j]));
		}
	}
}

void DNE::SaveRank(const TStr& OutFNm){
	double logValue = 0.0;
	double rankCoef2 = 10.0;
	int nodes = GroundTruth->GetNodes();
	TFOut FOut(OutFNm);
	FOut.PutStr(TStr::Fmt("%d \n", nodes)); 

	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			if(ranks[i][j] > 0){
				logValue = (log10(pow(ranks[i][j],rankCoef2) * numCascades[i][j]))/(log10(2.00));
				if(logValue > 0) {
					FOut.PutStr(TStr::Fmt("%d, %d\n", i, j)); 
					FOut.PutStr(TStr::Fmt("%d\n", (int)ceil(logValue)));
				}
			}
		}
	}

	cout << "DNE::SaveRank" << endl;
}

void DNE::SaveParameter_Transition(const TStr& OutFNm, const int nodes){ 
	TFOut FOut(OutFNm);


	// Default transition value for edges which have not been seen in training snapshots
	FOut.PutStr(TStr::Fmt("%d \n", transition_01_num));
	FOut.PutStr(TStr::Fmt("%d \n", transition_01_num + transition_00_num));
	FOut.PutStr(TStr::Fmt("%d \n", transition_11_num));
	FOut.PutStr(TStr::Fmt("%d \n", transition_11_num + transition_10_num));

	// Save the number of 01 and 11 transitions and the total number of 0* and 1* transitions for each edge
	for (int i = 0; i < nodes; i++) {
		for(int j = 0; j< nodes; j++) {
			FOut.PutStr(TStr::Fmt("%f \n", transition_01[i][j]));
			FOut.PutStr(TStr::Fmt("%f \n", transition_01[i][j]+transition_00[i][j]));
			FOut.PutStr(TStr::Fmt("%f \n", transition_11[i][j]));
			FOut.PutStr(TStr::Fmt("%f \n", transition_11[i][j]+transition_10[i][j]));
		}
	}
}

int DNE::findKey(const int i) {
	IDMAP::const_iterator it;
	for (it = mp.begin(); it != mp.end(); ++it)
		if (it->second == i)
			return it->first;
	return -1;
}

void DNE::compare(double Threshold, const TStr& ResultFNm, const TStr& e_probFNm, const TStr& InferredFNm, const int MxEdges, int snapShot, int prior){
	printf("Snapshot: %d\n", snapShot);
	printf("Loading Transition and Emission Probabilities\n");
	if(prior == 0) 
		loadTransitionProbabilty(TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), snapShot - 1));
	else if (prior == 1)
		loadTransitionProbabilty(TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), 1));
	loadEmissionProbability(e_probFNm.CStr());//, NBin); 

	printf("Computing P(E(t)|E(t-1),R(t))\n");

	double rankCoef2 = 10.0; double logValue = 0.0;

	// Initialize Probability Matrix
	double** probability = new double*[nodes];

	for(int i = 0; i < nodes; i++)
		probability[i] = new double[nodes];

	for(int i = 0; i < nodes; i++) 
		for(int j = 0; j < nodes; j++) 
			probability[i][j] = 0.0;


	cout << "Computing Probability Values\n";
	FILE* FWS = fopen((TStr::Fmt("%s/%d-ranks.txt", ResultFNm.CStr(), snapShot)).CStr(), "w+b");
	fprintf(FWS,"%d \n", nodes);
	int rank = 0;
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			int ii, jj, rank = 0;
			if(ranks[i][j] > 0) {
				logValue = (log10(pow(ranks[i][j],rankCoef2) * numCascades[i][j]))/(log10(2.00));
				if(logValue > 0) {
					rank = (int)ceil(logValue);
					if(rank > 0) {
						fprintf(FWS,"%d, %d\n", i, j); 
						fprintf(FWS,"%d\n", rank);
					}
				}
			}
			ii = findKey(i);
			jj = findKey(j);
			if(LastGroundTruth -> IsEdge(ii,jj)) {
				if(rank > 0 && transition_11[i][j] == 0) {					
					transition_11[i][j] = 0.004;
				}
				if(emission[1][rank] == 0) { // This emission value has not been seen in the training snapshot
					int yu = rank - 1;
					while( yu >= 0) {
						if(emission[1][yu] > 0) { // Set the default emission value of this rank to the emission value of biggest smaller seen rank
							probability[i][j] = ((transition_11[i][j] * emission[1][yu]) / ( (transition_11[i][j] * emission[1][yu]) + (transition_10[i][j] * emission[0][yu])));
							break;
						}
						yu--;
					}
				}
				else {
					probability[i][j] = ((transition_11[i][j] * emission[1][rank]) / ( (transition_11[i][j] * emission[1][rank]) + (transition_10[i][j] * emission[0][rank])));
					probability[i][j] = ((transition_11[i][j] * emission[1][rank]) / ( (transition_11[i][j] * emission[1][rank]) + (transition_10[i][j] * emission[0][rank])));

				}

			}
			if(!LastGroundTruth -> IsEdge(ii, jj)) {

				if(rank > 0 && transition_01[i][j] == 0) {
					transition_01[i][j] = 0.004;
				}

				if(emission[1][rank] == 0) {

					int yu = rank - 1;
					while(yu >= 0) {
						if(emission[1][yu] > 0) {
							probability[i][j] = ( (transition_01[i][j] * emission[1][yu]) / ( (transition_01[i][j] * emission[1][yu]) + (transition_00[i][j] * emission[0][yu])));
							break;
						}
						yu--;
					}
				}
				else {
					probability[i][j] = ( (transition_01[i][j] * emission[1][rank]) / ( (transition_01[i][j] * emission[1][rank]) + (transition_00[i][j] * emission[0][rank])));

				}

			}
		}
	}

	printf("Extracting Network");
	PrecisionRecall.Clr();
	int nodes = GroundTruth->GetNodes();
	double fmeasure = 0.0;

	inferredNetwork = TNGraph::New();
	// Inferred Network - Add Nodes
	for (TNGraph::TNodeI NI = ReferenceNetwork->BegNI(); NI < ReferenceNetwork->EndNI(); NI++) {
		inferredNetwork->AddNode(NI.GetId());
	}
	// Create File for Inferred Network
	FILE* inferredFOut = fopen(InferredFNm.CStr(), "w+b");

	PrecisionRecall.Clr();
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			if(probability[i][j] > Threshold) {
				int ii = findKey(i);
				int jj = findKey(j);
				// Save inferred edge
				if(!inferredNetwork->IsEdge(ii,jj,true)) {
					inferredNetwork->AddEdge(ii,jj);
					// Compute Recall, Precision 
					double precision = 0, recall = 0;
					if (PrecisionRecall.Len() > 1) {
						precision = PrecisionRecall[PrecisionRecall.Len()-1].Val2.Val;
						recall = PrecisionRecall[PrecisionRecall.Len()-1].Val1.Val;
					}	    
					if (GroundTruth->IsEdge(ii,jj)) 
						recall++;
					else 
						precision++;
					PrecisionRecall.Add(TPair<TFlt, TFlt>(recall, precision));
				}
			}
		}
	}

	// write edges to file (not allowing self loops in the network)
	for (TNGraph::TEdgeI EI = inferredNetwork->BegEI(); EI < inferredNetwork->EndEI(); EI++) {
		if (EI.GetSrcNId() != EI.GetDstNId()) {
			fprintf(inferredFOut,"%d,%d\r\n",EI.GetSrcNId(),EI.GetDstNId());
		}
	}
	fclose(inferredFOut);
	fclose(FWS);
	printf(" Snapshot %d: Inference Completed, Inferred Network Saved \n", snapShot);

	// Computing f-measure 	
	for (int i = 0; i < PrecisionRecall.Len(); i++) {
		PrecisionRecall[i].Val2 = PrecisionRecall[i].Val1/(double)GroundTruth->GetEdges();
		PrecisionRecall[i].Val1 = PrecisionRecall[i].Val1/(double)inferredNetwork->GetEdges();
		if(i == PrecisionRecall.Len() - 1) {
			fmeasure = ( 2 * PrecisionRecall[i].Val2.Val * PrecisionRecall[i].Val1.Val)/(PrecisionRecall[i].Val2.Val + PrecisionRecall[i].Val1.Val);
			fmeasureFile << PrecisionRecall[i].Val1.Val << "\t" << PrecisionRecall[i].Val2.Val << "\t" << fmeasure << endl;
			printf("fmeasure : %f \n ", fmeasure);
		}
	}
	// Update and Save Transition Probabilities
	if(prior == 0) {
		UpdateTransitionProbability(TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), snapShot - 1), TStr::Fmt("%s/%d-tprob.txt", ResultFNm.CStr(), snapShot),snapShot);//, ResultFNm.CStr());
		UpdateEmissionProbability(ResultFNm.CStr(),InferredFNm.CStr(),  snapShot);
	}
}

void DNE::loadEmissionProbability(const TStr& e_probFNm) {
	TFIn FInG2(e_probFNm);	
	TStrV NIdV; TStr Line;
	FInG2.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdV);
	int num_zero = NIdV[0].GetInt();
	FInG2.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdV);
	int num_one = NIdV[0].GetInt();

	// Compute Graphical Model Probability
	emissionProbability = new double*[2];
	emission= new double*[2];
	double* R_t = new double[NBin];
	double* R_t_x1 = new double[NBin];
	double* R_t_x0 = new double[NBin];

	int x1_num, x0_num, t_num;
	x1_num = 0; x0_num = 0;

	for(int i=0; i < 2; i++) {
		emissionProbability[i] = new double[NBin];
		emission[i] = new double[NBin];
	}
	for(int i=0; i < NBin; i++)
		R_t[i] = 0.0;
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < NBin; j++) {
			FInG2.GetNextLn(Line);
			Line.SplitOnAllCh('.', NIdV);
			t_num = (double)NIdV[0].GetInt();

			emission[i][j] = t_num;

			R_t[j] = R_t[j] + t_num;
			if(i == 0) {
				R_t_x0[j] = t_num;
				x0_num = x0_num + t_num;
			}else if(i == 1) {
				R_t_x1[j] = t_num;
				x1_num = x1_num + t_num;
			}
		}
	}
	t_num = 0;
	for(int j = 0; j < NBin; j++)	
		t_num = t_num + R_t[j];
	for(int j = 0; j < NBin; j++) {
		if((R_t_x1[j]/x1_num) == 0)	{
			emissionProbability[0][j] = 0.00;
		}
		else

			emissionProbability[0][j] = (R_t_x0[j])/(R_t_x0[j] + R_t_x1[j]);

		if((R_t_x0[j]/x0_num)==0) {
			emissionProbability[1][j]=0.00;
		}else

			emissionProbability[1][j] = (R_t_x1[j])/(R_t_x0[j] + R_t_x1[j]);
	}
}

void DNE::loadTransitionProbabilty(const TStr& t_probFNm) {
	TFIn FInG(t_probFNm);
	TStr Line;
	TStrV NIdV2, NIdV3,NIdV4, NIdV5;
	TStrV NIdvm;
	transition_01 = new double*[nodes];
	transition_11 = new double*[nodes];
	transition_00 = new double*[nodes];
	transition_10 = new double*[nodes];

	FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int tr_01=NIdvm[0].GetInt();

	FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int count_0=NIdvm[0].GetInt();

	FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int tr_11=NIdvm[0].GetInt();

	FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int count_1=NIdvm[0].GetInt();

	for (int i = 0; i < nodes; i++) {
		transition_01[i] = new double[nodes];
		transition_11[i] = new double[nodes];
		transition_00[i] = new double[nodes];
		transition_10[i] = new double[nodes];
	}

	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV2);
			FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV3);
			if(NIdV3[0].GetFlt() == 0) {
				transition_01[i][j]=(double)(tr_01/(double)count_0);
				transition_00[i][j]=(double)((count_0-tr_01)/(double)count_0);
			}else {
				transition_01[i][j] = (double)NIdV2[0].GetFlt()/NIdV3[0].GetFlt(); 
				transition_00[i][j] = (double)((NIdV3[0].GetFlt()-NIdV2[0].GetFlt())/NIdV3[0].GetFlt()); 
			}
			FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV4);
			FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV5);
			if(NIdV4[0].GetFlt() == 0) {
				transition_11[i][j]=(double)(tr_11/(double)count_1);
				transition_10[i][j]=(double)((count_1-tr_11)/(double)count_1);
			}
			else {
				transition_11[i][j] = (double)NIdV4[0].GetFlt()/NIdV5[0].GetFlt();
				transition_10[i][j] = (double)((NIdV5[0].GetFlt()-NIdV4[0].GetFlt())/NIdV5[0].GetFlt());
			}
		}
	}
}

void DNE::UpdateEmissionProbability(const TStr& ResultFNm, const TStr& networkFNm, const int snapshot) {
	cout << "Update Emission Probability - Inferred Snapshot :" << snapshot << "\tname:"<<networkFNm.CStr() << endl;
	int num_one = 0; int num_zero = 0; //int integration;
	// Load network
	TFIn FInG(TStr::Fmt(networkFNm.CStr()));
	LoadGroundTruthTxt(FInG);

	double** HelpTotalEmission=new double*[2];
	for(int w = 0; w < 2; w++) {
		HelpTotalEmission[w]=new double[NBin];
	}
	for(int y = 0; y < 2; y++) {
		for(int g = 0; g < NBin; g++) {
			HelpTotalEmission[y][g] = emission[y][g]; 
			if(y==0) {
				num_zero += emission[y][g];
			} else if(y == 1) {
				num_one += emission[y][g];
			}
		}
	}

	// Load ranks
	TFIn FInG2(TStr::Fmt("%s/%d-ranks.txt", ResultFNm.CStr(), snapshot));
	TStr Line; FInG2.GetNextLn(Line);
	TStrV NIdV; Line.SplitOnAllCh(' ', NIdV);
	int nodes = NIdV[0].GetInt();
	linkRanks = new int*[nodes];
	for(int i=0; i < nodes; i++)
		linkRanks[i] = new int[nodes];
	for(int i = 0; i < nodes; i++)
		for(int j = 0; j < nodes; j++)
			linkRanks[i][j] = 0;
	int a,b,c;
	while(!FInG2.Eof()) {
		TStr Line2;	
		FInG2.GetNextLn(Line2);
		TStrV NIdV;
		Line2.SplitOnAllCh(',', NIdV);
		a = NIdV[0].GetInt();
		b = NIdV[1].GetInt();
		FInG2.GetNextLn(Line2);
		Line2.SplitOnAllCh(' ', NIdV);
		c = NIdV[0].GetInt();
		linkRanks[a][b] = c;
	}
	// Calculate emission (Input: network edges, edge ranks)
	//integration = maxRank/(NBin-1);
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			int ii = findKey(i);
			int jj = findKey(j);
			if(GroundTruth -> IsEdge(ii,jj)) {

				if(HelpTotalEmission[1][linkRanks[i][j]] == 0) {
					emission[1][linkRanks[i][j]]++;
					num_one++;
				}

			}else if(!GroundTruth->IsEdge(ii,jj)) {

				if(HelpTotalEmission[0][linkRanks[i][j]] == 0) {
					emission[0][linkRanks[i][j]]++;
					num_zero++;
				}

			}
		}
	}

	// Save emission array
	TFOut FOut(TStr::Fmt("%s/eprob.txt", ResultFNm.CStr()));
	FOut.PutStr(TStr::Fmt("%d \n", num_zero));
	FOut.PutStr(TStr::Fmt("%d \n", num_one));
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < NBin; j++) {
			FOut.PutStr(TStr::Fmt("%f \n", emission[i][j]));
		}
	}
}

void DNE::UpdateTransitionProbability(const TStr& t_probFNm, const TStr& t_newProbFNm, int i){//, const char* address) {
	TFOut prob_FOut(t_newProbFNm);
	TFIn prob_FInG(t_probFNm);
	TStr Line; TStrV NIdV2, NIdV3; TStrV NIdvm;
	double total_0, total_1, t_01, t_11;
	int ii, jj;

	// Load default transition values
	prob_FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int tr_01 = NIdvm[0].GetInt();

	prob_FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int count_0 = NIdvm[0].GetInt();


	prob_FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int tr_11 = NIdvm[0].GetInt();

	prob_FInG.GetNextLn(Line);
	Line.SplitOnAllCh(' ', NIdvm);    
	int count_1 = NIdvm[0].GetInt();

	// Update default transition values
	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			ii = findKey(i);
			jj = findKey(j);
			if(LastGroundTruth -> IsEdge(ii, jj)) {
				count_1++;
			}else if(!LastGroundTruth -> IsEdge(ii, jj)) {
				count_0++;
			}
			if(LastGroundTruth -> IsEdge(ii,jj) && inferredNetwork -> IsEdge(ii, jj)) {
				tr_11++;
			}else if(!LastGroundTruth -> IsEdge(ii, jj) && inferredNetwork -> IsEdge(ii, jj)) {
				tr_01 += sFactor;
				count_0 += sFactor;
			}
		}
	}
	prob_FOut.PutStr(TStr::Fmt("%d \n", tr_01));
	prob_FOut.PutStr(TStr::Fmt("%d \n",  count_0));
	prob_FOut.PutStr(TStr::Fmt("%d \n", tr_11));
	prob_FOut.PutStr(TStr::Fmt("%d \n", count_1));

	for(int i = 0; i < nodes; i++) {
		for(int j = 0; j < nodes; j++) {
			// Load transition values of each edge
			prob_FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV2);
			prob_FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV3);
			t_01 = NIdV2[0].GetFlt();
			total_0 = NIdV3[0].GetFlt();

			prob_FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV2);
			prob_FInG.GetNextLn(Line);
			Line.SplitOnAllCh(' ', NIdV3);
			t_11 = NIdV2[0].GetFlt();
			total_1 = NIdV3[0].GetFlt();

			ii = findKey(i);
			jj = findKey(j);

			// Update transition value of each edge (i, j)
			if(LastGroundTruth -> IsEdge(ii, jj)) {
				total_1 ++;
			}else if(!LastGroundTruth -> IsEdge(ii, jj)) {
				total_0 ++;
			}
			if(LastGroundTruth -> IsEdge(ii, jj) && inferredNetwork -> IsEdge(ii, jj)) {
				t_11++;
			}else if(!LastGroundTruth -> IsEdge(ii, jj) && inferredNetwork -> IsEdge(ii, jj)) {
				t_01 = t_01 + sFactor;
				total_0 = total_0 + sFactor;						
			}
			prob_FOut.PutStr(TStr::Fmt("%f \n", t_01));
			prob_FOut.PutStr(TStr::Fmt("%f \n", total_0));

			prob_FOut.PutStr(TStr::Fmt("%f \n", t_11));
			prob_FOut.PutStr(TStr::Fmt("%f \n", total_1));
		}
	}
}

void DNE::SavePajek(const TStr& OutFNm) {
	TIntSet NIdSet;
	FILE *F = fopen(OutFNm.CStr(), "wt");
	fprintf(F, "*Vertices %d\r\n", NIdSet.Len());
	for (THash<TInt, TNodeInfo>::TIter NI = NodeNmH.BegI(); NI < NodeNmH.EndI(); NI++) {
		const TNodeInfo& I = NI.GetDat();
		fprintf(F, "%d \"%s\" ic Blue x_fact %f y_fact %f\r\n", NI.GetKey().Val,
			I.Name.CStr(), TMath::Mx<double>(log((double)I.Vol)-5,1), TMath::Mx<double>(log((double)I.Vol)-5,1));
	}
	fprintf(F, "*Arcs\r\n");
	for (TNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
		fprintf(F, "%d %d 1\r\n", EI.GetSrcNId(), EI.GetDstNId());
	}
	fclose(F);
}

void DNE::SavePlaneTextNet(const TStr& OutFNm) {
	TIntSet NIdSet;
	FILE *F = fopen(OutFNm.CStr(), "wt");
	for (THash<TInt, TNodeInfo>::TIter NI = NodeNmH.BegI(); NI < NodeNmH.EndI(); NI++) {
		const TNodeInfo& I = NI.GetDat();
		fprintf(F, "%d,%d\r\n", NI.GetKey().Val, NI.GetKey().Val);
	}

	fprintf(F, "\r\n");

	for (TNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
		fprintf(F, "%d,%d\r\n", EI.GetSrcNId(), EI.GetDstNId());
	}
	fclose(F);
}

void DNE::SaveEdgeInfo(const TStr& OutFNm) {
	FILE *F = fopen(OutFNm.CStr(), "wt");

	fprintf(F, "src dst vol marginal_gain median_timediff average_timediff\n");
	for (THash<TIntPr, TEdgeInfo>::TIter EI = EdgeInfoH.BegI(); EI < EdgeInfoH.EndI(); EI++) {
		TEdgeInfo &EdgeInfo = EI.GetDat();
		fprintf(F, "%s/%s/%d/%f/%f/%f\n",
			NodeNmH.GetDat(EI.GetKey().Val1.Val).Name.CStr(), NodeNmH.GetDat(EI.GetKey().Val2.Val).Name.CStr(),
			EdgeInfo.Vol.Val, EdgeInfo.MarginalGain.Val,
			EdgeInfo.MedianTimeDiff.Val,
			EdgeInfo.AverageTimeDiff.Val);
	}
	fclose(F);
}

void DNE::SaveObjInfo(const TStr& OutFNm) {
	TGnuPlot GnuPlot(OutFNm);

	TFltV Objective, Bound;

	for (THash<TIntPr, TEdgeInfo>::TIter EI = EdgeInfoH.BegI(); EI < EdgeInfoH.EndI(); EI++) {
		if (Objective.Len()==0) {
			Bound.Add(EI.GetDat().MarginalBound + EI.GetDat().MarginalGain); Objective.Add(EI.GetDat().MarginalGain); }
		else {
			Objective.Add(Objective[Objective.Len()-1]+EI.GetDat().MarginalGain);
			Bound.Add(EI.GetDat().MarginalBound + Objective[Objective.Len()-1]); }
	}

	GnuPlot.AddPlot(Objective, gpwLinesPoints, "NETINF");
	GnuPlot.AddPlot(Bound, gpwLinesPoints, "Upper Bound (Th. 4)");

	GnuPlot.SavePng();
}

bool operator> (const Edge &e1,const Edge &e2){
	return e1.value > e2.value;
}

bool operator<= (const Edge &e1,const Edge &e2){
	return e1.value <= e2.value;
}


bool operator>= (const Edge &e1,const Edge &e2){
	return e1.value >= e2.value;
}


bool operator< (const Edge &e1,const Edge &e2){
	return e1.value < e2.value;
}
