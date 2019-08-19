
#ifndef OpenKAI_src_Script_JSON_H_
#define OpenKAI_src_Script_JSON_H_

#include "../Base/common.h"
#include "picojson.h"

using namespace std;
using namespace picojson;

namespace kai
{

class JSON
{
private:
	picojson::value m_JSON;

public:
	JSON(void);
	~JSON(void);

	bool parse(string json);
	int checkErrorNum(void);

	bool v(const string& name, int* pVal);
	bool v(const string& name, bool* pVal);
	bool v(const string& name, uint8_t* pVal);
	bool v(const string& name, uint16_t* pVal);
	bool v(const string& name, uint32_t* pVal);
	bool v(const string& name, uint64_t* pVal);
	bool v(const string& name, double* pVal);
	bool v(const string& name, float* pVal);
	bool v(const string& name, string* pVal);
	bool v(const string& name, vFloat2* pV);
	bool v(const string& name, vFloat3* pV);

	int a(const string& name, int* pVal, int nElem);
	int a(const string& name, double* pVal, int nElem);
	int a(const string& name, float* pVal, int nElem);
	int a(const string& name, string* pVal, int nElem);
	bool array(const string& name, value::array* pVal);

};

}
#endif
