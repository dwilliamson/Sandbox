/*
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <DbgHelp.h>
#include <winuser.h>
#include <signal.h>

#include <typeinfo>


#include <vector>
#include <intrin.h>

#pragma comment(lib, "dbghelp.lib")

LARGE_INTEGER g_PerformanceCounterStart;
double g_PerformanceCounterScale = 0;


void InitTimer()
{
	LARGE_INTEGER performance_frequency;
	QueryPerformanceFrequency(&performance_frequency);
	g_PerformanceCounterScale = 1000000.0 / performance_frequency.QuadPart;
	QueryPerformanceCounter(&g_PerformanceCounterStart);
}


__int64 GetTimer()
{
	// Read counter and convert to microseconds
	LARGE_INTEGER performance_count;
	QueryPerformanceCounter(&performance_count);
	return __int64((performance_count.QuadPart - g_PerformanceCounterStart.QuadPart) * g_PerformanceCounterScale);
}

int BinarySearch (int a[], int low, int high, int key)
{
    int mid;

    if (low == high)
        return low;

    mid = low + ((high - low) / 2);

	if (mid >= 4)
		_asm int 3;

    if (key > a[mid])
        return BinarySearch (a, mid + 1, high, key);
    else
        return BinarySearch (a, low, mid, key);

    return mid;
}


int BinarySearch2(int a[], int nb_keys, int key)
{
	int first = 0;
	int last = nb_keys - 1;

	// Binary search for nearest match
	while (first < last)
	{
		// Compare with the mid point, shifting search to upper or lower half
		int mid = (first + last) / 2;
		if (key > a[mid])
			first = mid + 1;
		else
			last = mid;
	}

	return first;
}


void BinaryInsertionSortTest()
{
	int a[] = { 1, 3, 5, 7 };
	//int i0 = BinarySearch(a, 0, 4, 0);
	//int i2 = BinarySearch(a, 0, 4, 2);
	//int i4 = BinarySearch(a, 0, 4, 4);
	//int i6 = BinarySearch(a, 0, 4, 6);
	//int i8 = BinarySearch(a, 0, 4, 8);
	int i0 = BinarySearch2(a, 4, 0);
	int i2 = BinarySearch2(a, 4, 2);
	int i4 = BinarySearch2(a, 4, 4);
	int i6 = BinarySearch2(a, 4, 6);
	int i8 = BinarySearch2(a, 4, 8);
}


int Align(int v, int b)
{
	return v + ((b - 1) - ((v - 1) % b));
}


int gcd_pos(int a, int b)
{
	while (a != b)
	{
		if (a > b)
			a = a - b;
		else if (a < b)
			b = b - a;
	}

	//if (a > b)
	//	return gcd_pos(a - b, b);
	//else if (a < b)
	//	return gcd_pos(a, b - a);

	return a;
}


int lcm_pos(int a, int b)
{
	return a / gcd_pos(a, b) * b;
}


struct Base
{
	virtual void Destroy() = 0 ;
	virtual void Hello() = 0;
	void operator delete(void* p)
	{
		printf("Yeah right\n");
	}
	virtual Base* Get() { return 0; }
};

namespace Blah
{

	struct Derived : public Base
	{
		void Hello()
		{
		}

		void Destroy()
		{
			delete this;
		}

		Derived* Get() { return 0; }
	};
}


struct Derived2 : public Base
{
	void Destroy()
	{
		delete this;
	}
};

struct Vector
{
	Vector()
		: size(0)
		, capacity(0)
		, data(0)
	{
	}

	~Vector()
	{
		if (data)
			free(data);
	}

	int size;
	int capacity;
	char* data;
};


// http://www.geoffchappell.com/studies/msvc/language/predefined/
// http://www.openrce.org/articles/full_view/23
// http://www.kegel.com/mangle.html


#pragma warning (disable:4200)
struct TypeDescriptor
{
	unsigned long hash;
	void* spare;
	char name[0];
};
#pragma warning (default:4200)


struct PMD
{
    int mdisp;
    int pdisp;
    int vdisp;
};

struct RTTIBaseClassDescriptor
{
    TypeDescriptor* pTypeDescriptor;
    DWORD numContainedBases;
    PMD where;
    DWORD attributes;
};


#pragma warning (disable:4200)
struct RTTIBaseClassArray
{
	RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors [];
};
#pragma warning (default:4200)


struct RTTIClassHierarchyDescriptor
{
    DWORD signature;      //always zero?
    DWORD attributes;     //bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
    DWORD numBaseClasses; //number of classes in pBaseClassArray
    RTTIBaseClassArray* pBaseClassArray;
};


struct RTTICompleteObjectLocator
{
	DWORD signature; //always zero ?
	DWORD offset;    //offset of this vtable in the complete class
	DWORD cdOffset;  //constructor displacement offset
	TypeDescriptor* pTypeDescriptor; //TypeDescriptor of the complete class
	RTTIClassHierarchyDescriptor* pClassDescriptor; //describes inheritance hierarchy
};


RTTICompleteObjectLocator* TypeInfo(void* ptr)
{
	void** vptr = *(void***)ptr;
	return *(RTTICompleteObjectLocator**)(vptr - 1);
}


const char* UndecorateSymbol(const char* symbol)
{
	static char output[1024];
	UnDecorateSymbolName(symbol, output, sizeof(output), UNDNAME_COMPLETE);
	return output;
}


enum tABC { };
enum tXYZ { };


void Func(tABC abc)
{
	abc = tABC(3);
}

void Func(tXYZ xyz)
{
	xyz = tXYZ(3);
	int x = int(xyz);
}


typedef unsigned long long u64;

struct usTimer
{
	usTimer()
	{
		LARGE_INTEGER performance_frequency;

		// Calculate the scale from performance counter to microseconds
		QueryPerformanceFrequency(&performance_frequency);
		counter_scale = 1000000.0 / performance_frequency.QuadPart;

		// Record the offset for each read of the counter
		QueryPerformanceCounter(&counter_start);
	}

	u64 Get() const
	{
        LARGE_INTEGER performance_count;

        // Read counter and convert to microseconds
        QueryPerformanceCounter(&performance_count);
        return (u64)((performance_count.QuadPart - counter_start.QuadPart) * counter_scale);
	}

    LARGE_INTEGER counter_start;
    double counter_scale;
};


struct GameContext
{
};


struct Entity
{
	Entity()
	{
	}

	Entity(int)
	{
	}

	Entity(GameContext& ctx)
	{
	}
};


struct Component
{
	Component(int a, int b)
		: type(0)
	{
	}

	Component* SetType(int t)
	{
		type = t;
		return this;
	}

	int type;
};


template <typename TYPE>
void A(const TYPE& a)
{
	new Entity(const_cast<TYPE&>(a));
}


void BlahYeah(GameContext& ctx)
{
	A(ctx);
	A(3);
}


#define NewComponent(constructor)	\
	(new constructor)->SetType(3)

unsigned int icbrt64(unsigned __int64 x)
{
  int s;
  unsigned int y;
  unsigned __int64 b;

  y = 0;
  for (s = 63; s >= 0; s -= 3) {
    y += y;
    b = 3*y*((unsigned __int64) y + 1) + 1;
    if ((x >> s) >= b) {
      x -= b << s;
      y++;
    }
  }
  return y;
}

int icbrt1(unsigned int x)
{
   int s;
   unsigned int y, b;

   y = 0;
   for (s = 30; s >= 0; s = s - 3) {
      y = 2*y;
      b = (3*y*(y + 1) + 1) << s;
      if (x >= b) {
         x = x - b;
         y = y + 1;
      }
   }
   return y;
}


template <int s, int x, int y> struct CubeRootFormula
{
	static const unsigned int Ya = 2 * y;
	static const unsigned int B = (3 * Ya * (Ya + 1) + 1) << s;
	static const unsigned int X = (x >= B) ? x - B : x;
	static const unsigned int Yb = (x >= B) ? Ya + 1 : Ya;
};

template <int s, int x, int y> struct CubeRootLoop : public CubeRootFormula<s, x, y>
{
	static const unsigned int Result = CubeRootLoop<s - 3, X, Yb>::Result;
};

template <int x, int y> struct CubeRootLoop<0, x, y> : public CubeRootFormula<0, x, y>
{
	static const unsigned int Result = Yb;
};

template <int x> struct CubeRoot
{
	static const unsigned int Result = CubeRootLoop<30, x, 0>::Result;
};

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

struct AdapterPtr
{
	IP_ADAPTER_INFO* info;
	unsigned int sort_key;
};

int CompareAdapters(const void* a, const void* b)
{
	AdapterPtr& adp_a = *(AdapterPtr*)a;
	AdapterPtr& adp_b = *(AdapterPtr*)b;

	unsigned int ka = adp_a.sort_key;
	unsigned int kb = adp_b.sort_key;

	return (ka < kb) - (ka > kb);
}

void GetMACAddress()
{
	IP_ADAPTER_INFO* info = NULL;

	// Make an initial call to get size of required buffer
	// Rely on API returning an error -- with valid size -- if first parameter is NULL
	DWORD size = 0;
	if (GetAdaptersInfo(NULL, &size) == ERROR_BUFFER_OVERFLOW)
		info = (IP_ADAPTER_INFO*)malloc(size);
	if (info == NULL)
		return;

	// Get adapter linked list
	if (GetAdaptersInfo(info, &size) != NO_ERROR)
	{
		free(info);
		return;
	}

	// Build adapter list
	std::vector<AdapterPtr> adapters;
	for ( ; info != NULL; info = info->Next)
	{
		// Skip adapters that don't contain a MAC address
		if (info->AddressLength < 4)
			continue;

		AdapterPtr adapter;
		adapter.info = info;

		// Use 2 bits to prioritise ethernet adapters
		unsigned int type_priority = 0;
		switch (info->Type)
		{
			case MIB_IF_TYPE_ETHERNET: type_priority = 3; break;
			case IF_TYPE_IEEE80211: type_priority = 2; break;
			case MIB_IF_TYPE_PPP: type_priority = 1; break;
			default: type_priority = 0; break;
		}

		// Add to collection, giving highest priority to connected adapters
		adapter.sort_key = (info->LeaseObtained != 0) << 2 | type_priority;
		adapters.push_back(adapter);
	}

	// Sort by priority
	qsort(adapters.data(), adapters.size(), sizeof(AdapterPtr), CompareAdapters);

	free(info);
}


struct tZero
{
};

template <typename T>
struct tA : public tZero
{
	virtual void talk() { printf("A\n"); }
};

template <typename T>
struct tB : public tA<T>
{
	virtual void talk() { printf("B\n"); }
};

struct tC : public tZero
{
};

template <typename T>
void foo(tA<T>* obj)
{
	obj->talk();
}

void foo(tZero* obj)
{
	printf("wrong!\n");
}


template <typename T>
struct Test
{
	static void foo(Test* obj)
	{
		printf("wrong!\n");
	}
};

template <typename T>
struct Test<tB<T> >
{
	static void foo(Test* obj)
	{
		obj->Talk();
	}
};


template <typename TYPE>
struct Select
{
	void Do()
	{
	}
};
void Select(int x)
{
	printf("int\n");
}
void Select(unsigned int x)
{
	printf("unsigned int\n");
}



struct Obj
{
	int x;
};


#define out OutCatcher() <<

template <typename TYPE> struct Out
{
	explicit Out(TYPE& ref) : ref(ref) { }
	TYPE& ref;
};

struct OutCatcher
{
	template <typename TYPE> Out<TYPE> operator << (TYPE& obj)
	{
		return Out<TYPE>(obj);
	}
};

void Mutate(Out<Obj> obj)
{
	obj.ref.x = 3;
}

void Test()
{
	Obj obj;
	//Mutate(obj);			// compile error
	Mutate(out obj);		// fine
}


int HelloThere(int x)
{
	if (x == 0)
		return 1;
	return x * 3 + 75 + HelloThere(x - 1);
}

int Pow(int x)
{
	return x == 0 ? 1 : x * Pow(x - 1);
}

#include <assert.h>

struct Object
{
	static Object* New();
	static void Delete(Object* ptr);
  
	void Method1(int a, int b, int c);
	int Method2() const;
};


struct ObjectData : public Object
{
	ObjectData()
		: x(0)
		, y(1)
		, z(2)
	{
	}
  
	int x, y, z;
};

static ObjectData& Data(Object* ptr)
{
	assert(ptr != nullptr);
	return *static_cast<ObjectData*>(ptr);
}
static const ObjectData& Data(const Object* ptr)
{
	assert(ptr != nullptr);
	return *static_cast<const ObjectData*>(ptr);
}

Object* Object::New()
{
	return new ObjectData();
}

void Object::Delete(Object* ptr)
{
	delete ptr;
}

void Object::Method1(int a, int b, int c)
{
	ObjectData& data = Data(this);
	data.x += a;
	data.y += b;
	data.z += c;
}

int Object::Method2() const
{
	const ObjectData& data = Data(this);
	return data.x + data.y + data.z;
}


struct Interface
{
	virtual ~Interface() { }
	virtual void Blah() = 0;
};
struct Implementation : public Interface
{
	~Implementation()
	{
		printf("~Implementation\n");
	}

	void Blah()
	{
		printf("Blah\n");
	}
};


#define FB_ON 0x518f5e5c
#define FB_OFF 0x02268f8c
#define FB_USING(x) (1 / (((x) == FB_ON) ? 1 : (((x) == FB_OFF) ? 2 : 0)))

#define FB_DEBUG_ALLOC 2



struct float2
{
	static const bool two = true;
	float x, y;
};


struct float3
{
	static const bool three = true;
	float3()
		//: xy(*(float2*)this)
	{
	}

	//float2& xy;

	float x, y, z;
};


template<bool B, class T = void>
struct enable_if {};
 
template<class T>
struct enable_if<true, T> { typedef T type; };

template<class T>
void
Do(const T& t) 
{
}
template<class T>
typename enable_if<T::three>::type 
Do(const T& t) 
{
}


void test()
{
	float2 x;
	float3 y;
	Do(x);
}

int sig = 0;
void sigintHandler(int sig_num) {
    sig = sig_num;
    printf("Interrupted\n");
}

//int main()
//{
//	signal(SIGINT, sigintHandler);
//	float2 a;
//	a.x = 0;
//	a.y = 1;

//	float3 b;
//	b.x = 2;
//	b.y = 3;
//	b.z = 4;

//	test();

//	printf("%d %d %d\n", sizeof(short), sizeof(int), sizeof(long));

	//printf("%s\n", FB_USING(FB_DEBUG_ALLOC) ? "ON" : "OFF");
	/*Interface* i = new Implementation();
	i->Blah();
	delete i;*/

	//srand(time(NULL));

	/*Object* o = Object::New();
	o->Method1(rand(), rand(), rand());
	printf("%d\n", o->Method2());
	Object::Delete(o);*/


	//printf("%d\n", Pow(5));
	//printf("%d\n", HelloThere(HelloThere(HelloThere(5))));

	//Obj obj;
	//Mutate(out obj);

	//OutCatcher catcher;
	//catcher << obj;

	//unsigned int x = 0;
	//Select(x - 1);

	//tB<int>* obj = new tB<int>();
	//foo(obj);
	//foo(new tC);

	//GetMACAddress();

	/*int x = icbrt1(2000);

	char Array[CubeRoot<2000>::Result];*/

	/*const Entity e;
	Component* c = NewComponent(Component(1, 2));

	A(e);*/

	/*float x = -100;
	float y = floor(x);
	printf("%f\n", y);*/

	/*unsigned int x = 1;
	unsigned int y = 32;
	unsigned int z = x << y;
	printf("%d\n", z);*/

	/*
	// Can't be represented by float
	//double x = 1234566789;

	// Rounds to nearest integer when converted to float
	double x = 5337331.7812500000;

	float y = (float)x;
	int z = (int)y;
	unsigned int w = z & 0xFFFF;
	short& sw = (short&)w;
	printf("%f\n", y);*/

	/*usTimer timer;

	for (int i = 0; i < 30; i++)
	{
		u64 a = timer.Get();
		Sleep(30);
		u64 b = timer.Get();
		printf("%d\n", b - a);
	}*/

	/*SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SymGetOptions());

	tABC abc = (tABC)3;
	Func(abc);
	tXYZ xyz = (tXYZ)3;
	Func(xyz);

	Base* d = new Blah::Derived;
	int x = sizeof(Base);
	d->Hello();
	RTTICompleteObjectLocator* loc = TypeInfo(d);
	printf("%s\n", loc->pTypeDescriptor->name);
	printf("%s\n", UndecorateSymbol(loc->pTypeDescriptor->name));
	loc->pTypeDescriptor->spare = 0;
	delete d;

	InitTimer();

	int size = 32 * 32 * 32 * sizeof(int);
	char* src = new char[size];
	char* dest = new char[size];

	__int64 a = GetTimer();
	memcpy(dest, src, size);
	__int64 b = GetTimer();

	printf("%d\n", b - a);

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("Page size: %d\n", si.dwPageSize);

	BinaryInsertionSortTest();

	unsigned char buffer_size = 32;
	unsigned char W = 9;
	unsigned char R = 10;
	unsigned char S = 9;
	bool space_left = (char)(W - R) <= (buffer_size - 1);
	printf("space left: %d\n", space_left);

	for (int i = 0; i < 30; i++)
		printf("%d:%d ", i, Align(i, 48));
	printf("\n");

	int ia = 16, ib = 48;
	printf("%d\n", gcd_pos(ia, ib));
	printf("%d\n", lcm_pos(ia, ib));*/

	/*printf("MSVC\n");
	printf("\n");

	printf("%d\n", 10);
	printf("%4d\n", 10);
	printf("%4d\n", -10);
	printf("%04d\n", 10);
	printf("%04d\n", -10);
	printf("\n");

	printf("%-4dA\n", 10);
	printf("%-04dA\n", 10);
	printf("\n");

	printf("%8.4d\n", 10);
	printf("%08.4d\n", 10);
	printf("%8.4d\n", -10);
	printf("%08.4d\n", -10);
	printf("\n");

	printf("%4.8d\n", 10);
	printf("%04.8d\n", 10);
	printf("%4.8d\n", -10);
	printf("%04.8d\n", -10);
	printf("\n");

	printf("%-8.4dA\n", 10);
	printf("%-8.4dA\n", -10);
	printf("%-4.8dA\n", 10);
	printf("%-4.8dA\n", -10);
	printf("\n");

	printf("%u\n", 0xFFFFFFFF);
	printf("\n");

	printf("%x\n", 15);
	printf("%8x\n", 15);
	printf("%08x\n", 15);
	printf("%#8x\n", 15);
	printf("%#08x\n", 15);
	printf("\n");

	printf("%8.4x\n", 15);
	printf("%08.4x\n", 15);
	printf("%#8.4x\n", 15);
	printf("%#08.4x\n", 15);
	printf("\n");

	printf("%-8xA\n", 15);
	printf("%-08xA\n", 15);
	printf("%#-8xA\n", 15);
	printf("%#-08xA\n", 15);
	printf("\n");

	printf("%-8.4xA\n", 15);
	printf("%-08.4xA\n", 15);
	printf("%#-8.4xA\n", 15);
	printf("%#-08.4xA\n", 15);
	printf("\n");

	printf("%s\n", "hello");
	printf("%8s\n", "hello");
	printf("%08s\n", "hello");
	printf("%-8sA\n", "hello");
	printf("%-08sA\n", "hello");
	printf("\n");

	printf("%.4s\n", "hello");
	printf("%8.4s\n", "hello");
	printf("%08.4s\n", "hello");
	printf("%-8.4sA\n", "hello");
	printf("%-08.4sA\n", "hello");
	printf("\n");

	printf("%p\n", (void*)3);
	printf("\n");

	double e = 3.14158999999999988261834005243144929409027099609375;
	printf("%f\n", 0);
	printf("%f\n", 1);
	printf("%f\n", 1.0);
	printf("%f\n", 1.5);
	printf("%f\n", -1.0);
	printf("%f\n", -1.5);
	printf("%f\n", e);
	printf("%.0f\n", e);
	printf("%.1f\n", e);
	printf("%.2f\n", e);
	printf("%.32f\n", e);
	printf("\n");

	e = 100.34216;
	printf("%8.2f\n", e);
	printf("%08.2f\n", e);
	printf("%-8.2fA\n", e);
	printf("%-08.2fA\n", e);
	printf("%8.2f\n", -e);
	printf("%08.2f\n", -e);
	printf("%-8.2fA\n", -e);
	printf("%-08.2fA\n", -e);
	printf("\n");

	printf("%e\n", 1.0);
	printf("%e\n", 10.0);
	printf("%e\n", 100.0);
	printf("%e\n", 0.1);
	printf("%e\n", 0.01);
	printf("%e\n", 0.001);
	printf("\n");

	printf("%e\n", -1.0);
	printf("%e\n", -10.0);
	printf("%e\n", -100.0);
	printf("%e\n", -0.1);
	printf("%e\n", -0.01);
	printf("%e\n", -0.001);
	printf("\n");

	e = 65536.0 * 65536.0;
	e *= e;
	e *= e;
	e *= e;
	e *= e;
	printf("%e\n", e);
	printf("%e\n", 1.0 / e);
	printf("\n");

	printf("%.0e\n", e);
	printf("%.2e\n", e);
	printf("%8.2e\n", e);*/

//	return 0;
//}*/



// From
/*struct Yup
{
	int things_count;
	int* things;
};


// Add new type for dynamic and fixed arrays
template <typename T, int N=-1>
struct array
{
	T& operator[] (int);
	const T& operator[] (int) const;
	static const int count = N;
	T data[N];
};
template <typename T>
struct array<T, -1>
{
	T& operator[] (int);
	const T& operator[] (int) const;
	int count;
	T* data;
};


// To
struct Yup2
{
	// Dynamic
	array<int> things;

	// Fixed
	array<int, 3> more_things;
};



#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	array<int> x;
	array<int, 3> y;
	y.count;
	x.count;

	// Defaults
	FILTERKEYS keys = { sizeof(FILTERKEYS) };
	keys.iDelayMSec = 400;
	keys.iRepeatMSec = 4;
	keys.dwFlags = FKF_FILTERKEYSON|FKF_AVAILABLE;

	// Override from command-line
	if (argc > 1)
		keys.iDelayMSec = atoi(argv[1]);
	if (argc > 2)
		keys.iRepeatMSec = atoi(argv[2]);

	printf("Setting Delay(%d), Repeat(%d)", keys.iDelayMSec, keys.iRepeatMSec);

	return SystemParametersInfo (SPI_SETFILTERKEYS, 0, (LPVOID) &keys, 0) != TRUE;
}*/


/*#include <stdio.h>


struct Blah
{
	Blah()
	{
		printf("Wind\n");
	}

	~Blah()
	{
		printf("Unwind\n");
	}
};

void test()
{
	throw 0;
}

#include <float.h>


int main()
{
	float a = 1.0f;
	float b = -0.0f;
	float c = a / b;
	printf("%f\n", c);
	printf("%g\n", FLT_EPSILON);
	double g = 1 / 3.0;
	printf("%.55g\n", g);
	//Blah b;
	//test();
	return 0;
}*/


/*#include <windows.h>
#include <stdio.h>

struct Log
{
	Log() : root_caller(0)
	{
	}

	void SetRootCaller()
	{
		CaptureStackBackTrace(2, 1, &root_caller, NULL);
	}

	void Out(const char* text)
	{
		void* stack_trace[10];
		CaptureStackBackTrace(2, 10, stack_trace, NULL);

		for (int i = 0; i < 10 && stack_trace[i] != root_caller; i++)
			printf("   ");
		
		printf("%s\n", text);
	}

	void* root_caller;
} log;

void Nested2()
{
	log.Out("Nested2");
}

void Nested()
{
	log.Out("Nested");
	Nested2();
}

int main()
{
	log.SetRootCaller();

	log.Out("A");
	Nested();
	log.Out("C");
	return 1;
}*/

/*#include <stdio.h>

struct Base
{
	virtual void A() = 0;
	virtual void B() = 0;
};


struct Derived : public Base
{
	void A()
	{
		printf("A()\n");
	}
	void B()
	{
		printf("A()\n");
	}
};


struct VTbl
{
	void (*A)();
	void (*B)();
};


struct Container
{
	VTbl* vtbl;
};

template <class A, class B>
struct Disallow
{
	enum { val = 0 };
};
template <typename A>
struct Disallow<A, A>
{
};

template <class T> void Fn() { Disallow<T, int>::val; }

int main()
{
	Base* base = new Derived();
	base->A();
	base->B();

	Fn<int>();

	Disallow<char, int>::val;

	Container* container = new Container();
	container->vtbl->A();
	container->vtbl->B();

	return 0;
}*/


/*#include <windows.h>
#include <stdio.h>
#include <conio.h>


#pragma comment(lib, "winmm.lib")


LARGE_INTEGER g_PerformanceCounterStart;


class Timer
{
public:
	Timer()
		: m_PerformanceCounterScale(0)
	{
		LARGE_INTEGER performance_frequency;
		QueryPerformanceFrequency(&performance_frequency);
		m_PerformanceCounterScale = 1000000.0 / performance_frequency.QuadPart;
	}


	__int64 Get() const
	{
		LARGE_INTEGER performance_count;
		QueryPerformanceCounter(&performance_count);
		return performance_count.QuadPart;
	}


	double IntervalAsMs(__int64 start, __int64 end) const
	{
		return (end - start) * m_PerformanceCounterScale / 1000.0;
	}


	void Sleep(__int64 microseconds) const
	{
		__int64 start = Get();
		while (true)
		{
			__int64 delta = Get() - start;

			double time_left = microseconds - delta * m_PerformanceCounterScale;
			if (time_left <= 0)
				break;
			else if (time_left < 1000)
				// Notify CPU that code is spinning
				// (allows CPU to reschedule memory/HT ops, improves power consumption)
				_mm_pause();
			else if (time_left < 1500)
				// Yields to other threads on the same processor, potentially giving up a time slice
				SwitchToThread();
			else
				// Yields to other threads elsewhere
				Sleep(1);

			// and possibly larger sleeps...
		}
	}



private:
	double m_PerformanceCounterScale;
};


__declspec(thread) int XX = 0;
__declspec(thread) int XY = 1;


int main()
{
	Timer timer;

	//timeBeginPeriod(1);

	XX = 3;
	XX = 4;

	while (!_kbhit())
	{
		// Random microsecond sleep time
		__int64 sleep_time = rand()%32000;

		__int64 start = timer.Get();
		//Sleep(1);
		timer.Sleep(sleep_time);
		__int64 end = timer.Get();

		__int64 delta = sleep_time - (end - start);

		printf("%f\n", timer.IntervalAsMs(0, delta));
	}

	//timeEndPeriod(1);

	_getch();
	_getch();

	return 0;
}*/


/*#include <stdio.h>
#include <functional>


template <typename R, typename A0>
struct ICaller1
{
	virtual R Execute(A0&&) = 0;
};

template <typename F, typename R, typename A0>
struct Caller1 : public ICaller1<R, A0>
{
	Caller1(const F& f)
		: f(f)
	{
	}

	virtual R Execute(A0&& a0)
	{
		return f(static_cast<A0&&>(a0));
	}

	F f;
};

template <typename R, typename A0 = void, typename A1 = void>
struct Function
{
	typedef R ReturnType;
};

template <typename R, typename A0>
struct Function<R (A0)>
{
	typedef R ReturnType;

	Function()
		: caller(nullptr)
	{
	}

	template <typename F>
	Function(F f)
		: caller(nullptr)
	{
		caller = new Caller1<F, R, A0>(f);
	}

	R operator () (A0&& a0)
	{
		return caller->Execute(static_cast<A0&&>(a0));
	}

	ICaller1<R, A0>* caller;
};


void FA(int& x)
{
	x = 5;
}
void FB(int, int)
{
}


void doit(Function<void(int&)> a)
{
	int x = 3;
	a(x);
	printf("%d\n", x);
}*/
/*void doit(std::function<void(int&)> a)
{
	int x = 3;
	a(x);
	printf("%d\n", x);
}*/

/*void test()
{
	doit(FA);
	doit([](int&) { });
}*/


#include <stdio.h>
#include <assert.h>
#include <new>
#include <functional>
#include <string.h>
#include <tuple>


// First parameter deduced, the rest have to be specified
template <typename LAMBDA, typename RETURN, typename... Arguments>
RETURN Call(LAMBDA& x, Arguments&&... arguments)
{
	return x(arguments...);
}

	#define CORE_NEEDS_COPY_CONSTRUCTOR(type)	((__is_class(type) || __is_union(type)) && !__has_trivial_copy(type))
	#define CORE_NEEDS_DESTRUCTOR(type)			((__is_class(type) || __is_union(type)) && !__has_trivial_destructor(type))


	template <typename TYPE> void Destruct(void* dest)
	{
		((TYPE*)dest)->~TYPE();
	}
	template <typename TYPE> void CopyConstruct(void* dest, const void* src)
	{
		new ((TYPE*)dest) TYPE(*(TYPE*)src);
	}


	typedef void (*CopyConstructPtr)(void* dest, const void* src);
	typedef void (*DestructPtr)(void* dest);


// Don't allow copy-construction
// Make this move-only because I can't think of any scenario that would want to keep the function pointer alive AFTER copy
// It's also much more efficient
// And it also means you don't have to carry the allocator around and that the allocation for all components can be contigous
struct State
{
	State()
		: destructor(0)
		, data(0)
	{
	}

	template <typename Functor>
	State(Functor&& functor, void* allocated_data)
		: destructor(CORE_NEEDS_DESTRUCTOR(Functor) ? &Destruct<Functor> : nullptr)
		, data(allocator_data)
	{
		// Copy the functor and its state over
		// This could be a function pointer, lambda or class functor
		new (data) Functor(functor);
	}

	State(State&& rhs)
		: destructor(rhs.destructor)
		, data(rhs.data)
	{
		rhs.destructor = 0;
		rhs.data = 0;
	}

	~State()
	{
		if (destructor)
			destructor(data);
	}

	DestructPtr destructor;
	void* data;
};


template <typename Type>
struct StateImpl : public State
{
	Type data;
};


struct Context
{
	template <typename LAMBDA>
	Context(LAMBDA& lambda)
		: copy_construct(CORE_NEEDS_COPY_CONSTRUCTOR(LAMBDA) ? &CopyConstruct<LAMBDA> : nullptr)
		, destructor(CORE_NEEDS_DESTRUCTOR(LAMBDA) ? &Destruct<LAMBDA> : nullptr)
	{
		static_assert(sizeof(lambda) <= sizeof(data), "too big");
		new (data) LAMBDA(lambda);
	}
	Context(const Context& rhs)
		: copy_construct(rhs.copy_construct)
		, destructor(rhs.destructor)
	{
		if (copy_construct)
			copy_construct(data, rhs.data);
		else
			memcpy(data, rhs.data, 32);
	}
	~Context()
	{
		if (destructor)
			destructor(data);
	}

	CopyConstructPtr copy_construct;
	DestructPtr destructor;

	char data[32];
};

// TODO: Use VoidCall for FunctionRef


template <typename Return, typename... Arguments>
struct Caller
{
	// Destructor needed for cleaning up lambda state or bound arguments
	virtual ~Caller() { }

	virtual Return Call(Arguments&&...) = 0;
};


template <typename Functor, typename Return, typename... Arguments>
struct FunctorCaller : public Caller<Return, Arguments...>
{
	FunctorCaller(const Functor& functor)
		: functor(functor)
	{
	}

	// TODO: const?
	Return Call(Arguments&&... arguments) final
	{
		return functor(std::forward<Arguments>(arguments)...);
	}

	Functor functor;
};


template <typename Functor, typename... Arguments>
struct FunctionNew
{
};


template <typename Return, typename... Arguments>
struct FunctionNew<Return(Arguments...)>
{
	// TODO: Move constructor
	//       OR just expose the caller pointer

	FunctionNew()
		: caller(nullptr)
	{
	}

	template <typename Functor>
	FunctionNew(const Functor& functor)
		: caller(new FunctorCaller<Functor, Return, Arguments...>(functor))
	{
	}

	template <typename Functor, typename... BoundArguments>
	FunctionNew(const Functor& functor, BoundArguments&&... arguments)
	{
	}

	Return operator () (Arguments&&... arguments)
	{
		return caller->Call(std::forward<Arguments>(arguments)...);
	}

	Caller<Return, Arguments...>* caller;
};


template <typename F, typename ... Arguments>
struct Function
{
};

template <typename R, typename... Arguments>
struct Function<R(Arguments...)>
{
	using VoidCall = R(*)(void*, Arguments&&...);

	template <typename LAMBDA>
	Function(LAMBDA& lambda)
		: caller((VoidCall)&Call<LAMBDA, R, Arguments&&...>)
		, context(lambda)
	{
	}

	template <typename OBJECT>
	Function(R(OBJECT::*function)(Arguments...), OBJECT* object)
		: context(object)
	{
	}

	// Free functions require a context pointer so that operator() doesn't need to vary
	// and can be a plain function pointer
	template <typename CONTEXT>
	Function(R(*function)(CONTEXT, Arguments...), CONTEXT&& ctx)
		: caller((VoidCall)function)
		, context(ctx)
	{

	}

	Function(const Function& rhs)
		: caller(rhs.caller)
		, context(rhs.context)
	{
	}

	R operator () (Arguments&&... arguments)
	{
		return caller(context.data, static_cast<Arguments&&>(arguments)...);
	}

	VoidCall caller;
	Context context;
};

struct Test
{
	Test()
	{
		printf("Open\n");
	}
	Test(const Test&)
	{
		printf("OpenCopy\n");
	}
	~Test()
	{
		printf("Close\n");
	}
};


template <typename TYPE>
struct UnRef
{
	typedef TYPE Type;
};
template <typename TYPE>
struct UnRef<TYPE&>
{
	typedef TYPE Type;
};

struct StoredCall
{
	// Allow cleanup of captured data
	virtual ~StoredCall() { }

	// Execute stored call
	virtual void Execute() /*const*/ = 0;
};


template <typename First, typename... Rest>
struct Tuple : public Tuple<Rest...>
{
	using Type = First;
	using Next = Tuple<Rest...>;

	Tuple(First&& first, Rest&&... rest)
		: Next(std::forward<Rest>(rest)...)
		, a(first)
	{
	}

	First a;
};
template <typename First>
struct Tuple<First>
{
	using Type = First;

	Tuple(First&& first)
		: a(first)
	{
	}

	First a;
};


template <int Index, typename TupleType>
struct IndexTuple : public IndexTuple<Index - 1, typename TupleType::Next>
{
};
template <typename TupleType>
struct IndexTuple<0, TupleType>
{
	using Tuple = TupleType;
	using Type = typename Tuple::Type;
};


template <int Index, typename TupleType>
typename IndexTuple<Index, TupleType>::Type Get(TupleType& tuple)
{
	return ((IndexTuple<Index, TupleType>::Tuple&)tuple).a;
}


void TupleTest()
{
	using T = Tuple<int, char, float, double>;
	T x(1, 2, 3, 4);

	int a = Get<0>(x);
	char b = Get<1>(x);
	float c = Get<2>(x);
	double d = Get<3>(x);
}

namespace helper
{
	template <int... Is>
	struct index {};

	template <int N, int... Is>
	struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

	template <int... Is>
	struct gen_seq<0, Is...> : index<Is...>{};
}
template <typename Function, typename... Arguments>
struct FunctionCall : public StoredCall
{
	FunctionCall(Function f, Arguments&&... arguments)
		: function(f), arguments(std::forward<Arguments>(arguments)...)
	{
	}
	virtual ~FunctionCall()
	{
	}
	template <int... Is>
	void func(helper::index<Is...>)
	{
		function(Get<Is>(arguments)...);
	}
	void Execute() // const
	{
		func(helper::gen_seq<sizeof...(Arguments)>{});
	}
	Function function;
	Tuple<Arguments...> arguments;
	//typename UnRef<A0>::Type a0;
};


int Blah(int a, int b)
{
	return 0;
}

int NewIt(int x)
{
	printf("%d\n", x * 5);
	return x * 5;
}
struct TestTest
{
	TestTest()
		: y(72)
	{
	}
	void It(int x)
	{
		printf("%d\n", x + y);
	}

	int y;
};

int main()
{
	TupleTest();

	Test t;
	int y = 3;
	auto x = [&, t](int i)
	{
		printf("%d\n", y + i);
	};

	printf("Defined\n");

	typedef int(*BlahPtr)(int, int);
	FunctionCall<BlahPtr, int, int> f(&Blah, 1, 2);
	f.Execute();

	//Function<void(int)> callnow(x);

	Function<void(int)> caller(x);
	//Function<void(int)> calle2r(caller);
	caller(7);
	TestTest tt;
	Function<void(int)> ccc(&TestTest::It, &tt);
	ccc(4);
	//calle2r(8);
	//std::function<void(int)> caller(x);
	//std::function<void(int)> calle2r(caller);
	//caller(7);
	//calle2r(8);


	FunctionNew<int(int)> n0;
	FunctionNew<int(int)> n1(&NewIt);
	n1(5);
	FunctionNew<int(int)> n2([](int x){ return x * 6; });
	n2(11);
	FunctionNew<int()> n3(&NewIt, 5);
}