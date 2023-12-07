import sys
import functools

@functools.total_ordering
class Card:
    CARDS = [None, ['2','3','4','5','6','7','8','9','T','J','Q','K','A'], ['J','2','3','4','5','6','7','8','9','T','Q','K','A']]
    def __init__(self, hand, v):
        self.hand = hand
        self.v = v
    def __eq__(self, other):
        return self.hand == other.hand
    def __lt__(self, other):
        t1 = self.type()
        t2 = other.type()
        if t1 != t2:
            return t1 < t2
        for c1,c2 in zip(self.hand, other.hand):
            t1 = Card.CARDS[self.v].index(c1)
            t2 = Card.CARDS[self.v].index(c2)
            if t1 != t2:
                return t1 < t2
        return False
    def type(self):
        return self._type(self.hand)
    def _type(self, hand):
        s = set(hand)
        if len(s) == 1:
            return 7
        l = list(hand)
        if self.v == 2 and 'J' in hand:
            l[l.index('J')] = max((l.count(c),c) for c in s if c != 'J')[1]
            return self._type(''.join(l))
            
        if len(s) == 5:
            return 1
        elif len(s) == 4:
            return 2
        elif len(s) == 3:
            if any(l.count(c) == 3 for c in s):
                return 4
            return 3
        elif len(s) == 2:
            if any(l.count(c) == 4 for c in s):
                return 6
            return 5
        elif len(s) == 1:
            return 7
    def __str__(self):
        return self.hand
    def __repr__(self):
        return f"Card('{self.hand}')"
        

s = sys.stdin.read()
for i in (1,2):
    n = sum((i+1)*b for i,(c,b) in enumerate(sorted([(Card(a, i),int(b)) for a,b in [x.split() for x in s.split('\n') if x]])))
    print(n)
