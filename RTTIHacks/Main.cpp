
#include <windows.h>
#include <typeinfo>
#include <cstdio>
#include <cstdint>
#include <string>


struct Base
{
	virtual void Hello() = 0;
};


namespace Outer
{
	namespace Inner
	{
		class Derived : public Base
		{
			void Hello()
			{
				printf("Hello!\n");
			}
		};
	}
}


class Derived2 : public Outer::Inner::Derived
{
};


namespace clcpp
{
	namespace internal
	{
		unsigned int HashNameString(const char* name_string, unsigned int seed = 0)
		{
			return 0;
		}
	}

	struct Type
	{
	};


	struct Database
	{
		const Type* GetType(unsigned int hash)
		{
			return (const Type*)1;
		}
	};
}



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


typedef int blah(int);
blah x;

template <int Y(int)>
struct X
{
};


struct RTTIClassHierarchyDescriptor
{
    DWORD signature;      //always zero?
    DWORD attributes;     //bit 0 set = multiple inheritance, bit 1 set = virtual inheritance
    DWORD numBaseClasses; //number of classes in pBaseClassArray
    RTTIBaseClassArray* pBaseClassArray;
	X<x> x;
};


struct RTTICompleteObjectLocator
{
	DWORD signature; //always zero ?
	DWORD offset;    //offset of this vtable in the complete class
	DWORD cdOffset;  //constructor displacement offset
	TypeDescriptor* pTypeDescriptor; //TypeDescriptor of the complete class
	RTTIClassHierarchyDescriptor* pClassDescriptor; //describes inheritance hierarchy
};


RTTICompleteObjectLocator* TypeInfo(const void* ptr)
{
	void** vptr = *(void***)ptr;
	return *(RTTICompleteObjectLocator**)(vptr - 1);
}


struct RTTIRedirect
{
	// Map to equivalent clReflect type
	const clcpp::Type* type;

	// Pointer to next field to allow runtime detection of
	// whether RTTI has been redirected for this type
	TypeDescriptor* type_descriptor_ptr;

	// Copy of original type descriptor with trailing name
	TypeDescriptor type_descriptor;
};


void RedirectRTTIType(RTTICompleteObjectLocator* loc, const clcpp::Type* type)
{
	// Point to the type name
	const char* name_ptr = loc->pTypeDescriptor->name;
	uint32_t name_len = strlen(name_ptr) + 1;

	// Create a redirection object
	// Memory for this object will be cleaned up on program exit
	RTTIRedirect* rd = (RTTIRedirect*)malloc(sizeof(RTTIRedirect) + name_len);
	rd->type = type;
	rd->type_descriptor_ptr = &rd->type_descriptor;
	rd->type_descriptor = *loc->pTypeDescriptor;

	// Copy the name over, including the null terminator
	memcpy((char*)&rd->type_descriptor + sizeof(TypeDescriptor), name_ptr, name_len);

	// Remove write-protection from the region around the object locator
	MEMORY_BASIC_INFORMATION mem;
	VirtualQuery(loc, &mem, sizeof(mem));
	DWORD old_protect;
	VirtualProtect(mem.BaseAddress, mem.RegionSize, PAGE_READWRITE, &old_protect);

	// Replace existing type descriptor
	loc->pTypeDescriptor = (TypeDescriptor*)rd->type_descriptor_ptr;

	// Restore write-protection
	VirtualProtect(mem.BaseAddress, mem.RegionSize, old_protect, &old_protect);
}


RTTIRedirect* GetRTTIRedirect(RTTICompleteObjectLocator* loc)
{
	// TypeDescriptor is embedded as a field in RTTIRedirect so skip
	// back to the beginning
	TypeDescriptor* td = loc->pTypeDescriptor;
	return (RTTIRedirect*)((char*)td - offsetof(RTTIRedirect, type_descriptor));
}


bool HasRTTITypeBeenRedirected(RTTICompleteObjectLocator* loc)
{
	// Just need to verify that the RTTIRedirect type descriptor pointer is valid
	RTTIRedirect* rd = GetRTTIRedirect(loc);
	return rd->type_descriptor_ptr == &rd->type_descriptor;
}


const char* GetRTTIClassName(const char* class_name)
{
	// MSVC-specific: skip 'class'/'struct' keyword
	if (*class_name == 'c')
		return class_name + 6;
	if (*class_name == 's')
		return class_name + 7;
	return class_name;
}


template <typename TYPE>
const char* TypeHash(TYPE* ptr)
{
	const char* name = typeid(*ptr).name();
	return GetRTTIClassName(name);
}


std::string GetClassName(const char* symbol_name)
{
	const char* c = symbol_name;

	// Ensure this is a class/struct name
	if (*c++ != '.')
		return "";
	if (*c++ != '?')
		return "";
	if (*c++ != 'A')
		return "";
	char type = *c++;
	if (type != 'U' && type != 'V')
		return "";

	// Parse the symbol name
	std::string class_name, local_name;
	const char* end = symbol_name + strlen(symbol_name);
	while (c < end)
	{
		// Namespace separator?
		if (*c == '@')
		{
			// End of symbol name?
			if (++c < end && *c == '@')
				break;

			// Prefix local name and clear ready for the next part
			class_name = "::" + local_name + class_name;
			local_name.clear();
			continue;
		}

		local_name += *c++;
	}

	// Prefix last encountered local name
	return local_name + class_name;
}

clcpp::Database* g_DB;

const clcpp::Type* GetRuntimeType(const void* ptr)
{
	// Get the RTTI type and see if it's been redirected yet
	RTTICompleteObjectLocator* loc = TypeInfo(ptr);
	if (!HasRTTITypeBeenRedirected(loc))
	{
		// Undecorate the type name and use its hash to lookup the clReflect type
		std::string class_name = GetClassName(loc->pTypeDescriptor->name);
		uint32_t type_hash = clcpp::internal::HashNameString(class_name.c_str());
		const clcpp::Type* type = g_DB->GetType(type_hash);
		if (type == nullptr)
			return nullptr;

		// Redirect for future lookup
		RedirectRTTIType(loc, type);
	}

	// Constant-time lookup!
	RTTIRedirect* rd = GetRTTIRedirect(loc);
	return rd->type;
}


int main()
{
	printf("%d\n", sizeof(TypeDescriptor));
	printf("%d\n", sizeof(RTTIRedirect));

	Base* d = new Outer::Inner::Derived;
	//Base* d = new Derived2;
	d->Hello();
	RTTICompleteObjectLocator* loc = TypeInfo(d);
	GetRuntimeType(d);
	GetRuntimeType(d);
	printf("%s\n", loc->pTypeDescriptor->name);
	printf("%s\n", GetClassName(loc->pTypeDescriptor->name).c_str());
	printf("%s\n", typeid(*d).name());
	printf("%s\n", TypeHash(d));
	loc->pTypeDescriptor->spare = 0;
	delete d;
}