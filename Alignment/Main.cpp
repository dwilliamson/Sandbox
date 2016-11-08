
#include <stdio.h>
#include <stddef.h>


struct float2
{
	float x, y;
};
struct float3
{
	float x, y, z;
};
struct float4
{
	float x, y, z, w;
};


__declspec(align(16))
struct Data
{
	float4 a;
	float3 b;
	float2 c;
	float  d;
	float  e;
};


#define O(x) printf("%s : %d\n", #x, offsetof(Data, x));


int main()
{
	printf("%d\n", sizeof(Data));

	O(a);
	O(b);
	O(c);
	O(d);
	O(e);

	return 0;
}