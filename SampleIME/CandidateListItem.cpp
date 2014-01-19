#include "SampleIMEBaseStructure.h"

using namespace std;

CCandidateListItem::CCandidateListItem()  {
	_Sequence[1] = _Sequence[2] = 0;
}

CCandidateListItem& CCandidateListItem::operator =( const CCandidateListItem& rhs)
{
	_CharDescription = rhs._CharDescription;
	_CharUnicodeHex = rhs._CharUnicodeHex;
		
	return *this;
}

const wchar_t *CCandidateListItem::GetChar() {
	// Convert hex string to charcode
	uint32_t _CharCode = 0;
		
	for (wstring::size_type i=0; i < this->_CharUnicodeHex.length(); i++) {
		int digitValue = -1;

		if ( this->_CharUnicodeHex[i] >= 'A' && this->_CharUnicodeHex[i] <= 'F') {
			digitValue = this->_CharUnicodeHex[i] - 'A' + 10;
		}
		else if ( this->_CharUnicodeHex[i] >= 'a' && this->_CharUnicodeHex[i] <= 'f') {
			digitValue = this->_CharUnicodeHex[i] - 'a' + 10;
		}
		else if ( this->_CharUnicodeHex[i] >= '0' && this->_CharUnicodeHex[i] <= '9') {
			digitValue = this->_CharUnicodeHex[i] - '0';
		}
		else {
			assert(0);
		}

		if (digitValue == -1)
			continue;

		assert(digitValue >= 0 && digitValue <= 15);
		
		_CharCode = (_CharCode<< 4) + digitValue;
	}

	// Convert charcode to UTF-16
	CCandidateListItem::CharToString(_CharCode, _Sequence, 3);
	return _Sequence;
}

void CCandidateListItem::SetCodePoint(uint32_t cp) {
	WCHAR out[12];

	wsprintf(out, L"%04X", cp);
	this->_CharUnicodeHex = out;
}

HRESULT CCandidateListItem::CharToString( uint32_t unicode_char, wchar_t *str, int length ) {
	if (length <= 0) return false;
		
	if (length >= 2 && (unicode_char <= 0xD7FF || (unicode_char >= 0xE000 && unicode_char <= 0xFFFF)) ) {
		// No surrogate pair needed
		str[0] = (wchar_t) unicode_char;
		str[1] = L'\0';
		return S_OK;
	}
	else if (length >= 3 && unicode_char >= 0x010000 && unicode_char <= 0x10FFFF) {
		// Surrogate pair needed
		// High surrogates: D800 - DBFF (3FF chars = 10bits)
		// Low surrogates:  DC00 - DFFF (3FF chars = 10bits)
		// Highest UTF-16:  10FFFF = 21bits
		// calculations from http://www.unicode.org/faq/utf_bom.html

		const uint16_t LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
		const uint16_t TRAIL_OFFSET = 0xDC00;

		// computations
		uint16_t lead = (uint16_t) (LEAD_OFFSET + (unicode_char >> 10));
		uint16_t trail = (uint16_t) (TRAIL_OFFSET + (unicode_char & 0x3FF));

		str[0] = (wchar_t) lead;
		str[1] = (wchar_t) trail;
		str[2] = 0;
		return S_OK;
	}
	else {
		str[0] = 0;
		return S_FALSE;
	}
}