
#pragma once


// printf
#include <stdio.h>


// Types can inherit from these to easily aggregate true/false values
struct True		{ static const bool value = true; };
struct False	{ static const bool value = false; };

// Checks to see if a type is a C array
template <typename TYPE> struct IsArray					: False { };
template <typename TYPE> struct IsArray<TYPE[]>			: True{ };
template <typename TYPE, int N> struct IsArray<TYPE[N]> : True{ };

// Checks to see if a type is a function type
template <typename> struct IsFunction										: False { };
template <class Ret, class... Args> struct IsFunction<Ret(Args...)>			: True { };
template <class Ret, class... Args> struct IsFunction<Ret(Args..., ...)>	: True { };

// Remove const from a type if it exists
template <typename TYPE> struct RemoveConstImpl				{ using Type = TYPE; };
template <typename TYPE> struct RemoveConstImpl<const TYPE>	{ using Type = TYPE; };
template <typename TYPE> using RemoveConst = typename RemoveConstImpl<TYPE>::Type;

// Remove volatile from a type if it exists
template <typename TYPE> struct RemoveVolatileImpl					{ using Type = TYPE; };
template <typename TYPE> struct RemoveVolatileImpl<volatile TYPE>	{ using Type = TYPE; };
template <typename TYPE> using RemoveVolatile = typename RemoveVolatileImpl<TYPE>::Type;

// Remove const/volatile from a type if it exists
template <typename TYPE> using RemoveCV = RemoveConst<RemoveVolatile<TYPE>>;

// Remove a pointer from a type if it exists
template <typename TYPE> struct RemovePointerImpl			{ using Type = TYPE; };
template <typename TYPE> struct RemovePointerImpl<TYPE*>	{ using Type = TYPE; };
template <typename TYPE> using RemovePointer = typename RemovePointerImpl<TYPE>::Type;

// Remove a reference from a type if it exists
template <typename TYPE> struct RemoveReferenceImpl			{ using Type = TYPE; };
template <typename TYPE> struct RemoveReferenceImpl<TYPE&>	{ using Type = TYPE; };
template <typename TYPE> struct RemoveReferenceImpl<TYPE&&>	{ using Type = TYPE; };
template <typename TYPE> using RemoveReference = typename RemoveReferenceImpl<TYPE>::Type;

// Remove array qualifiers if present
template <typename TYPE> struct RemoveExtentImpl					{ using Type = TYPE; };
template <typename TYPE> struct RemoveExtentImpl<TYPE[]>			{ using Type = TYPE; };
template <typename TYPE, int N> struct RemoveExtentImpl<TYPE[N]>	{ using Type = TYPE; };
template <typename TYPE> using RemoveExtent = typename RemoveExtentImpl<TYPE>::Type;

// Adds a pointer qualifier to a type
template <typename TYPE> using AddPointer = typename RemoveReference<TYPE>*;

// Conditional type selection
template <bool, typename A, typename B> struct IfImpl { using Type = B; };
template<typename A, typename B> struct IfImpl<true, A, B> { using Type = A;  };
template <bool Cond, typename A, typename B> using If = typename IfImpl<Cond, A, B>::Type;

// Applies lvalue to rvalue, array to pointer and function to function pointer conversions for storage
// of forwarded parameters
template<class TYPE>
struct DecayImpl
{
	using ReflessType = RemoveReference<TYPE>;
	using Type =
		If<IsArray<ReflessType>::value,
			RemoveExtent<ReflessType>*, If<IsFunction<ReflessType>::value,
				AddPointer<ReflessType>, RemoveCV<ReflessType >> >;
};
template <typename TYPE> using Decay = typename DecayImpl<TYPE>::Type;


// Forwards arguments between functions
template <typename TYPE>
inline TYPE&& Forward(RemoveReference<TYPE>& arg)
{
	return static_cast<TYPE&&>(arg);
}
template <typename TYPE>
inline TYPE&& Forward(RemoveReference<TYPE>&& arg)
{
	return static_cast<TYPE&&>(arg);
}


template <typename... Types>
struct Tuple;


// A Tuple is effectively a type linked list with values stored for each type
// Repeatedly split a type list into (i, i+1...N) pairs, recursively inheriting
// from the second half until there's nothing left; similar to Lisp's car/cdr.
template <typename First, typename... Rest>
struct Tuple<First, Rest...> : public Tuple<Rest...>
{
	using Value = First;
	using Next = Tuple<Rest...>;

	Tuple(const Tuple&) = delete;
	Tuple& operator = (const Tuple&) = delete;

	// Constructor for unpacking an argument list
	// Ensure perfect parameter forwarding is used
	template <typename FirstFrom, typename... RestFrom>
	Tuple(FirstFrom&& first, RestFrom&&... rest)
		: Next(Forward<RestFrom>(rest)...)
		, value(first)
	{
	}

	Value value;
};

// Terminate a tuple's type list
template <>
struct Tuple<>
{
};


// Given an index, return a Tuples N'th type and value type
// Decay the Index value using recursive inheritance until zero
template <int Index, typename TupleType>
struct IndexTuple : public IndexTuple<Index - 1, typename TupleType::Next>
{
};
template <typename TupleType>
struct IndexTuple<0, TupleType>
{
	using Tuple = TupleType;
	using Value = typename Tuple::Value;
};


// Return type is the I'th tuple value type
// Cast incoming tuple to that and return the value
template <int Index, typename Tuple>
typename IndexTuple<Index, Tuple>::Value&& Get(Tuple& tuple)
{
	using IndexType = IndexTuple<Index, Tuple>;
	using ValueType = typename IndexType::Value;
	using TupleType = typename IndexType::Tuple;
	// RVO only applies to locals with auto-storage so must use forward
	return Forward<ValueType>(((TupleType&)tuple).value);
}


// A type for storing a list of integers
template <int... Indices>
struct Index
{
};


// Generate an Index type with increasing indices given an initial count
template <int N, int... Indices>
struct Sequence : Sequence<N - 1, N - 1, Indices...>
{
};
// The termination type inherits from Index specifying all the collected indices
template <int... Indices>
struct Sequence<0, Indices...> : Index<Indices...>
{
};


// Parameters:
//
//    1. Function to call.
//    2. List of parameters to pass stored in a tuple
//    3. Generated index sequence used to retrieve tuple parameters
template <typename Functor, typename Return, typename Tuple, int... Indices>
Return CallFunctorWithTuple(const Functor& functor, Tuple& tuple, Index<Indices...>)
{
	return functor(Get<Indices>(tuple)...);
}
template <typename Object, typename Method, typename Return, typename Tuple, int... Indices>
Return CallMethodWithTuple(Object* object, Method method, Tuple& tuple, Index<Indices...>)
{
	return (object->*method)(Get<Indices>(tuple)...);
}


struct EmitterActivity
{
	void Print() const
	{
		printf("Constructions:      %d\n", NbConstructorCalls);
		printf("Copy Constructions: %d\n", NbCopyConstructorCalls);
		printf("Move Constructions: %d\n", NbMoveConstructorCalls);
		printf("Destructions:       %d\n", NbDestructorCalls);
	}

	int NbConstructorCalls = 0;
	int NbCopyConstructorCalls = 0;
	int NbMoveConstructorCalls = 0;
	int NbDestructorCalls = 0;
};


struct Emitter
{
	static EmitterActivity Activity;

	Emitter() : value(123)
	{
		printf("Emitter:            constructor\n");
		Activity.NbConstructorCalls++;
	}
	Emitter(const Emitter& rhs) : value(rhs.value)
	{
		printf("Emitter:            copy constructor\n");
		Activity.NbCopyConstructorCalls++;
	}
	Emitter(Emitter&& rhs) : value(rhs.value)
	{
		printf("Emitter:            move constructor\n");
		Activity.NbMoveConstructorCalls++;
	}
	~Emitter()
	{
		printf("Emitter:            destructor\n");
		Activity.NbDestructorCalls++;
	}

	int value;
};


void PrintIndices(int a, int b, int c, int d, int e)
{
	printf("Indices:            %d, %d, %d, %d, %d\n", a, b, c, d, e);
}


void PrintTuple(int a, char b, float c, double d, const Emitter& e)
{
	printf("Tuple:              %d, %d, %f, %f, %d\n", a, b, c, d, e.value);
}


// Just enough functionality to evaluate codegen
// Doesn't work in any form as a usable type
template <typename Type>
struct Queue
{
	Queue()
		: write(data)
		, read(0)
	{
	}

	void* Allocate(int data_size)
	{
		int write_size = data_size + sizeof(int);
		if (write + write_size > data + sizeof(data))
			write = data;

		*(int*)write = data_size;
		write += sizeof(int);

		char* allocation = write;
		write += data_size;
		return allocation;
	}

	char data[1024];
	char* write;
	int read;

	// Want the minimal code generated for all test cases so use moves
	// Also demonstrates the use case I'm after where functions are unsable after push
	/*void push(Type&& value)
	{
		data[write] = std::move(value);
		printf("Pushed %x to %d\n", &value, write);
		write = (write + 1) & (sizeof(data) - 1);
	}

	// Implicit move on return
	Type pop()
	{
		Type value = std::move(data[read]);
		printf("Popped %x from %d\n", &value, read);
		read = (read + 1) & (sizeof(data) - 1);
		return value;
	}

	Type data[4];
	int write;
	int read;*/
};