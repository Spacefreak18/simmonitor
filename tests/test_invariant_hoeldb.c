#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/*
 * Security invariant: When constructing SQL query strings with integer session IDs,
 * the resulting query must NEVER exceed the allocated buffer size (150 bytes for
 * the first query, 250 bytes for the second). Any safe implementation must either
 * use dynamic allocation (asprintf) or perform bounds checking before writing.
 *
 * This test verifies that a safe query-building function never overflows its buffer
 * by checking the output length against the fixed buffer sizes used in the vulnerable code.
 */

#define SMALL_BUFFER_SIZE  150
#define LARGE_BUFFER_SIZE  250

/* Safe reimplementation using snprintf to demonstrate what the invariant requires */
static int safe_build_query_small(char *buf, size_t bufsize, int session_id) {
    int written = snprintf(buf, bufsize,
        "select stint_id, driver_id, team_member_id, session_id, car_id, game_car_id, laps, valid_laps, best_lap_id FROM %s WHERE session_id=%i",
        "Stints", session_id);
    return written; /* returns number of chars that would have been written */
}

static int safe_build_query_large(char *buf, size_t bufsize, int session_id) {
    int written = snprintf(buf, bufsize,
        "select stint_id, driver_id, team_member_id, session_id, car_id, game_car_id, laps, valid_laps, best_lap_id, start_time, end_time FROM %s WHERE session_id=%i AND valid=1",
        "Stints", session_id);
    return written;
}

/* Integer payloads as strings for documentation; we convert them to int */
static int adversarial_ids[] = {
    0,
    1,
    -1,
    INT_MAX,
    INT_MIN,
    INT_MAX - 1,
    INT_MIN + 1,
    999999999,
    -999999999,
    2147483647,
    -2147483648,
    100,
    9999,
    99999,
    999999,
    9999999,
    99999999,
    1000000000,
    -1000000000,
    42,
    0x7FFFFFFF,
    (int)0x80000000,
    (int)0xFFFFFFFF,
    (int)0xDEADBEEF,
    (int)0xCAFEBABE,
};

START_TEST(test_query_buffer_never_overflows_small)
{
    /* Invariant: The query string built with any integer session_id must fit
     * within SMALL_BUFFER_SIZE bytes (including null terminator).
     * If it does not fit, the implementation MUST NOT use sprintf with a fixed
     * 150-byte buffer — it must use dynamic allocation or bounds-checked functions.
     */
    int num_ids = sizeof(adversarial_ids) / sizeof(adversarial_ids[0]);

    for (int i = 0; i < num_ids; i++) {
        int session_id = adversarial_ids[i];
        char buf[SMALL_BUFFER_SIZE];
        memset(buf, 0xAA, sizeof(buf)); /* poison buffer */

        int would_write = safe_build_query_small(buf, sizeof(buf), session_id);

        /* The invariant: if the query would exceed the buffer, the code MUST NOT
         * use a fixed 150-byte buffer with sprintf. We assert that either:
         * (a) the query fits safely, OR
         * (b) the implementation detects overflow (would_write >= bufsize means truncated)
         *
         * Here we verify snprintf correctly truncated and did not write beyond buffer.
         */

        /* snprintf must never write more than bufsize bytes */
        ck_assert_msg(strlen(buf) < SMALL_BUFFER_SIZE,
            "Query string length %zu exceeds or equals buffer size %d for session_id=%d",
            strlen(buf), SMALL_BUFFER_SIZE, session_id);

        /* The null terminator must be present within the buffer */
        int null_found = 0;
        for (int j = 0; j < SMALL_BUFFER_SIZE; j++) {
            if (buf[j] == '\0') {
                null_found = 1;
                break;
            }
        }
        ck_assert_msg(null_found,
            "No null terminator found within buffer bounds for session_id=%d", session_id);

        /* If would_write >= SMALL_BUFFER_SIZE, the fixed buffer is insufficient
         * and the vulnerable sprintf would overflow. Flag this as a security concern. */
        if (would_write >= SMALL_BUFFER_SIZE) {
            /* The invariant is violated by the original code: document the overflow */
            ck_assert_msg(0,
                "SECURITY VIOLATION: query for session_id=%d requires %d bytes but buffer is only %d bytes. "
                "sprintf would overflow. Must use asprintf or dynamic allocation.",
                session_id, would_write + 1, SMALL_BUFFER_SIZE);
        }
    }
}
END_TEST

START_TEST(test_query_buffer_never_overflows_large)
{
    /* Invariant: The larger query string must fit within LARGE_BUFFER_SIZE bytes */
    int num_ids = sizeof(adversarial_ids) / sizeof(adversarial_ids[0]);

    for (int i = 0; i < num_ids; i++) {
        int session_id = adversarial_ids[i];
        char buf[LARGE_BUFFER_SIZE];
        memset(buf, 0xBB, sizeof(buf));

        int would_write = safe_build_query_large(buf, sizeof(buf), session_id);

        ck_assert_msg(strlen(buf) < LARGE_BUFFER_SIZE,
            "Large query string length %zu exceeds or equals buffer size %d for session_id=%d",
            strlen(buf), LARGE_BUFFER_SIZE, session_id);

        int null_found = 0;
        for (int j = 0; j < LARGE_BUFFER_SIZE; j++) {
            if (buf[j] == '\0') {
                null_found = 1;
                break;
            }
        }
        ck_assert_msg(null_found,
            "No null terminator found within large buffer bounds for session_id=%d", session_id);

        if (would_write >= LARGE_BUFFER_SIZE) {
            ck_assert_msg(0,
                "SECURITY VIOLATION: large query for session_id=%d requires %d bytes but buffer is only %d bytes. "
                "sprintf would overflow. Must use asprintf or dynamic allocation.",
                session_id, would_write + 1, LARGE_BUFFER_SIZE);
        }
    }
}
END_TEST

START_TEST(test_query_sql_injection_boundary)
{
    /* Invariant: Integer session IDs must be numeric only — no SQL injection possible
     * through integer formatting. The formatted integer must match expected numeric output.
     */
    int num_ids = sizeof(adversarial_ids) / sizeof(adversarial_ids[0]);

    for (int i = 0; i < num_ids; i++) {
        int session_id = adversarial_ids[i];

        /* Build expected integer string */
        char expected_int[32];
        snprintf(expected_int, sizeof(expected_int), "%i", session_id);

        /* Verify the integer string contains only digits and optional leading minus */
        size_t len = strlen(expected_int);
        ck_assert_msg(len > 0, "Integer string must not be empty for session_id=%d", session_id);

        int start = (expected_int[0] == '-') ? 1 : 0;
        for (size_t j = (size_t)start; j < len; j++) {
            ck_assert_msg(expected_int[j] >= '0' && expected_int[j] <= '9',
                "Non-numeric character '%c' in integer representation of session_id=%d — potential injection",
                expected_int[j], session_id);
        }

        /* The integer representation must not contain SQL metacharacters */
        const char *sql_meta = "';\"--/*\\";
        for (size_t j = 0; j < strlen(sql_meta); j++) {
            ck_assert_msg(strchr(expected_int, sql_meta[j]) == NULL,
                "SQL metacharacter '%c' found in integer representation of session_id=%d",
                sql_meta[j], session_id);
        }
    }
}
END_TEST

START_TEST(test_asprintf_dynamic_allocation_safety)
{
    /* Invariant: Using asprintf (dynamic allocation) must always succeed and produce
     * a correctly sized buffer — this is the safe alternative to fixed-size malloc+sprintf.
     */
    int num_ids = sizeof(adversarial_ids) / sizeof(adversarial_ids[0]);

    for (int i = 0; i < num_ids; i++) {
        int session_id = adversarial_ids[i];
        char *query = NULL;

        int ret = asprintf(&query,
            "select stint_id, driver_id, team_member_id, session_id, car_id, game_car_id, laps, valid_laps, best_lap_id FROM %s WHERE session_id=%i",
            "Stints", session_id);

        ck_assert_msg(ret >= 0, "asprintf failed for session_id=%d", session_id);
        ck_assert_msg(query != NULL, "asprintf returned NULL query for session_id=%d", session_id);

        /* The returned length must match actual string length */
        ck_assert_msg((size_t)ret == strlen(query),
            "asprintf reported length %d but strlen is %zu for session_id=%d",
            ret, strlen(query), session_id);

        /* The query must contain the table name */
        ck_assert_msg(strstr(query, "Stints") != NULL,
            "Query missing table name 'Stints' for session_id=%d", session_id);

        /* The query must contain the session_id value */
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "%i", session_id);
        ck_assert_msg(strstr(query, id_str) != NULL,
            "Query missing session_id value '%s' for session_id=%d", id_str, session_id);

        free(query);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_query_buffer_never_overflows_small);
    tcase_add_test(tc_core, test_query_buffer_never_overflows_large);
    tcase_add_test(tc_core, test_query_sql_injection_boundary);
    tcase_add_test(tc_core, test_asprintf_dynamic_allocation_safety);
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