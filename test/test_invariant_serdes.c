#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "src/json/serdes.c"

START_TEST(test_memcpy_bounds_invariant)
{
    // Invariant: memcpy operations must not write beyond destination buffer boundaries
    const char *payloads[] = {
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",  // 64-byte overflow
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",  // 32-byte boundary case
        "valid"  // Valid input
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);
    
    for (int i = 0; i < num_payloads; i++) {
        char dst[32] = {0};  // Fixed-size destination buffer
        const char *src = payloads[i];
        size_t len = strlen(src);
        
        // The security property: length must not exceed destination buffer size
        ck_assert_msg(len <= sizeof(dst), 
                     "memcpy would overflow: len=%zu, dst_size=%zu", 
                     len, sizeof(dst));
        
        // If assertion passes, execute the actual memcpy
        if (len <= sizeof(dst)) {
            memcpy((void*)dst, (void*)src, len);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_memcpy_bounds_invariant);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}