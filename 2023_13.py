import sys

def rflct_v(p, b=None):
    sprev = ""
    for i in range(len(p[0])):
        s = ""
        for j in range(len(p)):
            s += p[j][i]
        if i != b and s == sprev and vrfy_rflct_v(p, i):
            return i
        sprev = s
    return 0

def rflct_h(p, b=None):
    prev = ""
    for j in range(len(p)):
        if j != b and p[j] == prev and vrfy_rflct_h(p, j):
            return j
        prev = p[j]
    return 0

def vrfy_rflct_v(p, x):
    i1 = x-1
    i2 = x
    while i1 >= 0 and i2 < len(p[0]):
        s1 = ""
        s2 = ""
        for j in range(len(p)):
            s1 += p[j][i1]
            s2 += p[j][i2]
        if s1 != s2:
            return False
        i1 -= 1
        i2 += 1
    return True

def vrfy_rflct_h(p, y):
    j1 = y-1
    j2 = y
    while j1 >= 0 and j2 < len(p):
        if p[j1] != p[j2]:
            return False
        j1 -= 1
        j2 += 1
    return True

def alter(p):
    for j in range(len(p)):
        for i in range(len(p[0])):
            c = p[j][i]
            if c == '.':
                cc = '#'
            else:
                cc = '.'
            p[j][i] = cc
            yield p
            p[j][i] = c

l = [[list(y) for y in x.split('\n') if y] for x in sys.stdin.read().split('\n\n') if x]

lres = []
for p in l:
    x = rflct_v(p)
    if x > 0:
        lres.append(x)
        continue
    x = rflct_h(p)*100
    if x > 0:
        lres.append(x)
        continue
print(sum(lres))

res = 0
for bad,p in zip(lres,l):
    for altered_p in alter(p):
        b = None
        if bad < 100:
            b = bad
        x = rflct_v(altered_p, b)
        if x > 0:
            res += x
            break

        b = None
        if bad >= 100:
            b = bad // 100
        x = rflct_h(altered_p, b)
        if x > 0:
            res += x*100
            break
print(res)
            
