
#pragma once


#include "PreReq.h"


namespace bound_fn_custom
{
	// ------------------------------------------------------------------------------------------
	// Types required before Function can be implemented
	// ------------------------------------------------------------------------------------------


	// Parameters:
	//
	//    1. Function to call.
	//    2. List of parameters to pass stored in a tuple
	//    3. Generated index sequence used to retrieve tuple parameters
	template <typename Function, typename Tuple, int... Indices>
	void CallFunctionWithTuple(const Function& function, Tuple& tuple, Index<Indices...>)
	{
		function(Get<Indices>(tuple)...);
	}


	// ------------------------------------------------------------------------------------------


	struct FunctionCaller
	{
		// Allow cleanup of captured data
		virtual ~FunctionCaller() { }

		// Execute stored call
		virtual void Call() /*const*/ = 0;
	};


	template <typename Function, typename... Arguments>
	struct FunctionCallerImpl : public FunctionCaller
	{
		FunctionCallerImpl(Function f, Arguments&&... arguments)
			: function(f)
			, arguments(std::forward<Arguments>(arguments)...)
		{
		}
		FunctionCallerImpl(const FunctionCallerImpl& rhs)
			: function(rhs.function)
			, arguments(rhs.arguments)
		{
		}
		virtual ~FunctionCallerImpl()
		{
		}
		virtual void Call() final
		{
			CallFunctionWithTuple(function, arguments, Sequence<sizeof...(Arguments)>());
		}
		Function function;
		Tuple<Arguments...> arguments;
	};


	struct Function
	{
		Function()
			: caller(0)
		{
		}

		Function(Function&& rhs)
			: caller(rhs.caller)
		{
			rhs.caller = 0;
		}

		template <typename FunctionType, typename... Arguments>
		Function(FunctionType f, Arguments&&... arguments)
		{
			caller = new FunctionCallerImpl<FunctionType, Arguments...>(f, std::forward<Arguments>(arguments)...);
		}

		~Function()
		{
			delete caller;
		}

		void operator () ()
		{
			caller->Call();
		}

		Function& operator = (Function&& rhs)
		{
			if (this != &rhs)
			{
				caller = rhs.caller;
				rhs.caller = 0;
			}
			return *this;
		}

		// All tests geared toward moves so ensure no copies occur
		Function(const Function&) = delete;
		Function& operator = (const Function&) = delete;

		FunctionCaller* caller;
	};


	// TODO: Adapter approach


	void CodeGenTest()
	{
		printf("\nbound_fn_custom ------------------------------------------\n");
		Function f(PrintTuple, 10, 65, 3.141592f, 2.718281, Emitter());
		f();

		auto gl = [&](int){ printf("%x\n", (int)&f);  };
		Function g(gl, 3);
		g();

		Queue<Function> q;
		q.Allocate(3);
		//q.push(Function(PrintTuple, 10, 65, 3.141592f, 2.718281, Emitter()));
		//Function p = q.pop();
		//p();
	}
}


namespace bound_fn_custom
{
	// Input parameter is unused at runtime, used only at compile time to deduce the template parameter
	template <int... Indices>
	void TestIndex(Index<Indices...>)
	{
		// Unpack integers
		PrintIndices(Indices...);
	}


	// Extension of TestIndex to print tuple values using deduced indices
	template <typename Tuple, int... Indices>
	void TestIndexGet(Tuple& tuple, Index<Indices...>)
	{
		// Unpack indices and use them to index the tuple
		PrintTuple(Get<Indices>(tuple)...);
	}


	void Test()
	{
		{
			// Create a type/value list and access by index
			printf("\nTuple ----------------------------------------------------\n");
			using T = Tuple<int, char, float, double, Emitter>;
			T tuple(10, 65, 3.141592f, 2.718281, Emitter());
			int a = Get<0>(tuple);
			char b = Get<1>(tuple);
			float c = Get<2>(tuple);
			double d = Get<3>(tuple);
			Emitter e = Get<4>(tuple);
			PrintTuple(a, b, c, d, e);

			// Build a list of indices and print them and their index into the tuple
			printf("\nIndex ----------------------------------------------------\n");
			using I0 = Index<0, 1, 2, 3, 4>;
			TestIndex(I0());
			TestIndexGet(tuple, I0());

			printf("\nSequence -------------------------------------------------\n");
			using I1 = Sequence<5>;
			TestIndex(I1());
			TestIndexGet(tuple, I1());

			printf("\nCall Helper ----------------------------------------------\n");
			CallFunctionWithTuple(PrintTuple, tuple, Sequence<5>());

			printf("\nFunction -------------------------------------------------\n");
			Function f(PrintTuple, a, b, c, d, e);
			f();

			printf("\nShutdown -------------------------------------------------\n");
		}

		Emitter::Activity.Print();
	}
}
