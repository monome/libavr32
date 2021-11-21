#include "json/serdes.h"
#include "json/encoding.h"

#define JSON_DEBUG 1
#include "print_funcs.h"


void nop_free(void* ptr) {
}

json_read_result_t json_read_object(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_object_params_t* params = (json_read_object_params_t*)docdef->params;
	json_read_object_state_t* state = (json_read_object_state_t*)docdef->state;

	if (docdef->fresh) {
		docdef->fresh = false;
		state->object_state = JSON_OBJECT_MATCH_START;
		state->active_docdef = NULL;
		state->sections_handled = 0;
		state->depth = 0;
	}

	if (tok->type == JSMN_OBJECT
	 && tok->depth < state->depth
	 && text[tok->end - 1] == '}') {
		docdef->fresh = true;
		return JSON_READ_OK;
	}

	switch (state->object_state) {
	case JSON_OBJECT_MATCH_START:
		if (tok->type != JSMN_OBJECT) {
			print_dbg("\r\n!! bad token type for object start: ");
			print_dbg_hex(tok->type);
			return JSON_READ_MALFORMED;
		}
		state->depth = tok->depth + 1;
		state->object_state = JSON_OBJECT_MATCH_NAME;
		return JSON_READ_INCOMPLETE;
	case JSON_OBJECT_SKIP_SECTION:
		if (tok->depth > state->depth || tok->type != JSMN_STRING) {
			return JSON_READ_INCOMPLETE;
		}
		state->object_state = JSON_OBJECT_MATCH_NAME;
		// fallthrough
	case JSON_OBJECT_MATCH_NAME:
		if (tok->type != JSMN_STRING) {
			print_dbg("\r\n!! bad token type for object name match: ");
			print_dbg_hex(tok->type);
			return JSON_READ_MALFORMED;
		}
		if (tok->end > 0) {
			for (uint8_t i = 0; i < params->docdef_ct; i++) {
				if (strncmp(params->docdefs[i].name, text + tok->start, tok->end - tok->start) == 0) {
					if (params->docdefs[i].skip) {
						continue;
					}
					state->active_docdef = &params->docdefs[i];
					state->active_docdef->fresh = true;
					state->object_state = JSON_OBJECT_PARSE_PROPERTY;
					return JSON_READ_INCOMPLETE;
				}
			}
			state->object_state = JSON_OBJECT_SKIP_SECTION;
		}
		return JSON_READ_INCOMPLETE;
	case JSON_OBJECT_PARSE_PROPERTY: {
		json_read_result_t property_result = state->active_docdef->read(
			tok,
			copy, ram, state->active_docdef,
			text, text_len, dst_offset);
		switch(property_result) {
		case JSON_READ_INCOMPLETE:
			return JSON_READ_INCOMPLETE;
		case JSON_READ_KEEP_GOING:
			return JSON_READ_KEEP_GOING;
		case JSON_READ_OK:
			state->object_state = JSON_OBJECT_MATCH_NAME;
			return JSON_READ_INCOMPLETE;
		default:
			print_dbg("\r\n!! bad property value: ");
			print_dbg(state->active_docdef->name);
			return JSON_READ_MALFORMED;
		}
	}
	default:
		  print_dbg("\r\n!! bad object state: ");
		  print_dbg_hex(state->object_state);
		return JSON_READ_MALFORMED;
	}
}

static void copy_cached(char* dst, const char* src, size_t len) {
	memcpy((void*)dst, (void*)src, len);
}

json_read_result_t json_read_object_cached(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_object_params_t* params = (json_read_object_params_t*)docdef->params;
	json_read_object_state_t* state = (json_read_object_state_t*)docdef->state;

	if (docdef->fresh) {
		docdef->fresh = false;
		state->object_state = JSON_OBJECT_MATCH_START;
		state->active_docdef = NULL;
		state->sections_handled = 0;
		state->depth = 0;
		state->cache = NULL;
		state->cache = params->alloc(params->dst_size);
		if (state->cache == NULL) {
			print_dbg("\r\n!! allocation failed: ");
			print_dbg_hex(state->object_state);
			return JSON_READ_MALFORMED;
		}
	}


	if (tok->type == JSMN_OBJECT && tok->depth < state->depth && text[tok->end - 1] == '}') {
		docdef->fresh = true;
		copy(
			(char*)ram + params->dst_offset + dst_offset,
			state->cache,
			params->dst_size);
		if (state->cache != NULL) {
			params->free(state->cache);
		}
		return JSON_READ_OK;
	}

	switch (state->object_state) {
	case JSON_OBJECT_MATCH_START:
		if (tok->type != JSMN_OBJECT) {
			print_dbg("\r\n!! bad token type for object start: ");
			print_dbg_hex(tok->type);
			if (state->cache != NULL) {
				params->free(state->cache);
			}
			return JSON_READ_MALFORMED;
		}
		state->depth = tok->depth + 1;
		state->object_state = JSON_OBJECT_MATCH_NAME;
		return JSON_READ_INCOMPLETE;
	case JSON_OBJECT_SKIP_SECTION:
		if (tok->depth > state->depth || tok->type != JSMN_STRING) {
			return JSON_READ_INCOMPLETE;
		}
		state->object_state = JSON_OBJECT_MATCH_NAME;
		// fallthrough
	case JSON_OBJECT_MATCH_NAME:
		if (tok->type != JSMN_STRING) {
			print_dbg("\r\n!! bad token type for object name match: ");
			print_dbg_hex(tok->type);
			print_dbg(" (");
			print_dbg_hex(tok->start);
			print_dbg(" - ");
			print_dbg_hex(tok->end);
			print_dbg(")");
			if (state->cache != NULL) {
				params->free(state->cache);
			}
			return JSON_READ_MALFORMED;
		}
		if (tok->end > 0) {
			for (uint8_t i = 0; i < params->docdef_ct; i++) {
				if (strncmp(params->docdefs[i].name, text + tok->start, tok->end - tok->start) == 0) {
					if (params->docdefs[i].skip) {
						continue;
					}
					state->active_docdef = &params->docdefs[i];
					state->active_docdef->fresh = true;
					state->object_state = JSON_OBJECT_PARSE_PROPERTY;
					return JSON_READ_INCOMPLETE;
				}
			}
			state->object_state = JSON_OBJECT_SKIP_SECTION;
		}
		return JSON_READ_INCOMPLETE;
	case JSON_OBJECT_PARSE_PROPERTY: {
#if JSON_DEBUG
	  print_dbg("\r\n descending to ");
	  print_dbg(state->active_docdef->name);
	  print_dbg(" with offset ");
	  /* print_dbg_hex(dst_offset); */
	  /* print_dbg(" + "); */
	  print_dbg_hex(-params->dst_offset);
	  /* print_dbg(" = "); */
	  /* print_dbg_hex(dst_offset - params->dst_offset); */
	  print_dbg(", cache @ ");
	  print_dbg_hex(state->cache);
#endif
		// need to offset everything below this
		// to make up for the "absolute" offsets in the docdef
		json_read_result_t prop_result = state->active_docdef->read(
			tok,
			copy_cached, state->cache, state->active_docdef,
			text, text_len, -params->dst_offset);
		switch (prop_result) {
		case JSON_READ_KEEP_GOING:
#if JSON_DEBUG
		  print_dbg("\r\ncached object: keep going");
#endif
			return JSON_READ_KEEP_GOING;
		case JSON_READ_INCOMPLETE:
			return JSON_READ_INCOMPLETE;
		case JSON_READ_OK:
			state->object_state = JSON_OBJECT_MATCH_NAME;
			return JSON_READ_INCOMPLETE;
		default:
#if JSON_DEBUG
			print_dbg("\r\n!! bad property value: ");
			print_dbg(state->active_docdef->name);
#endif
			if (state->cache != NULL) {
				params->free(state->cache);
			}
			return JSON_READ_MALFORMED;
		}
	}
	default:
#if JSON_DEBUG
			print_dbg("\r\n!! bad cached object state: ");
			print_dbg_hex(state->object_state);
#endif
		if (state->cache != NULL) {
			params->free(state->cache);
		}
		return JSON_READ_MALFORMED;
	}
}

json_write_result_t json_write_object(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_object_params_t* params = (json_read_object_params_t*)docdef->params;

	write("{", 1);
	for (uint8_t i = 0; i < params->docdef_ct; i++) {
		if (params->docdefs[i].skip) {
			continue;
		}

		write("\"", 1);
		write(params->docdefs[i].name, strlen(params->docdefs[i].name));
		write("\": ", 3);
		params->docdefs[i].write(write, ram, &params->docdefs[i], src_offset);
		if (i + 1 < params->docdef_ct) {
			write(", ", 2);
		}
	}
	write("}", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_read_scalar(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_scalar_params_t* params = (json_read_scalar_params_t*)docdef->params;
	if (tok->type != JSMN_PRIMITIVE) {
		print_dbg("\r\n!! unexpected token type for scalar: ");
		print_dbg_hex(tok->type);
		return JSON_READ_MALFORMED;
	}
	if (tok->end < 0) {
		return JSON_READ_INCOMPLETE;
	}
	void* dst = (char*)ram + params->dst_offset + dst_offset;
	int val = decode_decimal(text + tok->start, tok->end - tok->start);

#if JSON_DEBUG
	print_dbg("\r\n> read scalar: ");
        print_dbg(docdef->name);
	print_dbg(" = ");
	print_dbg_hex(val);
	print_dbg(" @ ");
	print_dbg_hex(dst);
#endif

	switch (params->dst_size) {
	case sizeof(uint8_t): {
		uint8_t src = val;
		copy((char*)dst, (char*)&src, sizeof(uint8_t));
		break;
	}
	case sizeof(uint16_t): {
		uint16_t src = val;
		copy((char*)dst, (char*)&src, sizeof(uint16_t));
		break;
	}
	case sizeof(uint32_t): {
		uint32_t src = val;
		copy((char*)dst, (char*)&src, sizeof(uint32_t));
		break;
	}
	default:
		print_dbg("\r\n!! unknown scalar size: ");
		print_dbg_hex(params->dst_size);
		return JSON_READ_MALFORMED;
	}
	return JSON_READ_OK;
}

json_write_result_t json_write_number(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_scalar_params_t* params = (json_read_scalar_params_t*)docdef->params;
	void* src = (uint8_t*)ram + src_offset + params->dst_offset;
	char* dec;

#if JSON_DEBUG
	print_dbg("\r\nwrite scalar, size ");
	print_dbg_hex(params->dst_size);
#endif

	if (params->signed_val) {
#if JSON_DEBUG
		print_dbg(", signed");
#endif
		switch (params->dst_size) {
		case 4:
			dec = encode_decimal_signed(*(int32_t*)src);
			break;
		case 2:
			dec = encode_decimal_signed(*(int16_t*)src);
			break;
		case 1:
			dec = encode_decimal_signed(*(int8_t*)src);
			break;
		default:
			return JSON_WRITE_ERROR;
		}
	} else {
#if JSON_DEBUG
		print_dbg(", unsigned");
#endif
		switch (params->dst_size) {
		case 4:
			dec = encode_decimal_unsigned(*(uint32_t*)src);
			break;
		case 2:
			dec = encode_decimal_unsigned(*(uint16_t*)src);
			break;
		case 1:
			dec = encode_decimal_unsigned(*(uint8_t*)src);
			break;
		default:
			return JSON_WRITE_ERROR;
		}
	}
#if JSON_DEBUG
	print_dbg("\r\nencoded decimal has len ");
	print_dbg_hex(strlen(dec));
#endif
	write(dec, strlen(dec));
	return JSON_WRITE_OK;
}

json_write_result_t json_write_bool(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_scalar_params_t* params = (json_read_scalar_params_t*)docdef->params;

	bool val = *((uint8_t*)ram + src_offset + params->dst_offset);

	if (val) {
		write("true", 4);
	}
	else {
		write("false", 5);
	}
	return JSON_WRITE_OK;
}

json_write_result_t json_write_string(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;

	write("\"", 1);
	write((char*)ram + src_offset + params->dst_offset, params->dst_size);
	write("\"", 1);
	return JSON_WRITE_OK;
}

json_write_result_t json_write_constant_string(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_match_string_params_t* params = (json_match_string_params_t*)docdef->params;

	write("\"", 1);
	write(params->to_match, strlen(params->to_match));
	write("\"", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_match_string(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_match_string_params_t* params = (json_match_string_params_t*)docdef->params;
	if (tok->type != JSMN_STRING) {
		print_dbg("\r\n!! unexpected token type for string match: ");
		print_dbg_hex(tok->type);
		return JSON_READ_MALFORMED;
	}
	if (tok->end < 0) {
		return JSON_READ_INCOMPLETE;
	}
	if (!params->skip && strncmp(params->to_match, text + tok->start, tok->end - tok->start) != 0) {
		print_dbg("\r\n!! incorrect string match: ");
		print_dbg(params->to_match);
		return JSON_READ_MALFORMED;
	}
	return JSON_READ_OK;
}

json_read_result_t json_read_enum(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_enum_params_t* params = (json_read_enum_params_t*)docdef->params;

	char* dst = (char*)ram + params->dst_offset + dst_offset;

	if (tok->end < 0) {
		return JSON_READ_INCOMPLETE;
	}
	if (tok->type == JSMN_PRIMITIVE) {
		// the only primitive that starts with n is 'null'
		if (text[tok->start] == 'n') {
			copy(dst, (char*)&params->default_val, sizeof(int));
		}
		else {
			int decimal = decode_decimal(text + tok->start, tok->end - tok->start);
			copy(dst, (char*)&decimal, sizeof(int));
		}
		return JSON_READ_OK;
	}
	if (tok->type != JSMN_STRING) {
		print_dbg("\r\n!! bad token type for enum: ");
		print_dbg_hex(tok->type);
		return JSON_READ_MALFORMED;
	}

	for (int i = 0; i < params->option_ct; i++) {
		if (strncmp(params->options[i], text + tok->start, tok->end - tok->start) == 0) {
			copy(dst, (char*)&i, sizeof(int));
			return JSON_READ_OK;
		}
	}
	copy(dst, (char*)&params->default_val, sizeof(int));
	return JSON_READ_OK;
}

json_write_result_t json_write_enum(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_enum_params_t* params = (json_read_enum_params_t*)docdef->params;

	int src = *(int*)((uint8_t*)ram + src_offset + params->dst_offset);
	if (src < 0 || src >= params->option_ct) {
		src = params->default_val;
	}
	write("\"", 1);
	write(params->options[src], strlen(params->options[src]));
	write("\"", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_read_array(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_array_params_t* params = (json_read_array_params_t*)docdef->params;
	json_read_array_state_t* state = (json_read_array_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->array_state = JSON_ARRAY_MATCH_START;
		state->array_ct = 0;
		state->depth = 0;
	}

	if (tok->type == JSMN_ARRAY
	 && tok->depth < state->depth
	 && text[tok->end - 1] == ']') {
		docdef->fresh = true;
		return JSON_READ_OK;
	}

	switch (state->array_state) {
	case JSON_ARRAY_MATCH_START:
		if (tok->type != JSMN_ARRAY) {
			print_dbg("\r\n!! bad token type for array start: ");
			print_dbg_hex(tok->type);
			return JSON_READ_MALFORMED;
		}
		state->depth = tok->depth + 1;
		state->array_state = JSON_ARRAY_MATCH_ITEMS;
		params->item_docdef->fresh = true;
		return JSON_READ_INCOMPLETE;
	case JSON_ARRAY_MATCH_ITEMS:
		if (state->array_ct > params->array_len) {
			return JSON_READ_INCOMPLETE;
		}

#if JSON_DEBUG
		print_dbg("\r\narray ");
		print_dbg(docdef->name);
		print_dbg(" item #");
		print_dbg_hex(state->array_ct);
		print_dbg(" of size ");
		print_dbg_hex(params->item_size);
		print_dbg(" -> ");
		print_dbg_hex(ram);
		print_dbg(" + ");
		print_dbg_hex(dst_offset);
		print_dbg(" + ");
		print_dbg_hex(state->array_ct * params->item_size);
		print_dbg(" = ");
		print_dbg_hex(
			(char*)ram +
			dst_offset +
			state->array_ct * params->item_size);
#endif
		switch (params->item_docdef->read(
			tok,
			copy, ram, params->item_docdef,
			text, text_len, dst_offset + state->array_ct * params->item_size))
		{
		case JSON_READ_INCOMPLETE:
			return JSON_READ_INCOMPLETE;
		case JSON_READ_KEEP_GOING:
#if JSON_DEBUG
			print_dbg("\r\narray: keep going");
#endif
			return JSON_READ_KEEP_GOING;
		case JSON_READ_OK:
			state->array_ct++;
			return JSON_READ_INCOMPLETE;
		default:
#if JSON_DEBUG
			print_dbg("\r\n!! bad array item");
#endif
			return JSON_READ_MALFORMED;
		}
	default:
		return JSON_READ_MALFORMED;
	}
}

json_write_result_t json_write_array(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_array_params_t* params = (json_read_array_params_t*)docdef->params;

	write("[", 1);
	for (size_t i = 0; i < params->array_len; i++) {
		params->item_docdef->write(write, ram, params->item_docdef, src_offset + params->item_size * i);
		if (i + 1 < params->array_len) {
			write(", ", 2);
		}
	}
	write("]", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_read_buffer(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;
	json_read_buffer_state_t* state = (json_read_buffer_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->buf_pos = 0;
	}
	if (tok->type != JSMN_STRING) {
		print_dbg("\r\n!! bad tok type for buffer: ");
		print_dbg_hex(tok->type);
		return JSON_READ_MALFORMED;
	}
	size_t start = tok->start > 0 ? tok->start : 0;
	size_t input_len = (tok->end >= 0 ? tok->end : text_len) - start;
	if (input_len % 2 != 0) {
		// need to start the token over so that we decode whole bytes
		return JSON_READ_INCOMPLETE;
	}
	if (tok->end > 0) {
		if (state->buf_pos + (input_len / 2) != params->dst_size) {
			print_dbg("\r\n!! bad buffer len: ");
			print_dbg_hex(state->buf_pos + (input_len / 2));
			print_dbg("(expected ");
			print_dbg_hex(params->dst_size);
			print_dbg(")");
			return JSON_READ_MALFORMED;
		}
	}
	char* dst = (char*)ram + params->dst_offset + dst_offset;

#if JSON_DEBUG
	print_dbg("\r\n> read buffer: ");
	print_dbg(docdef->name);
	print_dbg("\r\n     ");
	print_dbg_hex(input_len);
	print_dbg(" -> ");

	print_dbg_hex(ram);
	print_dbg(" + ");
	print_dbg_hex(params->dst_offset);
	print_dbg(" + ");
	print_dbg_hex(dst_offset);
	print_dbg(" + ");
	print_dbg_hex(state->buf_pos);
	print_dbg(" = ");
	print_dbg_hex(dst + state->buf_pos);
	print_dbg(" - ");
	print_dbg_hex(dst + state->buf_pos + (input_len / 2));
#endif

	if (decode_hexbuf(
		copy,
		dst + state->buf_pos,
		text + start, input_len) < 0) {
		print_dbg("\r\n!! decoding hex failed");
		return JSON_READ_MALFORMED;
	}
	state->buf_pos += input_len / 2;
	if (tok->end >= 0) {
		docdef->fresh = true;
#if JSON_DEBUG
		print_dbg("\r\n> buffer ");
		if (docdef->name) {
			for (char* p = docdef->name; *p != 0; p++) {
				print_dbg_char(*p);
			}
		}
		print_dbg(" done");
#endif
		return JSON_READ_OK;
	}
#if JSON_DEBUG
	print_dbg("\r\nbuffer: keep going");
#endif
	return JSON_READ_KEEP_GOING;
}

json_write_result_t json_write_buffer(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;

	write("\"", 1);
	uint8_t* src = (uint8_t*)ram + src_offset + params->dst_offset;
	encode_hexbuf(write, src, params->dst_size);
	write("\"", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_read_string(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, int32_t dst_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;
	json_read_buffer_state_t* state = (json_read_buffer_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->buf_pos = 0;
	}
	if (tok->type != JSMN_STRING) {
		print_dbg("\r\n!! bad token type for string: ");
		print_dbg_hex(tok->type);
		return JSON_READ_MALFORMED;
	}
	size_t start = tok->start > 0 ? tok->start : 0;
	size_t len = (tok->end >= 0 ? tok->end : text_len) - start;
	if (tok->end >= 0) {
		if (state->buf_pos + len != params->dst_size) {
			print_dbg("\r\n!! bad string len: ");
			print_dbg_hex(state->buf_pos + len);
			print_dbg("(expected ");
			print_dbg_hex(params->dst_size);
			print_dbg(")");
			return JSON_READ_MALFORMED;
		}
	}
	char* dst = (char*)ram + params->dst_offset + dst_offset;
	copy(dst + state->buf_pos, text + start, len);
	if (tok->end >= 0) {
		docdef->fresh = true;
		return JSON_READ_OK;
	}
	state->buf_pos += len;
	return JSON_READ_KEEP_GOING;
}

json_read_state_t deserialize_state;

json_read_result_t json_read(
	json_gets_cb read, json_copy_cb copy, void* ram,
	json_docdef_t* docdef,
	char* textbuf, size_t textbuf_len,
	jsmntok_t* tokbuf, size_t tokbuf_len)
{
	enum jsmnerr jsmn_err;
	json_read_result_t result = JSON_READ_KEEP_GOING;
	int keep_ct = 0;
	jsmntok_t* last_tok;
	size_t bytes_read = 0;
	uint32_t total_tokens_read = 0;
	uint32_t total_bytes_read = 0;

	docdef->fresh = true;
	memset(textbuf, 0, textbuf_len);
	deserialize_state.text_ct = textbuf_len;
	deserialize_state.curr_tok = tokbuf_len;
	jsmn_init(&deserialize_state.jsmn);

	for (uint8_t i = 0; i < tokbuf_len; i++) {
		tokbuf[i].start = tokbuf[i].end = 0;
	}

	while (deserialize_state.text_ct >= 0) {
		if (deserialize_state.jsmn.pos > textbuf_len) {
			print_dbg("\r\n!! tokenizer out of bounds");
			return JSON_READ_MALFORMED;
		}

		// when a batch of tokens has been processed
		if (deserialize_state.curr_tok >= deserialize_state.jsmn.toknext) {
			keep_ct = 0;
			if (deserialize_state.jsmn.toknext > 0) {
				last_tok = &tokbuf[deserialize_state.jsmn.toknext - 1];
				if (result == JSON_READ_KEEP_GOING || last_tok->start == 0) {
					// if in the middle of a string or buffer value, don't backtrack
					keep_ct = 0;
				} else if (last_tok->end < 0) {
					// if the last token was incomplete, copy the text already read and
					// try to parse it again
					keep_ct = deserialize_state.text_ct - last_tok->start;
				} else {
					// if the last token was complete, copy everything after it
					keep_ct = deserialize_state.text_ct - last_tok->end;
				}
				if (keep_ct > 0) {
					memmove(textbuf,
						textbuf + deserialize_state.text_ct - keep_ct,
						keep_ct);
				}

				// rewind the parser state so it stays within the fixed buffers.
				// if we've already partially consumed a string or primitive,
				// we need to account for that as well.
				if (deserialize_state.jsmn.string_open) {
					deserialize_state.jsmn.pos -= deserialize_state.text_ct;
				}
				else if (deserialize_state.jsmn.number_open) {
					deserialize_state.jsmn.pos = 0;
				}
				else {
					deserialize_state.jsmn.pos -= deserialize_state.text_ct - keep_ct;
				}
				deserialize_state.jsmn.toknext = 0;
			}

			// read from stream and tokenize
#if JSON_DEBUG
			print_dbg("\r\n> reading, keep ");
			print_dbg_hex(keep_ct);
			print_dbg(", request ");
			print_dbg_hex(textbuf_len - keep_ct);
#endif
			bytes_read = read(textbuf + keep_ct, textbuf_len - keep_ct);
			total_bytes_read += bytes_read;
#if JSON_DEBUG
			print_dbg(", got ");
			print_dbg_hex(bytes_read);
#endif
			if (bytes_read < 0) {
#if JSON_DEBUG
				print_dbg("\r\n!! read error");
#endif
				return JSON_READ_MALFORMED;
			}
			deserialize_state.text_ct = keep_ct + bytes_read;

			jsmn_err = jsmn_parse(&deserialize_state.jsmn,
					      textbuf, deserialize_state.text_ct,
					      tokbuf, tokbuf_len);
			if (bytes_read == 0) {
				if (jsmn_err == JSMN_ERROR_PART || (int)jsmn_err == 0) {
#if JSON_DEBUG
					print_dbg("\r\n!! unexpected EOF");
#endif
					return JSON_READ_MALFORMED;
				}
			}
			if ((int)jsmn_err == 0) {
				deserialize_state.jsmn.pos = 0;
				continue;
			}
			if ((int)jsmn_err < 0
			 && jsmn_err != JSMN_ERROR_NOMEM
			 && jsmn_err != JSMN_ERROR_PART) {
#if JSON_DEBUG
				print_dbg("\r\n!! tokenizer error");
#endif
				return JSON_READ_MALFORMED;
			}
			deserialize_state.curr_tok = 0;
		}

		jsmntok_t* tok = &tokbuf[deserialize_state.curr_tok];
		result = docdef->read(
			tok,
			copy, ram, docdef,
			textbuf, textbuf_len, 0);
		deserialize_state.curr_tok++;
		total_tokens_read++;
		switch (result) {
		case JSON_READ_KEEP_GOING:
			continue;
		case JSON_READ_INCOMPLETE:
			continue;
		case JSON_READ_OK:
			print_dbg("\r\n> OK! total bytes read from disk: ");
			print_dbg_hex(total_bytes_read);
			print_dbg("\r\n> total tokens processed: ");
			print_dbg_hex(total_tokens_read);
			return JSON_READ_OK;
		default:
			print_dbg("\r\n> FAILED! total bytes read from disk: ");
			print_dbg_hex(total_bytes_read);
			print_dbg("\r\n> total tokens processed: ");
			print_dbg_hex(total_tokens_read);


			print_dbg("\r\nfailed on token type ");
			print_dbg_hex(tok->type);
			print_dbg(" that spans ");
			print_dbg_hex(tok->start);
			print_dbg(" - ");
			print_dbg_hex(tok->end);

			print_dbg("\r\n> contents of read buffer:\r\n\r\n");

			size_t i;
			for (i = 0; i < textbuf_len; i++) {
				print_dbg_char(textbuf[i]);
			}
			print_dbg("\r\n");

			i = 0;
			if (tok->start > 0) {
				for (i = 0; i < tok->start; i++) {
					print_dbg_char(' ');
				}
				print_dbg_char('^');
				i++;
			}
			for (; i < (tok->end > 0 ? (tok->end - 1) : textbuf_len); i++) {
				print_dbg_char('-');
			}
			if (tok->end >= 0) {
				print_dbg_char('^');
			}
			print_dbg("\r\n\r\n");
			return JSON_READ_MALFORMED;
		}
	}
	return JSON_READ_MALFORMED;
}

json_write_result_t json_write(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef) {
	json_write_result_t ret = docdef->write(write, ram, docdef, 0);
	return ret;
}

json_docdef_t* json_docdef_find_key(json_docdef_t* object_docdef, const char* name) {
	const json_read_object_params_t* params = (json_read_object_params_t*)object_docdef->params;
	for (int i = 0; i < params->docdef_ct; i++) {
		if (strcmp(params->docdefs[i].name, name) == 0) {
			return &params->docdefs[i];
		}
	}
	return NULL;
}
