/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Globals.hpp"

#include <QtCore/QString>


template < size_t N >
inline static QString WArrayString( const wchar_t ( & Array )[ N ] ) {
	Q_ASSERT( Array[ N - 1 ] == '\0' );

#ifdef Q_CC_MSVC

	static_assert( sizeof( wchar_t ) == 2, "wchar_t should store UTF-16" );

	// In Visual Studio C++ compiler wchar_t holds an UTF-16 value. Since
	// the same does QChar, we can omit unecessary memory operations and just
	// produce QString from static wchar_t array directly
	return QString::fromRawData(
		reinterpret_cast< const QChar * >( Array ), N - 1
	);

#else // ! Q_CC_MSVC

	// In other compilers, just use the usual deep-copy.
	return QString::fromWCharArray( Array, N - 1 );

#endif // Q_CC_MSVC
}
