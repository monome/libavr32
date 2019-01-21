#include "json/serdes.h"
#include "json/encoding.h"

json_read_result_t json_read_object(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, size_t dst_offset) {
	json_read_object_params_t* params = (json_read_object_params_t*)docdef->params;
	json_read_object_state_t* state = (json_read_object_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->object_state = JSON_OBJECT_MATCH_START;
		state->active_docdef = NULL;
		state->sections_handled = 0;
		state->depth = 0;
	}

	if (tok->type == JSMN_OBJECT && tok->depth < state->depth && text[tok->end - 1] == '}') {
		docdef->fresh = true;
		return JSON_READ_OK;
	}

	switch (state->object_state) {
	case JSON_OBJECT_MATCH_START:
		if (tok->type != JSMN_OBJECT) {
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
			return JSON_READ_MALFORMED;
		}
		if (tok->end > 0) {
			for (uint8_t i = 0; i < params->docdef_ct; i++) {
				if (strncmp(params->docdefs[i].name, text + tok->start, tok->end - tok->start) == 0) {
					state->active_docdef = &params->docdefs[i];
					state->active_docdef->fresh = true;
					state->object_state = JSON_OBJECT_PARSE_PROPERTY;
					return JSON_READ_INCOMPLETE;
				}
			}
			state->object_state = JSON_OBJECT_SKIP_SECTION;
		}
		return JSON_READ_INCOMPLETE;
	case JSON_OBJECT_PARSE_PROPERTY:
		switch(state->active_docdef->read(
			tok,
			copy, ram, state->active_docdef,
			text, text_len, dst_offset)) {
		case JSON_READ_INCOMPLETE:
			return JSON_READ_INCOMPLETE;
		case JSON_READ_OK:
			state->object_state = JSON_OBJECT_MATCH_NAME;
			return JSON_READ_INCOMPLETE;
		default:
			return JSON_READ_MALFORMED;
		}
	default:
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
	const char* text, size_t text_len, size_t dst_offset) {
	json_read_scalar_params_t* params = (json_read_scalar_params_t*)docdef->params;
	if (tok->type != JSMN_PRIMITIVE) {
		return JSON_READ_MALFORMED;
	}
	if (tok->end < 0) {
		return JSON_READ_INCOMPLETE;
	}
	void* dst = (char*)ram + params->dst_offset + dst_offset;
	int val = decode_decimal(text + tok->start, tok->end - tok->start);
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

	if (params->signed_val) {
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
	const char* text, size_t text_len, size_t dst_offset) {
	json_match_string_params_t* params = (json_match_string_params_t*)docdef->params;
	if (tok->type != JSMN_STRING) {
		return JSON_READ_MALFORMED;
	}
	if (tok->end < 0) {
		return JSON_READ_INCOMPLETE;
	}
	if (strncmp(params->to_match, text + tok->start, tok->end - tok->start) != 0) {
		return JSON_READ_MALFORMED;
	}
	return JSON_READ_OK;
}
json_read_result_t json_read_enum(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, size_t dst_offset) {
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
	const char* text, size_t text_len, size_t dst_offset) {
	json_read_array_params_t* params = (json_read_array_params_t*)docdef->params;
	json_read_array_state_t* state = (json_read_array_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->array_state = JSON_ARRAY_MATCH_START;
		state->array_ct = 0;
		state->depth = 0;
	}

	if (tok->type == JSMN_ARRAY && tok->depth < state->depth && text[tok->end - 1] == ']') {
		docdef->fresh = true;
		return JSON_READ_OK;
	}

	switch (state->array_state) {
	case JSON_ARRAY_MATCH_START:
		if (tok->type != JSMN_ARRAY) {
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

		switch (params->item_docdef->read(
			tok,
			copy, ram, params->item_docdef,
			text, text_len, dst_offset + state->array_ct * params->item_size))
		{
		case JSON_READ_MALFORMED:
			return JSON_READ_MALFORMED;
		case JSON_READ_INCOMPLETE:
			return JSON_READ_INCOMPLETE;
		case JSON_READ_OK:
			state->array_ct++;
			return JSON_READ_INCOMPLETE;
		default:
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
	const char* text, size_t text_len, size_t dst_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;
	json_read_buffer_state_t* state = (json_read_buffer_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->buf_pos = 0;
	}
	if (tok->type != JSMN_STRING) {
		return JSON_READ_MALFORMED;
	}
	size_t start = tok->start > 0 ? tok->start : 0;
	size_t len = (tok->end > 0 ? tok->end : text_len) - start;
	if (tok->end > 0) {
		if (state->buf_pos + len != params->dst_size * 2) {
			return JSON_READ_MALFORMED;
		}
	} else {
		if (len % 2 != 0) {
			// length needs to be even so we always decode full bytes
			return JSON_READ_INCOMPLETE;
		}
	}
	char* dst = (char*)ram + params->dst_offset + dst_offset;
	if (decode_hexbuf(
		copy,
		dst + state->buf_pos,
		text + start,
		len) < 0) {
		return JSON_READ_MALFORMED;
	}
	if (tok->end > 0) {
		docdef->fresh = true;
		return JSON_READ_OK;
	}
	state->buf_pos += len;
	return JSON_READ_INCOMPLETE;
}

json_write_result_t json_write_buffer(
	json_puts_cb write,
	void* ram, json_docdef_t* docdef,
	size_t src_offset) {
	char nybble;
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;

	write("\"", 1);
	uint8_t* src = (uint8_t*)ram + src_offset + params->dst_offset;
	for (size_t i = 0; i < params->dst_size; i++) {
		nybble = encode_nybble((src[i] & 0xF0) >> 4);
		write(&nybble, 1);
		nybble = encode_nybble(src[i] & 0x0F);
		write(&nybble, 1);
	}
	write("\"", 1);
	return JSON_WRITE_OK;
}

json_read_result_t json_read_string(
	jsmntok_t* tok,
	json_copy_cb copy, void* ram, json_docdef_t* docdef,
	const char* text, size_t text_len, size_t dst_offset) {
	json_read_buffer_params_t* params = (json_read_buffer_params_t*)docdef->params;
	json_read_buffer_state_t* state = (json_read_buffer_state_t*)docdef->state;
	if (docdef->fresh) {
		docdef->fresh = false;
		state->buf_pos = 0;
	}
	if (tok->type != JSMN_STRING) {
		return JSON_READ_MALFORMED;
	}
	size_t start = tok->start > 0 ? tok->start : 0;
	size_t len = (tok->end > 0 ? tok->end : text_len) - start;
	if (tok->end > 0) {
		if (state->buf_pos + len != params->dst_size) {
			return JSON_READ_MALFORMED;
		}
	}
	char* dst = (char*)ram + params->dst_offset + dst_offset;
	copy(dst + state->buf_pos, text + start, len);
	if (tok->end > 0) {
		docdef->fresh = true;
		return JSON_READ_OK;
	}
	state->buf_pos += len;
	return JSON_READ_INCOMPLETE;
}

json_read_state_t deserialize_state;

json_read_result_t json_read(
	json_gets_cb read, json_copy_cb copy, void* ram,
	json_docdef_t* docdef,
	char* textbuf, size_t textbuf_len,
	jsmntok_t* tokbuf, size_t tokbuf_len)
{
	json_read_result_t result;
	int keep_ct = 0;
	jsmntok_t* last_tok;
	size_t bytes_read = 0;

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
			return JSON_READ_MALFORMED;
		}
		if (deserialize_state.curr_tok >= deserialize_state.jsmn.toknext) {
			keep_ct = 0;
			if (deserialize_state.jsmn.toknext > 0) {
				// if the last token was incomplete, copy the text already read and
				// try to parse it again. otherwise copy the text that hasn't been parsed yet.
				last_tok = &tokbuf[deserialize_state.jsmn.toknext - 1];
				if (last_tok->end < 0) {
					keep_ct = deserialize_state.text_ct - last_tok->start;
				} else {
					keep_ct = deserialize_state.text_ct - last_tok->end;
				}
				strncpy(textbuf,
					textbuf + deserialize_state.text_ct - keep_ct,
					keep_ct);

				// rewind the parser state so it stays within the fixed buffers
				deserialize_state.jsmn.pos -= deserialize_state.text_ct - keep_ct;
				deserialize_state.jsmn.toknext = 0;
				if (deserialize_state.jsmn.toksuper >= 0) {
					deserialize_state.jsmn.toksuper -= deserialize_state.jsmn.toknext;
				}
			}
			bytes_read = read(textbuf + keep_ct, textbuf_len - keep_ct);
			if (bytes_read < 0) {
				return JSON_READ_MALFORMED;
			}
			deserialize_state.text_ct = keep_ct + bytes_read;
			int err = jsmn_parse(&deserialize_state.jsmn,
									  textbuf, deserialize_state.text_ct,
									  tokbuf, tokbuf_len);
			if (err == 0) {
				return JSON_READ_MALFORMED;
			}
			if (err < 0 && err != JSMN_ERROR_NOMEM && err != JSMN_ERROR_PART) {
				return JSON_READ_MALFORMED;
			}
			deserialize_state.curr_tok = 0;
		}

		result = docdef->read(
			&tokbuf[deserialize_state.curr_tok],
			copy, ram, docdef,
			textbuf, textbuf_len, 0);
		deserialize_state.curr_tok++;
		switch (result) {
		case JSON_READ_INCOMPLETE:
			continue;
		case JSON_READ_OK:
			return JSON_READ_OK;
		default:
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
