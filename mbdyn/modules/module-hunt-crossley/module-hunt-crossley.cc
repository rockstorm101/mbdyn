/* $Header$ */
/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2011
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
 * Author: Pierangelo Masarati <masarati@aero.polimi.it>
 */

#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */

#include <cmath>
#include <cfloat>

#include "dataman.h"
#include "constltp_impl.h"

class HuntCrossleyCL
: public ElasticConstitutiveLaw<doublereal, doublereal> {
protected:
	doublereal dAlpha;
	doublereal dK;
	doublereal dExp;
	bool m_bActive;			// is contact ongoing?
	bool m_bToggling;		// toggle m_bActive
	doublereal m_InitialEpsPrime;	// initial contact velocity

public:
	HuntCrossleyCL(const TplDriveCaller<doublereal> *pTplDC,
		doublereal dAlpha, doublereal dK, doublereal dExp)
	: ElasticConstitutiveLaw<doublereal, doublereal>(pTplDC, 0.),
	dAlpha(dAlpha), dK(dK), dExp(dExp),
	m_bActive(false), m_bToggling(false), m_InitialEpsPrime(0.)
	{
		NO_OP;
	};

	virtual ~HuntCrossleyCL(void) {
		NO_OP;
	};

	virtual ConstLawType::Type GetConstLawType(void) const {
		return ConstLawType::VISCOELASTIC;
	};

	virtual ConstitutiveLaw<doublereal, doublereal>* pCopy(void) const {
		ConstitutiveLaw<doublereal, doublereal>* pCL = 0;

		// pass parameters to copy constructor
		SAFENEWWITHCONSTRUCTOR(pCL, HuntCrossleyCL,
			HuntCrossleyCL(pGetDriveCaller()->pCopy(),
				dAlpha, dK, dExp));
		return pCL;
	};

	virtual std::ostream& Restart(std::ostream& out) const {
		out << "hunt crossley"
			<< ", alpha, " << dAlpha
			<< ", kappa, " << dK
			<< ", exp, " << dExp
			<< ", ", ElasticConstitutiveLaw<doublereal, doublereal>::Restart_int(out);
		return out;
	};

	virtual void Update(const doublereal& Eps, const doublereal& EpsPrime) {
		ConstitutiveLaw<doublereal, doublereal>::Epsilon = Eps - ElasticConstitutiveLaw<doublereal, doublereal>::Get();
		ConstitutiveLaw<doublereal, doublereal>::EpsilonPrime = EpsPrime;

		if (ConstitutiveLaw<doublereal, doublereal>::Epsilon >= 0.) {
			if (m_bActive) {
				if (!m_bToggling) {
					m_bToggling = true;
				}
			}

			ConstitutiveLaw<doublereal, doublereal>::F = 0.;
			ConstitutiveLaw<doublereal, doublereal>::FDE = 0.;
			ConstitutiveLaw<doublereal, doublereal>::FDEPrime = 0.;

		} else {
			if (!m_bActive) {
				if (!m_bToggling) {
					m_bToggling = true;
					m_InitialEpsPrime = EpsPrime;
				}
			}

			doublereal x = -ConstitutiveLaw<doublereal, doublereal>::Epsilon;
			doublereal xp = -ConstitutiveLaw<doublereal, doublereal>::EpsilonPrime;
			doublereal xn = std::pow(x, dExp);
			doublereal xnm1 = std::pow(x, dExp - 1.);

			ConstitutiveLaw<doublereal, doublereal>::F = -dK*xn - 1.5*dAlpha*dK*xn*xp;
			ConstitutiveLaw<doublereal, doublereal>::FDE = -dExp*dK*xnm1 - 1.5*dExp*dAlpha*dK*xnm1*xp;
			ConstitutiveLaw<doublereal, doublereal>::FDEPrime = -1.5*dAlpha*dK*xn;
		}
	};

	virtual void AfterConvergence(const doublereal& Eps, const doublereal& EpsPrime = 0.) {
		if (m_bToggling) {
#if 0
			silent_cout(">> HuntCrossleyCL::AfterConvergence() "
				"m_bToggling=" << (m_bToggling ? "true" : "false") << " "
				"m_bActive=" << (m_bActive ? "true" : "false") << " "
				"m_InitialEpsPrime=" << m_InitialEpsPrime << std::endl);
#endif
			if (m_bActive) {
				m_bActive = false;
				m_InitialEpsPrime = 0.;
			} else {
				m_bActive = true;
			}
			m_bToggling = false;
#if 0
			silent_cout("<< HuntCrossleyCL::AfterConvergence() "
				"m_bToggling=" << (m_bToggling ? "true" : "false") << " "
				"m_bActive=" << (m_bActive ? "true" : "false") << " "
				"m_InitialEpsPrime=" << m_InitialEpsPrime << std::endl);
#endif
		}
	};
};

/* specific functional object(s) */
struct HuntCrossleyCLR : public ConstitutiveLawRead<doublereal, doublereal> {
	virtual ConstitutiveLaw<doublereal, doublereal> *
	Read(const DataManager* pDM, MBDynParser& HP, ConstLawType::Type& CLType) {
		ConstitutiveLaw<doublereal, doublereal>* pCL = 0;

		CLType = ConstLawType::VISCOELASTIC;

		if (HP.IsKeyWord("help")) {
			silent_cerr("HuntCrossleyCL:\n"
				"        hunt crossley,\n"
				"                alpha, <alpha>,\n"
				"                kappa, <kappa>,\n"
				"                exp, <exp>,\n"
				"                [ , prestress, <prestress> ]\n"
				"                [ , prestrain, (DriveCaller)<prestrain> ]\n"
				<< std::endl);

			if (!HP.IsArg()) {
				throw NoErr(MBDYN_EXCEPT_ARGS);
			}
		}

		if (!HP.IsKeyWord("alpha")) {
			silent_cerr("HuntCrossleyCLR: \"alpha\" expected at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		doublereal dAlpha = HP.GetReal();
		if (dAlpha < 0.) {
			silent_cerr("HuntCrossleyCLR: invalid \"alpha\" at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		if (!HP.IsKeyWord("kappa")) {
			silent_cerr("HuntCrossleyCLR: \"kappa\" expected at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		doublereal dK = HP.GetReal();
		if (dK <= 0.) {
			silent_cerr("HuntCrossleyCLR: invalid \"kappa\" at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		if (!HP.IsKeyWord("exp")) {
			silent_cerr("HuntCrossleyCLR: \"exp\" expected at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		doublereal dExp = HP.GetReal();
		if (dExp < 1.) {
			silent_cerr("HuntCrossleyCLR: invalid \"exp\" at line " << HP.GetLineData() << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		/* Prestress and prestrain */
		// doublereal PreStress(0.);
		// GetPreStress(HP, PreStress);
		TplDriveCaller<doublereal> *pTplDC = GetPreStrain<doublereal>(pDM, HP);

		SAFENEWWITHCONSTRUCTOR(pCL, HuntCrossleyCL,
			HuntCrossleyCL(pTplDC, dAlpha, dK, dExp));

		return pCL;
	};
};

extern "C" int
module_init(const char *module_name, void *pdm, void *php)
{
#if 0
	DataManager	*pDM = (DataManager *)pdm;
	MBDynParser	*pHP = (MBDynParser *)php;
#endif

	ConstitutiveLawRead<doublereal, doublereal> *rf1D = new HuntCrossleyCLR;
	if (!SetCL1D("hunt" "crossley", rf1D)) {
		delete rf1D;

		silent_cerr("HuntCrossleyCL: "
			"module_init(" << module_name << ") "
			"failed" << std::endl);

		return -1;
	}

	return 0;
}

