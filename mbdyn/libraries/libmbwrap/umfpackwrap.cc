/* 
 * HmFe (C) is a FEM analysis code. 
 *
 * Copyright (C) 1996-2003
 *
 * Marco Morandini  <morandini@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
/* December 2001 
 * Modified to add a Sparse matrix in row form and to implement methods
 * to be used in the parallel MBDyn Solver.
 *
 * Copyright (C) 1996-2003
 *
 * Giuseppe Quaranta  <quaranta@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *      
 */
/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2003
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
 * Umfpack is used by permission; please read its Copyright,
 * License and Availability note.
 */

#ifdef HAVE_CONFIG_H
#include <mbconfig.h>           /* This goes first in every *.c,*.cc file */
#endif /* HAVE_CONFIG_H */

#ifdef USE_UMFPACK
#include <umfpackwrap.h>

#ifdef HAVE_UMFPACK4

/*
 * Wrap of the required functions; (mostly) uses the calling convention 
 * of umfpack 3.0 so the matrix must be square
 */

#define UMFPACKWRAP_defaults 		umfpack_di_defaults
#define UMFPACKWRAP_free_symbolic 	umfpack_di_free_symbolic
#define UMFPACKWRAP_free_numeric 	umfpack_di_free_numeric

#ifdef HAVE_UMFPACK4_1
#define UMFPACKWRAP_symbolic(size, app, aip, axp, sym, ctrl, info) \
	umfpack_di_symbolic(size, size, app, aip, axp, sym, ctrl, info)
#else /* !HAVE_UMFPACK4_1 */
#define UMFPACKWRAP_symbolic(size, app, aip, axp, sym, ctrl, info) \
	umfpack_di_symbolic(size, size, app, aip, sym, ctrl, info)
#endif /* !HAVE_UMFPACK4_1 */

#define UMFPACKWRAP_report_info 	umfpack_di_report_info
#define UMFPACKWRAP_report_status 	umfpack_di_report_status
#define UMFPACKWRAP_numeric 		umfpack_di_numeric
#define UMFPACKWRAP_solve 		umfpack_di_solve

/* required factorization type (A * x = b) */
#define SYS_VALUE 			UMFPACK_A

#else /* !HAVE_UMFPACK4 */

/*
 * Wrap of the required functions; uses the calling convention 
 * of umfpack 3.0
 */
#define UMFPACKWRAP_defaults 		umfpack_defaults
#define UMFPACKWRAP_free_symbolic 	umfpack_free_symbolic
#define UMFPACKWRAP_free_numeric 	umfpack_free_numeric
#define UMFPACKWRAP_symbolic(size, app, aip, axp, sym, ctrl, info) \
	umfpack_symbolic(size, app, aip, sym, ctrl, info)
#define UMFPACKWRAP_report_info 	umfpack_report_info
#define UMFPACKWRAP_report_status 	umfpack_report_status
#define UMFPACKWRAP_numeric 		umfpack_numeric
#define UMFPACKWRAP_solve 		umfpack_solve

/* required factorization type (A * x = b) */
#define SYS_VALUE 			"Ax=b"

#endif /* HAVE_UMFPACK4 */

UmfpackSparseLUSolutionManager::UmfpackSparseLUSolutionManager(integer Dim,
		integer dummy, doublereal dPivot)
: A(Dim),
xVH(0),
bVH(0), 
x(Dim),
b(Dim), 
Symbolic(0),
Numeric(0),
HasBeenReset(true),
CCReady(false)
{
	SAFENEWWITHCONSTRUCTOR(xVH, MyVectorHandler, 
			MyVectorHandler(Dim, &(x[0])));
	SAFENEWWITHCONSTRUCTOR(bVH, MyVectorHandler, 
			MyVectorHandler(Dim, &(b[0])));
	pdRhs = &(b[0]);
	pdSol = &(x[0]);
	UMFPACKWRAP_defaults(Control);

	if (dPivot != -1. && (dPivot >= 0. && dPivot <= 1.)) {
		/*
		 * 1.0: true partial pivoting
		 * 0.0: treated as 1.0
		 * 
		 * default: 0.1
		 */
		Control[UMFPACK_PIVOT_TOLERANCE] = dPivot;
	}
}

UmfpackSparseLUSolutionManager::~UmfpackSparseLUSolutionManager(void) 
{
	UMFPACKWRAP_free_symbolic(&Symbolic);
	ASSERT(Symbolic == 0);

	UMFPACKWRAP_free_numeric(&Numeric);
	ASSERT(Numeric == 0);
	
	SAFEDELETE(xVH);
	SAFEDELETE(bVH);
}

void
UmfpackSparseLUSolutionManager::MatrInit(const doublereal& d)
{
	if (!CCReady) {
		A.Reset(d);
	} else {
		Ac->Reset(d);
	}
	if (Numeric) {
		UMFPACKWRAP_free_numeric(&Numeric);
		ASSERT(Numeric == 0);
	}
	HasBeenReset = true;
}

bool 
UmfpackSparseLUSolutionManager::PrepareSymbolic(void)
{
	const int* const Aip = &(Ai[0]);
	const int* const App = &(Ap[0]);
	const doublereal* const Axp = &(Ax[0]);
	int status;

	status = UMFPACKWRAP_symbolic(b.size(), App, Aip, Axp,
			&Symbolic, Control, Info);
	if (status != UMFPACK_OK) {
		UMFPACKWRAP_report_info(Control, Info) ;
		UMFPACKWRAP_report_status(Control, status);
		std::cerr << "UMFPACKWRAP_symbolic failed" << std::endl;

		/* de-allocate memory */
		UMFPACKWRAP_free_symbolic(&Symbolic);
		ASSERT(Symbolic == 0);

		return false;
	}

	return true;
}

/* Risolve il sistema  Fattorizzazione + Bacward Substitution*/
void
UmfpackSparseLUSolutionManager::Solve(void)
{
	doublereal t = 0.;

#ifdef UMFPACK_REPORT
	t = umfpack_timer() ;
#endif /* UMFPACK_REPORT */

	if (HasBeenReset) {
		if (!CCReady) {
			A.MakeCompressedColumnForm(Ax, Ai, Ap);
			Ac = new CColMatrixHandler(Ax, Ai, Ap);
			CCReady = true;
		}
		const doublereal* const Axp = &(Ax[0]);
		const int* const Aip = &(Ai[0]);
		const int* const App = &(Ap[0]);
		int status;
		
		if (Symbolic == 0 && !PrepareSymbolic()) {
			THROW(ErrGeneric());
		}
#ifdef UMFPACK_REPORT
		UMFPACKWRAP_report_symbolic ("Symbolic factorization of A",
				Symbolic, Control) ;
		UMFPACKWRAP_report_info(Control, Info);
		doublereal t1 = umfpack_timer() - t;
#endif /* UMFPACK_REPORT */

		status = UMFPACKWRAP_numeric(App, Aip, Axp, Symbolic, 
				&Numeric, Control, Info);
		if (status == UMFPACK_ERROR_different_pattern) {
			UMFPACKWRAP_free_symbolic(&Symbolic);
			if (!PrepareSymbolic()) {
				THROW(ErrGeneric());
			}
			status = UMFPACKWRAP_numeric(App, Aip, Axp, Symbolic, 
					&Numeric, Control, Info);
		}

		if (status != UMFPACK_OK) {
			UMFPACKWRAP_report_info(Control, Info);
			UMFPACKWRAP_report_status(Control, status);
			std::cerr << "UMFPACKWRAP_numeric failed" << std::endl;

			/* de-allocate memory */
			UMFPACKWRAP_free_symbolic(&Symbolic);
			UMFPACKWRAP_free_numeric(&Numeric);
			ASSERT(Numeric == 0);

			THROW(ErrGeneric());
		}
		
#ifdef UMFPACK_REPORT
		UMFPACKWRAP_report_numeric ("Numeric factorization of A",
				Numeric, Control);
		UMFPACKWRAP_report_info(Control, Info);
		t1 = umfpack_timer() - t;
#endif /* UMFPACK_REPORT */

		HasBeenReset = false;
	}
	
	BackSub(t);
}

/* Bacward Substitution */
void
UmfpackSparseLUSolutionManager::BackSub(doublereal t_iniz)
{
	const doublereal* const Axp = &(Ax[0]);
	const int* const Aip = &(Ai[0]);
	const int* const App = &(Ap[0]);
	int status;

	ASSERT(HasBeenReset == false);
	
#ifdef UMFPACK_REPORT
	doublereal t = t_iniz;
#endif /* UMFPACK_REPORT */

	Control[UMFPACK_IRSTEP]= 0;
	status = UMFPACKWRAP_solve(SYS_VALUE,
			App, Aip, Axp, pdSol, pdRhs, 
			Numeric, Control, Info);
	if (status != UMFPACK_OK) {
		UMFPACKWRAP_report_info(Control, Info) ;
		UMFPACKWRAP_report_status(Control, status) ;
		std::cerr << "UMFPACKWRAP_solve failed" << std::endl;
		
		/* de-allocate memory */
		UMFPACKWRAP_free_numeric(&Numeric);
		ASSERT(Numeric == 0);

		THROW(ErrGeneric());
	}
	
#ifdef UMFPACK_REPORT
	UMFPACKWRAP_report_info(Control, Info);
	doublereal t1 = umfpack_timer() - t;
#endif /* UMFPACK_REPORT */
}

/* Rende disponibile l'handler per la matrice */
MatrixHandler*
UmfpackSparseLUSolutionManager::pMatHdl(void) const
{
	if (!CCReady) {
		return &A;
	} else {
		return Ac;
	}
}

/* Rende disponibile l'handler per il termine noto */
MyVectorHandler*
UmfpackSparseLUSolutionManager::pResHdl(void) const {
	return bVH;
}

/* Rende disponibile l'handler per la soluzione */
MyVectorHandler*
UmfpackSparseLUSolutionManager::pSolHdl(void) const {
	return xVH;
}

#endif /* USE_UMFPACK */

