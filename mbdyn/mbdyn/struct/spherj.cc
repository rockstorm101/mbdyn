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

/* Giunti sferici */

#ifdef HAVE_CONFIG_H
#include <mbconfig.h>           /* This goes first in every *.c,*.cc file */
#endif /* HAVE_CONFIG_H */

#include <spherj.h>

/* SphericalHingeJoint - begin */

/* Costruttore non banale */
SphericalHingeJoint::SphericalHingeJoint(unsigned int uL, const DofOwner* pDO,
					 const StructNode* pN1, 
					 const StructNode* pN2,
					 const Vec3& dTmp1, const Mat3x3& RTmp1h,
					 const Vec3& dTmp2, const Mat3x3& RTmp2h,
					 flag fOut)
: Elem(uL, Elem::JOINT, fOut), 
Joint(uL, Joint::SPHERICALHINGE, pDO, fOut),
pNode1(pN1), pNode2(pN2), 
d1(dTmp1), R1h(RTmp1h),
d2(dTmp2), R2h(RTmp2h), 
F(0.)
{
   NO_OP;
}


/* Distruttore banale */
SphericalHingeJoint::~SphericalHingeJoint(void)
{
   NO_OP;
};


/* Contributo al file di restart */
std::ostream& SphericalHingeJoint::Restart(std::ostream& out) const
{
   Joint::Restart(out) << ", spherical hinge, "
     << pNode1->GetLabel() << ", reference, node, ",
     d1.Write(out, ", ")  << ", hinge, reference, node, 1, ", (R1h.GetVec(1)).Write(out, ", ")
     << ", 2, ", (R1h.GetVec(2)).Write(out, ", ") << ", "       
     << pNode2->GetLabel() << ", reference, node, ",
     d2.Write(out, ", ") << ", hinge, reference, node, 1, ", (R2h.GetVec(1)).Write(out, ", ")
     << ", 2, ", (R2h.GetVec(2)).Write(out, ", ") << ';' << std::endl;
   
   return out;
}


/* Assemblaggio jacobiano */
VariableSubMatrixHandler& 
SphericalHingeJoint::AssJac(VariableSubMatrixHandler& WorkMat,
			    doublereal dCoef,
			    const VectorHandler& /* XCurr */ ,
			    const VectorHandler& /* XPrimeCurr */ )
{
   DEBUGCOUT("Entering SphericalHingeJoint::AssJac()" << std::endl);
      
   SparseSubMatrixHandler& WM = WorkMat.SetSparse();
   WM.ResizeInit(54, 1, 0.);
   
   integer iNode1FirstPosIndex = pNode1->iGetFirstPositionIndex();
   integer iNode1FirstMomIndex = pNode1->iGetFirstMomentumIndex();
   integer iNode2FirstPosIndex = pNode2->iGetFirstPositionIndex();
   integer iNode2FirstMomIndex = pNode2->iGetFirstMomentumIndex();
   integer iFirstReactionIndex = iGetFirstIndex();

   Mat3x3 R1(pNode1->GetRRef());
   Mat3x3 R2(pNode2->GetRRef());
   Vec3 dTmp1(R1*d1);
   Vec3 dTmp2(R2*d2);
   
   
   /* 
    * L'equazione di vincolo afferma che il punto in cui si trova la
    * cerniera deve essere consistente con la posizione dei due nodi:
    *      x2 + d2 = x1 + d1
    * 
    * con: d2 = R2 * d2_0
    *      d1 = R1 * d1_0
    * 
    * La forza e' data dalla reazione vincolare F, nel sistema globale
    * La coppia dovuta all'eccentricita' e' data rispettivamente da:
    *     -d1 /\ F    per il nodo 1,
    *      d2 /\ F    per il nodo 2
    *
    * 
    *         x1   g1        x2     g2        F
    * Q1 |  0      0         0      0         I    | | x1 |   | -F           |
    * G1 |  0      cF/\d1/\  0      0         d1/\ | | g1 |   | -d1/\F       |
    * Q2 |  0      0         0     -cF/\d2/\ -I    | | x2 | = |  F           |
    * G2 |  0      0         0      0        -d2/\ | | g2 |   |  d2/\F       |
    * F  | -c*I    c*d1/\    c*I   -c*d2/\    0    | | F  |   |  x1+d1-x2-d2 |
    * 
    * con d1 = R1*d01, d2 = R2*d02, c = dCoef
    */

   /* termini di reazione sul nodo 1 */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(iCnt, iNode1FirstMomIndex+iCnt, 
		  iFirstReactionIndex+iCnt, 1.);
   }
   WM.fPutCross(4, iNode1FirstMomIndex+3,
		iFirstReactionIndex, dTmp1);
   
   /* termini di reazione sul nodo 2 */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(9+iCnt, iNode2FirstMomIndex+iCnt, 
		  iFirstReactionIndex+iCnt, -1.);
   }
   WM.fPutCross(13, iNode2FirstMomIndex+3,
		iFirstReactionIndex, -dTmp2);

   /* Moltiplico la forza per il coefficiente del metodo.
    * Nota: F, la reazione vincolare, e' stata aggiornata da AssRes */
   Vec3 FTmp = F*dCoef;
   
   /* Termini diagonali del tipo: c*F/\d/\Delta_g 
    * nota: la forza e' gia' moltiplicata per dCoef */      
   WM.fPutMat3x3(19, iNode1FirstMomIndex+3,
		 iNode1FirstPosIndex+3, Mat3x3(FTmp, dTmp1));

   WM.fPutMat3x3(28, iNode2FirstMomIndex+3,
		 iNode2FirstPosIndex+3, Mat3x3(FTmp, -dTmp2));
   
   /* Modifica: divido le equazioni di vincolo per dCoef */
   
   /* termini di vincolo dovuti al nodo 1 */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(36+iCnt, iFirstReactionIndex+iCnt, 
		  iNode1FirstPosIndex+iCnt, -1.);
   }   
   WM.fPutCross(40, iFirstReactionIndex,
		iNode1FirstPosIndex+3, dTmp1);
      
   /* termini di vincolo dovuti al nodo 1 */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(45+iCnt, iFirstReactionIndex+iCnt, 
		  iNode2FirstPosIndex+iCnt, 1.);
   }
   WM.fPutCross(49, iFirstReactionIndex,
		iNode2FirstPosIndex+3, -dTmp2);
   
   return WorkMat;
}


/* Assemblaggio residuo */
SubVectorHandler& SphericalHingeJoint::AssRes(SubVectorHandler& WorkVec,
					      doublereal dCoef,
					      const VectorHandler& XCurr, 
					      const VectorHandler& /* XPrimeCurr */ )
{
   DEBUGCOUT("Entering SphericalHingeJoint::AssRes()" << std::endl);
      
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->WorkSpaceDim(&iNumRows, &iNumCols);
   WorkVec.Resize(iNumRows);
   WorkVec.Reset(0.);
      
   integer iNode1FirstMomIndex = pNode1->iGetFirstMomentumIndex();
   integer iNode2FirstMomIndex = pNode2->iGetFirstMomentumIndex();
   integer iFirstReactionIndex = iGetFirstIndex();
   
   /* Indici dei nodi */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {	
      WorkVec.fPutRowIndex(iCnt, iNode1FirstMomIndex+iCnt);
      WorkVec.fPutRowIndex(6+iCnt, iNode2FirstMomIndex+iCnt);
   }
   
   /* Indici del vincolo */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WorkVec.fPutRowIndex(12+iCnt, iFirstReactionIndex+iCnt);   
   }
   
   F = Vec3(XCurr, iFirstReactionIndex+1);
   
   Vec3 x1(pNode1->GetXCurr());
   Vec3 x2(pNode2->GetXCurr());
   Mat3x3 R1(pNode1->GetRCurr());
   Mat3x3 R2(pNode2->GetRCurr());
   
   Vec3 dTmp1(R1*d1);
   Vec3 dTmp2(R2*d2);
   
   WorkVec.Sub(1, F);
   WorkVec.Sub(4, dTmp1.Cross(F));
   WorkVec.Add(7, F);
   WorkVec.Add(10, dTmp2.Cross(F));
   
   /* Modifica: divido le equazioni di vincolo per dCoef */
   if (dCoef != 0.) {
      WorkVec.Add(13, (x1+dTmp1-x2-dTmp2)/dCoef);
   }
   
   return WorkVec;
}

/* Output (da mettere a punto) */
void SphericalHingeJoint::Output(OutputHandler& OH) const
{
   if (fToBeOutput()) {
#ifdef DEBUG   
      OH.Output() << "Joint " << uLabel << ", type \""
	<< psJointNames[Joint::SPHERICALHINGE] 
	<< "\", linked to nodes " << pNode1->GetLabel() 
	<< " and " << pNode2->GetLabel() << ':' << std::endl 
	<< "Distance from node 1 (node reference frame): " << std::endl << d1 << std::endl 
	<< "Distance from node 2 (node reference frame): " << std::endl << d2 << std::endl 
	<< "Current reaction: " << std::endl << F << std::endl;
#endif   
         
      Mat3x3 RTmp((pNode2->GetRCurr()*R2h).Transpose()*(pNode1->GetRCurr()*R1h));
      
      Joint::Output(OH.Joints(), "SphericalHinge", GetLabel(),
		    F, Zero3, F, Zero3) 
	<< " " << EulerAngles(RTmp) << std::endl;
   }   
}


/* Contributo allo jacobiano durante l'assemblaggio iniziale */
VariableSubMatrixHandler& 
SphericalHingeJoint::InitialAssJac(VariableSubMatrixHandler& WorkMat,
				   const VectorHandler& XCurr)
{
   DEBUGCOUT("Entering SphericalHingeJoint::InitialAssJac()" << std::endl);

   FullSubMatrixHandler& WM = WorkMat.SetFull();
   
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->InitialWorkSpaceDim(&iNumRows, &iNumCols);
   WM.ResizeInit(iNumRows, iNumCols, 0.);

   /* Equazioni: vedi joints.dvi */
    
   /* Indici */
   integer iNode1FirstPosIndex = pNode1->iGetFirstPositionIndex();
   integer iNode1FirstVelIndex = iNode1FirstPosIndex+6;
   integer iNode2FirstPosIndex = pNode2->iGetFirstPositionIndex();
   integer iNode2FirstVelIndex = iNode2FirstPosIndex+6;
   integer iFirstReactionIndex = iGetFirstIndex();
   integer iReactionPrimeIndex = iFirstReactionIndex+3;

   /* Setto gli indici */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {
      WM.fPutRowIndex(iCnt, iNode1FirstPosIndex+iCnt);
      WM.fPutColIndex(iCnt, iNode1FirstPosIndex+iCnt);
      WM.fPutRowIndex(6+iCnt, iNode1FirstVelIndex+iCnt);
      WM.fPutColIndex(6+iCnt, iNode1FirstVelIndex+iCnt);
      WM.fPutRowIndex(12+iCnt, iNode2FirstPosIndex+iCnt);
      WM.fPutColIndex(12+iCnt, iNode2FirstPosIndex+iCnt);
      WM.fPutRowIndex(18+iCnt, iNode2FirstVelIndex+iCnt);
      WM.fPutColIndex(18+iCnt, iNode2FirstVelIndex+iCnt);
      WM.fPutRowIndex(24+iCnt, iFirstReactionIndex+iCnt);
      WM.fPutColIndex(24+iCnt, iFirstReactionIndex+iCnt);
   }   
   
   /* Matrici identita' */
   
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      /* Contributo di forza all'equazione della forza, nodo 1 */
      WM.fPutCoef(iCnt, 24+iCnt, 1.);
      
      /* Contrib. di der. di forza all'eq. della der. della forza, nodo 1 */
      WM.fPutCoef(6+iCnt, 27+iCnt, 1.);
      
      /* Contributo di forza all'equazione della forza, nodo 2 */
      WM.fPutCoef(12+iCnt, 24+iCnt, -1.);
      
      /* Contrib. di der. di forza all'eq. della der. della forza, nodo 2 */
      WM.fPutCoef(18+iCnt, 27+iCnt, -1.);
      
      /* Equazione di vincolo, nodo 1 */
      WM.fPutCoef(24+iCnt, iCnt, -1.);
      
      /* Derivata dell'equazione di vincolo, nodo 1 */
      WM.fPutCoef(27+iCnt, 6+iCnt, -1.);
      
      /* Equazione di vincolo, nodo 2 */
      WM.fPutCoef(24+iCnt, 12+iCnt, 1.);
      
      /* Derivata dell'equazione di vincolo, nodo 2 */
      WM.fPutCoef(27+iCnt, 18+iCnt, 1.);
   }
   
   /* Recupera i dati */
   Mat3x3 R1(pNode1->GetRRef());
   Mat3x3 R2(pNode2->GetRRef());
   Vec3 Omega1(pNode1->GetWRef());
   Vec3 Omega2(pNode2->GetWRef());
   /* F e' stata aggiornata da InitialAssRes */
   Vec3 FPrime(XCurr, iReactionPrimeIndex+1);
   
   /* Distanza nel sistema globale */
   Vec3 d1Tmp(R1*d1);
   Vec3 d2Tmp(R2*d2);

   /* Matrici F/\d1/\, -F/\d2/\ */
   Mat3x3 FWedged1Wedge(F, d1Tmp);
   Mat3x3 FWedged2Wedge(F, -d2Tmp);
   
   /* Matrici (omega1/\d1)/\, -(omega2/\d2)/\ */
   Mat3x3 O1Wedged1Wedge(Omega1.Cross(d1Tmp));
   Mat3x3 O2Wedged2Wedge(d2Tmp.Cross(Omega2));
   
   /* Equazione di momento, nodo 1 */
   WM.Add(4, 4, FWedged1Wedge);
   WM.Add(4, 25, Mat3x3(d1Tmp));
   
   /* Equazione di momento, nodo 2 */
   WM.Add(16, 16, FWedged2Wedge);
   WM.Add(16, 25, Mat3x3(-d2Tmp));
   
   /* Derivata dell'equazione di momento, nodo 1 */
   WM.Add(10, 4, (Mat3x3(FPrime)+Mat3x3(F, Omega1))*Mat3x3(d1Tmp));
   WM.Add(10, 10, FWedged1Wedge);
   WM.Add(10, 25, O1Wedged1Wedge);
   WM.Add(10, 28, Mat3x3(d1Tmp));
   
   /* Derivata dell'equazione di momento, nodo 2 */
   WM.Add(22, 16, (Mat3x3(FPrime)+Mat3x3(F, Omega2))*Mat3x3(-d2Tmp));
   WM.Add(22, 22, FWedged2Wedge);
   WM.Add(22, 25, O2Wedged2Wedge);
   WM.Add(22, 28, Mat3x3(-d2Tmp));
      
   /* Equazione di vincolo */
   WM.Add(25, 4, Mat3x3(d1Tmp));
   WM.Add(25, 16, Mat3x3(-d2Tmp));
   
   /* Derivata dell'equazione di vincolo */
   WM.Add(28, 4, O1Wedged1Wedge);
   WM.Add(28, 10, Mat3x3(d1Tmp));
   WM.Add(28, 16, O2Wedged2Wedge);
   WM.Add(28, 22, Mat3x3(-d2Tmp));
   
   return WorkMat;
}


/* Contributo al residuo durante l'assemblaggio iniziale */   
SubVectorHandler& 
SphericalHingeJoint::InitialAssRes(SubVectorHandler& WorkVec,
				   const VectorHandler& XCurr)
{   
   DEBUGCOUT("Entering SphericalHingeJoint::InitialAssRes()" << std::endl);
   
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->InitialWorkSpaceDim(&iNumRows, &iNumCols);
   WorkVec.Resize(iNumRows);
   WorkVec.Reset(0.);
   
   /* Indici */
   integer iNode1FirstPosIndex = pNode1->iGetFirstPositionIndex();
   integer iNode1FirstVelIndex = iNode1FirstPosIndex+6;
   integer iNode2FirstPosIndex = pNode2->iGetFirstPositionIndex();
   integer iNode2FirstVelIndex = iNode2FirstPosIndex+6;
   integer iFirstReactionIndex = iGetFirstIndex();
   integer iReactionPrimeIndex = iFirstReactionIndex+3;
   
   /* Setta gli indici */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {	
      WorkVec.fPutRowIndex(iCnt, iNode1FirstPosIndex+iCnt);
      WorkVec.fPutRowIndex(6+iCnt, iNode1FirstVelIndex+iCnt);
      WorkVec.fPutRowIndex(12+iCnt, iNode2FirstPosIndex+iCnt);
      WorkVec.fPutRowIndex(18+iCnt, iNode2FirstVelIndex+iCnt);
      WorkVec.fPutRowIndex(24+iCnt, iFirstReactionIndex+iCnt);
   }
   
   /* Recupera i dati */
   Vec3 x1(pNode1->GetXCurr());
   Vec3 x2(pNode2->GetXCurr());
   Vec3 v1(pNode1->GetVCurr());
   Vec3 v2(pNode2->GetVCurr());
   Mat3x3 R1(pNode1->GetRCurr());
   Mat3x3 R2(pNode2->GetRCurr());
   Vec3 Omega1(pNode1->GetWCurr());
   Vec3 Omega2(pNode2->GetWCurr());
   F = Vec3(XCurr, iFirstReactionIndex+1);
   Vec3 FPrime(XCurr, iReactionPrimeIndex+1);
   
   /* Distanza nel sistema globale */
   Vec3 d1Tmp(R1*d1);
   Vec3 d2Tmp(R2*d2);

   /* Vettori omega1/\d1, -omega2/\d2 */
   Vec3 O1Wedged1(Omega1.Cross(d1Tmp));
   Vec3 O2Wedged2(Omega2.Cross(d2Tmp));
   
   /* Equazioni di equilibrio, nodo 1 */
   WorkVec.Add(1, -F);
   WorkVec.Add(4, F.Cross(d1Tmp)); /* Sfrutto il fatto che F/\d = -d/\F */
   
   /* Derivate delle equazioni di equilibrio, nodo 1 */
   WorkVec.Add(7, -FPrime);
   WorkVec.Add(10, FPrime.Cross(d1Tmp)-O1Wedged1.Cross(F));
   
   /* Equazioni di equilibrio, nodo 2 */
   WorkVec.Add(13, F);
   WorkVec.Add(16, d2Tmp.Cross(F)); 
   
   /* Derivate delle equazioni di equilibrio, nodo 2 */
   WorkVec.Add(19, FPrime);
   WorkVec.Add(22, d2Tmp.Cross(FPrime)+O2Wedged2.Cross(F));
   
   /* Equazione di vincolo */
   WorkVec.Add(25, x1+d1Tmp-x2-d2Tmp);
   
   /* Deivata dell'equazione di vincolo */
   WorkVec.Add(28, v1+O1Wedged1-v2-O2Wedged2);
      
   return WorkVec;
}

/* SphericalHingeJoint - end */


/* PinJoint - begin */
/* Costruttore non banale */
PinJoint::PinJoint(unsigned int uL, const DofOwner* pDO,	       
		   const StructNode* pN,
		   const Vec3& X0Tmp, const Vec3& dTmp, flag fOut)
: Elem(uL, Elem::JOINT, fOut), 
Joint(uL, Joint::PIN, pDO, fOut), pNode(pN), X0(X0Tmp), d(dTmp), F(0.)
{
   NO_OP;
}


/* Distruttore banale */
PinJoint::~PinJoint(void)
{
   NO_OP;
};


/* Contributo al file di restart */
std::ostream& PinJoint::Restart(std::ostream& out) const
{
   Joint::Restart(out) << ", pin, "
     << pNode->GetLabel() << ", reference, node, ",
     d.Write(out, ", ") << ", reference, global, ",
     X0.Write(out, ", ") << ';' << std::endl;
   
   return out;
}


/* Assemblaggio jacobiano */
VariableSubMatrixHandler& 
PinJoint::AssJac(VariableSubMatrixHandler& WorkMat,
		 doublereal dCoef,
		 const VectorHandler& /* XCurr */ ,
		 const VectorHandler& /* XPrimeCurr */ )
{
   DEBUGCOUT("Entering PinJoint::AssJac()" << std::endl);
      
   SparseSubMatrixHandler& WM = WorkMat.SetSparse();
   WM.ResizeInit(27, 0, 0.);
   
   integer iFirstPositionIndex = pNode->iGetFirstPositionIndex();
   integer iFirstMomentumIndex = pNode->iGetFirstMomentumIndex();
   integer iFirstReactionIndex = iGetFirstIndex();

   Mat3x3 R(pNode->GetRRef());
   Vec3 dTmp(R*d);
      
   /* 
    * L'equazione di vincolo afferma che il punto in cui si trova la
    * cerniera deve essere fissato:
    *      x + d = x0
    * 
    * con: d = R * d_0
    * 
    * La forza e' data dalla reazione vincolare F, nel sistema globale
    * La coppia dovuta all'eccentricita' e' data rispettivamente da:
    *     d /\ F
    *
    * 
    *       x      g         F
    * Q1 |  0      0         I   | | x |   | -F          |
    * G1 |  0      cF/\d1/\  d/\ | | g |   | -d/\F       |
    * F  |  I      d/\       0   | | F |   |  (x+d-x0)/c |
    * 
    * con d = R*d_0, c = dCoef
    */

   /* termini di reazione sul nodo */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(iCnt, iFirstMomentumIndex+iCnt, 
		  iFirstReactionIndex+iCnt, 1.);
   }   
   WM.fPutCross(4, iFirstMomentumIndex+3,
		iFirstReactionIndex, dTmp);
      
   /* Nota: F, la reazione vincolare, e' stata aggiornata da AssRes */
   
   /* Termini diagonali del tipo: c*F/\d/\Delta_g 
    * nota: la forza e' gia' moltiplicata per dCoef */      
   WM.fPutMat3x3(10, iFirstMomentumIndex+3,
		 iFirstPositionIndex+3, Mat3x3(F*dCoef, dTmp));

   /* Modifica: divido le equazioni di vincolo per dCoef */
   
   /* termini di vincolo dovuti al nodo 1 */
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      WM.fPutItem(18+iCnt, iFirstReactionIndex+iCnt, 
		  iFirstPositionIndex+iCnt, -1.);
   }
   WM.fPutCross(22, iFirstReactionIndex,
		iFirstPositionIndex+3, dTmp);
         
   return WorkMat;
}


/* Assemblaggio residuo */
SubVectorHandler& PinJoint::AssRes(SubVectorHandler& WorkVec,
					      doublereal dCoef,
					      const VectorHandler& XCurr,
					      const VectorHandler& /* XPrimeCurr */ )
{
   DEBUGCOUT("Entering PinJoint::AssRes()" << std::endl);
      
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->WorkSpaceDim(&iNumRows, &iNumCols);
   WorkVec.Resize(iNumRows);
   WorkVec.Reset(0.);
      
   integer iFirstMomentumIndex = pNode->iGetFirstMomentumIndex();
   integer iFirstReactionIndex = iGetFirstIndex();
   
   /* Indici dei nodi */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {
      WorkVec.fPutRowIndex(iCnt, iFirstMomentumIndex+iCnt);
   }
     
   
   /* Indici del vincolo */
   for(int iCnt = 1; iCnt <= 3; iCnt++) {
      WorkVec.fPutRowIndex(6+iCnt, iFirstReactionIndex+iCnt);
   }

   F = Vec3(XCurr, iFirstReactionIndex+1);
   
   Vec3 x(pNode->GetXCurr());
   Mat3x3 R(pNode->GetRCurr());
   
   Vec3 dTmp(R*d);
   
   WorkVec.Add(1, -F);
   WorkVec.Add(4, F.Cross(dTmp)); /* Sfrutto il fatto che F/\d = -d/\F */
   
   /* Modifica: divido le equazioni di vincolo per dCoef */
   if (dCoef != 0.) {
      WorkVec.Add(7, (x+dTmp-X0)/dCoef);
   }
   
   return WorkVec;
}

/* Output (da mettere a punto) */
void PinJoint::Output(OutputHandler& OH) const
{
   if (fToBeOutput()) {
#ifdef DEBUG   
      OH.Output() << "Joint " << uLabel << ", type \""
	<< psJointNames[Joint::PIN] 
	<< "\", linked to node " << pNode->GetLabel() << ':' << std::endl
	<< "Distance from node (node reference frame): " << std::endl << d << std::endl 
	<< "Current reaction: " << std::endl << F << std::endl;   
#endif   
   
      /*
      OH.Joints().write("Pin             ", 16)
	<< std::setw(8) << GetLabel() << " " << F << " " << Vec3(0.) << std::endl;
       */
      
      Mat3x3 RTmp(pNode->GetRCurr());
      
      Joint::Output(OH.Joints(), "Pin", GetLabel(),
		    F, Zero3, F, Zero3) 
	<< " " << EulerAngles(RTmp) << std::endl;      
   }   
}


/* Contributo allo jacobiano durante l'assemblaggio iniziale */
VariableSubMatrixHandler& 
PinJoint::InitialAssJac(VariableSubMatrixHandler& WorkMat,
				   const VectorHandler& XCurr)
{
   DEBUGCOUT("Entering PinJoint::InitialAssJac()" << std::endl);

   FullSubMatrixHandler& WM = WorkMat.SetFull();
   
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->InitialWorkSpaceDim(&iNumRows, &iNumCols);
   WM.ResizeInit(iNumRows, iNumCols, 0.);

   /* Equazioni: vedi joints.dvi */
    
   /* Indici */
   integer iFirstPositionIndex = pNode->iGetFirstPositionIndex();
   integer iFirstVelocityIndex = iFirstPositionIndex+6;
   integer iFirstReactionIndex = iGetFirstIndex();
   integer iReactionPrimeIndex = iFirstReactionIndex+3;

   /* Setto gli indici */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {
      WM.fPutRowIndex(iCnt, iFirstPositionIndex+iCnt);
      WM.fPutColIndex(iCnt, iFirstPositionIndex+iCnt);
      WM.fPutRowIndex(6+iCnt, iFirstVelocityIndex+iCnt);
      WM.fPutColIndex(6+iCnt, iFirstVelocityIndex+iCnt);
      WM.fPutRowIndex(12+iCnt, iFirstReactionIndex+iCnt);
      WM.fPutColIndex(12+iCnt, iFirstReactionIndex+iCnt);
   }   
   
   /* Matrici identita' */
   
   for (int iCnt = 1; iCnt <= 3; iCnt++) {
      /* Contributo di forza all'equazione della forza */
      WM.fPutCoef(iCnt, 12+iCnt, 1.);
      
      /* Contrib. di der. di forza all'eq. della der. della forza */
      WM.fPutCoef(6+iCnt, 15+iCnt, 1.);
      
      /* Equazione di vincolo */
      WM.fPutCoef(12+iCnt, iCnt, -1.);
      
      /* Derivata dell'equazione di vincolo */
      WM.fPutCoef(15+iCnt, 6+iCnt, -1.);
   }
   
   /* Recupera i dati */
   Mat3x3 R(pNode->GetRRef());
   Vec3 Omega(pNode->GetWRef());
   /* F e' stata aggiornata da InitialAssRes */
   Vec3 FPrime(XCurr, iReactionPrimeIndex+1);
   
   /* Distanza nel sistema globale */
   Vec3 dTmp(R*d);

   /* Matrici F/\d/\ */
   Mat3x3 FWedgedWedge(F, dTmp);
   
   /* Matrici (omega/\d)/\ */
   Mat3x3 OWedgedWedge(Omega.Cross(dTmp));
   
   /* Equazione di momento */
   WM.Add(4, 4, FWedgedWedge);
   WM.Add(4, 13, Mat3x3(dTmp));
   
   /* Derivata dell'equazione di momento */
   WM.Add(10, 4, (Mat3x3(FPrime)+Mat3x3(F, Omega))*Mat3x3(dTmp));
   WM.Add(10, 10, FWedgedWedge);
   WM.Add(10, 13, OWedgedWedge);
   WM.Add(10, 16, Mat3x3(dTmp));
   
   /* Equazione di vincolo */
   WM.Add(13, 4, Mat3x3(dTmp));
   
   /* Derivata dell'equazione di vincolo */
   WM.Add(16, 4, OWedgedWedge);
   WM.Add(16, 10, Mat3x3(dTmp));
   
   return WorkMat;
}


/* Contributo al residuo durante l'assemblaggio iniziale */   
SubVectorHandler& 
PinJoint::InitialAssRes(SubVectorHandler& WorkVec,
			const VectorHandler& XCurr)
{   
   DEBUGCOUT("Entering PinJoint::InitialAssRes()" << std::endl);
   
   /* Dimensiona e resetta la matrice di lavoro */
   integer iNumRows = 0;
   integer iNumCols = 0;
   this->InitialWorkSpaceDim(&iNumRows, &iNumCols);
   WorkVec.Resize(iNumRows);
   WorkVec.Reset(0.);
   
   /* Indici */
   integer iFirstPositionIndex = pNode->iGetFirstPositionIndex();
   integer iFirstVelocityIndex = iFirstPositionIndex+6;
   integer iFirstReactionIndex = iGetFirstIndex();
   integer iReactionPrimeIndex = iFirstReactionIndex+3;
   
   /* Setta gli indici */
   for (int iCnt = 1; iCnt <= 6; iCnt++) {	
      WorkVec.fPutRowIndex(iCnt, iFirstPositionIndex+iCnt);
      WorkVec.fPutRowIndex(6+iCnt, iFirstVelocityIndex+iCnt);
      WorkVec.fPutRowIndex(12+iCnt, iFirstReactionIndex+iCnt);
   }
   
   /* Recupera i dati */
   Vec3 x(pNode->GetXCurr());
   Vec3 v(pNode->GetVCurr());
   Mat3x3 R(pNode->GetRCurr());
   Vec3 Omega(pNode->GetWCurr());
   F = Vec3(XCurr, iFirstReactionIndex+1);
   Vec3 FPrime(XCurr, iReactionPrimeIndex+1);
   
   /* Distanza nel sistema globale */
   Vec3 dTmp(R*d);

   /* Vettori omega/\d */
   Vec3 OWedged(Omega.Cross(dTmp));
   
   /* Equazioni di equilibrio */
   WorkVec.Sub(1, F);
   WorkVec.Add(4, F.Cross(dTmp)); /* Sfrutto il fatto che F/\d = -d/\F */
   
   /* Derivate delle equazioni di equilibrio */
   WorkVec.Sub(7, FPrime);
   WorkVec.Add(10, FPrime.Cross(dTmp)-OWedged.Cross(F));
   
   /* Equazione di vincolo */
   WorkVec.Add(13, x+dTmp-X0);
   
   /* Derivata dell'equazione di vincolo */
   WorkVec.Add(16, v+OWedged);
      
   return WorkVec;
}

/* PinJoint - end */
