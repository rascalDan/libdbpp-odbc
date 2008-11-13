#ifndef ODBC_STRING_H
#define ODBC_STRING_H

#include <string>

namespace ODBC {
	class String : public std::basic_string<unsigned char> {
		public:
			String(const unsigned char *);
			String(const char *);
			String(std::basic_string<unsigned char>);
			String(std::basic_string<char>);
			String();
			operator unsigned char *() const;

			static String		Format(const char *, ...);
	};
}

#endif

