typedef struct {
    ulong s;
    ulong us;
} timeval_t;

typedef struct {
    ulong timestamp;
    uint pid;
    timeval_t t1, t2;
} input_t;

__kernel void attack(__global input_t *inputs, __global uint *samples, __global uint *output) {
    // Get the index of the current element
    int i = get_global_id(0);
    input_t input = inputs[i];

    // Calculate lcg_seed
    int lcg_s1 = input.t1.s ^ (input.t1.us << 11); // TODO if gettimeofday fails then lcg_s1 = 1
    int lcg_s2 = input.pid;
    lcg_s2 ^= (input.t2.us << 11); // TODO if gettimeofday fails then ^= 0

    // Calculate php_combined_lcg
    int q, z;
    q = lcg_s1/53668;
    lcg_s1=40014*(lcg_s1-53668*q)-12211*q;
    if(lcg_s1<0)lcg_s1+=2147483563L;
    q = lcg_s2/52774;
    lcg_s2=40692*(lcg_s2-52774*q)-3791*q;
    if(lcg_s2<0)lcg_s2+=2147483399L;

    z = lcg_s1 - lcg_s2;
    if (z < 1) {
        z += 2147483562;
    }
    double php_combined_lcg_out = z * 4.656613e-10;

    // This seed is used to seed srandom()
    uint seed = ((ulong)(input.timestamp * input.pid)) ^ ((ulong) (1000000.0 * php_combined_lcg_out));
    if (seed == samples[0]) {
       *output = seed;
    }

    const int rand_n = 10
    int r[344 + rand_n];
    int o[rand_n];
    int x, j;

    r[0] = seed;
    for (x = 1; x < 31; x++) {
        r[x] = (16807LL * r[x-1]) % 2147483647;
        if (r[x] < 0) {
            r[x] += 2147483647;
        }
    }
    for (x = 31; x < 34; x++) {
        r[x] = r[x-31];
    }
    for (x = 34; x < 344; x++) {
        r[x] = r[x-31] + r[x-3];
    }
    for (j = 0, x = 344; x < 344 + rand_n; x++, j++) {
        o[j] = r[x] = r[x-31] + r[x-3];
    }
}
