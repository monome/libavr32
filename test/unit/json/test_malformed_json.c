#include <stdint.h>
#include <string.h>

#include "unity.h"

#include "json_test_common.c"
#include "json_test_schema.c"

static const char* s[] = {
	"}{",
	"{\"x",
	"{\"x\": [{",
	"{123:456}",
	"0000000000000000000000000000000000000000000000000000000000",
};

void test_malformed_documents(void) {
	for (int i = 0; i < sizeof(s) / sizeof(s[0]); i++) {
		FILE* fp = write_temp_file("in.tmp", s[i], strlen(s[i]));
		set_fp(fp);
		memset(&json_test_dest, 0, sizeof(json_test_dest_t));

		json_read_result_t rd_result = json_read(
			read_fp, copy,
			&json_test_dest, &json_test_docdef,
			json_test_buf, sizeof(json_test_buf),
			json_test_tokens, sizeof(json_test_tokens) / sizeof(json_test_tokens[0]));
		fclose(fp);

		TEST_ASSERT_EQUAL_INT(rd_result, JSON_READ_MALFORMED);
	}
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_malformed_documents);

	return UNITY_END();
}
