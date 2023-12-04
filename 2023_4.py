import sys

s = [x.split(':')[1] for x in sys.stdin.read().split('\n') if x.strip()]

r = 0
counts = [1]*len(s)
for i, ss in enumerate(s):
    win, num = [[int(n.strip()) for n in x.split()] for x in ss.split('|')]
    n = sum([n in win for n in num])
    if n > 0:
        r += 2**(n-1)

    for j in range(n):
        counts[i+1+j] += counts[i]

print(r)
print(sum(counts))
