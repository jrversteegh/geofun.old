#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "geofun.hpp"

using namespace geofun;
using namespace std;

class VectorPositionTest : public CppUnit::TestFixture {
  void testNormAngle() {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.14, norm_angle_pipi(3.14), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.28 - two_pi, norm_angle_pipi(6.28), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.28, norm_angle_2pi(6.28), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(two_pi - 2, norm_angle_2pi(-2), 1E-12);
    double angle = 3;
    CPPUNIT_ASSERT(norm_angle_pi2pi2(&angle));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pi - 3, angle, 1E-12);
    angle = 1;
    CPPUNIT_ASSERT(!norm_angle_pi2pi2(&angle));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, angle, 1E-12);
    angle = -2;
    CPPUNIT_ASSERT(norm_angle_pi2pi2(&angle));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2 - pi, angle, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, angle_diff(-pi, pi - 1E-13), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, angle_diff(pi - 1E-13, -pi), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, angle_diff(-pi, pi), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, angle_diff(pi, -pi), 1E-12);
  }
  void testOperators() {
    Position p1(0.8, 0.8); // 45.8366236
    Position p2(1.0, 1.0); // 57.2957795
    Vector v1 = p2 - p1;
    Vector v2 = v1 * 0.01;
    Position p3(p1);
    for (int i = 0; i < 100; ++i) {
      p3 += v2;
    }
    Vector diff = p3 - p2;
    CPPUNIT_ASSERT(diff.r() < 20);
    Position p4 = p1 + v1;
    diff = p4 - p2;
    // Within 30 meters after more than 800 miles. Not bad.
    CPPUNIT_ASSERT(diff.r() < 30);
    // Make sure horizontal and vertical movements work
    Position p5(0.8, 0.8);
    Position p6(0.8, 1.0);
    Position p7(1.0, 0.8); 
    Vector v3 = p6 - p5;
    Vector v4 = p7 - p5;
    Position p8(p5);
    Position p9(p5);
    p8 += v3;
    p9 += v4;
    diff = p8 - p6;
    CPPUNIT_ASSERT(diff.r() < 20);
    diff = p9 - p7;
    CPPUNIT_ASSERT(diff.r() < 20);
  }
  void testDivisionByZero() {
    // This test verifies handling of division by zero required by the
    // operators to work properly for purely horizontal or vertical 
    // displacements.
    // The result of this test is likely processor and/or compiler dependent.
    // That's why it's here. IEEE754 floating point should pass this test. 
    double a = 1;
    double b = 0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(INFINITY, a / b, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, a / INFINITY, 1E-12);
  }
  void testDotCross() {
    Vector v1 = Vector(0.5, 3);
    Vector v2 = Vector(5, 2);
    double dot = v1.dot(v2);
    double cross = v1.cross(v2);
    Coord c1 = v1.cartesian();
    Coord c2 = v2.cartesian();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(c1.dot(c2), dot, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(c1.cross(c2), cross, 1E-12);
  }
public:
  CPPUNIT_TEST_SUITE(VectorPositionTest);
  CPPUNIT_TEST(testNormAngle);
  CPPUNIT_TEST(testOperators);
  CPPUNIT_TEST(testDivisionByZero);
  CPPUNIT_TEST(testDotCross);
  CPPUNIT_TEST_SUITE_END();
};

class LineTest : public CppUnit::TestFixture {
  void testIntersection() {
    Line l1(Position(1.0, 0.8), Position(0.8, 1.0));
    Line l2(Position(0.8, 0.8), Position(1.0, 1.0));
    Position x1 = l1.intersection(l2);
    Position x2 = l2.intersection(l1);
    double r = (x1 - x2).r();
    CPPUNIT_ASSERT(r < 30);
    Line l3(Position(0.8, 0.78), Position(1.0, 1.01));
    Position x3 = l2.intersection(l3);
    // Some tests crossing the 180 meridian
    Line l4(Position(0, pi - 0.1), Position(0.1, -pi + 0.1));
    Line l5(Position(0, pi - 0.05), Position(0.1, pi - 0.05));
    Line l6(Position(0, pi - 0.05), Position(0.1, -pi + 0.05));
    Line l7(Position(0, pi - 0.05), Position(0.1, -pi + 0.050001));
    Position x4 = l4.intersection(l5);
    Position x5 = l4.intersection(l6);
    Position x6 = l4.intersection(l7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pi - 0.05, x4.lon(), 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pi, x5.lon(), 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-pi, x6.lon(), 1E-5);
  }
public:
  CPPUNIT_TEST_SUITE(LineTest);
  CPPUNIT_TEST(testIntersection);
  CPPUNIT_TEST_SUITE_END();
};

class ArcTest : public CppUnit::TestFixture {
  void testDirectInverse() {
    Position p1(0.8, 0.8);
    Position p2(1.0, 1.0);
    Arc arc1(p1, p2);
    Line line1(p1, p2);

    CPPUNIT_ASSERT(arc1.v().r() < line1.v().r());
    Arc arc2(p1, arc1.v());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, arc2.p2().lat(), 1E-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, arc2.p2().lon(), 1E-6);
    Vector d = arc1.p2() - p2;

    CPPUNIT_ASSERT(d.r() < 10);
  }
public:
  CPPUNIT_TEST_SUITE(ArcTest);
  CPPUNIT_TEST(testDirectInverse);
  CPPUNIT_TEST_SUITE_END();
};

int main()
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(VectorPositionTest::suite());
  runner.addTest(LineTest::suite());
  runner.addTest(ArcTest::suite());
  if (runner.run()) 
    return 0; 
  else
    return 1;
}

