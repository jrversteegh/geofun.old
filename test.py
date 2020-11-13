#!/usr/bin/env python

import sys
from glob import glob
try:
    d = glob('build/lib.*')[0]
    sys.path.insert(0, d)
except IndexError:
    print('No build directory found? Module not yet build?')
    sys.exit(1)


from geofun import *

p1 = Position(0.8, 0.8)
p2 = Position(1.0, 1.0)
l1 = Line(p1, p2)
print(rad_to_deg(l1[1][0]), l1[1][1])
a1 = Arc(p1, p2)
print(rad_to_deg(a1[1][0]), a1[1][1])
v1 = Vector(0.5, 1450E3)
l2 = Line(p1, v1)
print(rad_to_deg(l2.p2.lat), rad_to_deg(l2.p2.lon))
v2 = p2 - p1
print(v2.a, v2.r)
p3 = Position(0.1, 0.1)
p3.lat = 0.2
print(p3.lat, p3.lon)
