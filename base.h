struct String {
	char *cstr;
	size_t count;
	size_t size;
};

struct StringArray {
	struct String *strs;
	size_t count;
	size_t size;
};

struct String make_string(void);
struct StringArray make_string_array(void);
