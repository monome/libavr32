#include "json_test_schema.h"

#include "json/serdes.h"

json_test_dest_t json_test_dest;
json_read_object_state_t test_json_object_state[2];
json_read_array_state_t test_json_array_state;
json_read_buffer_state_t test_json_buffer_state;

json_docdef_t json_test_docdef = {
	.read = json_read_object,
	.write = json_write_object,
	.state = &test_json_object_state[0],
	.params = &((json_read_object_params_t) {
		.docdef_ct = 12,
		.docdefs = ((json_docdef_t[]) {
			{
				.name = "ubyte",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, ubyte),
					.dst_offset = offsetof(json_test_dest_t, ubyte),
					.signed_val = false,
				}),
			},
			{
				.name = "sbyte",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, sbyte),
					.dst_offset = offsetof(json_test_dest_t, sbyte),
					.signed_val = true,
				}),
			},
			{
				.name = "ushort",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, ushort),
					.dst_offset = offsetof(json_test_dest_t, ushort),
					.signed_val = false,
				}),
			},
			{
				.name = "sshort",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, sshort),
					.dst_offset = offsetof(json_test_dest_t, sshort),
					.signed_val = true,
				}),
			},
			{
				.name = "ulong",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, ulong),
					.dst_offset = offsetof(json_test_dest_t, ulong),
					.signed_val = false,
				}),
			},
			{
				.name = "slong",
				.read = json_read_scalar,
				.write = json_write_number,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, slong),
					.dst_offset = offsetof(json_test_dest_t, slong),
					.signed_val = true,
				}),
			},
			{
				.name = "boolean",
				.read = json_read_scalar,
				.write = json_write_bool,
				.params = &((json_read_scalar_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, boolean),
					.dst_offset = offsetof(json_test_dest_t, boolean),
				}),
			},
			{
				.name = "test_enum",
				.read = json_read_enum,
				.write = json_write_enum,
				.params = &((json_read_enum_params_t) {
					.option_ct = 3,
					.options = ((const char*[]) {
						"ZERO",
						"ONE",
						"TWO",
					}),
					.dst_offset = offsetof(json_test_dest_t, test_enum),
					.default_val = 0,
				}),
			},
			{
				.name = "buffer",
				.read = json_read_buffer,
				.write = json_write_buffer,
				.state = &test_json_buffer_state,
				.params = &((json_read_buffer_params_t) {
					.dst_size = sizeof_field(json_test_dest_t, buffer),
					.dst_offset = offsetof(json_test_dest_t, buffer),
				}),
			},
			{
				.name = "nested",
				.read = json_read_object,
				.write = json_write_object,
				.state = &test_json_object_state[1],
				.params = &((json_read_object_params_t) {
					.docdef_ct = 1,
					.docdefs = ((json_docdef_t[]) {
						{
							.name = "ubyte",
							.read = json_read_scalar,
							.write = json_write_number,
							.params = &((json_read_scalar_params_t) {
								.dst_size = sizeof_field(json_test_dest_t, nested.ubyte),
								.dst_offset = offsetof(json_test_dest_t, nested.ubyte),
								.signed_val = false,
							}),
						},
					}),
				}),
			},
			{
				.name = "nested_array",
				.read = json_read_array,
				.write = json_write_array,
				.state = &test_json_array_state,
				.params = &((json_read_array_params_t) {
					.array_len = sizeof_field(json_test_dest_t, nested_array) / sizeof_field(json_test_dest_t, nested_array[0]),
					.item_size = sizeof_field(json_test_dest_t, nested_array[0]),
					.item_docdef = &((json_docdef_t) {
						.read = json_read_object,
						.write = json_write_object,
						.state = &test_json_object_state[1],
						.params = &((json_read_object_params_t) {
							.docdef_ct = 1,
							.docdefs = ((json_docdef_t[]) {
								{
									.name = "ubyte",
									.read = json_read_scalar,
									.write = json_write_number,
									.params = &((json_read_scalar_params_t) {
										.dst_size = sizeof_field(json_test_dest_t, nested_array[0].ubyte),
										.dst_offset = offsetof(json_test_dest_t, nested_array[0].ubyte),
										.signed_val = false,
									}),
								},
							}),
						}),
					}),
				}),
			},
			{
				.name = "longstring",
				.read = json_read_string,
				.write = json_write_string,
				.state = &test_json_buffer_state,
				.params = &((json_read_buffer_params_t) {
					.dst_offset = offsetof(json_test_dest_t, longstring),
					.dst_size = sizeof_field(json_test_dest_t, longstring),
				}),
			},
		}),
	}),
};
