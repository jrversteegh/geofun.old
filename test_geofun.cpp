#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "geofun.h"

using namespace geofun;
using namespace std;

class VectorPositionTest : public CppUnit::TestFixture {
  void testOperators() {
    Position p1(0.9, 0.9);
    cout << p1.lat() << " " << p1.lon() << endl;
    Position p2(1.0, 1.0);
    cout << p2.lat() << " " << p2.lon() << endl;
    Vector v1 = p2 - p1;
    cout << v1.a() << " " << v1.r() << endl;
    Vector v2 = v1 * 0.2;
    cout << v2.a() << " " << v2.r() << endl;
    Position p3(p1);
    for (int i = 0; i < 5; ++i) {
      p3 += v2;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(p2.lat(), p3.lat(), 0.0001);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(p2.lon(), p3.lon(), 0.0001);
    }
  }
public:
  CPPUNIT_TEST_SUITE(VectorPositionTest);
  CPPUNIT_TEST(testOperators);
  CPPUNIT_TEST_SUITE_END();
};

int main()
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(VectorPositionTest::suite());
  if (runner.run()) 
    return 0; 
  else
    return 1;
}

