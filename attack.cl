typedef struct {
    ulong s;
    ulong us;
} timeval_t;

typedef struct {
    ulong timestamp;
    uint pid;
    timeval_t t1, t2;
} input_t;

__kernel void attack(__global input_t *inputs, __global int *samples, __global int *output) {
    // Get the index of the current element
    int i = get_global_id(0);
    input_t input = inputs[i];

    // Calculate lcg_seed
    int lcg_s1 = input.t1.s ^ (input.t1.us << 11); // TODO if gettimeofday fails then lcg_s1 = 1
    int lcg_s2 = input.pid;
    lcg_s2 ^= (input.t2.us << 11);
}

/*
#define GENERATE_SEED() (((zend_long) (time(0) * getpid())) ^ ((zend_long) (1000000.0 * php_combined_lcg(TSRMLS_C))))

#define MODMULT(a, b, c, m, s) q = s/a;s=b*(s-a*q)-c*q;if(s<0)s+=m
PHPAPI double php_combined_lcg(TSRMLS_D)
{
	php_int32 q;
	php_int32 z;

	if (!LCG(seeded)) {
		lcg_seed(TSRMLS_C);
	}

	MODMULT(53668, 40014, 12211, 2147483563L, LCG(s1));
	MODMULT(52774, 40692, 3791, 2147483399L, LCG(s2));

	z = LCG(s1) - LCG(s2);
	if (z < 1) {
		z += 2147483562;
	}

	return z * 4.656613e-10;
}   

static void lcg_seed(TSRMLS_D)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0) {
		LCG(s1) = tv.tv_sec ^ (tv.tv_usec<<11);
	} else {
		LCG(s1) = 1;
	}
	LCG(s2) = (zend_long) getpid();

	if (gettimeofday(&tv, NULL) == 0) {
		LCG(s2) ^= (tv.tv_usec<<11);
	}

	LCG(seeded) = 1;
}
*/
