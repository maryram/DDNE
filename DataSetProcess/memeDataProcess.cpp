#include "stdafx.h"

int main(int argc, char* argv[]) {
  Try

  const TStr InFNm  = Env.GetIfArgPrefixStr("-i:", "example-cascades.txt", "Input cascades");
  const TStr OutFNm  = Env.GetIfArgPrefixStr("-o:", "network.txt", "Output file name(s) prefix");

  printf("\nmemeDataProcess.cpp - Start\n"); 
   
  TFIn FIn(InFNm);	
  TFOut FOut(OutFNm); 
  TStr Line;
  TStr* tempLine = new TStr[100];
  bool isQ = 0, isL =0;
  int index;
 while (!FIn.Eof()) {
	for(int i = 0; i < 100; i++)
		tempLine[i] = " ";
	FIn.GetNextLn(Line);
	index = 0;
	isQ = 0; 
	isL = 0;
	while(Line != ""){
		TStrV NIdV;
		Line.SplitOnAllCh(' ', NIdV);
		if(NIdV[0] == "Q"){
			isQ = 1;
		}
		else if(NIdV[0] == "L"){
			isL = 1;
		}
		tempLine[index] = Line;
		FIn.GetNextLn(Line);
		index++;
	}
	if(isQ && isL){
		for(int i = 0; i < index; i++){
			FOut.PutStr(TStr::Fmt("%s\n", tempLine[i].CStr()));
		}
		FOut.PutStr("\r\n");
	}
		
  }
 Catch
  printf("\nmemeDataProcess.cpp - End\n"); 
 return 0;
}
