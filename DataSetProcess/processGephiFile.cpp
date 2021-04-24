#include "stdafx.h"
 
int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("\n Processing Gephi Files. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try
	const TStr InFNm  = Env.GetIfArgPrefixStr("-inputFile:", "", "Input File \n");
	const TStr OutFNm = Env.GetIfArgPrefixStr("-outputFile:", "", "Output File \n");
	int operation = Env.GetIfArgPrefixInt("-operation:", 1, "1: Extract edges from gephi files 2: Extract edges per month \n");
	/*---------------------------------------------------------------------------*/
	printf("DDNE_RealData/ProcessGephiFile Running \n"); 
	switch (operation){
	case 1: // Edges of all times are extracted from gephi files
	{
		TFIn FIn(InFNm);	
		TFOut FOut(OutFNm); 
		TStr Line;
		TStrV NIdV_t, NIdV_0;
		while (!FIn.Eof()) {
		FIn.GetNextLn(Line);
		while (Line != "<edges>")
			FIn.GetNextLn(Line);	
		FIn.GetNextLn(Line); // skip the next line
		FIn.GetNextLn(Line); // start reading edge information
		while ((Line != " ") && (!FIn.Eof())){
			Line.SplitOnAllCh(' ', NIdV_t);
			if(NIdV_t.Len() > 2){
				Line.SplitOnAllCh('"', NIdV_0);
				if(NIdV_0[0] == "<edge source=")
					FOut.PutStr(TStr::Fmt("\nsrc-dsc "));			
				FOut.PutStr(TStr::Fmt("%s", NIdV_0[1].CStr()));
				FOut.PutStr(TStr::Fmt(" "));
				FOut.PutStr(TStr::Fmt("%s\n", NIdV_0[3].CStr()));
			}
			FIn.GetNextLn(Line);
		}	
		}
	break;
	}
	case 2: // Edges of each month are extracted
	{
		TStr Line;
		TStrV NIdV_1, NIdV_2, NIdV;
		int src, dsc;
		bool found = 0;
		for(int i = 1; i <= 12; i++) {		 
		// read file
		TFIn FIn(InFNm);	
		TFOut FOut(TStr::Fmt("%s/%d-network.txt", OutFNm.CStr(),i)); 
		FIn.GetNextLn(Line); // skip the first line
		while (!FIn.Eof()) {
			FIn.GetNextLn(Line);
			if(Line != "") {
				Line.SplitOnAllCh(' ', NIdV);
				if(NIdV.Len() > 2) {
					found = 0; // the edge is not processed yet
					src = NIdV[1].GetInt();
					dsc = NIdV[2].GetInt();								
				}else{
					if (found == 1) {
						continue; // the edge has been already processed				
					}
					NIdV[0].SplitOnAllCh('-', NIdV_1);	
					NIdV[1].SplitOnAllCh('-', NIdV_2);
					if(i < 11) { // Only two month are in 2012 (i = 1-10 -> (i + 2), i = 11 and i = 12 -> (i - 2))
						if ((NIdV_2[0].GetInt() == NIdV_1[0].GetInt()) && (NIdV_1[0].GetInt() == 2011)) {
							if (NIdV_2[1].GetInt() == i + 2) {
								found = 1;
								FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
								// save node in array	
							}
							else if (NIdV_2[1].GetInt() > i + 2) { // go check the start date
								if (NIdV_1[1].GetInt() > i + 2) found = 1;
								else {
									found = 1;
									FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
									
								}
							}
						}else if((NIdV_2[0].GetInt() == NIdV_1[0].GetInt()) && (NIdV_1[0].GetInt() == 2012)) {
							found = 1;
						}else if(NIdV_2[0].GetInt() != NIdV_1[0].GetInt()) {
							if (i + 2 >= NIdV_1[1].GetInt()) {
								FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
								
							}
							found = 1;	
						}
					}else {
						if (NIdV_2[0].GetInt() != NIdV_1[0].GetInt()) {
							if ((i-10) <= NIdV_2[1].GetInt()) {
								FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
							}
							found = 1;										
						}else if ((NIdV_2[0].GetInt() == NIdV_1[0].GetInt()) && (NIdV_1[0].GetInt() == 2012)) {
							if (NIdV_2[1].GetInt() == i - 10) {
								found = 1;
								FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
							}
							else if (NIdV_2[1].GetInt() > i - 10) { // go check the start date
								if (NIdV_1[1].GetInt() > i - 10) 
									found = 1;
								else {
									found = 1;
									FOut.PutStr(TStr::Fmt("%d,%d\n", src, dsc));
								}
							}
						}
					}			
				}
			}				
		}
	}
	break;
	}
	}
	Catch
	printf("DDNE_RealData/ProcessGephiFile Finished \n"); 
	return 0;
}
