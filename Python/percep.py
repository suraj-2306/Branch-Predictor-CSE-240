import math

for b in range(9, 14):
    for h in range(12, 34):
        result = (b + 1) * (h + 1)
        s = math.floor(pow(2, 17) / result)
        print("#", b, h, s, "->")
