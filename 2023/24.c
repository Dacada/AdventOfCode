#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#define TRACE

static int cmpden(__int128 numerator, __int128 quantity, __int128 denominator) {
	ASSERT(denominator != 0, "division by zero");

	__int128 a = numerator;
	__int128 b = quantity * denominator;
	
	int res;
	if (a < b) {
		res = -1;
	} else if (a > b) {
		res = 1;
	} else {
		res = 0;
	}

	if (denominator < 0) {
		res = -res;
	}
	return res;
}

static bool eq_frac(__int128 n1, __int128 d1, __int128 n2, __int128 d2) {
	return n1*d2 == n2*d1;
}

struct point {
	__int128 x;
	__int128 y;
	__int128 z;
};

#ifdef TRACE
static void point_print(const struct point *p) {
	fprintf(stderr, "%ld, %ld, %ld", (long)p->x, (long)p->y, (long)p->z);
}
#endif

struct line {
	struct point p;
	struct point v;
};

#ifdef TRACE
static void line_print(const struct line *l) {
	point_print(&l->p);
	fputs(" @ ", stderr);
	point_print(&l->v);
}
#endif

static bool line_intersect2d(const struct line *l1, const struct line *l2, struct point *intersection, __int128 *denominator) {
#ifdef TRACE
	fputs("Hailstone A: ", stderr);
	line_print(l1);
	fputc('\n', stderr);
	fputs("Hailstone B: ", stderr);
	line_print(l2);
	fputc('\n', stderr);
#endif
			
	// Formula for the intersection of two lines given two points of each line
	
	__int128 x1 = l1->p.x;
	__int128 y1 = l1->p.y;
	
	__int128 x2 = l1->p.x + l1->v.x;
	__int128 y2 = l1->p.y + l1->v.y;
	
	__int128 x3 = l2->p.x;
	__int128 y3 = l2->p.y;
	
	__int128 x4 = l2->p.x + l2->v.x;
	__int128 y4 = l2->p.y + l2->v.y;

	__int128 dx12 = x1 - x2;
	__int128 dy12 = y1 - y2;
	__int128 dy34 = y3 - y4;
	__int128 dx34 = x3 - x4;
	
	__int128 den = dx12*dy34 - dy12*dx34;
	
	if (den == 0) {
#ifdef TRACE
		fputs("Hailstones' paths are parallel; they never intersect.\n\n", stderr);
#endif
		return false;
	}

	__int128 n = (x1*y2 - y1*x2);
	__int128 m = (x3*y4 - y3*x4);
	
	__int128 xnum = n*dx34 - m*dx12;
	__int128 ynum = n*dy34 - m*dy12;

	// x = xnum / den;
	// y = ynum / den;
	// floating point is icky, let's keep those quotioents unrealized
	
	// need to determine whether they cross in the future
	
	// x = v*t + p
	// v*t = x - p
	// t = (x-p)/v
	// (x-p)/v >= 0

	// if v > 0: x-p >= 0
	// if v < 0: x-p <= 0

	// num/den - p >= 0
	// num/den >= p
	// num >= p*den (if den > 0, change inequality otherwise)

	bool l1_ok;
	if (l1->v.x > 0) {
		l1_ok = cmpden(xnum, l1->p.x, den) >= 0;
	} else if (l1->v.x < 0) {
		l1_ok = cmpden(xnum, l1->p.x, den) <= 0;
	} else {
		if (l1->v.y > 0) {
			l1_ok = cmpden(ynum, l1->p.y, den) >= 0;
		} else if (l1->v.y < 0) {
			l1_ok = cmpden(ynum, l1->p.y, den) <= 0;
		} else {
			FAIL("both velocities are 0 :(");
		}
	}

	bool l2_ok;
	if (l2->v.x > 0) {
		l2_ok = cmpden(xnum, l2->p.x, den) >= 0;
	} else if (l2->v.x < 0){
		l2_ok = cmpden(xnum, l2->p.x, den) <= 0;
	} else {
		if (l2->v.y > 0) {
			l2_ok = cmpden(ynum, l2->p.y, den) >= 0;
		} else if (l2->v.y < 0) {
			l2_ok = cmpden(ynum, l2->p.y, den) <= 0;
		} else {
			FAIL("both velocities are 0 :(");
		}
	}
	
	if (!l1_ok && !l2_ok) {
#ifdef TRACE
		fputs("Hailstones' paths crossed in the past for both hailstones.\n\n", stderr);
#endif
		return false;
	} else if (!l1_ok) {
#ifdef TRACE
		fputs("Hailstones' paths crossed in the past for hailstone A.\n\n", stderr);
#endif
		return false;
	} else if (!l2_ok) {
#ifdef TRACE
		fputs("Hailstones' paths crossed in the past for hailstone B.\n\n", stderr);
#endif
		return false;
	}

	intersection->x = xnum;
	intersection->y = ynum;
	*denominator = den;
	return true;
}

static void assert_string(const char **str, const char *expected) {
	int n = strlen(expected);
	ASSERT(strncmp(*str, expected, n) == 0, "parse error '%s' '%s'", *str, expected);
	*str += n;
}

static __int128 parse_int(const char **input) {
	bool neg = **input == '-';
	if (neg) {
		*input += 1;
	}

	ASSERT(isdigit(**input), "parse error %c(%d)", **input, **input);
	__int128 n = 0;
	while (isdigit(**input)) {
		n *= 10;
		n += **input - '0';
		*input += 1;
	}

	if (neg) {
		n = -n;
	}
	return n;
}

static void parse_line(const char **input, struct line *line) {
	line->p.x = parse_int(input);
	assert_string(input, ", ");
	line->p.y = parse_int(input);
	assert_string(input, ", ");
	line->p.z = parse_int(input);

	assert_string(input, " @ ");

	line->v.x = parse_int(input);
	assert_string(input, ", ");
	line->v.y = parse_int(input);
	assert_string(input, ", ");
	line->v.z = parse_int(input);
	
	ASSERT(**input == '\n' || **input == '\0', "parse error");
	if (**input == '\n') {
		*input += 1;
	}
}

static struct line *parse_input(const char *input, int *nlines) {
	int len = 0;
	int cap = 4;
	struct line *list = malloc(sizeof(*list)*cap);
	
	while (*input != '\0') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}
		parse_line(&input, &list[len++]);
	}

	*nlines = len;
	return list;
}

static void solution1(const char *const input, char *const output) {
	const __int128 from = 200000000000000L;
	const __int128 to = 400000000000000L;
	
	int nlines;
	struct line *lines = parse_input(input, &nlines);

	int res = 0;
	for (int i=0; i<nlines; i++) {
		for (int j=i+1; j<nlines; j++) {
			struct point intersect;
			__int128 den;
			if (line_intersect2d(&lines[i], &lines[j], &intersect, &den)) {
#ifdef TRACE
				double dden = den;
				double dxnum = intersect.x;
				double dynum = intersect.y;
				double x = dxnum/dden;
				double y = dynum/dden;
#endif
				if (cmpden(intersect.x, from, den) >= 0 &&
				    cmpden(intersect.x, to, den) <= 0 &&
				    cmpden(intersect.y, from, den) >= 0 &&
				    cmpden(intersect.y, to, den) <= 0) {
#ifdef TRACE
					fprintf(stderr, "Hailstones' paths will cross INSIDE the test area (at x=%f, y=%f)\n\n", x, y);
#endif
					res++;
				} else {					
#ifdef TRACE
					fprintf(stderr, "Hailstones' paths will cross OUTSIDE the test area (at x=%f, y=%f)\n\n", x, y);
#endif
				}
			}
		}
	}

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
	free(lines);
}

static bool all_lines_collide(struct line *lines, int nlines, __int128 adjust_x, __int128 adjust_y, struct point *intersection, __int128 *denominator) {
	struct point current_intersect;
	__int128 current_denominator = 0;
	bool found_intersect = false;

	for (int i=0; i<nlines; i++) {
		struct line l1 = lines[i];
		l1.v.x += adjust_x;
		l1.v.y += adjust_y;
		for (int j=0; j<nlines; j++) {
			struct line l2 = lines[j];
			l2.v.x += adjust_x;
			l2.v.y += adjust_y;
		
			struct point inter;
			__int128 den;
			if (line_intersect2d(&l1, &l2, &inter, &den)) {
				if (found_intersect) {
					if (eq_frac(current_intersect.x, current_denominator, inter.x, den) &&
					    eq_frac(current_intersect.y, current_denominator, inter.y, den)) {
					} else {
						return false;
					}
				} else {
					current_intersect = inter;
					current_denominator = den;
					found_intersect = true;
				}
			}
		}
	}

	*intersection = current_intersect;
	*denominator = current_denominator;
	return true;
}

static void time_at_point(const struct line *l, const struct point *point, __int128 denominator, __int128 *num, __int128 *det) {
	// t = (i.x/d - p.x)/v.x
	// t = i.x/(d*v.x) - p.x/v.x
	// t = i.x/(d*v.x) - (d*p.x)/(d*v.x)
	// t = (i.x - d * p.x) / (d * v.x)
	
	if (l->v.x == 0) {
		if (l->v.y == 0) {
			FAIL("both velocities are 0 :(");
		}
		*num = point->y - denominator * l->p.y;
		*det = denominator * l->v.y;
	} else {
		*num = point->x - denominator * l->p.x;
		*det = denominator * l->v.x;
	}
}

static bool get_z(const struct line *l1, const struct line *l2, const struct point *intersection, __int128 denominator, __int128 *znum, __int128 *zdet) {
	__int128 nt1, nt2, dt1, dt2;
	time_at_point(l1, intersection, denominator, &nt1, &dt1);
	time_at_point(l2, intersection, denominator, &nt2, &dt2);

	if (eq_frac(nt1, dt1, nt2, dt2)) {
		// p.z + t*v.z = p.z + nt/dt*v.z = (p.z*dt + nt*v.z)/dt
		__int128 num1 = l1->p.z*dt1 + nt1*l1->v.z;
		__int128 num2 = l2->p.z*dt2 + nt2*l2->v.z;
		ASSERT(eq_frac(num1, dt1, num2, dt2), "logic error");
		return false;
	}

	// (p1.z - p2.z + t1*v1.z - t2*v2.z) / (t1 - t2)
	// (p1.z - p2.z + (nt1*v1.z)/dt1 - (nt2*v2.z)/dt2) / (nt1/dt1 - nt2/dt2)
	// ((p1.z*dt1*dt2)/(dt1*dt2) - (p2.z*dt1*dt2)/(dt1*dt2) + (nt1*v1.z*dt2)/(dt1*dt2) - (nt2*v2.z*dt1)/(dt1*dt2)) / ((nt1*dt2)/(dt1*dt2) - (nt2*dt1)/(dt1*dt2))
	// ((p1.z*dt1*dt2 - p2.z*dt1*dt2 + nt1*v1.z*dt2 - nt2*v2.z*dt1)/(dt1*dt2)) / ((nt1*dt2 - nt2*dt1)/(dt1*dt2))
	// (p1.z*dt1*dt2 - p2.z*dt1*dt2 + nt1*v1.z*dt2 - nt2*v2.z*dt1) / (nt1*dt2 - nt2*dt1)
	__int128 num = l1->p.z*dt1*dt2 - l2->p.z*dt1*dt2 + nt1*l1->v.z*dt2 - nt2*l2->v.z*dt1;
	__int128 det = nt1*dt2 - nt2*dt1;

	*znum = num;
	*zdet = det;
	return true;
}

static bool find_third_coordinate(struct line *lines, int nlines, __int128 adjust_x, __int128 adjust_y, const struct point *intersection, __int128 denominator, __int128 *pos_z) {
	__int128 current_candidate_num = -1;
	__int128 current_candidate_den = -1;
	bool current_candidate_init = false;
	
	struct line l1 = lines[0];
	l1.v.x += adjust_x;
	l1.v.y += adjust_y;

	for (int i=0; i<nlines; i++) {
		struct line l2 = lines[i];
		l2.v.x += adjust_x;
		l2.v.y += adjust_y;
		
		__int128 candidate_num;
		__int128 candidate_den;
		if (get_z(&l1, &l2, intersection, denominator, &candidate_num, &candidate_den)) {
			if (current_candidate_init) {
				/*
				if (!eq_frac(candidate_num, candidate_den, current_candidate_num, current_candidate_den)) {
					return false;
				}
				*/
				
				// Sadly, I am getting integer overflows on this equality check even using 128 bit integers... So I'm doing this instead:
				if (candidate_num % candidate_den != 0) {
					continue;
				} else if (current_candidate_num % current_candidate_den != 0) {
					current_candidate_num = candidate_num;
					current_candidate_den = candidate_den;
				} else {
					__int128 a = current_candidate_num / current_candidate_den;
					__int128 b = current_candidate_num / current_candidate_den;
					if (a != b) {
						return false;
					}
				}
			} else {
				current_candidate_num = candidate_num;
				current_candidate_den = candidate_den;
				current_candidate_init = true;
			}
		}
	}

	//z = p.z + t*(v.z-c)
	//z = p.z + v.z*t - c*t
	//z = p.z + (v.z*nt)/dt - (nc*nt)/(dc*dt)
	//z = (p.z*dc*dt)/(dc*dt) + (v.z*nt*dc)/(dc*dt) - (nc*nt)/(dc*dt)
	//z = (p.z*dc*dt + v.z*nt*dc - nc*nt) / (dc*dt)

	struct line l = lines[0];
	l.v.x += adjust_x;
	l.v.y += adjust_y;
	
	__int128 time_num, time_den;
	time_at_point(&l, intersection, denominator, &time_num, &time_den);
	
	/*
	__int128 num = l.p.z*current_candidate_den*time_den + l.v.z*time_num*current_candidate_den - current_candidate_num*time_num;
	__int128 det = current_candidate_den * time_den;
	
	ASSERT(num % det == 0, "noooo! my fractions!! :(");
	*pos_z = num / det;
	*/

	// Sadly, that mess above is also giving me some integer overflows...
	ASSERT(time_num % time_den == 0, ":(");
	ASSERT(current_candidate_num % current_candidate_den == 0, ":(");
	*pos_z = l.p.z + (time_num/time_den)*(l.v.z - current_candidate_num/current_candidate_den);

	return true;
}

static void solution2(const char *const input, char *const output) {
	int nlines;
	struct line *lines = parse_input(input, &nlines);

	long res = -1;
	for (int coord_sum=0;; coord_sum++) {
		for (int xcoord=0; xcoord<=coord_sum; xcoord++) {
			int ycoord = coord_sum - xcoord;
			for (int negx=0; negx<=1; negx++) {
				if (negx && xcoord == 0) {
					continue;
				}
				for (int negy=0; negy<=1; negy++) {
					if (negy && ycoord == 0) {
						continue;
					}
					__int128 adjust_x = xcoord*(negx?-1:1);
					__int128 adjust_y = ycoord*(negy?-1:1);
					DBG("vx=%d vy=%d", (int)adjust_x, (int)adjust_y);

					struct point intersection;
					__int128 denominator;
					if (all_lines_collide(lines, nlines, adjust_x, adjust_y, &intersection, &denominator)) {
						DBG(" All lines have a 2D collision at t>=0 by adjusting velocity by %d,%d,X",
						    (int)adjust_x, (int)adjust_y);
						DBG(" The intersection happens at %f,%f,Z", (double)intersection.x/(double)denominator, (double)intersection.y/(double)denominator);

						__int128 pos_z;
						if (find_third_coordinate(lines, nlines, adjust_x, adjust_y, &intersection, denominator, &pos_z)) {
							DBG("  The value of Z is %f", (double)pos_z);
							ASSERT(intersection.x % denominator == 0, "my fractions :(");
							ASSERT(intersection.y % denominator == 0, "my fractions :(");
							res = pos_z + intersection.x / denominator + intersection.y / denominator;
							goto end;
						} else {
							DBG("  Could not find a valid Z.");
						}
					}
				}
			}
		}
	}

 end:
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
	free(lines);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
