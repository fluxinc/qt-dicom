/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_SERVERASSOCIATION_HPP
#define DICOM_SERVERASSOCIATION_HPP

#include "QtDicom/AcceptorAssociation.hpp"
#include "QtDicom/Globals.hpp"

namespace Dicom {

/**
 * The \em ServerAssociation class represents a special \ref AcceptorAssociation
 * designed explicitely to be used by the \ref AssociationServer object.
 *
 * The class provides a special constructor allowing and enforcing to set an
 * initialized already DCMTK's network structure.
 *
 * A normal association object manages its network structure internally and,
 * crucially, it destroys it in a destructor. The \em ServerAssociation is 
 * different in this regard because it uses network structure provided 
 * externally and during a life time of an object it remains intact. That
 * gives to a parimary user of this class -- the \ref AssociationServer --
 * a huge perfomance boost as it doesn't have to acquire and release a
 * bound socket each time a new connection is made.
 *
 * Apart from that, the \em ServerAssociation provides the additional
 * \ref isAssociationPending() method for checking incoming connections queue.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC ServerAssociation : public AcceptorAssociation {
	Q_OBJECT;

	public :
		/**
		 * Creates server association and sets its \a external network and 
		 * \a parameters.
		 *
		 * Allows for creating associations using externally-allocated
		 * networks, which aren't freed by \em accotiations's destructor.
		 *
		 * \note Be sure to use the same settings when creating an \a external
		 *       network as those passed through the \a parameters argument.
		 */
		ServerAssociation(
			T_ASC_Network * external,
			const ConnectionParameters & parameters,
			QObject * parent = 0
		);

		/**
		 * Destroys the association.
		 */
		~ServerAssociation();

		/**
		 * Returns \c true if there's an association waiting to be established
		 * hanging on a \ref port().
		 */
		bool isPending() const;

		/**
		 * Overriden method. Uses external provided network instead of the
		 * internal one.
		 */
		bool receive( bool * timedOut = 0 );

	private :
		/**
		 * Returns external network address.
		 */
		T_ASC_Network *& externalTAscNetwork() const;

	private :
		/**
		 * The external network structure. It is not deallocated when 
		 * association is destroyed.
		 */
		mutable T_ASC_Network * externalTAscNetwork_;
};

}; // Namespace DICOM ends here.

#endif
