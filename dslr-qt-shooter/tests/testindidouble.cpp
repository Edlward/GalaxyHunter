#include <QtTest/QTest>
#include "telescope/indidouble.h"

class TestINDIDouble : public QObject {
Q_OBJECT
private slots:
  void testDoubleToStringPrintf();
};

void TestINDIDouble::testDoubleToStringPrintf()
{
  QCOMPARE(INDIDouble(1.44, "%.2f").text(), QString("1.44"));
}

QTEST_MAIN(TestINDIDouble)


#include "testindidouble.moc"
