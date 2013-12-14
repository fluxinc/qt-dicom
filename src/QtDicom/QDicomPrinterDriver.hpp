/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMPRINTERDRIVER_HPP
#define QTDICOM_QDICOMPRINTERDRIVER_HPP

#include "Globals.hpp"
#include "QDicomPrinter.hpp"

#include <QtCore/QSharedDataPointer>

#include <QtNetwork/QHostAddress>


template < typename T >
class QList;
class QSize;
class QString;
class QXmlStreamReader;

/**
 * The \em QDicomPrinterDriver class allows to create and use drivers for DICOM
 * imagers supported by the \em QtDicom module.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QDicomPrinterDriver {
	public :
		enum Feature {
			FilmSessionLabel = 0x01,
			BorderDensity = 0x02,
			EmptyImageDensity = 0x04,
			Trim = 0x08,

			/**
			 * Indicates whether or not the imager supports \c HIGH value of the
			 * Requested Resolution ID attribute.
			 */
			HighQuality = 0x10,

			RequestedImageSize = 0x20,

			// Note! Update ::Feature[From]String and unit test file after 
			// adding another feature to this list! Also, make sure that onlye
			// one bit is set for each flag.
		};
		Q_DECLARE_FLAGS( Features, Feature );

	public :
		/**
		 * Adds printer \a driver to the list of system-available DICOM
		 * imager drivers. This method is thread-safe.
		 */
		static void addDriver(
			QDicomPrinter::Device, const QDicomPrinterDriver & driver
		);

		static QDicomPrinterDriver fromXml(
			QXmlStreamReader & stream, QString * message = nullptr
		);
		static QDicomPrinterDriver fromXml(
			const QByteArray & data, QString * message = nullptr
		);
		static QDicomPrinterDriver fromXmlFile(
			const QString & Path, QString * message = nullptr
		);

		static QDicomPrinterDriver forDevice(
			const QDicomPrinter::Device & device
		);

	public :
		QDicomPrinterDriver();

		const QList< quint8 > & depths() const;
		const Features & features() const;

		const QList< QDicomPrinter::FilmDestination > & filmDestinations() const;
		const QList< QDicomPrinter::FilmSize > & filmSizes() const;


		bool hasFeature( const Feature & ) const;

		bool isNull() const;
		bool isValid() const;

		const QList< QDicomPrinter::MagnificationType > & magnificationTypes() const;
		const quint16 & maxCopyCount() const;
		const QList< QDicomPrinter::MediumType > & mediumTypes() const;

		
		/**
		 * Returns printer's model name.
		 */
		const QString & modelName() const;

		/**
		 * Returns printer's name. If not set otherwise with the \ref setName(),
		 * the name is generated from printer vendor name, model and IP address.
		 */
		QString name() const;


		const QSize & printableArea(
			const QDicomPrinter::FilmSize &,
			const QDicomPrinter::Quality &
		) const;


		/**
		 * Returns printer's resolution in DPI when in normal mode.
		 */
		const quint16 & resolution(
			const QDicomPrinter::Quality & 
		) const;


		void setDepths( const QList< quint8 > & depths );
		void setFeatures( const Features & );
		void setFilmDestinations( const QList< QDicomPrinter::FilmDestination > & );
		void setFilmSizes( const QList< QDicomPrinter::FilmSize > & );
		void setMagnificationTypes( const QList< QDicomPrinter::MagnificationType > & );
		void setMaxCopyCount( const quint16 & );
		void setMediumTypes( const QList< QDicomPrinter::MediumType > & );
		void setModelName( const QString & );
		void setPrintableAreas(
			const QDicomPrinter::Quality &,
			const QMap< QDicomPrinter::FilmSize, QSize > &
		);
		void setResolutions( const quint16 & Normal, const quint16 & High );
		void setVendorName( const QString & Name );

		/**
		 * Returns printer's vendor name.
		 */
		const QString & vendorName() const;

	private :
		class XmlParser;

		struct Data : public QSharedData {
			static const int MaxAreas = 0x20;
			static const int MaxResolutions = 0x02;

			Data();

			QList< quint8 > Depths;
			Features Features;
			QList< QDicomPrinter::FilmDestination > FilmDestinations;
			QList< QDicomPrinter::FilmSize > FilmSizes;
			quint16 MaxCopyCount;
			QList< QDicomPrinter::MagnificationType > MagnificationTypes;
			QList< QDicomPrinter::MediumType > MediumTypes;
			QString ModelName;
			QSize PrintableAreas[ MaxAreas ];
			quint16 Resolutions[ MaxResolutions ];
			QString VendorName;
		};

	private :
		static QMap< QDicomPrinter::Device, QDicomPrinterDriver > & availableDirvers();

	private :

	private :
		const Data & data() const;
		Data & data();
		QSharedDataPointer< Data > d_;
};

#endif // ! QTDICOM_QDICOMPRINTERDRIVER_HPP

