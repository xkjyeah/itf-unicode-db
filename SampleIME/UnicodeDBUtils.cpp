#include "UnicodeDB.h"



UNICODE_T::UNICODE_T(const char *l) {
	strncpy(data, l, CODE_LENGTH_MAX);
}
UNICODE_T::UNICODE_T() {
	data[0] = 0;
}
UNICODE_T &UNICODE_T::operator=(const UNICODE_T &l) {
	if (this == &l) {
		return *this;
	}
	strncpy(this->data, l.data, CODE_LENGTH_MAX);
	return *this;
}
