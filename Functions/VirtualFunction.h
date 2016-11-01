
#pragma once


#include "PreReq.h"
#include <functional>


namespace virtual_fn
{
	// Caller interface, allowing binding of functions, methods, lambdas, functors, etc...
	template <typename Return, typename... Arguments>
	struct Caller
	{
		// Destructor needed for cleaning up lambda state or bound arguments
		virtual ~Caller() { }

		virtual Return Call(Arguments&&...) = 0;
	};


	// With no argument binding, calls functions, lambdas and functors
	template <typename Functor, typename Return, typename... Arguments>
	struct FunctorCaller : public Caller<Return, Arguments...>
	{
		FunctorCaller(const Functor& functor)
			: functor(functor)
		{
		}

		Return Call(Arguments&&... arguments) final
		{
			return functor(Forward<Arguments>(arguments)...);
		}

		Functor functor;
	};


	template <typename Functor, typename Return, typename... BoundArguments>
	struct BoundFunctorCaller : public Caller<Return>
	{
		BoundFunctorCaller(const Functor& functor, BoundArguments&&... arguments)
			: functor(functor)
			, arguments(Forward<Decay<BoundArguments>>(arguments)...)
		{
		}

		Return Call() final
		{
			return CallFunctorWithTuple<Functor, Return>(functor, arguments, Sequence<sizeof...(BoundArguments)>());
		}

		Functor functor;
		Tuple<Decay<BoundArguments>...> arguments;
	};


	// With no argument binding, calls methods
	template <typename Object, typename Method, typename Return, typename... Arguments>
	struct MethodCaller : public Caller<Return, Arguments...>
	{
		MethodCaller(Object* object, Method method)
			: object(object)
			, method(method)
		{
		}

		Return Call(Arguments&&... arguments) final
		{
			return (object->*method)(Forward<Arguments>(arguments)...);
		}

		Object* object;
		Method method;
	};


	// With no argument binding, calls methods
	template <typename Object, typename Method, typename Return, typename... BoundArguments>
	struct BoundMethodCaller : public Caller<Return>
	{
		BoundMethodCaller(Object* object, Method method, BoundArguments&&... in_arguments)
			: object(object)
			, method(method)
			, arguments(Forward<BoundArguments>(in_arguments)...)
		{
		}

		Return Call() final
		{
			return CallMethodWithTuple<Object, Method, Return>(object, method, arguments, Sequence<sizeof...(BoundArguments)>());
		}

		Object* object;
		Method method;
		Tuple<Decay<BoundArguments>...> arguments;
	};


	struct framed
	{
	};
	struct float3
	{
	};
	struct Instance
	{
		void SetTransform(const framed&, const float3&)
		{
		}
	};


	template <typename Functor, typename... Arguments>
	struct Function;


	// Specialisation with function signature
	template <typename Return, typename... Arguments>
	struct Function<Return(Arguments...)>
	{
		Function()
			: caller(nullptr)
		{
		}

		template <typename Object, typename... FnArguments, typename... BoundArguments>
		Function(Object* object, Return(Object::*method)(FnArguments...), BoundArguments&&... arguments)
			: caller(new BoundMethodCaller<Object, Return(Object::*)(FnArguments...), Return, BoundArguments...>
				(object, method, Forward<BoundArguments>(arguments)...))
		{
		}

		// Const methods with bound object and arguments
		/*template <typename Object, typename... FnArguments, typename... BoundArguments>
		Function(const Object* object, Return(Object::*method)(FnArguments...) const, BoundArguments&&... arguments)
			: caller(new BoundMethodCaller<const Object, Return(Object::*)(BoundArguments...) const, Return, BoundArguments...>
			(object, method, Forward<BoundArguments>(arguments)...))
		{
		}*/

		// Functions
		/*template <typename Functor>
		Function(const Functor& functor)
			: caller(new FunctorCaller<Functor, Return, Arguments...>(functor))
		{
		}*/

		/*template <typename Functor, typename... BoundArguments>
		Function(const Functor& functor, BoundArguments&&... arguments)
			: caller(new BoundFunctorCaller<Functor, Return, BoundArguments...>(functor, Forward<BoundArguments>(arguments)...))
		{
		}*/

		/*template <typename Object>
		Function(Object* object, Return(Object::*method)(Arguments...))
			: caller(new MethodCaller<Object, Return(Object::*)(Arguments...), Return, Arguments...>(object, method))
		{
		}

		template <typename Object>
		Function(const Object* object, Return(Object::*method)(Arguments...) const)
			: caller(new MethodCaller<const Object, Return(Object::*)(Arguments...) const, Return, Arguments...>(object, method))
		{
		}*/

		~Function()
		{
			delete caller;
		}

		// Non-copyable
		Function(const Function&) = delete;
		Function& operator = (const Function&) = delete;

		// Movable
		Function(Function&& rhs)
			: caller(rhs.caller)
		{
			rhs.caller = 0;
		}
		Function& operator = (Function&& rhs)
		{
			delete caller;
			caller = rhs.caller;
			rhs.caller = 0;
		}

		// Call with required arguments
		Return operator () (Arguments&&... arguments)
		{
			return caller->Call(Forward<Arguments>(arguments)...);
		}

		Caller<Return, Arguments...>* caller;
	};


		template <typename BoundArgument>
		void Blah(BoundArgument&& in_argument)
		{
			Tuple<Decay<BoundArgument>> argument(Forward<BoundArgument>(in_argument));
		}


	void blah(Instance* instance, const framed& a0, const float3& a1)
	{
		framed A0;
		float3 A1;
		Function<void()> fff(instance, &Instance::SetTransform, A0, A1);

		Blah(a0);
	}


	void CodeGenTest()
	{
		//Instance i;
		//framed a0;
		//float3 a1;
		//Function<void()> fff(&i, &Instance::SetTransform, a0, a1);

		printf("\nvirtual_fn -----------------------------------------------\n");

		//Function<void(int, char, float, double, const Emitter&)> f0(&PrintTuple);
		//f0(10, 65, 3.141592f, 2.718281, Emitter());

		//Function<void()> f1(&PrintTuple, 10, 65, 3.141592f, 2.718281, Emitter());
		//f1();
	}
}


namespace virtual_fn
{
	void CopyInt(int a)
	{
		printf("CopyInt:            %d\n", a);
	}
	void RefInt(int& a)
	{
		printf("RefInt before:      %d\n", a);
		a++;
		printf("RefInt after:       %d\n", a);
	}
	void ConstRefInt(const int& a)
	{
		printf("ConstRefInt:        %d\n", a);
	}


	struct FCopyInt
	{
		void operator () (int a)
		{
			printf("CopyInt:            %d\n", a);
		}
	};
	struct FRefInt
	{
		void operator () (int& a)
		{
			printf("RefInt before:      %d\n", a);
			a++;
			printf("RefInt after:       %d\n", a);
		}
	};
	struct FConstRefInt
	{
		void operator () (const int& a)
		{
			printf("ConstRefInt:        %d\n", a);
		}
	};


	struct Methods
	{
		Methods()
			: b(77)
		{
		}

		void CopyInt(int a)
		{
			printf("CopyInt:            %d\n", a * b);
		}
		void RefInt(int& a)
		{
			printf("RefInt before:      %d\n", a);
			a += b;
			printf("RefInt after:       %d\n", a);
		}
		void ConstRefInt(const int& a)
		{
			printf("ConstRefInt:        %d\n", a + b);
		}

		void CCopyInt(int a) const
		{
			printf("CopyInt:            %d\n", a * b);
		}
		void CRefInt(int& a) const
		{
			printf("RefInt before:      %d\n", a);
			a += b;
			printf("RefInt after:       %d\n", a);
		}
		void CConstRefInt(const int& a) const
		{
			printf("ConstRefInt:        %d\n", a + b);
		}

		int b;
	};


	void Test()
	{
		printf("\nvirtual_fn -----------------------------------------------\n\n");

		/*Function<void(int, char, float, double, Emitter)> f0(&PrintTuple);
		f0(10, 65, 3.141592f, 2.718281, Emitter());

		// Free functions

		Function<void()>{&CopyInt, 1}();

		Function<void(int)>{&CopyInt}(10);
		int a = 10;
		Function<void(int&)>{&RefInt}(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>{&ConstRefInt}(10);

		// Captureless lambda

		Function<void(int)>([](int a){ printf("LambdaCopy:         %d\n", a); })(10);
		Function<void(int&)>([](int& a)
		{
			printf("LambdaRef before:   %d\n", a);
			a++;
			printf("LambdaRef after:    %d\n", a);
		})(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>([](const int& a){ printf("LambdaConstRef:     %d\n", a); })(10);

		// Capturing lambda

		int cap = 8;
		Function<void(int)>([=](int a){ printf("CapLambdaCopy:      %d\n", a * cap); })(10);
		Function<void(int&)>([=](int& a)
		{
			printf("CapLambdaRef before:%d\n", a);
			a += cap;
			printf("CapLambdaRef after: %d\n", a);
		})(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>([=](const int& a){ printf("LambdaConstRef:     %d\n", a / cap); })(10);

		// Functors

		Function<void(int)>(FCopyInt{})(10);
		Function<void(int&)>(FRefInt{})(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>(FConstRefInt{})(10);

		// Methods

		Methods m;
		Function<void(int)>{&m, &Methods::CopyInt}(10);
		Function<void(int&)>{&m, &Methods::RefInt}(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>{&m, &Methods::ConstRefInt}(10);

		// Const Methods

		const Methods cm;
		Function<void(int)>{&cm, &Methods::CCopyInt}(10);
		Function<void(int&)>{&cm, &Methods::CRefInt}(a);
		printf("Modified a:         %d\n", a);
		Function<void(const int&)>{&cm, &Methods::CConstRefInt}(10);*/
	}
}