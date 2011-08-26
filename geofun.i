%module geofun

%include "typemaps.i"
%include "exception.i"
%include "std_string.i"

%{
#include "geofun.hpp"

// --> for bad_cast
#include <typeinfo>
%}

%rename (__getitem__) *::operator[];
%rename (__assign__) *::operator=;
%rename (__len__) *::size;
%rename (__cmp__) *::compare;

%rename (_get_lat) *::lat;
%rename (_set_lat) *::lat(const double);
%rename (_get_lon) *::lon;
%rename (_set_lon) *::lon(const double);
%rename (_get_a) *::a;
%rename (_set_a) *::a(const double);
%rename (_get_r) *::r;
%rename (_set_r) *::r(const double);
%rename (_get_x) *::x;
%rename (_set_x) *::x(const double);
%rename (_get_y) *::y;
%rename (_set_y) *::y(const double);

%rename (_get_p1) *::p1;
%rename (_set_p1) *::p1(const Position&);
%rename (_get_p2) *::p2;
%rename (_set_p2) *::p2(const Position&);
%rename (_get_v) *::v;
%rename (_set_v) *::v(const Vector&);
%rename (_get_r) *::r;
%rename (_set_r) *::r(const Vector&);

%exception *::operator[] {
  try {
    $action
  } 
  catch (const geofun::IndexError& e) {
    SWIG_exception(SWIG_IndexError, e.what());
  }
}

%exception set_earth_model {
  try {
    $action
  } 
  catch (const geofun::EarthModelError& e) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
}

%include "geofun.hpp"

%exception;

%exception {
  try {
    $action
  } 
  catch (const std::bad_cast& e) {
    SWIG_exception(SWIG_TypeError, e.what());
  }
}

%extend geofun::Position {
  bool operator==(const Simple& position) const {
    return *$self == dynamic_cast<const geofun::Position&>(position);
  }
  Position& operator=(const Position& position) { 
    return *$self = dynamic_cast<const geofun::Position&>(position);
  }
  Position& operator+=(const Simple& vector) {
    const geofun::Vector& v = dynamic_cast<const geofun::Vector&>(vector);
    return *$self += v;
  }
  Vector operator-(const Simple& position) const {
    const geofun::Position& p = dynamic_cast<const geofun::Position&>(position);
    return *$self - p;
  }
  Position operator+(const Simple& vector) const {
    const geofun::Vector& v = dynamic_cast<const geofun::Vector&>(vector);
    return *$self + v;
  }
};

%exception;

%pythoncode %{
def set_property(clss, name):
    getter = getattr(clss, "_get_" + name)
    setter = getattr(clss, "_set_" + name)
    clss.__swig_getmethods__[name] = getter
    clss.__swig_setmethods__[name] = setter
    setattr(clss, name, _swig_property(getter, setter))

set_property(Coord, 'x')
set_property(Coord, 'y')
set_property(Vector, 'a')
set_property(Vector, 'r')
set_property(Position, 'lat')
set_property(Position, 'lon')
set_property(Line, 'p1')
set_property(Line, 'p2')
set_property(Line, 'v')
set_property(Arc, 'p1')
set_property(Arc, 'p2')
set_property(Arc, 'v')
set_property(Arc, 'r')
%}

