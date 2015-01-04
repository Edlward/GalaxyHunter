#include <QtTest/QTest>
#include <QImage>
#include <QFile>
#include <Magick++.h>
#include <imaging/file2image.h>
class TestRawConversion : public QObject {
  Q_OBJECT
public:
    explicit TestRawConversion(QObject* parent = 0);
private slots:
  void raw2QImageDirectFailing();
  void raw2ImageMagick();
  void testFile2QImageOnSupportedFile();
  void testFile2QImageOnCanonRaw();
};
TestRawConversion::TestRawConversion(QObject* parent): QObject(parent)
{
  Magick::InitializeMagick("");
}


void TestRawConversion::raw2QImageDirectFailing()
{
  QVERIFY(QFile::exists(TESTS_SRC_DIR "/IMG_0344.CR2"));
  QImage image(TESTS_SRC_DIR "/IMG_0344.CR2");
  QVERIFY(image.isNull());
}

void TestRawConversion::raw2ImageMagick()
{
  Magick::Image image("cr2:" TESTS_SRC_DIR "/IMG_0344.CR2");
  QVERIFY(image.isValid());
}

void TestRawConversion::testFile2QImageOnSupportedFile()
{
  QImage image;
  QVERIFY(image.isNull());
  File2Image file2image(image);
  file2image.load(TESTS_SRC_DIR "/IMG_0344.png");
  QVERIFY(!image.isNull());
}

void TestRawConversion::testFile2QImageOnCanonRaw()
{
  QImage image;
  QVERIFY(image.isNull());
  File2Image file2image(image);
  file2image.load(TESTS_SRC_DIR "/IMG_0344.CR2", "cr2");
  QVERIFY(!image.isNull());
}


QTEST_MAIN(TestRawConversion)

#include "testrawconversion.moc"