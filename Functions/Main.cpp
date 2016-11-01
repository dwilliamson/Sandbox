
// TODO: Add STL version of bound function call
// TODO: Add pycgen version


//#include "StdVersion.h"
//#include "BoundFnCustom.h"
#include "VirtualFunction.h"

#include <string.h>
#include <typeinfo>


EmitterActivity Emitter::Activity;


template <class T>
void PrintType()
{
	printf("%.*s\n", sizeof(__FUNCSIG__) - 81, __FUNCSIG__ + 79);
}

template <class T>
const char* Typename()
{
	static const char* sig = __FUNCSIG__;
	static const char* start = sig + 29;
	static const char* end = sig + sizeof(__FUNCSIG__) - 8;
	static char name[sizeof(__FUNCSIG__)] = { 0 };
	if (name[0] == 0)
	{
		memcpy(name, start, end - start);
		name[end - start] = 0;
	}

	return name;
}


template <typename... BoundArguments>
void Deduce(BoundArguments&&... arguments)
{
	using T = Tuple<BoundArguments...>;
	T bound(Forward<BoundArguments>(arguments)...);
	printf("%s\n", typeid(T).name());
}


int main()
{
	//bound_fn_custom::Test();
	//bound_fn_custom::CodeGenTest();
	//std_version::CodeGenTest();
	virtual_fn::Test();
	virtual_fn::CodeGenTest();

	Emitter::Activity.Print();

	Emitter e;
	int x = 1;
	Deduce(x, e);

	return 0;
}