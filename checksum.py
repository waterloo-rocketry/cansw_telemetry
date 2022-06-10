import sys

def checksum(s):
    sum1 = 0
    sum2 = 0
    for c in s:
        if c.lower() in "0123456789abcdef":
            sum1 = (sum1 + int(c, 16)) % 15
            sum2 = (sum1 + sum2) % 15
    return hex(sum1 ^ sum2)[-1]


if len(sys.argv) < 2:
    print("Usage: checksum.py mSSS,DD,DD,DD,...;")
    sys.exit(1)

if sys.argv[1][-1] != ";":
    sys.argv[1] += ";"
print(sys.argv[1] + checksum(sys.argv[1]))
