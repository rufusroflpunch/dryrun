#ifndef DRYRUN_H
#define DRYRUN_H

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>
#include <set>
#include <chrono>

#define BEGIN_TEST() int main(int argc, char **argv) { test_suite suite; bench_suite benchmarks
#define END_TEST() dry_run(argc, argv, suite); dry_run_benchmarks(argc, argv, benchmarks); return 0; }
#define TEST(desc, func) suite.add_test(desc, func)
#define BENCHMARK(desc, reps, func)  benchmarks.add_benchmark(desc, reps, func)

#define DRY_RUN_MAJ_VER 0
#define DRY_RUN_MIN_VER 1

#define COLOR_OFF "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"


void print_help()
{
  std::cout << "Dry Run " << DRY_RUN_MAJ_VER << "." << DRY_RUN_MIN_VER << "\n"
            "Usage: test [options]\n"
            "Options:\n\n"
            "-d\t\tRun tests in determinate mode (non-randomized).\n"
            "-r\t\tRepeat tests multiple times. Ex: test -r 10\n"
            "-h,--help\tShow this help screen.\n"
            "-c\t\tUse ANSI colors for easier reading.";
}

struct test_case
{
  std::string desc;
  std::function<bool ()> test;

  test_case(std::string Desc, std::function<bool ()> Test)
  {
    desc = Desc;
    test = Test;
  }

};
bool operator==(const test_case& lhs, const test_case& rhs)
{
  return lhs.desc == rhs.desc;
}
bool operator<(const test_case& lhs, const test_case& rhs)
{
  return lhs.desc < rhs.desc;
}

struct test_suite
{
  std::vector<test_case> test_list;
  std::function<void ()> before_func, before_each_func;
  std::function<void ()> after_func, after_each_func;

  void add_test(std::string desc, std::function<bool ()> test)
  {
    test_list.push_back(test_case(desc, test));
  }

  void before(std::function<void ()> func)
  {
    before_func = func;
  }

  void before_each(std::function<void ()> func)
  {
    before_each_func = func;
  }

  void after(std::function<void ()> func)
  {
    after_func = func;
  }

  void after_each(std::function<void ()> func)
  {
    after_each_func = func;
  }
};

struct bench_case
{
  std::string desc;
  std::function<void ()> test;
  int reps;

  bench_case(std::string Desc, int Reps, std::function<void ()> Test)
  {
    desc = Desc;
    reps = Reps;
    test = Test;
  }
};

struct bench_suite
{
  std::vector<bench_case> bench_list;

  void add_benchmark(std::string desc, int reps, std::function<void ()> test)
  {
    bench_list.push_back(bench_case(desc, reps, test));
  }
};


void dry_run(int argc, char** argv, test_suite& tests)
{
  if (tests.test_list.empty()) return;

  if (tests.before_func)
  {
    tests.before_func();
  }

  bool determinate = false;
  bool colors = false;
  int repeat = 0;

  for(int i = 0; i < argc; i++)
  {
    if(std::string(argv[i]) == "-d") determinate = true;
    else if(std::string(argv[i]) == "-r") repeat = std::stoi(std::string(argv[++i]));
    else if (std::string(argv[i]) == "-c") colors = true;
    else if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      print_help();
      return;
    }
  }

  if(repeat > 0)
  {
    std::vector<test_case> copied_tests;

    for(int i = 0; i < repeat; i++)
      std::copy(tests.test_list.begin(), tests.test_list.end(), std::back_inserter(copied_tests));

    tests.test_list = copied_tests;
  }

  std::srand(time(0));
  if(!determinate) std::random_shuffle(tests.test_list.begin(), tests.test_list.end());

  std::vector<test_case> failures;
  for(auto it : tests.test_list)
  {
    if (tests.before_each_func)
    {
      tests.before_each_func();
    }
    if(it.test())
    {
      if (colors) std::cout << COLOR_GREEN;
      std::cout << ".";
      if (colors) std::cout << COLOR_OFF;
    }
    else
    {
      if (colors) std::cout << COLOR_RED;
      std::cout << "F";
      std::cout << COLOR_OFF;
      failures.push_back(it);
    }
    if (tests.after_each_func)
    {
      tests.after_each_func();
    }
  }
  std::cout << "\n\n";

  if (tests.after_func)
  {
    tests.after_func();
  }

  if(!failures.empty())
  {
    std::sort(failures.begin(), failures.end());
    auto uniq_end = std::unique(failures.begin(), failures.end());
    if (colors) std::cout << COLOR_MAGENTA;
    std::cout << "Failures: " << std::endl;
    if (colors) std::cout << COLOR_OFF;
    for(auto it = failures.begin(); it != uniq_end; it++)
    {
      if (colors) std::cout << COLOR_RED;
      std::cout << it->desc << std::endl;
      if (colors) std::cout << COLOR_OFF;
    }
  }

  std::cout << "\n\n";
}

void dry_run_benchmarks(int argc, char **argv, bench_suite &benchmarks)
{
  bool colors = false;
  for(int i = 0; i < argc; i++)
  {
    if (std::string(argv[i]) == "-c") colors = true;
    else if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      print_help();
      return;
    }
  }

  if (benchmarks.bench_list.empty()) return;

  if (colors) std::cout << COLOR_MAGENTA;
  std::cout << "Benchmarks:" << std::endl;
  if (colors) std::cout << COLOR_OFF;
  std::cout << "TIME\t\t\tREPETITIONS\tDESCRIPTION\n\n";

  for (auto i : benchmarks.bench_list)
  {
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for (int count = i.reps; count > 0; count --)
      i.test();

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    if (colors) std::cout << COLOR_GREEN;
    std::cout << elapsed_seconds.count() << "s";
    if (colors) std::cout << COLOR_OFF;
    std::cout << "\t\t" << i.reps << "\t\t" << i.desc << std::endl;
  }

  std::cout << "\n\n";
}

#endif
