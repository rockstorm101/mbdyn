/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2000
 *
 * Pierangelo Masarati	<masarati@aero.polimi.it>
 * Paolo Mantegazza	<mantegazza@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 * 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ELECNODE_H
#define ELECNODE_H

#include "node.h"

/* AbstractNode - begin */

/* Nodo astratto, consente l'accesso ad un grado di liberta' messo in comune
 * e privo di significato fisico proprio. Gli elementi di tipo ElectricBulk
 * usano i nodi elettrici. Numerosi elementi elettrici elementari sono 
 * associati a nodi elettrici (impedenze, amplificatori, generatori di 
 * tensione e di corrente, ecc.). Altri elementi, di tipo Electric, possono 
 * essere associati sia a nodi elettrici (tipicamente a coppie di nodi, in 
 * quanto usano differenze di tensione), oppure a nodi astratti,
 * ovvero oggetti che consentono un riferimento esplicito a Dof messi in
 * comune che non hanno un significato fisico preciso */

/* Numero di dof del tipo di nodo - usato anche dal DofManager (?) */
const int iAbstractNodeDofNumber = 1;

class AbstractNode : public ScalarDifferentialNode {
 public:   
   /* Costruttore definitivo (da mettere a punto) */
   AbstractNode(unsigned int uL, const DofOwner* pDO,
		doublereal dx, doublereal dxp, flag fOut)
     : ScalarDifferentialNode(uL, pDO, dx, dxp, fOut) { 
	NO_OP; 
     };
   
   /* Distruttore (per ora e' banale) */
   virtual ~AbstractNode(void) {
      NO_OP;
   };
   
   /* Tipo di nodo */
   virtual Node::Type GetNodeType(void) const {
      return Node::ABSTRACT;
   };
   
   /* Contributo del nodo astratto al file di restart */
   virtual std::ostream& Restart(std::ostream& out) const { 
      return out << "  abstract: " << GetLabel() << ", " 
	<< dX << ", " << dXP << ';' << std::endl;
   };
   
   /* Output del nodo */
   virtual void Output(OutputHandler& OH) const;
   
   /* Aggiorna dati in base alla soluzione */
   virtual void Update(const VectorHandler& XCurr, 
		       const VectorHandler& XPrimeCurr);
   
   /* Funzioni di inizializzazione, ereditate da DofOwnerOwner */
   virtual void SetInitialValue(VectorHandler& /* X */ ) const { 
      NO_OP; 
   };
   
   virtual void SetValue(VectorHandler& X, VectorHandler& XP) const;
   
   virtual void AfterPredict(VectorHandler& X, VectorHandler& XP) {
      this->Update(X, XP);
   };
   
#ifdef DEBUG
   virtual const char* sClassName(void) const { 
      return "AbstractNode";
   };
#endif   
};

/* AbstractNode - end */


/* ElectricNode - begin */

/* Nodo elettrico, descrive fisicamente un nodo di una rete elettrica.
 * Viene usato con gli elementi ElectricBulk. Numerosi elementi elettrici 
 * elementari sono associati a questi nodi (impedenze, amplificatori, 
 * generatori di tensione e di corrente, ecc.). Altri elementi, di tipo
 * Electric, possono essere associati sia a nodi elettrici (tipicamente a
 * coppie, in quanto usano differenze di tensione), oppure a nodi astratti,
 * ovvero oggetti che consentono un riferimento esplicito a Dof messi in
 * comune che non hanno un significato fisico preciso */

/* Numero di dof del tipo di nodo - usato anche dal DofManager (?) */
const int iElectricNodeDofNumber = iAbstractNodeDofNumber;

class ElectricNode : public AbstractNode {
 public:
   /* Costruttore */
   ElectricNode(unsigned int uL, const DofOwner* pDO, 
		doublereal dx, doublereal dxp, flag fOut)
     : AbstractNode(uL, pDO, dx, dxp, fOut) { 
	NO_OP; 
     };
   
   /* Distruttore (per ora e' banale) */
   virtual ~ElectricNode(void) { 
      NO_OP; 
   };

   /* Tipo di nodo */
   virtual Node::Type GetNodeType(void) const { 
      return Node::ELECTRIC;
   };
   
   /* Contributo del nodo elettrico al file di restart */
   virtual std::ostream& Restart(std::ostream& out) const { 
      return out << "  electric: " << GetLabel() << ", "
	<< dX << ", " << dXP << ';' << std::endl;
   };
   
   /* Output del nodo */
   virtual void Output(OutputHandler& OH) const;
      
#ifdef DEBUG
   virtual const char* sClassName(void) const { 
      return "ElectricNode";
   };
#endif   
};

/* ElectricNode - end */

#endif /* #define ELECNODE_H */
