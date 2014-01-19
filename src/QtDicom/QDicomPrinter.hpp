/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMPRINTER_HPP
#define QTDICOM_QDICOMPRINTER_HPP

#include "Globals.hpp"
#include "QDicomPrintEngine.hpp"

#include <QtGui/QPaintDevice>

#include <QtNetwork/QHostAddress>


class QDicomPrinterDriver;


class QDICOM_DLLSPEC QDicomPrinter : public QPaintDevice {
	friend class QDicomPrintEngine;

	public :
		enum Device {
			Generic_IHE_254 = 1,
			Generic_IHE_300,
			Generic_IHE_320,

#ifdef _DEBUG
			Debug,
#endif
			
			Agfa_DRYSTAR_5300,
			Agfa_DRYSTAR_5302,
			Agfa_DRYSTAR_5500,
			Agfa_DRYSTAR_5503,
			Agfa_DRYSTAR_AXYS,

			Carestream_DryView_5700,
			Carestream_DryView_5800,
			Carestream_DryView_5850,
			Carestream_DryView_5950,
			Carestream_DryView_6800,
			Carestream_DryView_6850,
			Carestream_DryView_8150,
			Carestream_DryView_8900,
			Carestream_DryView_Chroma,

			Codonics_Horizon_v2,

			Fuji_DRYPIX_1000,
			Fuji_DRYPIX_2000,
			Fuji_DRYPIX_3000,
			Fuji_DRYPIX_4000,
			Fuji_DRYPIX_5000,
			Fuji_DRYPIX_7000,
			Fuji_DRYPIX_FMDPL,

			Konica_Minolta_Drypro_722,
			Konica_Minolta_Drypro_751,
			Konica_Minolta_Drypro_752,
			Konica_Minolta_Drypro_771,
			Konica_Minolta_Drypro_793,
			Konica_Minolta_Drypro_832,
			Konica_Minolta_Drypro_873,

			Sony_UP_970AD,
			Sony_UP_990AD,
			Sony_UP_D72XR,
			Sony_UP_D74XRD,
			Sony_UP_D77MD,
			Sony_UP_D897,
			Sony_UP_DF500,
			Sony_UP_DF550,
			Sony_UP_DF750,

			User = 0x1000
		};

		enum FilmDestination {
			Magazine = 1,
			Processor,
			Bin_1,
			Bin_2,
			Bin_3,
			Bin_4,
			Bin_5,
			Bin_6,
			Bin_7,
			Bin_8,
			Bin_9,
			Bin_10,
		};

		enum FilmSize {
			A3 = 1,
			A4,
			CM_24x24,
			CM_24x30,
			CM_25_7x36_4,
			IN_8x10,
			IN_8_5x11,
			IN_10x12,
			IN_10x14,
			IN_11x14,
			IN_11x17,
			IN_14x14,
			IN_14x17
		};

		enum MagnificationType {
			None = 1,
			Replicate,
			Bilinear,
			Cubic
		};

		enum MediumType {
			ClearFilm = 1,
			BlueFilm,
			MammoClearFilm,
			MammoBlueFilm,
			Paper
		};

		enum Orientation {
			VerticalOrientation = 1,
			HorizontalOrientation,
		};

		enum Quality {
			NormalQuality = 1,
			HighQuality
		};

	public :
		/**
		 * Creates a null DICOM printer. Use the \ref setPrinterDriver() and
		 * set other properties (like connection or output settings) in order
		 * to make the printer valid before printing on it.
		 */
		QDicomPrinter();

		/**
		 * Creates a DICOM printer, setting its connection parameters like 
		 * \a address, \a port, \a remoteAe and \a localAe; and the \a driver
		 * to use. Film size, orientation, quality and depth settings are set
		 * to their default values: \ref IN_8x10, \ref Horizontal and \c 8;
		 * respectively.
		 *
		 * Make sure the printer is valid before printing on it by calling 
		 * \ref isValid().
		 */
		QDicomPrinter(
			const QDicomPrinterDriver & driver, 
			const QHostAddress & address, const quint16 & port,
			const QString & remoteAe = "",
			const QString & localAe = "FLUX"
		);

		/**
		 * Creates a DICOM printer, settings its connection parameters like
		 * \a address, \a port, \a remoteAe and \a localAe. The driver is loaded
		 * from the internal \em QtDicom library (see \ref QDicomPrinterDriver).
		 */
		QDicomPrinter(
			const Device & device, 
			const QHostAddress & address, const quint16 & port,
			const QString & remoteAe = "",
			const QString & localAe = "FLUX"
		);
		~QDicomPrinter();


		const quint16 & copyCount() const;


		/**
		 * Returns the characteristics of this printer.
		 */
		const QDicomPrinterDriver & driver() const;


		const qreal & emptyAreaDensity() const;

		const QString & errorString() const;

		const FilmDestination & filmDestination() const;

		const FilmSize & filmSize() const;

		bool hasError() const;

		const QHostAddress & hostAddress() const;


		bool isNull() const;

		/**
		 * Returns \c true when a printer was successfully selected (either
		 * using \ref setPrinterInfo() or \ref setPrinterName()) and if no error
		 * was encoutered so far.
		 */
		bool isValid() const;

		const QString & localAeTitle() const;

		const MagnificationType & magnificationType() const;

		const MediumType & mediumType() const;

		QString name() const;

		bool newPage();

		const Orientation & orientation() const;

		QPaintEngine * paintEngine() const;

		const quint16 & portNumber() const;

		const Quality & quality() const;

		const QString & remoteAeTitle() const;


		void setCopyCount( const quint16 & );
		void setDepth( const quint8 & );


		/**
		 * Sets the printer driver to \a driver. Please call this method before
		 * using \em QPainter::begin().
		 */
		void setDriver( const QDicomPrinterDriver & driver );
		void setEmptyAreaDensity( const qreal & density );
		void setHostAddres( const QHostAddress & );
		void setFilmDestination( const FilmDestination & );
		void setFilmSize( const FilmSize & );
		void setLocalAeTitle( const QString & );
		void setMagnificationType( const MagnificationType & );
		void setMediumType( const MediumType & );
		void setOrientation( const Orientation & );
		void setPortNumber( const quint16 & );
		void setQuality( const Quality & );
		void setRemoteAeTitle( const QString & );
		void setTrim( const bool & enable );
		const bool & trim() const;

	protected :
		virtual int metric( PaintDeviceMetric metric ) const;

	private :
		void raiseError( const QString & message );

	private :
		quint16 copyCount_;
		quint8 depth_;

		QDicomPrinterDriver & driver();
		// We have to make driver a pointer because otherwise the Driver class
		// and this one reference each other.
		QDicomPrinterDriver * driver_;

		qreal emptyAreaDensity_;
		mutable QDicomPrintEngine engine_;

		QString errorString_;

		FilmDestination filmDestination_;
		FilmSize filmSize_;

		QHostAddress hostAddress_;

		MagnificationType magnificationType_;

		Orientation orientation_;
		QString localAeTitle_;
		MediumType mediumType_;
		quint16 portNumber_;
		Quality quality_;
		QString remoteAeTitle_;


};

#endif // ! QTDICOM_QDICOMPRINTER_HPP
