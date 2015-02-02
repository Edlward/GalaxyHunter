#include <QtTest/QTest>

class TestINDIDoubleFormat : public QObject {
Q_OBJECT
private slots:
  void testDoubleToStringPrintf();
};

void TestINDIDoubleFormat::testDoubleToStringPrintf()
{
  QCOMPARE(1, 1);
}

QTEST_MAIN(TestINDIDoubleFormat)


#include "testindidoubleformat.moc"
