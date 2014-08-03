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

// Marcos for easier test writing
#define BEGIN_TEST() int main(int argc, char **argv) { test_suite suite; bench_suite benchmarks
#define END_TEST() dry_run(argc, argv, suite); dry_run_benchmarks(argc, argv, benchmarks); return 0; }
#define TEST(desc, func) suite.add_test(desc, func)
#define BENCHMARK(desc, reps, func)  benchmarks.add_benchmark(desc, reps, func)
#define BEFORE(func) suite.before(func)
#define BEFORE_EACH(func) suite.before_each(func)
#define AFTER(func) suite.after(func)
#define AFTER_EACH(func) suite.after_each(func)

// Current dry run version.
#define DRY_RUN_MAJ_VER 0
#define DRY_RUN_MIN_VER 2

// Color constants
#define COLOR_OFF "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"


/*
 * This prints the help for the testable application.
 */
void print_help()
{
  // Since both dry_run and dry_run_benchmark can process --help and show this help screen,
  // this static variable ensure that typing --help will only run this once and not twice.
  static bool already_printed = false;  
  if (!already_printed)
    std::cout << "Dry Run " << DRY_RUN_MAJ_VER << "." << DRY_RUN_MIN_VER << "\n"
            "Usage: test [options]\n"
            "Options:\n\n"
            "-d\t\tRun tests in determinate mode (non-randomized).\n"
            "-r\t\tRepeat tests multiple times. Ex: test -r 10\n"
            "-h,--help\tShow this help screen.\n"
            "-c\t\tUse ANSI colors for easier reading.\n"
            "-b\t\tShow brief output (less verbose).\n"
            "-B\t\tRun only benchmarks.\n"
            "-T\t\tRun only tests.\n\n";
  already_printed = true;
}

/*
 * Each test_case stores a runnable test with a description and a function object to actually.
 * The function object is the test that is run. It must return a bool. True is considered a
 * successful test, false is unsuccessful.
 */
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

/*
 * These operator overloads are required for sorting and randomized
 * shuffling of the test_cases. Since function objects can't be compared,
 * the comparisons are done solely based upon the description of the
 * test_case.
 */
bool operator==(const test_case& lhs, const test_case& rhs)
{
  return lhs.desc == rhs.desc;
}
bool operator<(const test_case& lhs, const test_case& rhs)
{
  return lhs.desc < rhs.desc;
}

/// The primary class for running test cases.
/** The primary way to create a test is to created a test_suite, then call
 * add_test on the objects to add consecutive tests.
 */
struct test_suite
{
  std::vector<test_case> test_list;
  std::function<void ()> before_func, before_each_func;
  std::function<void ()> after_func, after_each_func;

  /** The method used to add runnable tests. 
   * @param   desc    A string description (typically short) to describe the test.
   * @param   test    A function object, usually a C++11 lambda, that returns a bool.
   */
  void add_test(std::string desc, std::function<bool ()> test)
  {
    test_list.push_back(test_case(desc, test));
  }

  /** Perform an action before the battery of tests, once.
   * @param   func    Function object to perform.
   */
  void before(std::function<void ()> func)
  {
    before_func = func;
  }

  /** Perform an action before each test.
   * @param   func    Function object to perform.
   */
  void before_each(std::function<void ()> func)
  {
    before_each_func = func;
  }

  /** Perform an action once after all tests have run.
   * @param   func    Function object to perform.
   */
  void after(std::function<void ()> func)
  {
    after_func = func;
  }

  /** Perform an action after every individual test.
   * @param   func    Function object to perform.
   */
  void after_each(std::function<void ()> func)
  {
    after_each_func = func;
  }
};

// Much like the test_case class, this represents a runnable benchmark.
// Along with the string description, it also uses a function object (which
// return void) and an integer reps that describes the number of times to
// repeat the benchmark.
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

// The primary class for running benchmarks.
/** The primary way to create a benchmark is to created a bench_suite, then call
 * add_benchmark on the objects to add consecutive benchmarks.
 */
struct bench_suite
{
  std::vector<bench_case> bench_list;

  void add_benchmark(std::string desc, int reps, std::function<void ()> test)
  {
    bench_list.push_back(bench_case(desc, reps, test));
  }
};


/** This is the meat and potatoes method of Dry Run testing. After setting up the tests
 * to run, evoke this function to actually run them. This function processes command
 * line arguments for the user as well, which is why argc and argv are passed in
 * from main().
 * Output: The test results
 * @param   argc    Passed in from main.
 * @param   argv    Passed in from main.
 * @param   tests   The test_suite created that contains the actual tests to run.
 */
void dry_run(int argc, char** argv, test_suite& tests)
{
  bool determinate = false;
  bool colors = false;
  bool brief = false;
  bool bench_only = false;
  int repeat = 0;

  // Parse cmd line options.
  for(int i = 0; i < argc; i++)
  {
    if(std::string(argv[i]) == "-d") determinate = true;
    else if(std::string(argv[i]) == "-r") repeat = std::stoi(std::string(argv[++i]));
    else if (std::string(argv[i]) == "-c") colors = true;
    else if (std::string(argv[i]) == "-b") brief = true;
    else if (std::string(argv[i]) == "-B") bench_only = true;
    else if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      print_help();
      return;
    }
  }

  // Quit if there are no tests to run, or running only benchmarks.
  if (tests.test_list.empty() || bench_only) return;

  // Execute the command that is run before the entire suite.
  if (tests.before_func)
  {
    tests.before_func();
  }

  // Duplicate the test vector as required to repeat the tests the requested number of times.
  if(repeat > 0)
  {
    std::vector<test_case> copied_tests;

    for(int i = 0; i < repeat; i++)
      std::copy(tests.test_list.begin(), tests.test_list.end(), std::back_inserter(copied_tests));

    tests.test_list = copied_tests;
  }

  // Randomize the test vector
  std::srand(time(0));
  if(!determinate) std::random_shuffle(tests.test_list.begin(), tests.test_list.end());

  // Print test header
  if (colors) std::cout << COLOR_MAGENTA;
  if (!brief) std::cout << "Tests:\n";
  if (colors) std::cout << COLOR_OFF;

  // Conduct the actual tests.
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

  // Execute the after command
  if (tests.after_func)
  {
    tests.after_func();
  }

  // Output a more detailed view of the failures.
  if(!failures.empty() && !brief)
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
  else    // If there are no failure, let us know.
  {
    if (colors) std::cout << COLOR_GREEN;
    if (!brief) std::cout << "All Passed.";
    if (colors) std::cout << COLOR_OFF;
  }

  std::cout << "\n\n";
}

/** The method used to execute benchmarks. After setting up the benchmarks
 * to run, evoke this function to actually run them. This function processes command
 * line arguments for the user as well, which is why argc and argv are passed in
 * from main(). This work in tandem with dry_run() so there is no conflic if
 * both benchmarks and tests are run.
 * Output: The test results
 * @param   argc    Passed in from main.
 * @param   argv    Passed in from main.
 * @param   tests   The bench_suite created that contains the actual tests to run.
 */
void dry_run_benchmarks(int argc, char **argv, bench_suite &benchmarks)
{
  bool colors = false;
  bool test_only = false;
  bool brief = false;

  // Parse cmd line args.
  for(int i = 0; i < argc; i++)
  {
    if (std::string(argv[i]) == "-c") colors = true;
    else if (std::string(argv[i]) == "-T") test_only = true;
    else if (std:: string(argv[i]) == "-b") brief = true;
    else if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      print_help();
      return;
    }
  }

  // Exit if there are no benchmarks to run, or running only tests.
  if (benchmarks.bench_list.empty() || test_only) return;

  if (colors) std::cout << COLOR_MAGENTA;
  if (!brief) std::cout << "Benchmarks:" << std::endl;
  if (colors) std::cout << COLOR_OFF;
  if (!brief) std::cout << "TIME\t\t\tREPETITIONS\tDESCRIPTION\n\n";

  // Actually execute the benchmarks.
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
