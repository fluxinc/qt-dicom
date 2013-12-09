/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMPRINTERDRIVER_XMLPARSER_HPP
#define QTDICOM_QDICOMPRINTERDRIVER_XMLPARSER_HPP

#include "QDicomPrinterDriver.hpp"


class QStringRef;
class QXmlStreamReader;


class QDicomPrinterDriver::XmlParser {
	public :
		enum DocumentVersion {
			Version_0_1 = 1
		};

	public :
		static QDicomPrinterDriver process(
			QXmlStreamReader & xml, QString * message = nullptr
		);

	private :
		static int documentVersionFromString( const QStringRef & );

	private :
		XmlParser( QXmlStreamReader &, QDicomPrinterDriver & );
		
		void readDepths();
		void readFeatures();
		void readFilmDestinations();
		void readFilmSizes();
		void readMagnificationTypes();
		void readMediumTypes();
		void readPrintableAreas();
		bool readPrintableAreas_Items(
			const QStringList & Items,
			const QDicomPrinter::Quality &,
			QString & message
		);
		QPair< QDicomPrinter::FilmSize, QSize > readPrintableAreas_Item(
			const QString & Item,
			QString & message
		);
		void readQDicomPrinterDriver_1();
		void readResolutions();

	private :
		QDicomPrinterDriver & driver();
		QDicomPrinterDriver & driver_;

		QXmlStreamReader & stream();
		QXmlStreamReader & stream_;
};

#endif // QTDICOM_QDICOMPRINTERDRIVER_XMLPARSER_HPP
