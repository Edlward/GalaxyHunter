#include <QtTest/QTest>
#include "telescope/indidouble.h"

class TestINDIDouble : public QObject {
Q_OBJECT
private slots:
  void testInvalidFormat();
  void testDoubleToStringPrintf();
};

void TestINDIDouble::testInvalidFormat()
{
  auto result = INDIDouble(1.5, "abcq123");
  QVERIFY2(! result, "Result should be not valid");
}


void TestINDIDouble::testDoubleToStringPrintf()
{
  QCOMPARE(INDIDouble(1.44, "%.2f").text(), QString("1.44"));
}

QTEST_MAIN(TestINDIDouble)


#include "testindidouble.moc"
