# Dry Run
## Quick and simple unit tests/benchmarks for C++

### About

This a single-file, header only library to write and run quick and easy unit tests and/or benchmarks for your code. It's extremely simple and nearly boiler-plate free! You also have the option to use idiomatic C++ or macros.

### Macro Approach

This is probably the quickest way to get a test off the ground. Let's look at a quick example:

```
#include <dryrun.h>

BEGIN_TEST();

TEST("Test #1", [] { /* Test code here */ });
TEST("Test #2", [] { /* Test code here */ });
BENCHMARK("Benchmark #1", 1000, [] { /* Benchmark code here executes 1000 times */ });

END_TEST();
```

After this, all you need to do is compile the file and run the executable. It will execute the tests and benchmarks, then output the results. By default, it runs them in randomized order, and displays the results with no color. This behavior can be changed.

The benchmark code specified above will be executed 1000 times, but you can specify any integer you like.

**NOTE**: The functions passed into TEST and BENCHMARK are C++11 lambdas. The return types for tests must be **bool** (true for success). It is **void** for benchmarks (not return type).

There are also four additional macros that can be used as well. These apply only to tests, not benchmarks.

```
BEFORE([] { /* Code that is executed once, before the suite of tests */ });
AFTER([] { /* Code that is executed once, after the suite of tests */ });
BEFORE_EACH([] { /* Code that is executed before each test */ });
AFTER_EACH([] { /* Code that is executed after each test */ });
```

Like benchmarks, the return type for the function objects is **void**.

### Idiotmatic approach

It's not much different than above for normal C++:

```
#include <dryrun.h>

int main(int argc, char **argv)
{
    // Create the objects that hold the tests and benchmarks.
    test_suite tests;
    bench_suite benchmarks;

    // Add the tests and benchmarks to the queue. Note, you can specify an integer
    // with the benchmarks, and it will execute the benchmark that many consecutive
    // times.
    tests.add_test("Test #1", [] { /* Test code here */ });
    benchmarks.add_benchmark("Benchmark #1", 1000, [] { /* Benchmark code here */ });

    // As above, you can specify code to be run before and after the suite of tests,
    // as well as before and after each individual test.
    tests.before([] { });
    tests.before_each([] { });
    tests.after([] { });
    tests.after_each([] { });

    // Finally, execute the tests and benchmarks. Make sure to pass in argc and argv
    // so that the generated application properly handles command line arguments
    dry_run(argc, argv, tests);
    dry_run_benchmarks(argc, argv, benchmarks);
    
    return 0;
}
```

As you can see, there is a tad more boilerplate with this method, but it is not much more.

### Command Line

The final generated test application can accept the following parameters:

```
Usage: test [options]
Options:

-d		    Run tests in determinate mode (non-randomized).
-r		    Repeat tests multiple times. Ex: test -r 10
-h,--help	Show this help screen.
-c		    Use ANSI colors for easier reading.
-b		    Show brief output (less verbose).
-B		    Run only benchmarks.
-T		    Run only tests.
```

### License

This is distributed as-is in the public domain. I am not liable for any unforseen issues caused by this code. You are not required to redistribute the source code for this publically and you may include it in your projects without attributing credit, though credit would certainly be appreciated by me, the author, none-the-less.
