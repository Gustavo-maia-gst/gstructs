from gstructs import *

a = array(100)

for i in range(100):
    a.set(i**2, i)

for x in a:
    print(x)

print(f"15² = {a.get(15)}")

for x in a:
    print(x)
