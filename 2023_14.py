import sys

l = [list(x) for x in sys.stdin.read().split('\n') if x]

def north_load(l):
    res = 0
    for j in range(len(l)):
        for i in range(len(l[0])):
            if l[j][i] == 'O':
                res += len(l)-j
    return res

def tilt_north(l):
    for i in range(len(l[0])):
        y = 0
        for j in range(len(l)):
            c = l[j][i]
            if c == '#':
                y = j+1
            elif c == 'O':
                l[j][i] = '.'
                l[y][i] = 'O'
                y += 1

def tilt_west(l):
    for j in range(len(l)):
        x = 0
        for i in range(len(l[0])):
            c = l[j][i]
            if c == '#':
                x = i+1
            elif c == 'O':
                l[j][i] = '.'
                l[j][x] = 'O'
                x += 1

def tilt_south(l):
    for i in range(len(l[0])):
        y = len(l)-1
        for j in reversed(range(len(l))):
            c = l[j][i]
            if c == '#':
                y = j-1
            elif c == 'O':
                l[j][i] = '.'
                l[y][i] = 'O'
                y -= 1

def tilt_east(l):
    for j in range(len(l)):
        x = len(l[0])-1
        for i in reversed(range(len(l[0]))):
            c = l[j][i]
            if c == '#':
                x = i-1
            elif c == 'O':
                l[j][i] = '.'
                l[j][x] = 'O'
                x -= 1

def cycle(l):
    tilt_north(l)
    tilt_west(l)
    tilt_south(l)
    tilt_east(l)

def stringify(l):
    return '\n'.join(''.join(x) for x in l)

tilt_north(l)
print(north_load(l))
tilt_west(l)
tilt_south(l)
tilt_east(l)

lim = 1000000000
hist = [stringify(l)]
for i in range(lim):
    cycle(l)
    s = stringify(l)
    if s in hist:
        hist.append(s)
        break
    hist.append(s)

starts = hist.index(s)
period = hist[starts:].index(s, 1)
ll = hist[starts+(lim-starts)%period-1]

print(north_load(ll.split('\n')))
