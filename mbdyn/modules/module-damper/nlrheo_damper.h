
#ifndef NLRHEO_DAMPER_H
#define NLRHEO_DAMPER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gsl/gsl_errno.h>
#include "gsl/gsl_interp.h"
#include "gsl/gsl_odeiv.h"
#include "gsl/gsl_vector.h"
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_blas.h"
#include "gsl/gsl_linalg.h"

typedef struct sym_params {
	int n_parallelo;
	int *n_serie;			/* [n_parallelo] */
	int max_n_serie;
	double **s_max;			/* [max_n_serie][n_parallelo] */
	double **v_max;			/* [max_n_serie][n_parallelo] */
	int n_elementi;
	int *npti_ks;			/* [n_elementi] */
	int *npti_kv;			/* [n_elementi] */
	int *npti_cs;			/* [n_elementi] */
	int *npti_cv;			/* [n_elementi] */
	int n_variabili_k, n_variabili_c, n_variabili;
	int max_npti_ks, max_npti_kv, max_npti_cs, max_npti_cv;
	double **k_s;			/* [n_elementi][max_npti_ks] */
	double **k_v;			/* [n_elementi][max_npti_kv] */
	double **c_s;			/* [n_elementi][max_npti_cs] */
	double **c_v;			/* [n_elementi][max_npti_cv] */
	gsl_interp ** ik_s, ** ik_v, ** ic_s, ** ic_v;	/* tutti [n_elementi] */
	double *x, *y, *y_dummy, *yp, *yp_saved, *yp_prev;
	double nlrheo_t_cur, nlrheo_t_prev;

	/* scale factors */
	double scale_eps;
	double scale_f;
	/* filter 1/angular frequency */
	double hi_freq_force_filter_coeff;

	/* per integrazione */
	double sf, si, vf, vi, tf, ti;
	double f, f_s, f_v;

	double F, FDE, FDEPrime;

	/* di servizio */
	gsl_matrix **gsl_C, **gsl_K;
	gsl_vector **gsl_xp, **gsl_x, **gsl_b;
	gsl_permutation **gsl_perm;

	const gsl_odeiv_step_type *T;
	gsl_odeiv_step *stepint;
	gsl_odeiv_evolve * evolve;
	gsl_odeiv_control * control;
	gsl_odeiv_system sys;
	double prev_time, current_time, dt, dtmin;
	double prev_s, prev_sPrime;
	double eps_abs;
	double eps_rel;
	
	double low_freq_displ_filter_coeff;
	double static_low_freq_stiffness;

	int nsubsteps;
} sym_params;

/* read functions */
extern int nlrheo_get_int(int *i);
extern int nlrheo_get_real(double *d);

extern "C" int nlrheo_init(sym_params *nlrheo);
extern "C" int nlrheo_destroy(sym_params *nlrheo);
extern "C" int nlrheo_update(sym_params *nlrheo,
	double t_curr, double eps, double epsPrime, int do_try);
extern "C" int nlrheo_parse(sym_params **nlrheop,
	double scale_eps, double scale_f, double hi_filter,
	double lo_filter, double lo_stiffness, int nsubsteps, double dtmin);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ! NLRHEO_DAMPER_H */

