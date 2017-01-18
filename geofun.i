%module geofun

%include "typemaps.i"
%include "exception.i"
%include "std_string.i"

/* Deal with null references in equality operator */
%feature("pythonprepend") operator== %{
    if args[0] is None:
        return False
%}

%{
#include "geofun.hpp"

// --> for bad_cast
#include <typeinfo>
%}

%rename (__getitem__) *::operator[];
%rename (__assign__) *::operator=;
%rename (__len__) *::size;
%rename (__cmp__) *::compare;
%rename (__lt__) *::operator<;
%rename (__gt__) *::operator>;
%rename (__eq__) *::operator==;
%rename (__le__) *::operator<=;
%rename (__ge__) *::operator>=;

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
  Position& operator+=(const Simple& vector) {
    const geofun::Vector& v = dynamic_cast<const geofun::Vector&>(vector);
    return *$self += v;
  }
  Position operator+(const Simple& vector) const {
    const geofun::Vector& v = dynamic_cast<const geofun::Vector&>(vector);
    return *$self + v;
  }
};

%exception;

// Add __repr__ and __str__ methods

%extend geofun::Coord {
  char* __str__() {
    static char temp[64];
    sprintf(temp, "%.4f, %.4f", geofun::m_to_nm($self->get_x()), 
                                geofun::m_to_nm($self->get_y()));
    return &temp[0];
  }
  char* __repr__() {
    static char temp[64];
    sprintf(temp, "Coord(%f, %f)", $self->get_x(), $self->get_y());
    return &temp[0];
  }
}

%extend geofun::Vector {
  char* __str__() {
    static char temp[64];
    sprintf(temp, "%.2f, %.2f", geofun::rad_to_deg(geofun::to_rads($self->get_a())), 
                                geofun::m_to_nm($self->get_r()));
    return &temp[0];
  }
  char* __repr__() {
    static char temp[64];
    sprintf(temp, "Vector(%f, %f)", $self->get_a(), $self->get_r());
    return &temp[0];
  }
}

%extend geofun::Position {
  char* __str__() {
    static char temp[64];
    sprintf(temp, "%.5f, %.5f", geofun::rad_to_deg(geofun::to_rads($self->get_lat())), 
                                geofun::rad_to_deg(geofun::to_rads($self->get_lon())));
    return &temp[0];
  }
  char* __repr__() {
    static char temp[64];
    sprintf(temp, "Position(%f, %f)", $self->get_lat(), $self->get_lon());
    return &temp[0];
  }
}

%extend geofun::Line {
  char* __str__() {
    static char temp[64];
    sprintf(temp, "%.5f, %.5f - %.5f, %.5f", 
        geofun::rad_to_deg(geofun::to_rads($self->get_p1().get_lat())),
        geofun::rad_to_deg(geofun::to_rads($self->get_p1().get_lon())),
        geofun::rad_to_deg(geofun::to_rads($self->get_p2().get_lat())),
        geofun::rad_to_deg(geofun::to_rads($self->get_p2().get_lon())));
    return &temp[0];
  }
  char* __repr__() {
    static char temp[128];
    sprintf(temp, "Line(Position(%f, %f), Position(%f, %f))", 
        $self->get_p1().get_lat(),
        $self->get_p1().get_lon(),
        $self->get_p2().get_lat(),
        $self->get_p2().get_lon());
    return &temp[0];
  }
}


/* Turn some C++ getters and setters into python properties */
%pythoncode %{
def set_property(clss, name):
    getter = getattr(clss, "get_" + name)
    setter = getattr(clss, "set_" + name)
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

