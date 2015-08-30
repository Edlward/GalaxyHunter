#include <QtTest/QTest>
#include "utils/sequence.h"

class TestSequence : public QObject {
Q_OBJECT
private slots:
  void testSequenceRunningAllExecutions();
  void testRunLast();
  void testSequenceHaltingAtFirstError();
  void testDontRunErrorFunctionOnNoErrors();
  void testRunErrorfunction();
  void testWithDifferentDefault();
  void testWithDifferentReturnValueInitialization();
  void testWithDifferentComparison();
};

void TestSequence::testSequenceRunningAllExecutions()
{
  int executions = 0;
  {
    sequence<int, 0> s{{
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
      { [&executions]{ executions++; return 0; }},
    }};
    QCOMPARE(executions, 0);
  }
  QCOMPARE(executions, 3);
}

void TestSequence::testRunLast()
{
  int executions = 0;
  bool run_last = false;
  sequence<int, 0>{{
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
  }}.run_last([&run_last] { run_last=true; } );
  QCOMPARE(executions, 3);
  QVERIFY(run_last);
}

void TestSequence::testSequenceHaltingAtFirstError()
{
  int executions = 0;
  sequence<int, 0> {{
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 1; }},
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
  }};
  QCOMPARE(executions, 2);
}

void TestSequence::testDontRunErrorFunctionOnNoErrors()
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
    s.on_error([&errorCode](int, std::string){ errorCode = 10; });
  }
  QCOMPARE(executions, 4);
  QCOMPARE(errorCode, 0);
}

void TestSequence::testRunErrorfunction()
{
  int executions = 0;
  int errorCode = 0;
  sequence<int, 0>{{
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 0; }},
    { [&executions]{ executions++; return 0; }},
  }}.on_error([&errorCode](int e, std::string){ errorCode = e+1; });
  QCOMPARE(executions, 2);
  QCOMPARE(errorCode, 4);
}

void TestSequence::testWithDifferentDefault()
{
  int executions = 0;
  sequence<int, 10>{{
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 10; }},
  }};
  QCOMPARE(executions, 2);
}

void TestSequence::testWithDifferentReturnValueInitialization()
{
  int executions = 0;
  sequence<int, 10>{{
    { [&executions]{ executions++; return 10; }, "", 0},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return 10; }},
  }};
  QCOMPARE(executions, 1);
}

void TestSequence::testWithDifferentComparison()
{
  int executions = 0;
  sequence<int, 0, std::greater_equal<int>>{{
    { [&executions]{ executions++; return 10; }, "", 0},
    { [&executions]{ executions++; return 3; }},
    { [&executions]{ executions++; return 10; }},
    { [&executions]{ executions++; return -1; }},
    { [&executions]{ executions++; return 10; }},
  }};
  QCOMPARE(executions, 4);
}


QTEST_MAIN(TestSequence)

#include "testsequence.moc"

