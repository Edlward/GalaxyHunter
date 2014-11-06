#define BOOST_TEST_MAIN
#include "test_helper.h"
#include "utils/sequence.h"

BOOST_AUTO_TEST_CASE(TestSequenceRunningAllExecutions)
{
  int executions = 0;
  {
    sequence<int, 0> s{{
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
    }};
    BOOST_REQUIRE_EQUAL(0, executions);
  }
  BOOST_REQUIRE_EQUAL(3, executions);
}

BOOST_AUTO_TEST_CASE(TestSequenceHaltingAtFirstError)
{
  int executions = 0;
  sequence<int, 0> {{
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 1; }},
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
  }};
  BOOST_REQUIRE_EQUAL(2, executions);
}

BOOST_AUTO_TEST_CASE(TestDontRunErrorFunctionOnNoErrors)
{
  int executions = 0;
  int errorCode = 0;
  {
    sequence<int, 0> s{{
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
    }};
    s.run_on_error([&errorCode](int){ errorCode = 10; });
  }
  BOOST_REQUIRE_EQUAL(4, executions);
  BOOST_REQUIRE_EQUAL(0, errorCode);
}

BOOST_AUTO_TEST_CASE(TestRunErrorfunction)
{
  int executions = 0;
  int errorCode = 0;
  sequence<int, 0>{{
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
  }}.run_on_error([&errorCode](int e){ errorCode = e+1; });
  BOOST_REQUIRE_EQUAL(2, executions);
  BOOST_REQUIRE_EQUAL(4, errorCode);
}


BOOST_AUTO_TEST_CASE(TestWithDifferentDefault)
{
  int executions = 0;
  sequence<int, 10>{{
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 10; }},
  }};
  BOOST_REQUIRE_EQUAL(2, executions);
}
BOOST_AUTO_TEST_CASE(TestWithDifferentReturnValueInitialization)
{
  int executions = 0;
  sequence<int, 10>{{
    { [&executions]{ executions++; return 10; }, 0},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 10; }},
  }};
  BOOST_REQUIRE_EQUAL(1, executions);
}