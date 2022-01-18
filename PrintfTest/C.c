
typedef struct my_struct
{
	char foo[16000];
} my_struct;

my_struct get_foo(int N)
{
	my_struct s;
	for (int i = 0; i < N; i++)
	{
		s.foo[i] = 123;
	}
	return s;
}

void do_foo(int N)
{
	my_struct s = get_foo(N);
	for (int i = 0; i < N; i++)
	{
		printf("%d\n", s.foo[i]);
	}
}