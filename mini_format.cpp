
#ifndef HELLO_MINI_PRINTER_H
#define HELLO_MINI_PRINTER_H

#include <iostream>
#include <limits.h>
#include <string.h>
#include <string>
#include <sstream>
#include <tuple>
#include <charconv>

struct Functor
{
	template <typename T>
	inline void operator()(std::string& out, const T& t) const
	{
		out.append(static_cast<std::ostringstream&&>(std::ostringstream() << t).str());
	}

	inline void operator()(std::string& out, std::nullptr_t t) const { out += "nullptr"; }

	//    inline void operator()(std::string &out, const char *t) const { out += t; }
	template <size_t N>
	inline void operator()(std::string& out, const char (&t)[N]) const
	{
		out.append(t, t[N - 1] == '\0' ? N - 1 : N);
	}

	inline void operator()(std::string& out, const std::string& t) const { out += t; }

	inline void operator()(std::string& out, int t) const
	{
		auto sz0 = out.size();
		out.resize(out.size() + 12);
		char* pst = &out.front() + sz0;
		char* pend = std::to_chars(pst, &out.back() + 1, t, 10).ptr;
		out.resize(sz0 + pend - pst);
	}
};

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type for_index(int, std::tuple<Tp...>&, FuncT, std::string&)
{
}

template <std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if < I<sizeof...(Tp), void>::type for_index(int index, std::tuple<Tp...>& t, FuncT f, std::string& s)
{
	if (index == 0)
		f(s, std::get<I>(t));
	else
		for_index<I + 1, FuncT, Tp...>(index - 1, t, f, s);
}

template <typename... Args>
std::string& mini_format_impl(std::string& outs, const std::string& fmt, std::tuple<Args...>&& args)
{
	size_t sz = sizeof...(Args);
	size_t start = 0;
	size_t i = 0;
	bool autoIndex = false;
	bool manIndex = false;
	for (; start < fmt.size();)
	{
		std::size_t pos = fmt.find('{', start), pos2;
		if (pos == std::string::npos)
		{
			pos2 = fmt.find('}', start);
			if (pos2 != std::string::npos && pos2 != fmt.size() - 1 && fmt[pos2 + 1] == '}')
			{
				outs.append(fmt.begin() + start, fmt.begin() + pos2);
				outs.append("}");
				start = pos2 + 2;
				continue;
			}
			break;
		}
		if (pos == fmt.size() - 1) break;

		if (fmt[pos + 1] == '{')
		{
			outs.append(fmt.begin() + start, fmt.begin() + pos);
			outs.append("{");
			start = pos + 2;
		} else
		{
			pos2 = fmt.find('}', pos);
			if (pos2 == std::string::npos) break;
			if (pos2 == pos + 1)
			{
				if (i >= sz) { return outs = "Error format!Too much {}\n"; };
				if (manIndex) { return outs = "Error format!Cannot mixed format {0} or {}\n"; }
				outs.append(fmt.begin() + start, fmt.begin() + pos);
				for_index(i, args, Functor(), outs);
				start = pos + 2 /* length of "{}" */;
				i++;
				autoIndex = true;
				continue;
			} else
			{
				char* numEnd = nullptr;
				unsigned long idx = strtoul(&fmt[pos + 1], &numEnd, 0);
				if (idx == ULONG_MAX)
				{
					break;
				} else if (idx == 0)
				{
					if (!(pos2 == pos + 2 && fmt[pos + 1] == '0')) { break; }
					// "{0}"
				}
				if (i >= sz) { return outs = "Error format!Too much {}\n"; };
                if (idx >= sz) { return outs = "Error format!{} index overflow\n"; };
				if (autoIndex) { return outs = "Error format!Cannot mixed format {0} or {}\n"; }
				outs.append(fmt.begin() + start, fmt.begin() + pos);
				for_index(idx, args, Functor(), outs);
				start = pos2 + 1 /* length of "{123}" */;
				manIndex = true;
				continue;
			}
		}
	}
	return outs.append(fmt.begin() + start, fmt.end());
}

template <typename... Args>
inline void mini_format_to(std::string& outs, const std::string& fmt, Args&&... args)
{
	outs.clear();
	mini_format_impl(outs, fmt, std::forward_as_tuple(args...));
}

template <typename... Args>
inline std::string mini_format(const std::string& fmt, Args&&... args)
{
	std::string outs;
	mini_format_impl(outs, fmt, std::make_tuple(args...));
	return outs;
}

template <typename... Args>
inline void mini_print(const std::string& fmt, const Args&... args)
{
	std::string str = mini_format(fmt, args...);
	fwrite(str.c_str(), str.size(), 1, stdout);
}


#endif


struct Student
{
	Student() {}

	Student(const Student& rhs) : id(rhs.id), name(rhs.name) {}

	Student(Student&& rhs) : id(rhs.id), name(std::move(rhs.name)) {}

	friend std::ostream& operator<<(std::ostream& os, const Student& m)
	{
		os << "id:" << m.id << " name:" << m.name;
		return os;
	}

	int id{ 100 };
	std::string name{ "Jack" };
};


void func()
{
	auto funcname = __FUNCTION__;
	constexpr int funcsize = sizeof(__FUNCTION__);
	mini_print("func {},len {},type {}\n", funcname, funcsize, typeid(decltype(__FUNCTION__)).name());
	return void();
}


#include <chrono>
#include <thread>
#include <atomic>

using namespace std;

int main()
{
	mini_print("");
	mini_print("hello world\n");
	mini_print("hello world\n", 1, 32, 24234);
	mini_print("hello {{\n");
	mini_print("hello }}\n");
	mini_print("\n");
	mini_print("hello {\n");
	mini_print("hello }\n");
	mini_print("hello {}\n");
	mini_print("hello }{\n");
	mini_print("\n");
	mini_print("hello {{{{\n");
	mini_print("hello }}}}\n");
	mini_print("\n");
	mini_print("hello {}, my name is {},age {},height {}\n", "world", "KJ", 20, 175.585);
	mini_print("hello {}, my name is {}\n", std::string("KJ"), std::string("OMG"));
	mini_print("hello {0}, my name is {1}\n", std::string("C"), std::string("C++"));
	mini_print("hello {1}, my name is {0}\n", std::string("C"), std::string("C++"));
	mini_print("hello {10}, my name is {2},id {0}\n", 10086, std::string("C"), std::string("C++"), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mini_print("hello {1}, my name is {{}}\n", std::string("C"), std::string("C++"));
	mini_print("hello {1}, my name is {}\n", std::string("C"), std::string("C++"));
	mini_print("hello {}, my name is {1}\n", std::string("C"), std::string("C++"));
    mini_print("hello {}, my name is {3}\n", std::string("C"), std::string("C++"));

	mini_print("hello {{, my name is {}\n\n", std::string("C"), std::string("C++"));
	{
		func();
		Student st0;
		mini_print("st0: {}\n\n", st0);
	}

	using namespace std::chrono;
	constexpr uint64_t NUM = 6e6;
	constexpr uint64_t THREADS = 4;

	string dst(500, 'A');
	char cdst[500];
	string emytys;
	std::ostringstream os;
	std::ostringstream::sync_with_stdio(false);
	os << dst;
	{
		string fmt = "{},hello {},my name is {},age {}\n";
		high_resolution_clock::time_point st{ high_resolution_clock::now() };
		for (uint64_t i = 0; i < NUM; i++)
		{
			mini_format_to(dst, fmt, "test for many times", "world", "jack", 22);
		}
		high_resolution_clock::time_point ed{ high_resolution_clock::now() };
		mini_print("format {} ms\n", (duration_cast<milliseconds>(ed - st)).count());
	}
	{
		high_resolution_clock::time_point st{ high_resolution_clock::now() };
		for (uint64_t i = 0; i < NUM; i++)
		{
			os.str(emytys);
			os << "test for many times" << ",hello " << "world" << " my name is " << "jack" << ",age " << 22 << '\n';
		}
		high_resolution_clock::time_point ed{ high_resolution_clock::now() };
		mini_print("ostringstream operator<< {} ms\n", (duration_cast<milliseconds>(ed - st)).count());
	}
	{
		high_resolution_clock::time_point st{ high_resolution_clock::now() };
		for (uint64_t i = 0; i < NUM; i++)
		{
			sprintf(cdst, "%s,hello %s,my name is %s,age %d\n", "test for many times", "world", "jack", 22);
		}
		high_resolution_clock::time_point ed{ high_resolution_clock::now() };
		mini_print("printf {} ms\n", (duration_cast<milliseconds>(ed - st)).count());
	}
	mini_print("{}{}{}", dst, os.str(), cdst);

	return 0;
}
