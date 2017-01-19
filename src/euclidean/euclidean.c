#include "euclidean.h"
#include "data.h"

static const char* table_euclidean[32] = {
    (const char*)table_euclidean_1,  (const char*)table_euclidean_2,
    (const char*)table_euclidean_3,  (const char*)table_euclidean_4,
    (const char*)table_euclidean_5,  (const char*)table_euclidean_6,
    (const char*)table_euclidean_7,  (const char*)table_euclidean_8,
    (const char*)table_euclidean_9,  (const char*)table_euclidean_10,
    (const char*)table_euclidean_11, (const char*)table_euclidean_12,
    (const char*)table_euclidean_13, (const char*)table_euclidean_14,
    (const char*)table_euclidean_15, (const char*)table_euclidean_16,
    (const char*)table_euclidean_17, (const char*)table_euclidean_18,
    (const char*)table_euclidean_19, (const char*)table_euclidean_20,
    (const char*)table_euclidean_21, (const char*)table_euclidean_22,
    (const char*)table_euclidean_23, (const char*)table_euclidean_24,
    (const char*)table_euclidean_25, (const char*)table_euclidean_26,
    (const char*)table_euclidean_27, (const char*)table_euclidean_28,
    (const char*)table_euclidean_29, (const char*)table_euclidean_30,
    (const char*)table_euclidean_31, (const char*)table_euclidean_32
};

static char get_byte(const char* a, int n) {
    return a[n / 8];
}

static int get_bit(const char* a, int k) {
    char byte = get_byte(a, k);
    int bit_index = 7 - (k % 8);
    return (byte & (1 << bit_index)) != 0;
}

int euclidean(int fill, int len, int step) {
    if (len < 1 || len > 32) return 0;
    if (fill < 1 || fill > len) return 0;

    const char* len_table = table_euclidean[len - 1];
    int entry_size = len / 8;
    if (len % 8 > 0) entry_size++;
    const char* table = &len_table[fill * entry_size];
    // adjust step, s.t. 0 <= step < len
    int remainder = step % len;
    int modulo = remainder < 0 ? remainder + len : remainder;
    return get_bit(table, modulo);
}
