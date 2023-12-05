import sys

m = [x for x in sys.stdin.read().split('\n\n') if x.strip()]
seed = [int(x) for x in m[0].split()[1:]]
m = [[[int(z) for z in y.split()] for y in x.split('\n')[1:] if y] for x in m[1:]]
m = [[(range(a,a+l),range(b,b+l)) for a,b,l in x] for x in m]

r =[]
for s in seed:
    for t in m:
        for r1,r2 in t:
            if s in r2:
                s = r1.start + r2.index(s)
                break
    r.append(s)
print(min(r))

rseed = [seed[i:i+2] for i in range(0, len(seed), 2)]
rseed = [range(a,a+b) for a,b in rseed]

r = []
for rs in rseed:
    lrs = [rs]
    for t in m:
        nlrs = []
        for rs in lrs:
            rs_left = [rs]
            for r1,r2 in t:
                n_rs_left = []
                for rs in rs_left:
                    if r2.start in rs or rs.start in r2:
                        olp = range(max(r2.start, rs.start), min(r2.stop, rs.stop))
                        olpc = range(r1.start + olp.start - r2.start, r1.stop + olp.stop - r2.stop)
                        nlrs.append(olpc)
                        if rs.start < r2.start:
                            lft = range(rs.start, r2.start)
                            n_rs_left.append(lft)
                        if rs.stop > r2.stop:
                            rgt = range(r2.stop, rs.stop)
                            n_rs_left.append(rgt)
                    else:
                        n_rs_left.append(rs)
                rs_left = n_rs_left
            nlrs.extend(rs_left)
        lrs = nlrs
    r.append(min(x.start for x in lrs))
print(min(r))
                
