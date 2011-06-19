// Sumit Gulwani, Krishna K. Mehra, Trishul M. Chilimbi: SPEED: precise and
// efficient static estimation of program computational complexity.
// POPL 2009: 127-139
// figure 3b

// $Id$

// parameters

#define DO_CONTROL 0
#define DO_CHECKING 1
#define GOOD (c <= n)

// tools

#include <stdlib.h>
#include <stdio.h>

int rand_b(void) {
	return rand() % 2;
}
int rand_z(void) {
	return rand() - rand();
}

#define OR(t1, t2) {if (rand_b()) {t1} else {t2}}
#define LOOP(j) {while (rand_b()) {j}}

void deadlock() {
	printf("deadlock\n");
	while (1);
}
#define ASSUME(c) {if (!(c)) deadlock();}

#if DO_CONTROL == 0
#define CONTROL(c)
#else
void control_error(void) {
	fprintf(stderr, "control error");
	exit(1);
}
#define CONTROL(c) {if (!(c)) control_error();}
#endif

#if DO_CHECKING == 0
#define CHECK(c)
#define CHECK_NOT(c)
#else
void checking_error(void) {
	fprintf(stderr, "checking error");
	exit(2);
}
#define CHECK(c) {if (!(c)) checking_error();}
#define CHECK_NOT(c) {if (c) checking_error();}
#endif

#define COMMAND_NOCHECK(g, a) {ASSUME(g); a;}
#ifdef GOOD
#define COMMAND(g, a) {COMMAND_NOCHECK(g, a); CHECK(GOOD);}
#else
#ifdef BAD
#define COMMAND(g, a) {COMMAND_NOCHECK(g, a); CHECK_NOT(BAD);}
#endif
#endif

// control and commands

#define S1 CONTROL(s == 1 && x < n)
#define S2 CONTROL(s == 2 && x < n)
#define S3 CONTROL(s == 1 && x >= n)
#define S4 CONTROL(s == 2 && x >= n)

#define G1 (s == 1 && x < n)
#define G1a (s == 1 && x < n - 1)
#define G1b (s == 1 && x == n - 1)
#define A1 {x++; c++;}
#define C1 COMMAND(G1, A1)
#define C1a COMMAND(G1a, A1)
#define C1b COMMAND(G1b, A1)

#define G2 (s == 1)
#define A2 {s = 2;}
#define C2 COMMAND(G2, A2)

#define G3 (s == 2 && x < n)
#define G3a (s == 2 && x < n - 1)
#define G3b (s == 2 && x == n - 1)
#define A3 {x++; c++;}
#define C3 COMMAND(G3, A3)
#define C3a COMMAND(G3a, A3)
#define C3b COMMAND(G3b, A3)

#define INI {n = rand(); x = c = 0; s = 1;}

// transition system

void ts_singlestate(void) {
	int n, x, c, s;
	INI;
	LOOP(OR(C1, OR(C2, C3)));
}

void ts_restructured(void) {
	int n, x, c, s;
	INI;
	if (x >= n) {
		S3; C2; S4;
	}
	else {
		S1; LOOP(C1a; S1);
		OR(
			C1b; S3; C2; S4,
			
			C2; S2; LOOP(C3a; S2); C3b; S4
		)
	}
}

int main(void) {
	ts_singlestate();
	ts_restructured();
	return 0;
}

