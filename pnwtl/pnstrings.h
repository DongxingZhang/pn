/**
 * @file pnstrings.h
 * @brief Utility classes and functions for strings.
 * @author Simon Steele
 * @note Copyright (c) 2002 Simon Steele <s.steele@pnotepad.org>
 *
 * Programmers Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 *
 * Classes in this file:
 *   CustomFormatStringBuilder	- Build up a string based on %x and $(x) format specs.
 */

#ifndef pnstrings_h__included
#define pnstrings_h__included

#if defined(UNICODE)
	typedef std::wostream tstream;
#else
	typedef std::ostream tstream;
#endif

static TCHAR* tcsnewdup(LPCTSTR strin)
{
	TCHAR* ret = new TCHAR[_tcslen(strin)+1];
	_tcscpy(ret, strin);
	return ret;
}

static tstring IntToTString(int x)
{
	TCHAR _buffer[32];
	_sntprintf(_buffer, 32, _T("%0d"), x);
	
	return tstring(_buffer);
}

static int strFirstNonWS(const char* lineBuf)
{
	PNASSERT(AtlIsValidString(lineBuf));

	const char* p = lineBuf;
	int i = 0;
	while(*p != 0 && (*p == _T(' ') || *p == 0x9))
	{
		i++;
		p++;
	}

	return i;
}

static int strLastNonWSChar(const char* lineBuf, int lineLength)
{
	PNASSERT(AtlIsValidString(lineBuf));

	const char* p = &lineBuf[lineLength-1];
	int i = lineLength-1;
	while(p > lineBuf && (*p == _T(' ') || *p == 0x9))
	{
		p--;
		i--;
	}

	return i;
}

#include <vector>
using std::vector;

template <typename TStringType>
static void StringTokenise(const TStringType& str,
                      vector<TStringType>& tokens,
                      const TStringType& delimiters/* = _T(" ")*/)
{
    // Skip delimiters at beginning.
    TStringType::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    TStringType::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (TStringType::npos != pos || TStringType::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

/**
 * This class builds strings using custom format specifiers. It
 * supports both %x style format strings and also $(var) style
 * strings. The user must implement at least one of OnFormatChar
 * or OnFormatKey and add text to m_string.
 */
template <class T>
class CustomFormatStringBuilder
{
	public:
		const tstring& Build(LPCTSTR str)
		{
			TCHAR next;
			T* pT = static_cast<T*>(this);
			int len = _tcslen(str);

			m_string = _T("");

			for(int i = 0; i < len; i++)
			{
				if(str[i] == _T('%'))
				{
					next = SafeGetNextChar(str, i, len);
					
					if(next == NULL)
					{
						m_string += str[i];
					}
					else if(next == _T('%'))
					{
						m_string += next;
						// Push past the next %
						i++;
					}
					else
					{
						pT->OnFormatChar(next);
						i++;
					}
				}
				else if(str[i] == _T('$') && (i != (len-1)))
				{
					if( str[i+1] == _T('$') )
					{
						// If we are seeing a $$( then it means we want the $ sign.
						if(SafeGetNextChar(str, i+1, len) == _T('('))
						{
							m_string += str[i];
							// Skip the next dollar sign as well.
							i += 1;
							continue;
						}
					}
					else if( str[i+1] != _T('(') )
					{
						m_string += str[i];
						continue;
					}

					// we matched a $(x) property...
					LPCTSTR pProp = &str[i+2];
					LPCTSTR endProp = _tcschr(pProp, _T(')'));
					if(endProp != NULL)
					{
						int keylen = (endProp - pProp) / sizeof(TCHAR);
						TCHAR* buf = new TCHAR[keylen+1];

						try
						{
							_tcsncpy(buf, pProp, keylen);
							buf[keylen] = _T('\0');
							pT->OnFormatKey(buf);
						}
						catch(...)
						{
						}

						i += (2 + keylen); // skip ( + len + )

						delete [] buf;
					}
				}
				else
				{
					m_string += str[i];
				}				
			}

			return m_string;
		}

		void OnFormatChar(TCHAR thechar){}
		void OnFormatKey(LPCTSTR key){}

	protected:
		TCHAR SafeGetNextChar(LPCTSTR str, int i, int len)
		{
			PNASSERT(i < len);
			PNASSERT(i >= 0);

			if(i == (len-1))
				return NULL;
            
			return str[i+1];
		}

		tstring	m_string;
};

///@todo could this be faster at all?
void XMLSafeString(LPCTSTR from, tstring& to);
void XMLSafeString(tstring& str);

struct FormatXML {
   tstring str_;
   explicit FormatXML(const tstring& str) : str_(str) { XMLSafeString(str_); }
   friend tstream& operator<<(tstream& s, const FormatXML& x)
   {
		s << x.str_;
		return s;
   }
};

#endif // #ifndef pnstrings_h__included