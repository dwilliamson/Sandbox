
#pragma once


#include "PreReq.h"

// std::function, std::bind
#include <functional>


namespace std_version
{
	void CodeGenTest()
	{
		printf("\nstd_version ----------------------------------------------\n");

		std::function<void(int, char, float, double, const Emitter&)> f0(&PrintTuple);
		f0(10, 65, 3.141592f, 2.718281, Emitter());

		std::function<void()> f1(std::bind(PrintTuple, 10, 65, 3.141592f, 2.718281, Emitter()));
		f1();

		//Queue<std::function<void()>> q;
		//q.push(std::bind(PrintTuple, 10, 65, 3.141592f, 2.718281, Emitter()));
		//std::function<void()> p = q.pop();
		//p();
	}
}
