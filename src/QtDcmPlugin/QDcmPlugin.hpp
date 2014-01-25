/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDCMPLUGIN_HPP
#define QDCMPLUGIN_HPP

#include <QtCore/QStringList>

#include <QtGui/QImageIOHandler>


/**
 * DICOM images plug-in. Reimplements all necessary methods to enable Qt
 * applications access DICOM image data.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDcmPlugin : public QImageIOPlugin {
	public:
		/**
		 * Creates a plug-in.
		 */
		QDcmPlugin();

		/**
		 * Destroys the plug-in.
		 */
		~QDcmPlugin();

		/**
		 * Returns a list of strings with only one value: \c "dcm", as this is 
		 * the only recommended file extenstion for DICOM files (according to
		 * RFC 3240).
		 */
		QStringList keys() const;

		/**
		 * Returns plug-ins capabilities when accessing the \a device and using 
		 * provided \a format.
		 *
		 * \note As for the version 0.0.1 of this plug-in only \em CanRead 
		 *       capability is supported.
		 */
		Capabilities capabilities(
			QIODevice * device, const QByteArray & format
		) const;

		/**
		 * Creates the \ref QDcmHandler (subclass of the \en QImageIOHandler)
		 * and sets its \a device and \a format.
		 */
		QImageIOHandler * create(
			QIODevice * device, const QByteArray & format = QByteArray()
		) const;
};

#endif
