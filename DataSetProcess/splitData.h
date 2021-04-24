// Node info (name and number of cascades)
#include <map>
class maps{
public:
typedef map<int, int, less<int> > IDMAP;
IDMAP mp;
IDMAP::const_iterator it;
};
class DynamicEdgeInfo {
public:
  double* value; 
  int timestep;
public:
  DynamicEdgeInfo() { }
  DynamicEdgeInfo(int y) { }
  void init(){
  	timestep = 500;
        value = new double[timestep];
 	for(int i = 0; i < timestep; i++){
		value[i] = 0;
	}
  }
  void setValue(double val, int timestep){
  	value[timestep] = val;
  }  
  double getValue(int timestep){
  	return value[timestep];
  }
};

