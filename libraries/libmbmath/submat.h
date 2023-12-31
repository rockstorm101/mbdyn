/* $Header$ */
/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2023
 *
 * Pierangelo Masarati	<pierangelo.masarati@polimi.it>
 * Paolo Mantegazza	<paolo.mantegazza@polimi.it>
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

/* Sottomatrici */


#ifndef SUBMAT_H
#define SUBMAT_H


#include <myassert.h>
#include <except.h>


/* include del programma */
/* si assume che solman.h includa piu' o meno direttamnte f2c.h,
 * che contiene le dichiarazioni dei tipi derivati dal fortran. */

#include <solman.h>
#include <fullmh.h>
#include <matvec3.h>
#include <matvec3n.h>
#include <sp_gradient.h>
#include <vector>

/* SubMatrixHandler - begin */

/*
 Classe virtuale delle sottomatrici.
 Le SubMatrixHandler sono matrici dotate di vettori di incidenza
 per righe e colonne.
 Sono usate per scrivere le sottomatrici di ogni elemento, che poi si
 sommano con apposite routines alla matrice jacobiana.
 */

class SubMatrixHandler : public MatrixHandler {
public:
     /* Errori */

     /*
      * Errore di ridimensionamento illegale.
      */
     class ErrResize : public MBDynErrBase {
     public:
          ErrResize(MBDYN_EXCEPT_ARGS_DECL) : MBDynErrBase(MBDYN_EXCEPT_ARGS_PASSTHRU) {};
     };

     /* Costruttori */

     /*
      * Distruttore virtuale.
      * Necessario per la corretta distruzione degli oggetti derivati
      */
     virtual ~SubMatrixHandler(void);

     using MatrixHandler::operator=;

     /* Metodi di servizio */

#ifdef DEBUG
     /*
      * Routine di verifica della validita' dell'oggetto.
      * Usata per il debug.
      */
     virtual void IsValid(void) const = 0;
#endif /* DEBUG */


     /* Inizializzazione */


     /*
      * Ridimensiona la matrice.
      * Nota: nell'implementazione corrente le dimensioni possono essere solamente
      * inferiori alle dimensioni massime con cui e' stata dimensionata.
      */
     virtual void Resize(integer, integer) override = 0;

     /*
      * Ridimensiona ed inizializza.
      * Combina le due funzioni precedenti in una chiamata.
      */
     virtual void ResizeReset(integer, integer) override = 0;

     /* Gestione dei vettori di incidenza */

     /*
      * Scrive l'indice di riga.
      */
     virtual void PutRowIndex(integer, integer) = 0;

     /*
      * Scrive l'indice di colonna.
      */
     virtual void PutColIndex(integer, integer) = 0;

     /*
      * Ottiene l'indice di riga.
      */
     virtual integer iGetRowIndex(integer) const = 0;

     /*
      * Ottiene l'indice di colonna.
      */
     virtual integer iGetColIndex(integer) const = 0;

     /* Funzioni di interazione con le matrici */

     /*
      * Si somma ad una matrice.
      * Nota: le dimensioni devono essere compatibili.
      */
     virtual MatrixHandler& AddTo(MatrixHandler& MH) const = 0;

     /*
      * Si somma ad una matrice, trasposta.
      * Nota: le dimensioni devono essere compatibili.
      */
     virtual MatrixHandler& AddToT(MatrixHandler& MH) const = 0;

     /*
      * Needed for Newton Krylov solver
      * A += (*this) * Y;
      */
     virtual VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const = 0;

     /*
      * Si sottrae da una matrice.
      * Nota: le dimensioni devono essere compatibili.
      */
     virtual MatrixHandler& SubFrom(MatrixHandler& MH) const = 0;

     /*
      * Si sottrae da una matrice, trasposta.
      * Nota: le dimensioni devono essere compatibili.
      */
     virtual MatrixHandler& SubFromT(MatrixHandler& MH) const = 0;

     virtual SubMatrixHandler* Copy() const override;

     virtual void EnumerateNz(const std::function<EnumerateNzCallback>& func) const override;
};

/* SubMatrixHandler - end */


/* FullSubMatrixHandler */

/*
 * Sottomatrice piena. E' costituita da un vettore di interi, piRow, che
 * contiene i due vettori di incidenza, e da un vettore di reali, pdMat,
 * che contiene la matrice.
 * Il vettore di incidenza delle righe e' piRow, mentre quello delle colonne
 * e' piCol = piRow+iNumRows. Le dimensioni possono essere cambiate con
 * Resize(), con i vincoli che la somma di righe e colonne non deve eccedere
 * la lunghezza del vettore di interi, ed il loro prodotto non deve eccedere
 * la lunghezza del vettore di reali.
 */

class FullSubMatrixHandler :
     public SubMatrixHandler, public FullMatrixHandler {
     friend std::ostream&
     operator << (std::ostream& out, const FullSubMatrixHandler& m);

     friend class NaiveMatrixHandler;
     friend class NaivePermMatrixHandler;

protected:
     /* Dimensione totale del vettore di incidenza */
     integer iVecSize;
     /* Puntatore al vettore di incidenza delle righe.
      * Nota: coincide con il puntatore al vettore di incidenza */
     integer* piRow;
     integer* piRowm1;
     /* Puntatore al vettore di incidenza delle colonne */
     integer* piColm1;

private:
     FullSubMatrixHandler(const FullSubMatrixHandler&) = delete;

public:

     /* Costruttori */

     /*
      * Riceve i vettori per gli indici ed i coefficienti, con relative
      * dimensioni
      * @param iIntSize    dimensione del vettore degli indici
      * @param iDoubleSize dimensione del vettore dei coefficienti
      * @param piTmpVec    puntatore al vettore degli indici
      * @param pdTmpMat    puntatore al vettore dei coefficienti
      */
     FullSubMatrixHandler(integer iIntSize, integer* piTmpVec,
                          integer iDoubleSize, doublereal* pdTmpMat,
                          integer iMaxCols, doublereal **ppdCols);

     FullSubMatrixHandler(integer iNR, integer iNC = 0);

     /*
      * Distruttore banale.
      * Nota: l'elemento non possiede memoria e quindi non ne dealloca.
      */
     virtual ~FullSubMatrixHandler(void);

     using MatrixHandler::operator=;

     /* Metodi di servizio */

#ifdef DEBUG
     /*
      * Routine di verifica della validita' dell'oggetto.
      * Usata per il debug.
      */
     virtual void IsValid(void) const;
#endif /* DEBUG */

     /*
      * Numero di righe della sottomatrice
      */
     integer iGetNumRows(void) const override {
          return iNumRows;
     };

     /*
      * Numero di colonne della sottomatrice
      */
     integer iGetNumCols(void) const override {
          return iNumCols;
     };

     /* Metodi di inizializzazione */

     /*
      * Inizializza la porzione utilizzata con il valore desiderato
      */
     void Reset(void) override;

     /*
      * Modifica le dimensioni correnti
      */
     void Resize(integer iNewRow, integer iNewCol) override;

     /* Ridimensiona ed inizializza. */
     virtual void ResizeReset(integer, integer) override;

     /*
      * Collega la matrice Full alla memoria che gli viene passata
      * in ingresso
      */
     void Attach(int iRows, int iCols, integer* piTmpIndx);

     /* Gestione dei coefficienti */

     /* Inserisce un coefficiente */

     /*
      * NOTE:
      *
      * these functions need be redefined here because their
      * inheritance is ambiguous, and we want to use the
      * (very efficient) FullMatrixHandler version instead
      * of the (less efficient) MatrixHandler version, which
      * is a wrapper for the () operator.
      *
      * However, if compiled with appropriate optimization,
      * they are absolutely equivalent to directly accessing
      * the matrix, as the #if 0'ed code does.
      */
     inline void
     PutCoef(integer iRow, integer iCol, const doublereal& dCoef) override;

     /* Incrementa un coefficiente - se non esiste lo crea */
     inline void
     IncCoef(integer iRow, integer iCol, const doublereal& dCoef) override;

     /* Decrementa un coefficiente - se non esiste lo crea */
     inline void
     DecCoef(integer iRow, integer iCol, const doublereal& dCoef) override;

     /* Restituisce un coefficiente - zero se non e' definito */
     inline const doublereal&
     dGetCoef(integer iRow, integer iCol) const override;

// FIXME: disambiguate operator()
     inline const doublereal&
     operator () (integer iRow, integer iCol) const override;

     inline doublereal&
     operator () (integer iRow, integer iCol) override;
// end of FIXME: disambiguate operator()

     /* Gestione degli indici */

     /*
      * Scrive un indice di riga
      */
     inline void
     PutRowIndex(integer iSubRow, integer iRow) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubRow > 0) && (iSubRow <= iNumRows));

          piRowm1[iSubRow] = iRow;
     };

     /*
      * Scrive un indice di colonna
      */
     inline void
     PutColIndex(integer iSubCol, integer iCol) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubCol > 0) && (iSubCol <= iNumCols));

          piColm1[iSubCol] = iCol;
     };

     /*
      * Legge un indice di riga
      */
     inline integer
     iGetRowIndex(integer iSubRow) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubRow > 0) && (iSubRow <= iNumRows));

          return piRowm1[iSubRow];
     };

     /*
      * Legge un indice di colonna
      */
     inline integer
     iGetColIndex(integer iSubCol) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubCol > 0) && (iSubCol <= iNumCols));

          return piColm1[iSubCol];
     };

     /*
      * Somma un vettore di tipo Vec3 in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     Add(integer iRow, integer iCol, const Vec3& v);

     /*
      * Sottrae un vettore di tipo Vec3 in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     Sub(integer iRow, integer iCol, const Vec3& v);

     /*
      * Scrive un vettore di tipo Vec3 in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     Put(integer iRow, integer iCol, const Vec3& v);

     /*
      * Somma un vettore di tipo Vec3 trasposto in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3 trasposto (riga).
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     AddT(integer iRow, integer iCol, const Vec3& v);

     /*
      * Sottrae un vettore di tipo Vec3 trasposto in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3 trasposto (riga).
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     SubT(integer iRow, integer iCol, const Vec3& v);

     /*
      * Scrive un vettore di tipo Vec3 trasposto in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3 trasposto (riga).
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     PutT(integer iRow, integer iCol, const Vec3& v);

#if 0 /* FIXME: replace original? */
     /*
      * Somma un vettore di tipo Vec3 in una data posizione in diagonale.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     AddDiag(integer iRow, integer iCol, const Vec3& v);

     /*
      * Sottrae un vettore di tipo Vec3 in una data posizione in diagonale.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     SubDiag(integer iRow, integer iCol, const Vec3& v);

     /*
      * Scrive un vettore di tipo Vec3 in una data posizione in diagonale.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     PutDiag(integer iRow, integer iCol, const Vec3& v);

     /*
      * Somma un vettore di tipo Vec3 in una data posizione [ v x ].
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     AddCross(integer iRow, integer iCol, const Vec3& v);

     /*
      * Sottrae un vettore di tipo Vec3 in una data posizione [ v x ].
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     SubCross(integer iRow, integer iCol, const Vec3& v);

     /*
      * Scrive un vettore di tipo Vec3 in una data posizione [ v x ].
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per il vettore 3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param v    Vec3 da sommare
      */
     void
     PutCross(integer iRow, integer iCol, const Vec3& v);
#endif

     /*
      * Somma una matrice di tipo Mat3x3 in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3x3 da sommare
      */
     void
     Add(integer iRow, integer iCol, const Mat3x3& m);
     void
     AddT(integer iRow, integer iCol, const Mat3x3& m);

     /*
      * Sottrae una matrice di tipo Mat3x3 da una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3x3 da sottrarre
      */
     void
     Sub(integer iRow, integer iCol, const Mat3x3& m);
     void
     SubT(integer iRow, integer iCol, const Mat3x3& m);

     /*
      * Scrive una matrice di tipo Mat3x3 in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3x3 da scrivere
      */
     void
     Put(integer iRow, integer iCol, const Mat3x3& m);
     void
     PutT(integer iRow, integer iCol, const Mat3x3& m);

     /*
      * Somma una matrice di tipo Mat3xN in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3xN
      */
     void
     Add(integer iRow, integer iCol, const Mat3xN& m);
     void
     AddT(integer iRow, integer iCol, const Mat3xN& m);

     /*
      * Sottrae una matrice di tipo Mat3xN da una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3xN
      */
     void
     Sub(integer iRow, integer iCol, const Mat3xN& m);
     void
     SubT(integer iRow, integer iCol, const Mat3xN& m);

     /*
      * Scrive una matrice di tipo Mat3xN in una data posizione.
      * Nota: si assume che nella sottomatrice vi sia spazio
      * per la matrice 3x3.
      * Nota: gli indici sono a base 1, in stile FORTRAN.
      * @param iRow indice di riga della sottomatrice da cui iniziare
      * @param iCol indice di colonna della sottomatrice da cui iniziare
      * @param m    Mat3xN
      */
     void
     Put(integer iRow, integer iCol, const Mat3xN& m);
     void
     PutT(integer iRow, integer iCol, const Mat3xN& m);

     /* come sopra, ma per matrici Nx3 **/
     void Add(integer iRow, integer iCol, const MatNx3& m);
     void Sub(integer iRow, integer iCol, const MatNx3& m);
     void Put(integer iRow, integer iCol, const MatNx3& m);

     void PutDiag(integer iFirstRow, integer iFirstCol, const Vec3& v);
     void PutDiag(integer iFirstRow, integer iFirstCol, const doublereal& v);
     void PutCross(integer iFirstRow, integer iFirstCol, const Vec3& v);

     void Add(integer iRow,  integer iCol,
              const FullMatrixHandler & source);
     void Sub(integer iRow,  integer iCol,
              const FullMatrixHandler & source);
     void Put(integer iRow,  integer iCol,
              const FullMatrixHandler & source);
     void Add(integer iRow,  integer iCol,
              const FullMatrixHandler & source, const doublereal dCoef);
     void Sub(integer iRow,  integer iCol,
              const FullMatrixHandler & source, const doublereal dCoef);
     void Put(integer iRow,  integer iCol,
              const FullMatrixHandler & source, const doublereal dCoef);
     void AddT(integer iRow,  integer iCol,
               const FullMatrixHandler & source);
     void SubT(integer iRow,  integer iCol,
               const FullMatrixHandler & source);
     void PutT(integer iRow,  integer iCol,
               const FullMatrixHandler & source);
     void AddT(integer iRow,  integer iCol,
               const FullMatrixHandler & source, const doublereal dCoef);
     void SubT(integer iRow,  integer iCol,
               const FullMatrixHandler & source, const doublereal dCoef);
     void PutT(integer iRow,  integer iCol,
               const FullMatrixHandler & source, const doublereal dCoef);

     /* Interazione con le matrici */

     /*
      * Somma la matrice ad un matrix handler usando i metodi generici
      */
     MatrixHandler& AddTo(MatrixHandler& MH) const override;

     /*
      * Somma la matrice, trasposta, ad un matrix handler usando i metodi generici
      */
     MatrixHandler& AddToT(MatrixHandler& MH) const override;

     /*
      * Somma la matrice ad un FullMatrixHandler
      */
     MatrixHandler& AddTo(FullMatrixHandler& MH) const;

     /*
      * Somma la matrice, trasposta, ad un FullMatrixHandler
      */
     MatrixHandler& AddToT(FullMatrixHandler& MH) const;

     VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const override;
     /*
      * Sottrae la matrice da un matrix handler usando i metodi generici
      */
     MatrixHandler& SubFrom(MatrixHandler& MH) const override;

     /*
      * Sottrae la matrice, trasposta, da un matrix handler usando i metodi generici
      */
     MatrixHandler& SubFromT(MatrixHandler& MH) const override;

     /*
      * Sottrae la matrice da un FullMatrixHandler
      */
     MatrixHandler& SubFrom(FullMatrixHandler& MH) const;

     /*
      * Sottrae la matrice, trasposta, da un FullMatrixHandler
      */
     MatrixHandler& SubFromT(FullMatrixHandler& MH) const;
};

/* Inserisce un coefficiente */
inline void
FullSubMatrixHandler::PutCoef(integer iRow, integer iCol,
                              const doublereal& dCoef)
{
#if 0
     ppdColsm1[iCol][iRow] = dCoef;
#endif
     FullMatrixHandler::PutCoef(iRow, iCol, dCoef);
}

/* Incrementa un coefficiente - se non esiste lo crea */
inline void
FullSubMatrixHandler::IncCoef(integer iRow, integer iCol,
                              const doublereal& dCoef)
{
#if 0
     ppdColsm1[iCol][iRow] += dCoef;
#endif
     FullMatrixHandler::IncCoef(iRow, iCol, dCoef);
}

/* Decrementa un coefficiente - se non esiste lo crea */
inline void
FullSubMatrixHandler::DecCoef(integer iRow, integer iCol,
                              const doublereal& dCoef)
{
#if 0
     ppdColsm1[iCol][iRow] -= dCoef;
#endif
     FullMatrixHandler::DecCoef(iRow, iCol, dCoef);
}

/* Restituisce un coefficiente - zero se non e' definito */
inline const doublereal&
FullSubMatrixHandler::dGetCoef(integer iRow, integer iCol) const
{
#if 0
     return ppdColsm1[iCol][iRow];
#endif
     return FullMatrixHandler::dGetCoef(iRow, iCol);
}

// FIXME: disambiguate operator()
inline const doublereal&
FullSubMatrixHandler::operator () (integer iRow, integer iCol) const
{
     return FullMatrixHandler::operator()(iRow, iCol);
}

inline doublereal&
FullSubMatrixHandler::operator () (integer iRow, integer iCol)
{
     return FullMatrixHandler::operator()(iRow, iCol);
}
// end of FIXME: disambiguate operator()

/* FullSubMatrixHandler - end */

class FullSubMatrixHandlerAd: public FullSubMatrixHandler {
public:
     FullSubMatrixHandlerAd(integer iIntSize, integer* piTmpVec,
                            integer iDoubleSize, doublereal* pdTmpMat,
                            integer iMaxCols, doublereal **ppdCols);
     FullSubMatrixHandlerAd(integer iNR, integer iNC);
     using MatrixHandler::operator=;

     virtual MatrixHandler&
     AddTo(MatrixHandler& MH) const override final;
};

/* SparseSubMatrixHandler */

/*
 * Gestore di sottomatrici sparse, piuttosto rozzo, va usato con cautela.
 * E' formato da due vettori di interi e da uno di reali, tutti della stessa
 * lunghezza.
 * Ad ogni indice del sotto-vettore corrisponde un coefficiente con i suoi
 * due indici nella matrice completa.
 * La scrittura non e' strutturata, per cui l'utilizzatore deve badare a non
 * lasciare vuoti e a non ripetere i coefficienti (in realta' non succede
 * nulla se la si usa per assemblare, solo uno stesso coefficiente puo' essere
 * dato da piu' contributi).
 */

class SparseSubMatrixHandler : public SubMatrixHandler {
     friend class SparseMatrixHandler;
     friend class FullMatrixHandler;
     friend class NaiveMatrixHandler;
     friend class NaivePermMatrixHandler;

public:
     /* Errori */

     class ErrResize : MBDynErrBase {
     public:
          ErrResize(MBDYN_EXCEPT_ARGS_DECL) : MBDynErrBase(MBDYN_EXCEPT_ARGS_PASSTHRU) {};
     };

protected:
     bool bOwnsMemory;
     /* Dimensioni dell'array degli indici */
     integer iIntSize;
     /* Dimensioni dell'array dei coefficienti */
     integer iDoubleSize;
     /* Numero di entries definite */
     integer iNumItems;
     /* Puntatore all'array degli indici di riga.
      * Coincide con il puntatore all'array degli inidici, che e' unico */
     integer* piRow, *piRowm1;
     /* Puntatore all'array degli indici di colonna */
     integer* piColm1;
     /* Puntatore all'array dei coefficienti */
     doublereal* pdMat, *pdMatm1;

private:
     SparseSubMatrixHandler(const SparseSubMatrixHandler&) = delete;

public:
     /* Costruttori */


     /* Costruttore.
      * @param iTmpInt    dimensione dell'array degli indici
      * @param iTmpDouble dimensione dell'array dei coefficienti
      * @param piTmpIndex puntatore all'array degli indici
      * @param pdTmpMat   puntatore all'array dei coefficienti
      */
     SparseSubMatrixHandler(integer iTmpInt, integer* piTmpIndex,
                            integer iTmpDouble, doublereal* pdTmpMat);

     explicit SparseSubMatrixHandler(integer iTmpInt);

     /* Distruttore banale.
      * Nota: dato che la classe non possiede la memoria,
      * non ne deve deallocare
      */
     virtual ~SparseSubMatrixHandler(void);

     using MatrixHandler::operator=;

     /* Metodi di servizio */

#ifdef DEBUG
     /*
      * Routine di verifica della validita' dell'oggetto.
      * Usata per il debug.
      */
     virtual void IsValid(void) const;
#endif /* DEBUG */

     /*
      * Numero di righe della sottomatrice.
      * Nota: rappresenta il numero totale di entries della sottomatrice.
      */
     integer iGetNumRows(void) const override {
          return iNumItems;
     };

     /*
      * Numero di colonne della sottomatrice.
      * Nota: e' sempre 1, ovvero la matrice e' interpretata
      * come un vettore.
      */
     integer iGetNumCols(void) const override {
          return 1;
     };

     /* Metodi di inizializzazione */

     /*
      * Ridimensiona la matrice.
      * Nota: solo il primo argomento viene considerato,
      * e rappresenta il numero totale di entries.
      * Questo metodo deve essere chiamato prima di qualsiasi
      * operazione sulla matrice.
      */
     void Resize(integer iNewRow, integer iNewCol) override;

     /*
      * Ridimensiona ed inizializza.
      * Unione dei due metodi precedenti
      */
     void ResizeReset(integer iNewRow, integer iNewCol) override;

     /* Azzera */
     void Reset(void) override;

     /*
      * Collega la matrice sparsa alla memoria che gli viene passata
      * in ingresso
      */
     void Attach(int iNumEntr, doublereal* pdTmpMat, integer* piTmpIndx);

     /* Gestione dei coefficienti */

     /*
      * Scrive un coefficiente in base ai sottoindici.
      */
     inline void
     PutCoef(integer iSubIt, integer iDmy, const doublereal& dCoef) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          pdMatm1[iSubIt] = dCoef;
     };

     /*
      * Incrementa un coefficiente in base ai sottoindici.
      */
     inline void
     IncCoef(integer iSubIt, integer iDmy, const doublereal& dCoef) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));
          pdMatm1[iSubIt] += dCoef;
     };

     /*
      * Decrementa un coefficiente in base ai sottoindici.
      */
     inline void
     DecCoef(integer iSubIt, integer iDmy, const doublereal& dCoef) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));
          pdMatm1[iSubIt] -= dCoef;
     };

     /*
      * Ottiene un coefficiente in base ai sottoindici.
      */
     inline const doublereal&
     dGetCoef(integer iSubIt, integer iDmy) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          return pdMatm1[iSubIt];
     };

     /*
      * Ottiene un coefficiente in base ai sottoindici.
      */
     inline const doublereal&
     operator () (integer iSubIt, integer iDmy) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          return pdMatm1[iSubIt];
     };

     /*
      * Ottiene un coefficiente in base ai sottoindici.
      */
     inline doublereal&
     operator () (integer iSubIt, integer iDmy) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          return pdMatm1[iSubIt];
     };

     /*
      * Scrive un indice di riga
      */
     inline void
     PutRowIndex(integer iSubIt, integer iRow) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));
          piRowm1[iSubIt] = iRow;
     };

     /*
      * Scrive un indice di colonna
      */
     inline void
     PutColIndex(integer iSubIt, integer iCol) override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));
          piColm1[iSubIt] = iCol;
     };

     /*
      * Ottiene un indice di riga
      */
     inline integer
     iGetRowIndex(integer iSubIt) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          return piRowm1[iSubIt];
     };

     /*
      * Ottiene un indice di colonna
      */
     inline integer
     iGetColIndex(integer iSubIt) const override {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));

          return piColm1[iSubIt];
     };

     /*
      * Scrive un'entry completa.
      * @param iSubIt   sottoindice (numero della entry)
      * @param iRow     indice di riga
      * @param iCol     indice di colonna
      * @param dCoef    coefficiente
      */
     inline void
     PutItem(integer iSubIt, integer iRow, integer iCol,
             const doublereal& dCoef) {
#ifdef DEBUG
          IsValid();
#endif /* DEBUG */

          ASSERT((iSubIt > 0) && (iSubIt <= iNumItems));
          ASSERT(iRow > 0);
          ASSERT(iCol > 0);

          pdMatm1[iSubIt] = dCoef;
          piRowm1[iSubIt] = iRow;
          piColm1[iSubIt] = iCol;
     };

     /*
      * Scrive una matrice diagonale nella posizione assegnata.
      * @param iSubIt     sottoindice iniziale (numero della prima entry)
      * @param iFirstRow  indice della prima riga della matrice completa
      * @param iFirstCol  indice della prima colonna della matrice completa
      * @param v          vettore diagonale della matrice
      */
     void
     PutDiag(integer iSubIt, integer iFirstRow, integer iFirstCol,
             const Vec3& v);

     /*
      * Scrive una matrice diagonale nella posizione assegnata.
      * @param iSubIt     sottoindice iniziale (numero della prima entry)
      * @param iFirstRow  indice della prima riga della matrice completa
      * @param iFirstCol  indice della prima colonna della matrice completa
      * @param d          coefficiente della diagonale della matrice
      */
     void
     PutDiag(integer iSubIt, integer iFirstRow, integer iFirstCol,
             const doublereal& d);

     /*
      * Scrive una matrice prodotto vettore nella posizione assegnata.
      * @param iSubIt     sottoindice iniziale (numero della prima entry)
      * @param iFirstRow  indice della prima riga della matrice completa
      * @param iFirstCol  indice della prima colonna della matrice completa
      * @param v          vettore da cui viene calcolata la matrice prodotto
      *                   vettore
      */
     void
     PutCross(integer iSubIt, integer iFirstRow, integer iFirstCol,
              const Vec3& v);

     /*
      * Scrive una Mat3x3 nella posizione assegnata.
      * @param iSubIt     sottoindice iniziale (numero della prima entry)
      * @param iFirstRow  indice della prima riga della matrice completa
      * @param iFirstCol  indice della prima colonna della matrice completa
      * @param m          matrice da inserire
      */
     void
     PutMat3x3(integer iSubIt, integer iFirstRow, integer iFirstCol,
               const Mat3x3& m);

     /* Interazione con le matrici */

     /*
      * Somma la matrice ad un matrix handler usando i metodi generici
      */
     MatrixHandler& AddTo(MatrixHandler& MH) const override;

     /*
      * Somma la matrice, trasposta, ad un matrix handler usando i metodi generici
      */
     MatrixHandler& AddToT(MatrixHandler& MH) const override;

     /*
      * Somma la matrice ad un FullMatrixHandler
      */
     MatrixHandler& AddTo(FullMatrixHandler& MH) const;

     /*
      * Somma la matrice, trasposta, ad un FullMatrixHandler
      */
     MatrixHandler& AddToT(FullMatrixHandler& MH) const;

     VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const override;

     /*
      * Sottrae la matrice da un matrix handler usando i metodi generici
      */
     MatrixHandler& SubFrom(MatrixHandler& MH) const override;

     /*
      * Sottrae la matrice, trasposta, da un matrix handler usando i metodi generici
      */
     MatrixHandler& SubFromT(MatrixHandler& MH) const override;

     /*
      * Sottrae la matrice da un FullMatrixHandler
      */
     MatrixHandler& SubFrom(FullMatrixHandler& MH) const;

     /*
      * Sottrae la matrice, trasposta, da un FullMatrixHandler
      */
     MatrixHandler& SubFromT(FullMatrixHandler& MH) const;
};

class SparseSubMatrixHandlerAd: public SparseSubMatrixHandler {
public:
     SparseSubMatrixHandlerAd(integer iTmpInt, integer* piTmpIndex,
                              integer iTmpDouble, doublereal* pdTmpMat);

     explicit SparseSubMatrixHandlerAd(integer iNumItems);

     using MatrixHandler::operator=;

     virtual MatrixHandler&
     AddTo(MatrixHandler& MH) const override final;
};

class SpGradientSubMatrixHandler: public SubMatrixHandler {
public:
     explicit SpGradientSubMatrixHandler(integer iNumItemsMax);
     virtual ~SpGradientSubMatrixHandler();
     using MatrixHandler::operator=;
     virtual void Resize(integer, integer) override;
     virtual void Reset() override;
     virtual void ResizeReset(integer, integer) override;
     virtual doublereal& operator()(integer iRow, integer iCol) override;
     virtual const doublereal& operator()(integer iRow, integer iCol) const override;
     virtual integer iGetNumRows() const override;
     virtual integer iGetNumCols() const override;
     virtual void PutRowIndex(integer iSubIt, integer iRow) override;
     virtual void PutColIndex(integer iSubIt, integer iRow) override;
     virtual integer iGetRowIndex(integer) const override;
     virtual integer iGetColIndex(integer) const override;
     virtual MatrixHandler& AddTo(MatrixHandler& HM) const override;
     virtual MatrixHandler& SubFrom(MatrixHandler& HM) const override;
     virtual MatrixHandler& AddToT(MatrixHandler& HM) const override;
     VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const override;
     virtual MatrixHandler& SubFromT(MatrixHandler& HM) const override;

#ifdef DEBUG
     virtual void IsValid(void) const;
#endif

     bool AddItem(integer iEquationIdx, const sp_grad::SpGradient& oResidual) override;

private:
     struct ResidualItem {
          ResidualItem(integer iEquationIdx, const sp_grad::SpGradient& oResidual)
               :iEquationIdx(iEquationIdx),
#ifdef USE_MULTITHREAD
                bInserted(false),
#endif
                oResidual(oResidual) {
          }

          integer iEquationIdx;
#ifdef USE_MULTITHREAD
          mutable bool bInserted;
#endif
          sp_grad::SpGradient oResidual;
     };

     std::vector<ResidualItem> oVec;
};

/* SparseSubMatrixHandler - end */


/* VariableSubMatrixHandler - begin */

/*
 * Matrice che puo' diventare via via una sottomatrice piena o una sottomatrice
 * sparsa, condividendo la memoria.
 * Viene passata agli elementi che, a seconda della loro convenienza,
 * la configurano nel modo piu' opportuno.
 * Quindi, con metodi opportuni, viene sommata alla matrice completa.
 */

class VariableSubMatrixHandler
{
public:
     enum MatrixHandlerStatusType {
          NULLMATRIX,
          FULL,
          SPARSE,
          SPARSE_GRADIENT
     };

protected:
     VariableSubMatrixHandler(const VariableSubMatrixHandler&) = delete;

     VariableSubMatrixHandler();

public:
     virtual ~VariableSubMatrixHandler();

     void operator=(const VariableSubMatrixHandler&) = delete;

     MatrixHandlerStatusType GetStatus() const {
          return eStatus;
     }

     bool bIsNullMatrix() const {
          return (eStatus == NULLMATRIX);
     }

     bool bIsFull() const {
          return (eStatus == FULL);
     }

     bool bIsSparse() const {
          return (eStatus == SPARSE);
     }

     bool bIsSparseGradient() const {
          return (eStatus == SPARSE_GRADIENT);
     }

     void SetNullMatrix() {
          eStatus = NULLMATRIX;
     }

     virtual FullSubMatrixHandler& SetFull() = 0;

     virtual SparseSubMatrixHandler& SetSparse() = 0;

     virtual SpGradientSubMatrixHandler& SetSparseGradient() = 0;

     virtual const FullSubMatrixHandler& GetFull() const = 0;

     virtual const SparseSubMatrixHandler& GetSparse() const = 0;

     virtual const SpGradientSubMatrixHandler& GetSparseGradient() const = 0;

     virtual MatrixHandler& AddTo(MatrixHandler& MH) const = 0;

     virtual MatrixHandler& AddToT(MatrixHandler& MH) const = 0;

     virtual VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const = 0;

     virtual MatrixHandler& SubFrom(MatrixHandler& MH) const = 0;

     virtual MatrixHandler& SubFromT(MatrixHandler& MH) const = 0;

protected:
     MatrixHandlerStatusType eStatus;
};

class VariableSubMatrixHandlerNonAd: public VariableSubMatrixHandler
{
public:
     /* Costruttori */

     /*
      * Costruttore: riceve gli spazi di lavoro con le loro dimensioni
      * ed inizializza le matrici piena e sparsa.
      * @param iIntSize    dimensioni dell'array degli indici
      * @param iDoubleSize dimensioni dell'array dei coefficienti
      * @param piInt       array degli indici
      * @param pdDouble    array dei coefficienti
      */
     VariableSubMatrixHandlerNonAd(integer iIntSize, integer* piInt,
                                   integer iDoubleSize, doublereal* pdDouble,
                                   integer iMaxRows, integer iMaxCols);

     VariableSubMatrixHandlerNonAd(const VariableSubMatrixHandlerNonAd&) = delete;

     VariableSubMatrixHandlerNonAd(integer iMaxRows, integer iMaxCols, integer iNumItems = -1);

     virtual ~VariableSubMatrixHandlerNonAd();

     void operator=(const VariableSubMatrixHandlerNonAd&) = delete;

     /*
      * Setta la matrice come piena.
      * Ritorna un riferimento a matrice piena, che puo' essere usato
      * per le normali operazioni di scrittura delle matrici piene.
      */
     virtual FullSubMatrixHandler& SetFull() override final;

     /*
      * Setta la matrice come sparsa.
      * Ritorna un riferimento a matrice sparsa, che puo' essere usato
      * per le normali operazioni di scrittura delle matrici sparse.
      */
     virtual SparseSubMatrixHandler& SetSparse() override final;

     virtual SpGradientSubMatrixHandler& SetSparseGradient() override final;

     virtual const FullSubMatrixHandler& GetFull() const override final;

     virtual const SparseSubMatrixHandler& GetSparse() const override final;

     virtual const SpGradientSubMatrixHandler& GetSparseGradient() const override final;

     /* Interazione con le matrici */

     /*
      * Si somma ad una matrice completa con metodi generici.
      */
     virtual MatrixHandler& AddTo(MatrixHandler& MH) const override final;

     /*
      * Si somma, trasposta, ad una matrice completa con metodi generici.
      */
     virtual MatrixHandler& AddToT(MatrixHandler& MH) const override final;

     virtual VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const override final;

     /*
      * Si sottrae da una matrice completa con metodi generici.
      */
     virtual MatrixHandler& SubFrom(MatrixHandler& MH) const override final;

     /*
      * Si sottrae, trasposta, da una matrice completa con metodi generici.
      */
     virtual MatrixHandler& SubFromT(MatrixHandler& MH) const override final;

protected:
     FullSubMatrixHandler oFullMH;
     SparseSubMatrixHandler oSparseMH;
};

/* VariableSubMatrixHandler - end */

class VariableSubMatrixHandlerAd: public VariableSubMatrixHandler {
public:
     VariableSubMatrixHandlerAd(const VariableSubMatrixHandlerAd&) = delete;

     VariableSubMatrixHandlerAd(integer iIntSize, integer* piInt,
                                integer iDoubleSize, doublereal* pdDouble,
                                integer iMaxRows, integer iMaxCols);

     VariableSubMatrixHandlerAd(integer iMaxRows, integer iMaxCols, integer iNumItems = -1);

     virtual ~VariableSubMatrixHandlerAd();

     void operator=(const VariableSubMatrixHandlerAd&) = delete;

     virtual FullSubMatrixHandlerAd& SetFull() override final;

     virtual SparseSubMatrixHandlerAd& SetSparse() override final;

     virtual SpGradientSubMatrixHandler& SetSparseGradient() override final;

     virtual const FullSubMatrixHandlerAd& GetFull() const override final;

     virtual const SparseSubMatrixHandlerAd& GetSparse() const override final;

     virtual const SpGradientSubMatrixHandler& GetSparseGradient() const override final;

     virtual MatrixHandler& AddTo(MatrixHandler& MH) const override final;

     virtual MatrixHandler& AddToT(MatrixHandler& MH) const override final;

     virtual VectorHandler& MultAddTo(VectorHandler& A, const VectorHandler& Y) const override final;

     virtual MatrixHandler& SubFrom(MatrixHandler& MH) const override final;

     virtual MatrixHandler& SubFromT(MatrixHandler& MH) const override final;

private:
     FullSubMatrixHandlerAd oFullMH;
     SparseSubMatrixHandlerAd oSparseMH;
     SpGradientSubMatrixHandler oGradMH;
};

/* SubVectorHandler - begin */

/*
 * Classe virtuale dei sottovettori.
 */

class SubVectorHandler : public VectorHandler {
public:
     /* Costruttori */

     /*
      * Distruttore virtuale.
      */
     virtual ~SubVectorHandler(void) {
          NO_OP;
     };

     using VectorHandler::operator=;

     /* Metodi di servizio */

#ifdef DEBUG
     /*
      * Routine di verifica della validita' dell'oggetto.
      * Usata per il debug.
      */
     virtual void IsValid(void) const = 0;
#endif /* DEBUG */

     /* Operazioni su indici e coefficienti */

     /*
      * Scrive un indice di riga
      */
     virtual void PutRowIndex(integer iSubRow, integer iRow) = 0;

     /*
      * Ottiene un indice di riga
      */
     virtual integer iGetRowIndex(integer iSubRow) const = 0;

     /*
      * Scrive una entry completa.
      * @param iSubRow numero della entry (indice del sotto-vettore)
      * @param iRow    indice della entry
      * @param dCoef   coefficiente della entry
      */
     virtual inline void PutItem(integer iSubRow, integer iRow,
                                 const doublereal& dCoef) {
          PutRowIndex(iSubRow, iRow);
          PutCoef(iSubRow, dCoef);
     };

     /* Interazione con i vettori */

     /*
      * Si somma ad un vettore con metodi generici
      */
     virtual VectorHandler& AddTo(VectorHandler& VH) const = 0;

     /*
      * Si somma in valore assoluto ad un vettore con metodi generici
      */
     virtual VectorHandler& AddAbsValuesTo(VectorHandler& VH) const = 0;
};

/*
 * Sottovettore standard, formato da un vettore di reali con associato
 * un vettore di interi che contiene gli indici di ogni coefficiente.
 * Per il vettore di reali viene usato un VectorHandler, da cui la classe e'
 * derivata.
 */
class MySubVectorHandler : public SubVectorHandler, public MyVectorHandler {
     friend std::ostream&
     operator << (std::ostream& out, const SubVectorHandler& v);

protected:
     /* Puntatore all'array degli indici
      * Usato per rendere piu' efficiente l'accesso,
      * dato che gli indici sono a base 1, in stile FORTRAN
      */
     integer* piRow, *piRowm1;

private:
     MySubVectorHandler(const MySubVectorHandler&);

public:
     /* Costruttori */

     /*
      * Costruttore per memoria posseduta.
      * Specifica solo la dimensione dell'array, che deve essere non-nulla.
      * La memoria viene allocata e gestita dal VectorHandler.
      */
     MySubVectorHandler(integer iSize);

     /*
      * Costruttore per memoria in prestito.
      * Riceve la dimensione dell'array e i puntatori alla memoria.
      */
     MySubVectorHandler(integer iSize, integer* piTmpRow,
                        doublereal* pdTmpVec);

     /*
      * Distruttore.
      */
     virtual ~MySubVectorHandler(void) {
          Detach();
     };

     using VectorHandler::operator=;
     using MyVectorHandler::operator=;

     /*
      * Tutti questi metodi sono richiesti perche'
      * la classe MySubVectorHandler dipende due volte
      * da VectorHandler e quindi vi e' un'ambiguita'
      * che va risolta (in realta' solo le funzioni di MyVectorHandler
      * sono definite, tutte le altre sono virtuali pure!)
      */

     /* Metodi di servizio */

     /*
      * Puntatore alla base del vettore (deprecato)
      */
     virtual doublereal* pdGetVec(void) const {
          return MyVectorHandler::pdGetVec();
     };

     /*
      * Dimensioni del vettore
      */
     virtual integer iGetSize(void) const {
          return MyVectorHandler::iGetSize();
     };

     /*
      * Ridimensiona il vettore.
      * Nota: se il vettore possiede la memoria a cui punta,
      * la nuova dimensione puo' eccedere la massima dimensione corrente.
      */
     virtual void Resize(integer iSize);

     /*
      * Inizializza il vettore con d
      */
     virtual void Reset(void) {
          MyVectorHandler::Reset();
     };

     /*
      * Scollega il vettore dalla memoria ad esso associata.
      * Se il vettore possiede la memoria, viene delallocata.
      */
     void Detach(void);

     /*
      * Collega il vettore alla memoria che gli viene passata.
      * La memoria a cui il vettore era collegato viene deallocata se
      * era posseduta dal vettore.
      */
     void
     Attach(integer iSize, doublereal* pd, integer* pi, integer iMSize = 0);
#ifdef DEBUG
     /*
      * Verifica la validita' del vettore.
      * Usata per debug
      */
     virtual void IsValid(void) const;
#endif /* DEBUG */

     /* Operazioni sugli indici e sui coefficienti */

     /*
      * Scrive un coefficiente in base al sottoindice.
      */
     virtual void PutCoef(integer i, const doublereal& d) {
          MyVectorHandler::PutCoef(i, d);
     };

     /*
      * Incrementa un coefficiente in base al sottoindice.
      */
     virtual void IncCoef(integer i, const doublereal& d) {
          MyVectorHandler::IncCoef(i, d);
     };

     /*
      * Decrementa un coefficiente in base al sottoindice.
      */
     virtual void DecCoef(integer i, const doublereal& d) {
          MyVectorHandler::DecCoef(i, d);
     };

     /*
      * Ottiene un coefficiente in base al sottoindice.
      */
     virtual const doublereal& dGetCoef(integer i) const {
          return MyVectorHandler::dGetCoef(i);
     };

     virtual inline const doublereal& operator () (integer iRow) const {
          return MyVectorHandler::operator () (iRow);
     };

     virtual inline doublereal& operator () (integer iRow) {
          return MyVectorHandler::operator () (iRow);
     };

     /*
      * Scrive un indice di riga in base al sottoindice.
      */
     virtual inline void PutRowIndex(integer iSubRow, integer iRow);

     /*
      * Ottiene un indice di riga in base al sottoindice.
      */
     virtual inline integer iGetRowIndex(integer iSubRow) const;

     /*
      * Scrive una entry completa.
      * @param iSubRow numero della entry (indice del sotto-vettore)
      * @param iRow    indice della entry
      * @param dCoef   coefficiente della entry
      */
     virtual inline void
     PutItem(integer iSubRow, integer iRow, const doublereal& dCoef);

     /* Interazione con i vettori */

     /*
      * Si somma ad un vettore con metodi generici
      */
     virtual VectorHandler& AddTo(VectorHandler& VH) const;

     /*
      * Si somma ad un MyVectorHandler
      */
     virtual VectorHandler& AddTo(MyVectorHandler& VH) const;

     /*
      * Si somma in valore assoluto ad un vettore con metodi generici
      */
     virtual VectorHandler& AddAbsValuesTo(VectorHandler& VH) const;

     /*
      * Si somma in valore assoluto ad un MyVectorHandler
      */
     virtual VectorHandler& AddAbsValuesTo(MyVectorHandler& VH) const;
};

inline void
MySubVectorHandler::PutRowIndex(integer iSubRow, integer iRow)
{
#ifdef DEBUG
     IsValid();
     ASSERT((iSubRow > 0) && (iSubRow <= iCurSize));
     ASSERT(iRow > 0);
#endif /* DEBUG */

     piRowm1[iSubRow] = iRow;

#ifdef DEBUG
     IsValid();
#endif
}

inline integer
MySubVectorHandler::iGetRowIndex(integer iSubRow) const
{
#ifdef DEBUG
     IsValid();
     ASSERT((iSubRow > 0) && (iSubRow <= iCurSize));
#endif /* DEBUG */

     return piRowm1[iSubRow];
}

inline void
MySubVectorHandler::PutItem(integer iSubRow, integer iRow,
                            const doublereal& dCoef)
{
#ifdef DEBUG
     IsValid();
     ASSERT((iSubRow > 0) && (iSubRow <= iCurSize));
     ASSERT(iRow > 0);
#endif /* DEBUG */

     piRowm1[iSubRow] = iRow;
     pdVecm1[iSubRow] = dCoef;

#ifdef DEBUG
     IsValid();
#endif
}

/* Operazioni esterne su SubMatrixHandler e su SubVectorHandler */

/*
 * Operatore per scrittura di SubVectorHandler su ostream.
 * Usato principalmente per debug
 */
extern std::ostream&
operator << (std::ostream& out, const SubVectorHandler& v);

/*
 * Operatore per scrittura di FullSubMatrixHandler su ostream.
 * Usato principalmente per debug
 */
extern std::ostream&
operator << (std::ostream& out, const FullSubMatrixHandler& m);


/* SubVectorHandler - end */

#endif /* SUBMAT_H */
