#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "json/jsmn/jsmn.h"



//// docdef helper macros

// e.g.
//   .dst_size = sizeof_field(nvram_data_t, kria_state.k[0].p[0]),
//   .dst_offset = offsetof(nvram_data_t, kria_state.k[0].p[0]),
#define sizeof_field(s, m) (sizeof((((s*)0)->m)))

// e.g.
//    DECLARE_STATIC_ALLOC(kria_data_t, k)
//    ...
//    .alloc = STATIC_ALLOC(kria_data_t, k),
//    .free = nop_free,
#define DECLARE_STATIC_ALLOC(type, cache)          \
  static void* static_alloc_##type(size_t size);   \
  static void* static_alloc_##type(size_t size) {  \
    void* ret = NULL;                              \
    if (size == sizeof(type)) {                    \
      ret = &cache;                                \
    }                                              \
    if (ret == NULL) {				   \
      print_dbg("\r\nalloc FAILED");		   \
    }						   \
    return ret;                                    \
  }

#define STATIC_ALLOC(type, cache) static_alloc_##type

void nop_free(void* ptr);


//// error codes
typedef enum {
  JSON_READ_OK,         // finished reading a value
  JSON_READ_INCOMPLETE, // need more text for current value
  JSON_READ_KEEP_GOING, // need more text, read from stream without rewinding
  JSON_READ_MALFORMED,  // invalid json or unexpected property value type
} json_read_result_t;

typedef enum {
  JSON_WRITE_OK,
  JSON_WRITE_ERROR,
} json_write_result_t;



//// callbacks
typedef size_t(*json_gets_cb)(char* dst, size_t len);                  // read from stream
typedef void(*json_puts_cb)(const char* src, size_t len);              // write to stream
typedef void(*json_copy_cb)(char* dst, const char* src, size_t len);;  // save src to dst

typedef void*(*json_alloc_cb)(size_t size);
typedef void(*json_free_cb)(void* ptr);

struct json_docdef_t;

// these are the primary API functions
//   read/write - will be called to read from/write to the byte stream,
//          should be able to handle any read up to textbuf_len
//   copy - will be called to load data from the json stream into the
//          destination address
//   ram - pointer to the destination struct
json_read_result_t json_read(json_gets_cb read,
			     json_copy_cb copy,
			     void* ram, struct json_docdef_t* docdef,
			     char* textbuf, size_t textbuf_len,
			     jsmntok_t* tokbuf, size_t tokbuf_len);
json_write_result_t json_write(json_puts_cb write,
			       void* ram, struct json_docdef_t* docdef);

// callbacks for handling an object/array/string/...
//   ram - base address of the structure mapped to/from JSON
//   dst/src_offset - amount of offset to apply in addition to the
//     offset from the docdef's params, for e.g. array items
typedef json_read_result_t(*json_read_subtree_cb)(
	jsmntok_t* token,
	json_copy_cb copy, void* ram, struct json_docdef_t* docdef,
	const char* text, size_t text_len,
	int32_t dst_offset);
typedef json_write_result_t(*json_write_subtree_cb)(
	json_puts_cb write,
	void* ram, struct json_docdef_t* docdef,
	size_t src_offset);

// describes how to read/write a JSON subtree.
// fresh - true if the state should be re-initialized on next usage
// state/params - up to you. most of the basic ones have params:
//   dst_offset - the offsetof(ram_t, field) the field in ram_t to read/write
//   dst_size - the sizeof_field(ram_t, field) the field
typedef struct json_docdef_t {
	char* name;
	json_read_subtree_cb read;
	json_write_subtree_cb write;
	bool fresh;
	bool skip;
	void* state;
	void* params;
} json_docdef_t;

json_read_result_t json_read_object(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len,
	int32_t dst_offset);

json_read_result_t json_read_object_cached(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len,
	int32_t dst_offset);

json_write_result_t json_write_object(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);



//// JSON node handler parameters and state
typedef struct {
	json_docdef_t* docdefs;
	uint8_t docdef_ct;
        int32_t dst_offset;
        size_t dst_size;
        json_alloc_cb alloc;
        json_free_cb free;
} json_read_object_params_t;

typedef enum {
	JSON_OBJECT_MATCH_START,
	JSON_OBJECT_SKIP_SECTION,
	JSON_OBJECT_MATCH_NAME,
	JSON_OBJECT_PARSE_PROPERTY,
} json_object_read_phase_t;
typedef struct {
	json_object_read_phase_t object_state;
	json_docdef_t* active_docdef;
	uint8_t sections_handled;
	unsigned int depth;
        void* cache;
} json_read_object_state_t;


typedef struct {
	size_t array_len;
	size_t item_size;
	json_docdef_t* item_docdef;
} json_read_array_params_t;

json_read_result_t json_read_array(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len,
	int32_t dst_offset);
json_write_result_t json_write_array(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);
typedef enum {
	JSON_ARRAY_MATCH_START,
	JSON_ARRAY_MATCH_ITEMS,
} json_array_read_phase_t;
typedef struct {
        json_array_read_phase_t array_state;
	size_t array_ct;
	unsigned int depth;
} json_read_array_state_t;


typedef struct {
	int32_t dst_offset;
	uint8_t dst_size;
	bool signed_val;
} json_read_scalar_params_t;

json_read_result_t json_read_scalar(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset);
json_write_result_t json_write_number(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);
json_write_result_t json_write_bool(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);


typedef struct {
	uint8_t option_ct;
	const char** options;
	int32_t dst_offset;
	int default_val;
} json_read_enum_params_t;

json_read_result_t json_read_enum(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset);
json_write_result_t json_write_enum(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);

typedef struct {
	size_t dst_size;
	int32_t dst_offset;
} json_read_buffer_params_t;

json_read_result_t json_read_buffer(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset);
json_write_result_t json_write_buffer(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);
typedef struct {
	size_t buf_pos;
} json_read_buffer_state_t;


// uses read_buffer_params
json_read_result_t json_read_string(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset);
json_write_result_t json_write_string(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);


typedef struct {
	const char* to_match;
	bool skip;
} json_match_string_params_t;

json_read_result_t json_match_string(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset);
json_write_result_t json_write_constant_string(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset);


// top-level parser state
// will be initialized during json_read call
typedef struct {
	jsmn_parser jsmn;
	size_t text_ct;
	unsigned int curr_tok;
} json_read_state_t;


// helpers for visiting docdefs
json_docdef_t* json_docdef_find_key(json_docdef_t* object_docdef, const char* name);
