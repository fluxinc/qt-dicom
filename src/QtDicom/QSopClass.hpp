/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QSOPCLASS_HPP
#define QTDICOM_QSOPCLASS_HPP

#include "Globals.hpp"


class QDICOM_DLLSPEC QSopClass {
	public :
		enum Id {
			Unknown = 0,

			// First come SOP Classes that do not belong to a meta-group. They
			// get IDs as they go but there cannot be more SOP Classes than
			// 126.

			/**
			 * The Basic Annotation Box Class represents an annotation (e.g. 
			 * text string) on a film. It is an optional SOP Class used during
			 * print operations.
			 */
			BasicAnnotationBox,


			// Then come the meta SOP Classes, they also receive IDs as they go
			// but starting from 0x100. Their value cannot exceed 0x107

			/**
			 * Basic Grayscale Print Management is a meta SOP class that groups
			 * the following SOP Classes:
			 *
			 *   * Basic Film Session,
			 *   * Basic Film Box,
			 *   * Basic Grayscale Image Box,
			 *   * Printer.
			 */
			BasicGrayscalePrintManagement = 0x100,


			// Finally come SOP classes that do belong to a meta-group. They
			// are assigned IDs manually, using their meta-group class ID number
			// as a prefix. Their ID value belongs to the [0x80,0xFF] set, where
			// the higher nibble points to the meta group ID.

			/**
			 * The Basic Grayscale Image Box Class represents an image and image
			 * related data in the image area of a film. The Class is used
			 * during print operation and belongs to the Basic Grayscale Print
			 * Management meta SOP Class.
			 */
			BasicGrayscaleImageBox =
				( ( BasicGrayscalePrintManagement & 0x07 ) << 4 ) | 0x80 
			,

			/**
			 * The Basic Film Box SOP Class represents a single film during a 
			 * print session. It describes the presentation parameters which
			 * are common for all images on a given sheet of film. The Class
			 * belongs to the Basic Grayscale Print Management meta SOP Class.
			 */
			BasicFilmBox =
				( ( BasicGrayscalePrintManagement & 0x07 ) << 4 ) | 0x81
			,

			/**
			 * The Basic Film Session SOP Class represents parameters which are
			 * common for all the film of a film session (e.g. number of copies,
			 * film destination). The Film Session instance is used as a reference
			 * by the Film Box SOP Classes. It belongs to the Basic Grayscale
			 * Print Management meta SOP Class.
			 */
			BasicFilmSession =
				( ( BasicGrayscalePrintManagement & 0x07 ) << 4 ) | 0x82
			,
			
			/**
			 * The Print SOP Class is an abstraction of hard copy printer and 
			 * can be used to monitor the status of a printer. The class belongs to
			 * the Basic Grayscale Print Management meta SOP Class.
			 */
			Printer = 
				( ( BasicGrayscalePrintManagement & 0x07 ) << 4 ) | 0x83
			,
		};

	public :
		static QSopClass fromUid( const QByteArray & Uid );
		static const QByteArray & uid( const Id & );

	public :
		QSopClass();
		QSopClass( const Id & );
		~QSopClass();

		operator const Id &() const;

		bool hasMeta() const;
		bool isMeta() const;
		bool isNull() const;
		bool isValid() const;
		QSopClass meta() const;
		const QString & name() const;
		const QByteArray & uid() const;

	private :
		static const int MaxId = 0x107;

	private :
		static const QString & name( const Id & );

	private :
		Id id_;
		
};

#endif // ! QTDICOM_QSOPCLASS_HPP
