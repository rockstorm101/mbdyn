/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2006
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

#ifdef HAVE_CONFIG_H
#include <mbconfig.h>           /* This goes first in every *.c,*.cc file */
#endif /* HAVE_CONFIG_H */

#include "ac/math.h"
#include "ac/float.h"

#ifdef USE_MPI
#include "mbcomm.h"
#endif /* USE_MPI */

#include "dataman.h"
#include "drive_.h"
#include "dofdrive.h"
#include "privdrive.h"
#include "filedrv.h"
#include "ddrive.h"

/* StringDriveCaller - begin */

StringDriveCaller::StringDriveCaller(const DriveHandler* pDH,
	const char* const sTmpStr)
: DriveCaller(pDH), sEvalStr(0), iEvalStrLen(0)
{
	ASSERT(sTmpStr != 0);
	iEvalStrLen = strlen(sTmpStr);
	SAFESTRDUP(sEvalStr, sTmpStr);
}


StringDriveCaller::~StringDriveCaller(void)
{
	ASSERT(sEvalStr != 0);
	if (sEvalStr != 0) {
		SAFEDELETEARR(sEvalStr);
	}
}


/* Copia */
DriveCaller *
StringDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		StringDriveCaller,
		StringDriveCaller(pDrvHdl, sEvalStr));

	return pDC;
}


/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
StringDriveCaller::Restart(std::ostream& out) const
{
	return out << "string, \"" << sEvalStr << "\"";
}

/* StringDriveCaller - end */


/* TimeDriveCaller - begin */

TimeDriveCaller::TimeDriveCaller(const DriveHandler* pDH)
: DriveCaller(pDH)
{
	NO_OP;
}

TimeDriveCaller::~TimeDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
TimeDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC, TimeDriveCaller, TimeDriveCaller(pDrvHdl));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
TimeDriveCaller::Restart(std::ostream& out) const
{
	return out << "time";
}

/* TimeDriveCaller - end */


/* ConstDriveCaller - begin */

ConstDriveCaller::ConstDriveCaller(doublereal d)
: DriveCaller(0), dConst(d)
{
	NO_OP;
}

ConstDriveCaller::~ConstDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller* ConstDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC, ConstDriveCaller, ConstDriveCaller(dConst));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
ConstDriveCaller::Restart(std::ostream& out) const
{
	return out << " const, " << dConst;
}

/* ConstDriveCaller - end */


/* LinearDriveCaller - begin */

LinearDriveCaller::LinearDriveCaller(const DriveHandler* pDH,
	doublereal d0, doublereal d1)
: DriveCaller(pDH), dC0(d0), dC1(d1)
{
	NO_OP;
}

LinearDriveCaller::~LinearDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
LinearDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC, LinearDriveCaller, LinearDriveCaller(pDrvHdl, dC0, dC1));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
LinearDriveCaller::Restart(std::ostream& out) const
{
	return out << " linear, " << dC0 << ", " << dC1;
}

/* LinearDriveCaller - end */


/* ParabolicDriveCaller - begin */

ParabolicDriveCaller::ParabolicDriveCaller(const DriveHandler* pDH,
	doublereal d0, doublereal d1, doublereal d2)
: DriveCaller(pDH), dC0(d0), dC1(d1), dC2(d2)
{
	NO_OP;
}

ParabolicDriveCaller::~ParabolicDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
ParabolicDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC, ParabolicDriveCaller, ParabolicDriveCaller(pDrvHdl, dC0, dC1, dC2));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
ParabolicDriveCaller::Restart(std::ostream& out) const
{
	return out <<  " parabolic, " << dC0 << ", " << dC1 << ", " << dC2;
}

/* ParabolicDriveCaller - end */


/* CubicDriveCaller - begin */

CubicDriveCaller::CubicDriveCaller(const DriveHandler* pDH,
	doublereal d0, doublereal d1,
	doublereal d2, doublereal d3)
: DriveCaller(pDH), dC0(d0), dC1(d1), dC2(d2), dC3(d3)
{
	NO_OP;
}

CubicDriveCaller::~CubicDriveCaller(void)
{
   NO_OP;
}

/* Copia */
DriveCaller *
CubicDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC, CubicDriveCaller, CubicDriveCaller(pDrvHdl, dC0, dC1, dC2, dC3));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
CubicDriveCaller::Restart(std::ostream& out) const
{
	return out << " cubic, "
		<< dC0 << ", " << dC1 << ", " << dC2 << ", " << dC3;
}

/* CubicDriveCaller - end */


/* StepDriveCaller - begin */

StepDriveCaller::StepDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2, doublereal d3)
: DriveCaller(pDH),
dStepTime(d1), dStepValue(d2), dInitialValue(d3)
{
	NO_OP;
}

StepDriveCaller::~StepDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
StepDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		StepDriveCaller,
		StepDriveCaller(pDrvHdl, dStepTime, dStepValue, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
StepDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " step, " << dStepTime
		<< ", " << dStepValue
		<< ", " << dInitialValue;
}

/* StepDriveCaller - end */


/* DoubleStepDriveCaller - begin */

DoubleStepDriveCaller::DoubleStepDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2, doublereal d3, doublereal d4)
: DriveCaller(pDH), dStepTime(d1), dStepValue(d2),
dEndStepTime(d3), dInitialValue(d4)
{
	NO_OP;
}

DoubleStepDriveCaller::~DoubleStepDriveCaller(void)
{
	NO_OP;
}


/* Copia */
DriveCaller *
DoubleStepDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		DoubleStepDriveCaller,
		DoubleStepDriveCaller(pDrvHdl, dStepTime, dStepValue, dEndStepTime, dInitialValue));
	return pDC;
}


/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
DoubleStepDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " double step, " << dStepTime
		<< ", " << dEndStepTime
		<< ", " << dStepValue
		<< ", " << dInitialValue;
}

/* DoubleStepDriveCaller - end */


/* RampDriveCaller - begin */

RampDriveCaller::RampDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2, doublereal d3, doublereal d4)
: DriveCaller(pDH), dSlope(d1), dStartTime(d2), dEndTime(d3), dInitialValue(d4)
{
	NO_OP;
}

RampDriveCaller::~RampDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
RampDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		RampDriveCaller,
		RampDriveCaller(pDrvHdl, dSlope, dStartTime, dEndTime, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
RampDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< "ramp, " << dSlope
		<< ", " << dStartTime
		<< ", " << dEndTime
		<< ", " << dInitialValue;
}

/* RampDriveCaller - end */


/* DoubleRampDriveCaller - begin */

DoubleRampDriveCaller::DoubleRampDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2, doublereal d3,
	doublereal d4, doublereal d5, doublereal d6,
	doublereal d7)
: DriveCaller(pDH),
dAscendingSlope(d1), dAscendingStartTime(d2), dAscendingEndTime(d3),
dDescendingSlope(d4), dDescendingStartTime(d5), dDescendingEndTime(d6),
dInitialValue(d7)
{
	NO_OP;
}

DoubleRampDriveCaller::~DoubleRampDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
DoubleRampDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		DoubleRampDriveCaller,
		DoubleRampDriveCaller(pDrvHdl, dAscendingSlope, dAscendingStartTime, dAscendingEndTime,
			dDescendingSlope, dDescendingStartTime, dDescendingEndTime, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
DoubleRampDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " double ramp, " << dAscendingSlope
		<< ", " << dAscendingStartTime
		<< ", " << dAscendingEndTime
		<< ", " << dDescendingSlope
		<< ", " << dDescendingStartTime
		<< ", " << dDescendingEndTime
		<< ", " << dInitialValue;
}

/* DoubleRampDriveCaller - end */


/* SineDriveCaller - begin */

SineDriveCaller::SineDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2, doublereal d3,
	integer iNumCyc, doublereal d4)
: DriveCaller(pDH),
dStartTime(d1), dOmega(d2), dAmplitude(d3),
iNumCycles(iNumCyc), dInitialValue(d4), bNeverEnd(false)
{
	/* Onde di seno che partono da zero ed arrivano a0 */
	if (iNumCycles > 0) {
		dEndTime = dStartTime + 2.*M_PI/dOmega*(doublereal(iNumCycles) - .5);
		dFinalValue = dInitialValue;

	/* Onde di seno che continuano all'infinito */
	} else if (iNumCycles == 0) {
		dEndTime = 0.;
		bNeverEnd = true;

	/* Onde di seno che partono da 0 ed arrivano ad 1
	 * con tangente orizzontale */
	} else {
		dEndTime = dStartTime + 2.*M_PI/dOmega*(doublereal(-iNumCycles) - .75);
		dFinalValue = dInitialValue + dAmplitude;
	}
}

SineDriveCaller::~SineDriveCaller(void)
{
   NO_OP;
}

/* Copia */
DriveCaller *
SineDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		SineDriveCaller,
		SineDriveCaller(pDrvHdl, dStartTime, dOmega, dAmplitude, iNumCycles, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
SineDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " sine, " << dStartTime
		<< ", " << dOmega
		<< ", " << dAmplitude
		<< ", " << iNumCycles
		<< ", " << dInitialValue;
}

/* SineDriveCaller - end */


/* FourierSeriesDriveCaller - begin */

FourierSeriesDriveCaller::FourierSeriesDriveCaller(const DriveHandler* pDH,
	doublereal dStartTime,
	doublereal dOmega,
	std::vector<doublereal>& a,
	integer iNumCyc,
	doublereal dInitialValue)
: DriveCaller(pDH),
dStartTime(dStartTime),
dOmega(dOmega),
iNumCycles(iNumCyc),
dInitialValue(dInitialValue),
bNeverEnd(false)
{
	ASSERT(iNumCycles >= 0);

	if (iNumCycles > 0) {
		dEndTime = dStartTime
			+ 2.*M_PI/dOmega*doublereal(iNumCycles);

	/* Onde di coseno che continuano all'infinito */
	} else if (iNumCycles == 0) {
		dEndTime = 0.;
		bNeverEnd = true;
	}

	amplitudes.resize(a.size());
	for (unsigned i = 0; i < a.size(); i++) {
		amplitudes[i] = a[i];
	}
}

FourierSeriesDriveCaller::~FourierSeriesDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
FourierSeriesDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;

	SAFENEWWITHCONSTRUCTOR(pDC,
		FourierSeriesDriveCaller,
		FourierSeriesDriveCaller(pDrvHdl,
			dStartTime,
			dOmega,
			amplitudes,
			iNumCycles,
			dInitialValue));
	return pDC;
}


/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
FourierSeriesDriveCaller::Restart(std::ostream& out) const
{
	out
		<< " fourier series, "
		<< dStartTime << ", "
		<< dOmega << ", "
		<< amplitudes.size()/2 << ", "
		<< 2.*amplitudes[0] << ", ";

	for (unsigned i = 1; i < amplitudes.size(); i++) {
		out << amplitudes[i] << ", ";
	}

	out
		<< iNumCycles << ", "
		<< dInitialValue;

	return out;
}

/* FourierSeriesDriveCaller - end */


/* CosineDriveCaller - begin */

CosineDriveCaller::CosineDriveCaller(const DriveHandler* pDH,
	doublereal d1, doublereal d2,
	doublereal d3,
	integer iNumCyc, doublereal d4)
: DriveCaller(pDH),
dStartTime(d1), dOmega(d2), dAmplitude(d3),
iNumCycles(iNumCyc), dInitialValue(d4), bNeverEnd(false)
{
	/* Onde di coseno che partono da zero ed arrivano a 0 */
	if (iNumCycles > 0) {
		dEndTime = dStartTime + 2.*M_PI/dOmega*doublereal(iNumCycles);
		dFinalValue = dInitialValue;

	/* Onde di coseno che continuano all'infinito */
	} else if(iNumCycles == 0) {
		dEndTime = 0.;
		bNeverEnd = true;

	/* Onde di coseno che partono da 0 ed arrivano ad 1
	 * con tangente orizzontale */
	} else {
		dEndTime = dStartTime + 2.*M_PI/dOmega*(doublereal(-iNumCycles) - .5);
		dFinalValue = dInitialValue + 2.*dAmplitude;
	}
}

CosineDriveCaller::~CosineDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller* CosineDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		CosineDriveCaller,
		CosineDriveCaller(pDrvHdl, dStartTime, dOmega, dAmplitude, iNumCycles, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
CosineDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " cosine, " << dStartTime
		<< ", " << dOmega
		<< ", " << dAmplitude
		<< ", " << iNumCycles
		<< ", " << dInitialValue;
}

/* CosineDriveCaller - end */


/* FreqSweepDriveCaller - begin */

FreqSweepDriveCaller::FreqSweepDriveCaller(const DriveHandler* pDH,
	doublereal d1,
	const DriveCaller* pOmega,
	const DriveCaller* pAmplitude,
	doublereal d2,
	doublereal d3,
	doublereal d4)
: DriveCaller(pDH),
dStartTime(d1), pOmega((DriveCaller *)pOmega),
pAmplitude((DriveCaller *)pAmplitude),
dInitialValue(d2), dEndTime(d3), dFinalValue(d4),
bNeverEnd(false)
{
	ASSERT(pOmega != 0);
	ASSERT(pAmplitude != 0);

	if (dEndTime <= dStartTime) {
		bNeverEnd = true;
	}
}

FreqSweepDriveCaller::~FreqSweepDriveCaller(void)
{
	if (pOmega) {
		SAFEDELETE(pOmega);
	}

	if (pAmplitude) {
		SAFEDELETE(pAmplitude);
	}
}

/* Copia */
DriveCaller *
FreqSweepDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		FreqSweepDriveCaller,
		FreqSweepDriveCaller(pDrvHdl, dStartTime, pOmega->pCopy(),
			pAmplitude->pCopy(), dInitialValue, dEndTime, dFinalValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
FreqSweepDriveCaller::Restart(std::ostream& out) const
{
	out
		<< " frequency sweep, " << dStartTime
		<< ", ", pOmega->Restart(out)
		<< ", ", pAmplitude->Restart(out)
		<< ", " << dInitialValue
		<< ", " << dEndTime
		<< ", " << dFinalValue;
	return out;
}

/* FreqSweepDriveCaller - end */


/* ExpDriveCaller - begin */

ExpDriveCaller::ExpDriveCaller(const DriveHandler* pDH,
	doublereal dA, doublereal dT,
	doublereal dS, doublereal dI)
: DriveCaller(pDH),
dAmplitude(dA), dTimeConst(dT), dStartTime(dS), dInitialValue(dI)
{
	NO_OP;
}

ExpDriveCaller::~ExpDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller* ExpDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		ExpDriveCaller,
		ExpDriveCaller(pDrvHdl, dAmplitude, dTimeConst, dStartTime, dInitialValue));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
ExpDriveCaller::Restart(std::ostream& out) const
{
	return out
		<< " exponential, " << dAmplitude
		<< ", " << dTimeConst
		<< ", " << dStartTime
		<< ", " << dInitialValue;
}

/* ExpDriveCaller - end */


/* RandDriveCaller - begin */

RandDriveCaller::RandDriveCaller(const DriveHandler* pDH,
	doublereal dA, doublereal dR,
	doublereal dS, doublereal dE, integer iS)
: DriveCaller(pDH), dAmplitude(dA), dRefVal(dR),
dStartTime(dS), dEndTime(dE), iSteps(iS)
{
	iBase = rand();
	iRandDriveNumber = pDrvHdl->iRandInit(iSteps);
}

RandDriveCaller::~RandDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
RandDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		RandDriveCaller,
		RandDriveCaller(pDrvHdl, dAmplitude, dRefVal, dStartTime, dEndTime, iSteps));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
RandDriveCaller::Restart(std::ostream& out) const
{
	out
		<< " random, " << dAmplitude
		<< ", " << dRefVal
		<< ", " << dStartTime
		<< ", " << dEndTime;
	if (iSteps > 1) {
		out << ", steps, " << iSteps;
	}
	return out;
}

/* RandDriveCaller - end */


/* MeterDriveCaller - begin */

MeterDriveCaller::MeterDriveCaller(const DriveHandler* pDH,
	doublereal dS, doublereal dE, integer iS)
: DriveCaller(pDH),
dStartTime(dS),
dEndTime(dE),
iSteps(iS)
{
	iMeterDriveNumber = pDrvHdl->iMeterInit(iSteps);
}

MeterDriveCaller::~MeterDriveCaller(void)
{
	NO_OP;
}

/* Copia */
DriveCaller *
MeterDriveCaller::pCopy(void) const
{
	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		MeterDriveCaller,
		MeterDriveCaller(pDrvHdl, dStartTime, dEndTime, iSteps));
	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
MeterDriveCaller::Restart(std::ostream& out) const
{
	out
		<< " meter, " << dStartTime
		<< ", " << dEndTime;
	if (iSteps > 1) {
		out << ", steps, " << iSteps;
	}
	return out;
}

/* MeterDriveCaller - end */


/* PiecewiseLinearDriveCaller - begin */

PiecewiseLinearDriveCaller::PiecewiseLinearDriveCaller(const DriveHandler* pDH,
	unsigned int i, doublereal *p)
: DriveCaller(pDH), iNumPoints(i), pPoints(p), pVals(p + i)
{
	ASSERT(i >= 2);
	ASSERT(p != 0);
}

PiecewiseLinearDriveCaller::~PiecewiseLinearDriveCaller(void)
{
	if (pPoints != 0) {
		SAFEDELETEARR(pPoints);
	}
}

/* Copia */
DriveCaller *
PiecewiseLinearDriveCaller::pCopy(void) const
{
	doublereal *p = 0;
	SAFENEWARR(p, doublereal, 2*iNumPoints);
	for (unsigned int i = 0; i < 2*iNumPoints; i++) {
		p[i] = pPoints[i];
	}

	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
			PiecewiseLinearDriveCaller,
			PiecewiseLinearDriveCaller(pDrvHdl, iNumPoints, p));

	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
PiecewiseLinearDriveCaller::Restart(std::ostream& out) const
{
	out << "piecewise linear, " << iNumPoints;

	for (unsigned int i = 0; i < iNumPoints; i++) {
		out << ", " << pPoints[i] << ", " << pVals[i];
	}

	return out;
}

/* PiecewiseLinearDriveCaller - end */


/* DriveArrayCaller - begin */

DriveArrayCaller::DriveArrayCaller(const DriveHandler* pDH,
	unsigned short int i, const DriveCaller** ppDC)
: DriveCaller(pDH), iNumDrivers(i), ppDriveCallers(ppDC)
{
#ifdef DEBUG
	ASSERT(iNumDrivers > 0);
	ASSERT(ppDriveCallers != 0);
	for (int i = 0; i < iNumDrivers; i++) {
		ASSERT(ppDC[i] != 0);
	}
#endif /* DEBUG */
}

DriveArrayCaller::~DriveArrayCaller(void)
{
	ASSERT(ppDriveCallers != 0);
	ASSERT(iNumDrivers > 0);

	for (int i = 0; i < iNumDrivers; i++) {
		SAFEDELETE(ppDriveCallers[i]);
	}

	SAFEDELETEARR(ppDriveCallers);
}

/* Copia */
DriveCaller *
DriveArrayCaller::pCopy(void) const
{
	ASSERT(ppDriveCallers != 0);
	ASSERT(iNumDrivers > 0);

	DriveCaller** ppDC = 0;
	SAFENEWARR(ppDC, DriveCaller*, iNumDrivers);

	for (int i = 0; i < iNumDrivers; i++) {
		ppDC[i] = ppDriveCallers[i]->pCopy();
	}

	DriveCaller* pDC = 0;
	SAFENEWWITHCONSTRUCTOR(pDC,
		DriveArrayCaller,
		DriveArrayCaller(pDrvHdl, iNumDrivers, (const DriveCaller**)ppDC));

	return pDC;
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
DriveArrayCaller::Restart(std::ostream& out) const
{
	out << " array, " << iNumDrivers;
	for (int i = 0; i < iNumDrivers; i++) {
		ASSERT(ppDriveCallers[i] != 0);
		out << ", ", ppDriveCallers[i]->Restart(out);
	}
	return out;
}

/* DriveArrayCaller - end */

/* bag that contains functions to parse drive callers */

typedef std::map<std::string, DriveCallerRead *, ltstrcase> DrvFuncMapType;
static DrvFuncMapType DrvFuncMap;

struct DriveWordSetType : public HighParser::WordSet {
	bool IsWord(const char *s) const {
		return DrvFuncMap.find(std::string(s)) != DrvFuncMap.end();
	};
};
static DriveWordSetType DriveWordSet;

bool
SetDriveData(const char *name, DriveCallerRead *rf)
{
	return DrvFuncMap.insert(DrvFuncMapType::value_type(name, rf)).second;
}

/* Legge i dati dei drivers */

DriveCaller *
ReadDriveData(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	DEBUGCOUTFNAME("ReadDriveData()");

	const char *s = HP.IsWord(DriveWordSet);
	if (s == 0) {
		s = "const";
	}

	DrvFuncMapType::iterator func = DrvFuncMap.find(std::string(s));
	if (func == DrvFuncMap.end()) {
		silent_cerr("unknown drive type \"" << s << "\" "
			"at line " << HP.GetLineData() << std::endl);
		throw DataManager::ErrGeneric();
	}

	return func->second->Read(pDM, HP, bDeferred);
}

void
DriveCallerRead::NeedDM(const DataManager* pDM, MBDynParser& HP, bool bDeferred,
	const char *const name)
{
	if (pDM == 0 && !bDeferred) {
		silent_cerr("\"" << name << "\" drive needs data manager "
			"at line " << HP.GetLineData() << std::endl);
		throw DataManager::ErrNeedDataManager();
	}
}

struct TimeDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
TimeDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "time");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	SAFENEWWITHCONSTRUCTOR(pDC, TimeDriveCaller, TimeDriveCaller(pDrvHdl));

	return pDC;
}

struct NullDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* /* pDM */ , MBDynParser& /* HP */ , bool /* bDeferred */ );
};

DriveCaller *
NullDCR::Read(const DataManager* /* pDM */ , MBDynParser& /* HP */ , bool /* bDeferred */ )
{
	DriveCaller *pDC = 0;

        SAFENEW(pDC, NullDriveCaller);

	return pDC;
}

struct OneDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* /* pDM */ , MBDynParser& /* HP */ , bool /* bDeferred */ );
};

DriveCaller *
OneDCR::Read(const DataManager* /* pDM */ , MBDynParser& /* HP */ , bool /* bDeferred */ )
{
	DriveCaller *pDC = 0;

        SAFENEW(pDC, OneDriveCaller);

	return pDC;
}

struct ConstDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool /* bDeferred */ );
};

DriveCaller *
ConstDCR::Read(const DataManager* pDM, MBDynParser& HP, bool /* bDeferred */ )
{
	DriveCaller *pDC = 0;

	/* lettura dei dati specifici */
	doublereal dConst = HP.GetReal();
	DEBUGCOUT("Const value: " << dConst << std::endl);

	/* allocazione e creazione */
	if (dConst == 0.) {
		SAFENEW(pDC, NullDriveCaller);

	} else if (dConst == 1.) {
		SAFENEW(pDC, OneDriveCaller);

	} else {
		SAFENEWWITHCONSTRUCTOR(pDC, ConstDriveCaller, ConstDriveCaller(dConst));
	}

	return pDC;
}

struct LinearDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
LinearDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "linear");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* lettura dei dati specifici */
	doublereal dC0 = HP.GetReal();
	DEBUGCOUT("Offset: " << dC0 << std::endl);

	doublereal dC1 = HP.GetReal();
	DEBUGCOUT("Slope: " << dC1 << std::endl);

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		LinearDriveCaller,
		LinearDriveCaller(pDrvHdl, dC0, dC1));

	return pDC;
}

struct ParabolicDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
ParabolicDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "parabolic");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* lettura dei dati specifici */
	doublereal dC0 = HP.GetReal();
	DEBUGCOUT("Offset: " << dC0 << std::endl);

	doublereal dC1 = HP.GetReal();
	DEBUGCOUT("Slope: " << dC1 << std::endl);

	doublereal dC2 = HP.GetReal();
	DEBUGCOUT("Parabolic slope: " << dC2 << std::endl);

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		ParabolicDriveCaller,
		ParabolicDriveCaller(pDrvHdl, dC0, dC1, dC2));

	return pDC;
}

struct CubicDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
CubicDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "cubic");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* lettura dei dati specifici */
	doublereal dC0 = HP.GetReal();
	DEBUGCOUT("Offset: " << dC0 << std::endl);

	doublereal dC1 = HP.GetReal();
	DEBUGCOUT("Slope: " << dC1 << std::endl);

	doublereal dC2 = HP.GetReal();
	DEBUGCOUT("Parabolic slope: " << dC2 << std::endl);

	doublereal dC3 = HP.GetReal();
	DEBUGCOUT("Cubic slope: " << dC3 << std::endl);

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		CubicDriveCaller,
		CubicDriveCaller(pDrvHdl, dC0, dC1, dC2, dC3));

	return pDC;
}

struct FunctionDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
FunctionDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	return ReadDriveData(pDM, HP, bDeferred);
}

struct StepDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
StepDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "step");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	doublereal dStepTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dStepTime << std::endl);

	doublereal dStepValue = HP.GetReal(1.);
	DEBUGCOUT("Step Value: " << dStepValue << std::endl);

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		StepDriveCaller,
		StepDriveCaller(pDrvHdl, dStepTime, dStepValue, dInitialValue));

	return pDC;
}

struct DoubleStepDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
DoubleStepDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "double step");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	doublereal dStepTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dStepTime << std::endl);

	doublereal dEndStepTime = HP.GetReal();
	DEBUGCOUT("Final time: " << dEndStepTime << std::endl);

	if (dEndStepTime <= dStepTime) {
		silent_cerr("Warning at line "
			<< HP.GetLineData()
			<< ": final time " << dEndStepTime
			<< " is less than or equal to initial time " << dStepTime
			<< " in double step func drive" << std::endl);
	}

	doublereal dStepValue = HP.GetReal(1.);
	DEBUGCOUT("Step Value: " << dStepValue << std::endl);

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		DoubleStepDriveCaller,
		DoubleStepDriveCaller(pDrvHdl,
			dStepTime,
			dStepValue,
			dEndStepTime,
			dInitialValue));

	return pDC;
}

struct RampDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
RampDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "ramp");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* Rampa saturata */
	doublereal dSlope = HP.GetReal(1.);
	DEBUGCOUT("Slope Value: " << dSlope << std::endl);

	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dFinalTime = DBL_MAX;
	if (!HP.IsKeyWord("forever")) {
		dFinalTime = HP.GetReal();
	}
	DEBUGCOUT("Final time: " << dFinalTime << std::endl);

	if (dFinalTime <= dInitialTime) {
		silent_cerr("Warning at line "
			<< HP.GetLineData()
			<< ": final time " << dFinalTime
			<< " is less than or equal to initial time " << dInitialTime
			<< " in ramp func drive" << std::endl);
	}

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		RampDriveCaller,
		RampDriveCaller(pDrvHdl, dSlope, dInitialTime,
			dFinalTime, dInitialValue));

	return pDC;
}

struct DoubleRampDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
DoubleRampDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "double ramp");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* Rampa doppia */
	doublereal dAscendingSlope = HP.GetReal(1.);
	DEBUGCOUT("Ascending Slope Value: " << dAscendingSlope << std::endl);

	doublereal dAscendingInitialTime = HP.GetReal();
	DEBUGCOUT("Ascending Initial time: " << dAscendingInitialTime << std::endl);

	doublereal dAscendingFinalTime = HP.GetReal();
	DEBUGCOUT("Ascending Final time: " << dAscendingFinalTime << std::endl);

	if (dAscendingFinalTime <= dAscendingInitialTime) {
		silent_cerr("Warning at line "
			<< HP.GetLineData() << ": ascending final time "
			<< dAscendingFinalTime
			<< " is less than or equal to ascending initial time "
			<< dAscendingInitialTime
			<< " in double ramp func drive" << std::endl);
		throw ErrGeneric();
	}

	doublereal dDescendingSlope = HP.GetReal(-1.);
	DEBUGCOUT("Descending Slope Value: " << dDescendingSlope << std::endl);

	doublereal dDescendingInitialTime = DBL_MAX;
	if (!HP.IsKeyWord("forever")) {
		dDescendingInitialTime = HP.GetReal();
	}
	DEBUGCOUT("Descending Initial time: " << dDescendingInitialTime << std::endl);

	if (dDescendingInitialTime < dAscendingFinalTime) {
		silent_cerr("Warning at line "
			<< HP.GetLineData() << ": descending initial time "
			<< dDescendingInitialTime
			<< " is less than ascending final time "
			<< dAscendingFinalTime
			<< " in double ramp func drive" << std::endl);
		throw ErrGeneric();
	}

	doublereal dDescendingFinalTime = HP.GetReal();
	DEBUGCOUT("Descending Final time: " << dDescendingFinalTime << std::endl);

	if (dDescendingFinalTime <= dDescendingInitialTime) {
		silent_cerr("Warning at line "
			<< HP.GetLineData() << ": descending final time "
			<< dDescendingFinalTime
			<< " is less than descending initial time "
			<< dDescendingInitialTime
			<< " in double ramp func drive" << std::endl);
		throw ErrGeneric();
	}

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		DoubleRampDriveCaller,
		DoubleRampDriveCaller(pDrvHdl,
			dAscendingSlope, dAscendingInitialTime, dAscendingFinalTime,
			dDescendingSlope, dDescendingInitialTime, dDescendingFinalTime,
			dInitialValue));

	return pDC;
}

struct SineCosineDCR {
protected:
	virtual ~SineCosineDCR( void ) { NO_OP; };
	virtual DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred, bool bSine);
};

DriveCaller *
SineCosineDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred, bool bSine)
{
	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* Seno e coseno (limitati, illimitati, saturati) */
	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dOmega = HP.GetReal(1.);
	DEBUGCOUT("Omega: " << dOmega << std::endl);

	doublereal dAmplitude = HP.GetReal();
	DEBUGCOUT("Amplitude: " << dAmplitude << std::endl);

	integer iNumCycles;
	if (HP.IsKeyWord("forever")) {
		iNumCycles = 0;

	} else if (HP.IsKeyWord("one")) {
		iNumCycles = 1;

	} else if (HP.IsKeyWord("half")) {
		iNumCycles = -1;

	} else {
		iNumCycles = HP.GetInt();
	}
	DEBUGCOUT("Number of cycles: " << iNumCycles << std::endl);

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	if (bSine) {
		SAFENEWWITHCONSTRUCTOR(pDC,
			SineDriveCaller,
			SineDriveCaller(pDrvHdl, dInitialTime, dOmega, dAmplitude, iNumCycles, dInitialValue));

	} else {
		SAFENEWWITHCONSTRUCTOR(pDC,
			CosineDriveCaller,
			CosineDriveCaller(pDrvHdl, dInitialTime, dOmega, dAmplitude, iNumCycles, dInitialValue));
	}

	return pDC;
}

struct SineDCR : public DriveCallerRead, protected SineCosineDCR {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
SineDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "sine");

	return SineCosineDCR::Read(pDM, HP, bDeferred, true);
}

struct CosineDCR : public DriveCallerRead, protected SineCosineDCR {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
CosineDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "cosine");

	return SineCosineDCR::Read(pDM, HP, bDeferred, false);
}

struct FourierSeriesDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
FourierSeriesDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "Fourier series");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dOmega = HP.GetReal(1.);
	DEBUGCOUT("Omega: " << dOmega << std::endl);

	int n = HP.GetInt();
	if (n <= 0) {
		silent_cerr("FourierSeriesDriveCaller: invalid order " << n
			<< " at line " << HP.GetLineData() << std::endl);
		throw ErrGeneric();
	}

	std::vector<doublereal> a(1 + 2*n);
	for (unsigned i = 0; i < 1 + 2*unsigned(n); i++) {
		a[i] = HP.GetReal();
	}

	/* don't remember why, but the series starts with a_0/2 */
	a[0] /= 2.;

	integer iNumCycles;
	if (HP.IsKeyWord("forever")) {
		iNumCycles = 0;

	} else if (HP.IsKeyWord("one")) {
		iNumCycles = 1;

	} else {
		iNumCycles = HP.GetInt();
	}
	DEBUGCOUT("Number of cycles: " << iNumCycles << std::endl);
	if (iNumCycles < 0) {
		silent_cerr("FourierSeriesDriveCaller: invalid number of cycles "
			<< iNumCycles << " at line " << HP.GetLineData() << std::endl);
		throw ErrGeneric();
	}

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("InitialValue: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		FourierSeriesDriveCaller,
		FourierSeriesDriveCaller(pDrvHdl,
			dInitialTime,
			dOmega,
			a,
			iNumCycles,
			dInitialValue));

	return pDC;
}

struct FrequencySweepDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
FrequencySweepDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "frequency sweep");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	DriveCaller* pOmega = 0;
	pOmega = HP.GetDriveCaller();

	DriveCaller* pAmplitude = 0;
	pAmplitude = HP.GetDriveCaller();

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("Initial value: " << dInitialValue << std::endl);

	doublereal dFinalTime = DBL_MAX;
	if (!HP.IsKeyWord("forever")) {
		dFinalTime = HP.GetReal();
	}
	DEBUGCOUT("Final time: " << dFinalTime << std::endl);

	doublereal dFinalValue = HP.GetReal();
	DEBUGCOUT("Final value: " << dFinalValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		FreqSweepDriveCaller,
		FreqSweepDriveCaller(pDrvHdl, dInitialTime, pOmega, pAmplitude,
			dInitialValue, dFinalTime, dFinalValue));

	return pDC;
}

struct ExponentialDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
ExponentialDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "exponential");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	doublereal dAmplitude = HP.GetReal(1.);
	DEBUGCOUT("Amplitude value: " << dAmplitude << std::endl);

	doublereal dTimeConst = HP.GetReal();
	DEBUGCOUT("Time constant value: " << dTimeConst << std::endl);

	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dInitialValue = HP.GetReal();
	DEBUGCOUT("Initial value: " << dInitialValue << std::endl);

	SAFENEWWITHCONSTRUCTOR(pDC,
		ExpDriveCaller,
		ExpDriveCaller(pDrvHdl, dAmplitude, dTimeConst,
			dInitialTime, dInitialValue));

	return pDC;
}

struct RandomDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
RandomDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "random");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* Numero casuale */
	doublereal dAmplitude = HP.GetReal(1.);
	DEBUGCOUT("Amplitude value: " << dAmplitude << std::endl);

	doublereal dRefVal = HP.GetReal();
	DEBUGCOUT("Mean value: " << dRefVal << std::endl);

	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dFinalTime = DBL_MAX;
	if (!HP.IsKeyWord("forever")) {
		dFinalTime = HP.GetReal();
	}
	DEBUGCOUT("Final time: " << dFinalTime << std::endl);

	/* Type of random number (additional data) */
	integer iSteps = 1;
	while (true) {
		if (HP.IsKeyWord("steps")) {
			iSteps = HP.GetInt();
			if (iSteps <= 0) {
				silent_cerr("Warning: Steps number " << iSteps
					<< " is illegal; resorting to default value" << std::endl);
				iSteps = 1;
			}
			DEBUGCOUT("Force changes every " << iSteps
				<< " steps" << std::endl);

		} else if (HP.IsKeyWord("seed")) {
			if (HP.IsKeyWord("time")) {
				DEBUGCOUT("(Re)Seeding random numbers with current time ...");
				srand(time(0));
			} else {
				DEBUGCOUT("(Re)Seeding random numbers with given value ...");
				srand(HP.GetInt());
			}
		} else {
			break;
		}
	}

	SAFENEWWITHCONSTRUCTOR(pDC,
		RandDriveCaller,
		RandDriveCaller(pDrvHdl, dAmplitude, dRefVal, dInitialTime, dFinalTime, iSteps));

	return pDC;
}

struct MeterDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
MeterDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "meter");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* spike every N steps */
	doublereal dInitialTime = HP.GetReal();
	DEBUGCOUT("Initial time: " << dInitialTime << std::endl);

	doublereal dFinalTime = DBL_MAX;
	if (!HP.IsKeyWord("forever")) {
		dFinalTime = HP.GetReal();
	}
	DEBUGCOUT("Final time: " << dFinalTime << std::endl);

	/* Type of random number (additional data) */
	integer iSteps = 1;
	while (true) {
		if (HP.IsKeyWord("steps")) {
			iSteps = HP.GetInt();
			if (iSteps <= 0) {
				silent_cerr("Warning: Steps number " << iSteps
					<< " is illegal; resorting to default value" << std::endl);
				iSteps = 1;
			}
			DEBUGCOUT("Force changes every " << iSteps
				<< " steps" << std::endl);
		} else {
			break;
		}
	}

	SAFENEWWITHCONSTRUCTOR(pDC,
		MeterDriveCaller,
		MeterDriveCaller(pDrvHdl, dInitialTime, dFinalTime, iSteps));

	return pDC;
}

struct PiecewiseLinearDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
PiecewiseLinearDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "piecewise linear");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* Lineare a pezzi */
	unsigned int n = HP.GetInt();
	DEBUGCOUT("number of points: " << n << std::endl);

	if (n < 2) {
		silent_cerr("Need at least two points for piecewise linear drive at line "
			<< HP.GetLineData() << std::endl);
		throw DataManager::ErrGeneric();
	}

	doublereal *p = 0;
	SAFENEWARR(p, doublereal, 2*n);
	p[0] = HP.GetReal();
	p[n] = HP.GetReal();
	for (unsigned int i = 1; i < n; i++) {
		p[i] = HP.GetReal();
		if (p[i] <= p[i-1]) {
			silent_cerr("point " << p[i]
				<< " is smaller than or equal to preceding point " << p[i-1]
				<< " at line " << HP.GetLineData() << std::endl);
			throw DataManager::ErrGeneric();
		}
		p[i+n] = HP.GetReal();
	}

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		PiecewiseLinearDriveCaller,
		PiecewiseLinearDriveCaller(pDrvHdl, n, p));

	return pDC;
}

struct StringDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
StringDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "string");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* driver stringa da valutare */
	/* lettura dei dati specifici */
	const char* sTmp = HP.GetStringWithDelims();
	DEBUGCOUT("String to evaluate: \"" << sTmp << '\"' << std::endl);

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		StringDriveCaller,
		StringDriveCaller(pDrvHdl, sTmp));

	return pDC;
}

struct DofDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
DofDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "dof");

	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* driver legato ad un grado di liberta' nodale */
	if (pDM == 0) {
		silent_cerr("sorry, since the driver is not owned by a DataManager" << std::endl
			<< "no DOF dependent drivers are allowed;" << std::endl
			<< "aborting ..." << std::endl);
		throw DataManager::ErrGeneric();
	}

	ScalarDof SD = ReadScalarDof(pDM, HP, 1);

#ifdef USE_MPI
	if (MPI::Is_initialized() && MBDynComm.Get_size() > 1) {
		silent_cerr("warning: add explicit connection entry for "
			<< psNodeNames[SD.pNode->GetNodeType()]
			<< "(" << SD.pNode->GetLabel() << ") dof drive"
			" at line " << HP.GetLineData() << std::endl);
	}
#endif /* USE_MPI */

	/* Chiamata ricorsiva a leggere il drive supplementare */
	DriveCaller* pTmp = 0;
	pTmp = HP.GetDriveCaller();

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		DofDriveCaller,
		DofDriveCaller(pDrvHdl, pTmp, SD));

	return pDC;
}

struct ElementDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
ElementDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "element");

	/* driver legato ai dati privati di un elemento */
	if (pDM == 0) {
		silent_cerr("sorry, since the driver is not owned by a DataManager" << std::endl
			<< "no element dependent drivers are allowed;" << std::endl
			<< "aborting ..." << std::endl);
		throw DataManager::ErrGeneric();
	}

	const DriveHandler* pDrvHdl = pDM->pGetDrvHdl();
	DriveCaller *pDC = 0;

	unsigned uLabel = HP.GetInt();
	KeyTable Kel(HP, psReadElemsElems);
	int k = HP.IsKeyWord();
	if (k == -1) {
		const char *s = HP.GetString();
		silent_cerr("unknown element type \"" << s
			<< "\" at line " << HP.GetLineData()
			<< std::endl);
		throw ErrGeneric();
	}
	Elem *pElem = (Elem*)pDM->pFindElem(Elem::Type(k), uLabel);
	if (pElem ==  0) {
		silent_cerr("unable to find " << psElemNames[Elem::Type(k)]
			<< "(" << uLabel << ") at line "
			<< HP.GetLineData() << std::endl);
		throw ErrGeneric();
	}
	unsigned int iMaxIndex = pElem->iGetNumPrivData();
	unsigned int iIndex = 0;
	const char *sIndexName = 0;
	if (HP.IsKeyWord("string")) {
		const char *s = HP.GetStringWithDelims();
		iIndex = pElem->iGetPrivDataIdx(s);
		SAFESTRDUP(sIndexName, s);

	} else if (HP.IsKeyWord("index")) {
		iIndex = HP.GetInt();

	} else if (iMaxIndex == 1) {
		iIndex = 1;

	} else {
		silent_cerr("need a private data index for "
			<< psElemNames[pElem->GetElemType()]
			<< "(" << pElem->GetLabel() << ") "
			"at line " << HP.GetLineData() << std::endl);
		throw ErrGeneric();
	}

	if (iIndex < 1 || iIndex > iMaxIndex) {
		silent_cerr("illegal index " << iIndex << " for "
			<< psElemNames[pElem->GetElemType()]
			<< "(" << pElem->GetLabel() << ") "
			"at line " << HP.GetLineData() << std::endl);
		throw ErrGeneric();
	}

#ifdef USE_MPI
	/* FIXME: todo ... */
	if (MPI::Is_initialized() && MBDynComm.Get_size() > 1) {
		silent_cerr("warning: add explicit connection entry for "
			<< psElemNames[pElem->GetElemType()]
			<< "(" << pElem->GetLabel() << ") element drive"
			" at line " << HP.GetLineData() << std::endl);
	}
#endif /* USE_MPI */

	/* Chiamata ricorsiva a leggere il drive supplementare */
	DriveCaller* pTmp = 0;
	pTmp = HP.GetDriveCaller();

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		PrivDriveCaller,
		PrivDriveCaller(pDrvHdl, pTmp, pElem, iIndex, sIndexName));
	if (sIndexName) {
		SAFEDELETEARR(sIndexName);
	}

	return pDC;
}

struct DriveDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
DriveDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	DriveCaller *pD1 = ReadDriveData(pDM, HP, bDeferred);
	DriveCaller *pD2 = ReadDriveData(pDM, HP, bDeferred);

	SAFENEWWITHCONSTRUCTOR(pDC,
		DriveDriveCaller,
		DriveDriveCaller(pDrvHdl, pD1, pD2));

	return pDC;
}

struct ArrayDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
ArrayDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	const DriveHandler* pDrvHdl = 0;
	if (pDM != 0) {
		pDrvHdl = pDM->pGetDrvHdl();
	}

	DriveCaller *pDC = 0;

	/* driver legato ad un grado di liberta' nodale */
	unsigned short int iNumDr = (unsigned short int)HP.GetInt();
	if (iNumDr == 0) {
		silent_cerr("Sorry, at least one driver is required" << std::endl);
		throw ErrGeneric();

	/* creazione di un driver normale mediante chiamata ricorsiva */
	} else if (iNumDr == 1) {
		pDC = HP.GetDriveCaller();

	} else {
		DriveCaller** ppDC = 0;
		SAFENEWARR(ppDC, DriveCaller*, iNumDr);
		for (int i = 0; i < iNumDr; i++) {
			ppDC[i] = HP.GetDriveCaller();
		}

		/* allocazione e creazione array */
		SAFENEWWITHCONSTRUCTOR(pDC,
			DriveArrayCaller,
			DriveArrayCaller(pDrvHdl, iNumDr,
			(const DriveCaller**)ppDC));
	}

	return pDC;
}

struct FileDCR : public DriveCallerRead {
	DriveCaller *
	Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred);
};

DriveCaller *
FileDCR::Read(const DataManager* pDM, MBDynParser& HP, bool bDeferred)
{
	NeedDM(pDM, HP, bDeferred, "file");

	/* driver legato ai driver */
	if (pDM == 0) {
		silent_cerr("sorry, since the driver is not owned by a DataManager" << std::endl
			<< "no driver dependent drivers are allowed;" << std::endl
			<< "aborting ..." << std::endl);
		throw DataManager::ErrGeneric();
	}

	const DriveHandler* pDrvHdl = pDM->pGetDrvHdl();
	DriveCaller *pDC = 0;

	/* drive file */
	/* lettura dei dati specifici */
	unsigned int uL = HP.GetInt();
	FileDrive* pDrv = (FileDrive*)pDM->pFindDrive(Drive::FILEDRIVE, uL);
	if (pDrv == 0) {
		silent_cerr("line " << HP.GetLineData()
			<< ": can't find FileDrive(" << uL << ")" << std::endl);
		throw ErrGeneric();
	}

	integer id = 1;
	if (HP.IsArg()) {
		id = HP.GetInt(id);
	}

	doublereal da = 1.;
	if (HP.IsKeyWord("amplitude")) {
		da = HP.GetReal();
	}

	/* allocazione e creazione */
	SAFENEWWITHCONSTRUCTOR(pDC,
		FileDriveCaller,
		FileDriveCaller(pDrvHdl, pDrv, id, da));

	return pDC;
}

static unsigned done = 0;

void
InitDriveData(void)
{
	if (::done++ > 0) {
		return;
	}

	SetDriveData("time", new TimeDCR);
	SetDriveData("null", new NullDCR);
	SetDriveData("one", new OneDCR);	/* deprecated */
	SetDriveData("unit", new OneDCR);
	SetDriveData("const", new ConstDCR);
	SetDriveData("linear", new LinearDCR);
	SetDriveData("parabolic", new ParabolicDCR);
	SetDriveData("cubic", new CubicDCR);
	SetDriveData("function", new FunctionDCR);
	SetDriveData("step", new StepDCR);
	SetDriveData("double" "step", new DoubleStepDCR);
	SetDriveData("ramp", new RampDCR);
	SetDriveData("double" "ramp", new DoubleRampDCR);
	SetDriveData("sine", new SineDCR);
	SetDriveData("cosine", new CosineDCR);
	SetDriveData("fourier" "series", new FourierSeriesDCR);
	SetDriveData("frequency" "sweep", new FrequencySweepDCR);
	SetDriveData("exponential", new ExponentialDCR);
	SetDriveData("random", new RandomDCR);
	SetDriveData("meter", new MeterDCR);
	SetDriveData("piecewise" "linear", new PiecewiseLinearDCR);
	SetDriveData("file", new FileDCR);
	SetDriveData("string", new StringDCR);
	SetDriveData("dof", new DofDCR);
	SetDriveData("element", new ElementDCR);
	SetDriveData("drive", new DriveDCR);
	SetDriveData("array", new ArrayDCR);

	/* NOTE: add here initialization of new built-in drive callers;
	 * alternative ways to register new custom drive callers are:
	 * - call SetCL*D() from anywhere in the code
	 * - write a module that calls SetDriveData() from inside a function
	 *   called module_init(), and load it using "module load".
	 */
}

void
DestroyDriveData(void)
{
	if (::done == 0) {
		silent_cerr("DestroyDriveData() called once too many" << std::endl);
		throw ErrGeneric();
	}

	if (--::done > 0) {
		return;
	}

	/* free stuff */
	for (DrvFuncMapType::iterator i = DrvFuncMap.begin(); i != DrvFuncMap.end(); i++) {
		delete i->second;
	}
}
