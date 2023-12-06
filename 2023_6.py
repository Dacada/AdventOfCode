import sys

def isqrt(n):
    x = n
    y = (x + 1) // 2
    while y < x:
        x = y
        y = (x + n // x) // 2
    return x

p = zip(*[[int(y) for y in x.split()[1:]] for x in sys.stdin.read().split('\n') if x])

tt = ''
xx = ''
r = 1
for t,x in p:
    tt += str(t)
    xx += str(x)
    
    # solve for i: (t-i)*i=x
    i1 = (t + isqrt(t*t - 4*x))//2
    # and adjust
    if (t-i1)*i1 <= x:
        i1 -= 1

    # same here
    i2 = (t - isqrt(t*t - 4*x))//2
    if (t-i2)*i2 <= x:
        i2 += 1
    
    d = i1-i2+1
    r *= d
print(r)

t = int(tt)
x = int(xx)
i1 = (t + isqrt(t*t - 4*x))//2
if (t-i1)*i1 <= x:
    i1 -= 1
i2 = (t - isqrt(t*t - 4*x))//2
if (t-i2)*i2 <= x:
    i2 += 1
print(i1-i2+1)
