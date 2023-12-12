import sys
import functools

def log_fun(f):
    def wrapper(*args):
        r = f(*args)
        print("called with", args, "returns", r)
        return r
    return wrapper

def count(s, t):
    n = 0
    for c in s:
        if c in t:
            n += 1
        else:
            break
    return n

@functools.cache
def arrangements(springs, nums):
    if not nums:
        return 1 if '#' not in springs else 0
    if not springs:
        return 0

    c = springs[0]
    n = nums[0]

    if c == '.' or c == '?':
        rest = arrangements(springs[1:], nums)
    else:
        rest = 0
    
    if c == '.':
        return rest
    
    spaces = count(springs, ('#', '?'))
    if spaces < n:
        return rest

    next_springs = springs[n:]
    if next_springs and next_springs[0] == '#':
        return rest
    return rest + arrangements(next_springs[1:], nums[1:])
        

l = [x.strip().split() for x in sys.stdin.read().split('\n') if x.strip()]
l = [(a, tuple([int(x) for x in b.split(',')])) for a,b in l]
print(sum(arrangements(*x) for x in l))
l = [('?'.join([a]*5),b*5) for a,b in l]
print(sum(arrangements(*x) for x in l))
