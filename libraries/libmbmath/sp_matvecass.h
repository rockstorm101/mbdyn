/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2020
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

/*
  AUTHOR: Reinhard Resch <mbdyn-user@a1.net>
  Copyright (C) 2020(-2020) all rights reserved.

  The copyright of this code is transferred
  to Pierangelo Masarati and Paolo Mantegazza
  for use in the software MBDyn as described
  in the GNU Public License version 2.1
*/

#ifndef ___SP_MATVECASS_H__INCLUDED___
#define ___SP_MATVECASS_H__INCLUDED___

#if defined(HAVE_FINITE)
#include <cmath>
#endif

#include "sp_gradient.h"
#include "sp_matrix_base.h"
#include "submat.h"

namespace sp_grad {
     template <typename T>
     class SpGradientVectorHandler;

     template <>
     class SpGradientVectorHandler<doublereal> {
     public:
	  SpGradientVectorHandler(const VectorHandler& vh)
	       :vh(vh) {

	  }

	  void dGetCoef(integer iRow, doublereal& dVal, doublereal dCoef) const {
	       dVal = vh.dGetCoef(iRow);
	  }

	  template <index_type N_rows>
	  void GetVec(integer iRow, SpColVector<double, N_rows>& v, doublereal dCoef) const {
	       for (integer i = 0; i < v.iGetNumRows(); ++i) {
		    v(i + 1) = vh.dGetCoef(iRow + i);
	       }
	  }

     private:
	  const VectorHandler& vh;
     };

     template <>
     class SpGradientVectorHandler<SpGradient> {
     public:
	  SpGradientVectorHandler(const VectorHandler& vh)
	       :vh(vh) {

	  }

	  void dGetCoef(integer iRow, SpGradient& gVal, doublereal dCoef) const {
	       gVal.Reset(vh.dGetCoef(iRow), iRow, -dCoef);
	  }

	  template <index_type N_rows>
	  void GetVec(integer iRow, SpColVector<SpGradient, N_rows>& v, doublereal dCoef) const {
	       for (integer i = 0; i < v.iGetNumRows(); ++i) {
		    v(i + 1).Reset(vh.dGetCoef(iRow + i), iRow + i, -dCoef);
	       }
	  }
     private:
	  const VectorHandler& vh;
     };

     class SpGradientAssVecBase {
     public:
	  enum SpAssMode { RESET, APPEND };
     };

     template <typename T>
     class SpGradientAssVec;

     template <>
     class SpGradientAssVec<doublereal>: SpGradientAssVecBase {
     public:
	  SpGradientAssVec(SubVectorHandler& vh, SpAssMode mode = RESET)
	       :WorkVec(vh) {

	       switch (mode) {
	       case APPEND:
		    iSubRow = WorkVec.iGetSize();
		    break;

	       case RESET:
		    iSubRow = 0;
		    WorkVec.Resize(iSubRow);
		    break;

	       default:
		    SP_GRAD_ASSERT(0);
		    iSubRow = -1;
	       }
	  }

	  template <typename T>
	  static void
	  AssRes(T* pElem,
		 SubVectorHandler& WorkVec,
		 doublereal dCoef,
		 const VectorHandler& XCurr,
		 const VectorHandler& XPrimeCurr,
		 SpFunctionCall func,
		 SpAssMode mode = RESET) {
	       const SpGradientVectorHandler<doublereal> XCurr_grad(XCurr);
	       const SpGradientVectorHandler<doublereal> XPrimeCurr_grad(XPrimeCurr);
	       SpGradientAssVec WorkVec_grad(WorkVec, mode);

	       pElem->AssRes(WorkVec_grad, dCoef, XCurr_grad, XPrimeCurr_grad, func);
	  }

	  template <typename T>
	  static void
	  InitialAssRes(T* pElem,
			SubVectorHandler& WorkVec,
			const VectorHandler& XCurr,
			SpFunctionCall func,
			SpAssMode mode = RESET) {
	       const SpGradientVectorHandler<doublereal> XCurr_grad(XCurr);
	       SpGradientAssVec WorkVec_grad(WorkVec, mode);

	       pElem->InitialAssRes(WorkVec_grad, XCurr_grad, func);
	  }

	  void AddItem(const integer iRow, const double dCoef) {
	       WorkVec.Resize(++iSubRow);

	       SP_GRAD_ASSERT(iSubRow <= WorkVec.iGetSize());
	       SP_GRAD_ASSERT(std::isfinite(dCoef));
	       
	       WorkVec.PutItem(iSubRow, iRow, dCoef);
	  }

	  template <index_type N_rows>
	  void AddItem(const integer iFirstRow, const SpColVector<doublereal, N_rows>& v) {
	       // zero based index according to VectorHandler::Put(integer iRow, const Vec3& v)
	       WorkVec.Resize(iSubRow + v.iGetNumRows());

	       for (integer i = 0; i < v.iGetNumRows(); ++i) {
		    SP_GRAD_ASSERT(iSubRow + 1 <= WorkVec.iGetSize());
		    SP_GRAD_ASSERT(std::isfinite(v(i + 1)));
		    
		    WorkVec.PutItem(++iSubRow, iFirstRow + i, v(i + 1));
	       }
	  }

     private:
	  SubVectorHandler& WorkVec;
	  integer iSubRow;
     };

     template <>
     class SpGradientAssVec<SpGradient>: public SpGradientAssVecBase {
     public:
	  SpGradientAssVec(SparseSubMatrixHandler& mh, SpAssMode mode = RESET)
	       :WorkMat(mh) {

	       switch (mode) {
	       case RESET:
		    iNextItem = 1;
		    WorkMat.ResizeReset(iNextItem, 0);
		    WorkMat.PutItem(1, 1, 1, 0.); //FIXME: avoid SIGSEGV if the matrix is empty
		    break;

	       case APPEND:
		    iNextItem = WorkMat.iGetNumRows() + 1;
		    break;

	       default:
		    SP_GRAD_ASSERT(0);
	       }

	  }

	  template <typename T>
	  static void AssJac(T* pElem,
			     SparseSubMatrixHandler& WorkMat,
			     doublereal dCoef,
			     const VectorHandler& XCurr,
			     const VectorHandler& XPrimeCurr,
			     SpFunctionCall func,
			     SpAssMode mode = RESET) {

	       const SpGradientVectorHandler<SpGradient> XCurr_grad(XCurr);
	       const SpGradientVectorHandler<SpGradient> XPrimeCurr_grad(XPrimeCurr);

	       SpGradientAssVec WorkMat_grad(WorkMat, mode);

	       pElem->AssRes(WorkMat_grad, dCoef, XCurr_grad, XPrimeCurr_grad, func);
	  }

	  template <typename T>
	  static void InitialAssJac(T* pElem,
				    SparseSubMatrixHandler& WorkMat,
				    const VectorHandler& XCurr,
				    SpFunctionCall func,
				    SpAssMode mode = RESET) {

	       const SpGradientVectorHandler<SpGradient> XCurr_grad(XCurr);

	       SpGradientAssVec WorkMat_grad(WorkMat, mode);

	       pElem->InitialAssRes(WorkMat_grad, XCurr_grad, func);
	  }

	  void AddItem(integer iRow, const SpGradient& g) {
	       for (const auto& r:g) {
		    const double dCoef = r.dDer;
		    
		    if (dCoef) {
			 const index_type iDofIndex = r.iDof;
			 WorkMat.Resize(iNextItem, 0);
			 
			 SP_GRAD_ASSERT(iNextItem <= WorkMat.iGetNumRows());
			 SP_GRAD_ASSERT(std::isfinite(dCoef));

			 WorkMat.PutItem(iNextItem++, iRow, iDofIndex, dCoef);
		    }
	       }
	  }

	  template <index_type N_rows>
	  void AddItem(integer iFirstRow, const SpColVector<SpGradient, N_rows>& v) {
	       // zero based index according to VectorHandler::Put(integer iRow, const Vec3& v)

	       for (integer i = 0; i < v.iGetNumRows(); ++i) {
		    AddItem(iFirstRow + i, v(i + 1));
	       }
	  }
     private:
	  SparseSubMatrixHandler& WorkMat;
	  integer iNextItem;
     };

} // namespace

#endif /* ___SP_MATVECASS_H__INCLUDED___ */
