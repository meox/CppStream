#include <iostream>
#include <utility>
#include <stdexcept>
#include <functional>
#include <memory>



namespace z
{
	using value_t = unsigned int;

	struct list;
	using list_ptr = std::shared_ptr<list>;
	using delay_t = std::function<list_ptr()>;

	struct list
	{
		list(value_t v) : value{v}, is_value{true} {}
		list(delay_t v) : fvalue{v}, is_fvalue{true} {}

		list(value_t v, const list_ptr& n)
		: value{v}, is_value{true}, next{n}
		{}

		list(delay_t v, const list_ptr& n)
		: fvalue{v}, is_fvalue{true}, next{n}
		{}

		value_t value;
		delay_t fvalue;
		bool is_value{false};
		bool is_fvalue{false};
		list_ptr next{nullptr};
	};

	delay_t delay(const list_ptr& exp)
	{
		return [exp] () -> list_ptr{
			return exp;
		};
	}

	delay_t delay(value_t v)
	{
		return delay(std::make_shared<list>(v));
	}

	bool stream_null(const list_ptr& s) { return !s; }

	list_ptr cons(value_t x)
	{
		return std::make_shared<list>(x);
	}

	list_ptr cons(value_t x, const list_ptr& r)
	{
		return std::make_shared<list>(x, r);
	}

	list_ptr cons(value_t x, delay_t y)
	{
		return std::make_shared<list>(x, std::make_shared<list>(y));
	}

	list_ptr cons(const list_ptr& x, delay_t y)
	{
		x->next = std::make_shared<list>(y);
		return x;
	}

	list_ptr car(const list_ptr& s)
	{
		if (stream_null(s))
			return nullptr;
		else
			return s;
	}

	list_ptr cdr(const list_ptr& s)
	{
		if (stream_null(s))
			return nullptr;
		else
			return s->next;
	}

	list_ptr force(const list_ptr& e)
	{
		if (!e)
			return nullptr;

		if (e->is_fvalue)
			return e->fvalue();
		else
			return nullptr;
	}

	void print(const value_t & e)
	{
		std::cout << e << " ";
	}

	void print(const list_ptr& s)
	{
		if (stream_null(s))
			return;

		print(s->value);
		print(cdr(s));
	}

	void println(const list_ptr& s)
	{
		print(s);
		std::cout << std::endl;
	}

	void println(const value_t& v)
	{
		print(v);
		std::cout << std::endl;
	}

	list_ptr stream_cons(auto x, delay_t fy)
	{
		return cons(x, fy);
	}

	list_ptr stream_car(const list_ptr& s)
	{
		return car(s);
	}

	list_ptr stream_cdr(const list_ptr& s)
	{
		return force(cdr(s));
	}

	void stream_print(const list_ptr& s)
	{
		if (stream_null(s))
			return;

		print(s->value);
		stream_print(stream_cdr(s));
	}

	void stream_println(const list_ptr& s)
	{
		stream_print(s);
		std::cout << std::endl;
	}

	list_ptr stream_ref(const list_ptr& s, size_t n)
	{
		if (n == 0)
			return stream_car(s);
		else
			return stream_ref(stream_cdr(s), n - 1);
	}

	template <typename F>
	list_ptr stream_map(const list_ptr& s, F f)
	{
		if (stream_null(s))
			return s;
		else
			return stream_cons(f(stream_car(s)), [=]{
				return stream_map(stream_cdr(s), f);
			});
	}

	template <typename F>
	value_t stream_reduce(const list_ptr& s, F f, value_t acc)
	{
		value_t r{acc};
		if (stream_null(s))
			return r;
		else
			return stream_reduce(stream_cdr(s), f, f(stream_car(s)->value, acc));
	}

	template <typename F>
	list_ptr stream_filter(const list_ptr& s, F pred)
	{
		if (stream_null(s))
			return s;

		value_t c = stream_car(s)->value;
		if (pred(c))
		{
			return stream_cons(c,  [=]{
				return stream_filter(stream_cdr(s), pred);
			});
		}
		else
		{
			return stream_filter(stream_cdr(s), pred);
		}
	}

	list_ptr s_interval(size_t l, size_t h)
	{
		if (l > h)
			return nullptr;
		else
			return stream_cons(l, [=]{ return s_interval(l+1, h); });
	}
}


bool divides(uint x, uint n)
{
	return n % x == 0;
}

bool is_even(uint n)
{
	return n % 2 == 0;
}

uint small_divisor(uint n)
{
	if (is_even(n))
		return 2;
	else
	{
		for (uint e = 3; e*e <= n; e += 2)
			if (divides(e, n)) return e;
	}
	return n;
}


bool is_prime(uint n)
{
	return n > 1 and small_divisor(n) == n;
}

unsigned int sum_prime(size_t l, size_t h)
{
	using namespace z;

	auto s_int = s_interval(l, h);
	auto s_prime = stream_filter(s_int, is_prime);

	stream_println(s_prime);
	return stream_reduce(s_prime, [](value_t acc, value_t c){
		return acc+c;
	}, 0);
}

int main()
{
	using namespace z;
	//auto s = cons(0, cons(1, cons(7)));
	//println(s);

	auto sa = cons(7, []{
		return cons(1, []{
			return cons(2);
		});
	});

	stream_println(sa);

	auto r = stream_ref(sa, 1);
	println(r->value);

	auto m = stream_map(sa, [](const list_ptr& e){
		return e->value+1;
	});

	stream_println(m);

	auto s_int = s_interval(1, 10);
	stream_println(s_int);


	std::cout << "sum(prime(2, 21)): " << sum_prime(2, 21) << std::endl;
	return 0;
}