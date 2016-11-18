
#include <vector>
#include <random>
#include <algorithm>
#include <cstdio>
#include <chrono>


void Print(const std::vector<int>& values)
{
	for (size_t i = 0; i < values.size(); i += 16)
	{
		for (size_t j = 0; j < 16; j++)
			printf("%3d ", values[i + j]);

		printf("\n");
	}
}


void Verify(const std::vector<int>& values)
{
	for (size_t i = 1; i < values.size(); i++)
	{
		if (values[i] < values[i - 1])
			printf("%d failed\n", i);
	}
}


size_t BitonicSort_0(std::vector<int>& values)
{
	size_t nb_passes = 0;
	for (size_t sub_array_size = 2; sub_array_size <= values.size(); sub_array_size *= 2)
	{
		for (size_t compare_offset = sub_array_size / 2; compare_offset > 0; compare_offset /= 2)
		{
			for (size_t i = 0; i < values.size(); i++)
			{
				size_t j = i ^ compare_offset;
				if (j > i)
				{
					if ((i & sub_array_size) == 0)
					{
						if (values[i] > values[j])
							std::swap(values[i], values[j]);
					}
					else
					{
						if (values[i] < values[j])
							std::swap(values[i], values[j]);
					}
				}
			}

			nb_passes++;
		}
	}

	return nb_passes;
}


size_t BitonicSort_1(std::vector<int>& values)
{
	size_t nb_passes = 0;
	for (size_t sub_array_size = 2; sub_array_size <= values.size(); sub_array_size *= 2)
	{
		for (size_t compare_offset = sub_array_size / 2; compare_offset > 0; compare_offset /= 2)
		{
			for (size_t i = 0; i < values.size() / 2; i++)
			{
				size_t j = i / compare_offset * compare_offset + i;
				if ((i & (sub_array_size >> 1)) == 0)
				{
					if (values[j] > values[j + compare_offset])
						std::swap(values[j], values[j + compare_offset]);
				}
				else
				{
					if (values[j] < values[j + compare_offset])
						std::swap(values[j], values[j + compare_offset]);
				}
			}

			nb_passes++;
		}
	}

	return nb_passes;
}


int main()
{
	// Generate random number list
	std::uniform_int_distribution<int> distribution(0, 999);
	std::default_random_engine generator;
	std::vector<int> values(64*64*64);
	std::generate(values.begin(), values.end(), [&] { return distribution(generator); });

	//Print(values);

	auto start = std::chrono::steady_clock::now();

	//size_t nb_passes = BitonicSort_0(values);
	size_t nb_passes = BitonicSort_1(values);
	//std::sort(values.begin(), values.end());

	auto end = std::chrono::steady_clock::now();

	printf("\n");
	//Print(values);

	printf("\n");
	Verify(values);

	printf("Parallel Outer Complexity: %f\n", std::log2(values.size()));
	printf("Number of Passes: %d\n", nb_passes);
	printf("Time Taken: %fms\n", std::chrono::duration<float, std::milli>(end - start).count());
}
