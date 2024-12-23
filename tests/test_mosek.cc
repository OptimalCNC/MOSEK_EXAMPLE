/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      acc1.c

   Purpose :   Tutorial example for affine conic constraints.
               Models the problem:

               maximize c^T x
               subject to  sum(x) = 1
                           gamma >= |Gx+h|_2
 */
#include <mosek.h>
#include <gtest/gtest.h>
#include <iostream>

#define MOSEK_MUST_SUCCEED(x, msg) do { \
  MSKrescodee _r = (x); \
  ASSERT_EQ(_r, MSK_RES_OK) << (msg); \
} while(0)


namespace mosek_example {
namespace test {

static void MSKAPI printstr(void* handle, const char str[]) {
  printf("%s", str);
} /* printstr */

TEST(TestMosekLisence, basic) {
  MSKint32t i, j;
  MSKrescodee r;
  MSKenv_t env = NULL;
  MSKtask_t task = NULL;
  MSKint64t quadDom;

  /* Input data dimensions */
  const MSKint32t n = 3, k = 2;

  /* Create the mosek environment. */
  MOSEK_MUST_SUCCEED(MSK_makeenv(&env, NULL), "Failed to create MOSEK environment.");

  /* Create the optimization task. */
  MOSEK_MUST_SUCCEED(MSK_maketask(env, 0, 0, &task), "Failed to create MOSEK task.");

  MOSEK_MUST_SUCCEED(MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr), "Failed to link function to task stream.");

  /* Create n free variables */
  MOSEK_MUST_SUCCEED(MSK_appendvars(task, n), "Failed to append variables.");
  MOSEK_MUST_SUCCEED(MSK_putvarboundsliceconst(task, 0, n, MSK_BK_FR, -MSK_INFINITY, +MSK_INFINITY), "Failed to put variable bounds.");

  /* Set up the objective */
  {
    double c[] = {2.0, 3.0, -1.0};
    MOSEK_MUST_SUCCEED(MSK_putcslice(task, 0, n, c), "Failed to put objective coefficients.");
    MOSEK_MUST_SUCCEED(MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE), "Failed to set objective sense.");
  }

  /* One linear constraint sum(x) == 1 */
  MOSEK_MUST_SUCCEED(MSK_appendcons(task, 1), "Failed to append constraints.");
  MOSEK_MUST_SUCCEED(MSK_putconbound(task, 0, MSK_BK_FX, 1.0, 1.0), "Failed to put constraint bounds.");
  for (i = 0; i < n; i++)
    MOSEK_MUST_SUCCEED(MSK_putaij(task, 0, i, 1.0), "Failed to put constraint coefficients.");

  for (i = 0; i < n; i++)
    MOSEK_MUST_SUCCEED(MSK_putaij(task, 0, i, 1.0), "Failed to put constraint coefficients.");

  /* Append empty AFE rows for affine expression storage */
  MOSEK_MUST_SUCCEED(MSK_appendafes(task, k + 1), "Failed to append AFE rows.");

  {
    /* Fill in the affine expression storage with data */
    /* F matrix in sparse form */
    MSKint64t Fsubi[] = {1, 1, 2, 2}; /* G is placed from row 1 of F */
    MSKint32t Fsubj[] = {0, 1, 0, 2};
    double Fval[] = {1.5, 0.1, 0.3, 2.1};
    int numEntries = 4;
    /* Other data */
    double h[] = {0, 0.1};
    double gamma = 0.03;

    /* Fill in F storage */
    MOSEK_MUST_SUCCEED(MSK_putafefentrylist(task, numEntries, Fsubi, Fsubj, Fval), "Failed to put AFE entry list.");

    /* Fill in g storage */
    MOSEK_MUST_SUCCEED(MSK_putafeg(task, 0, gamma), "Failed to put g storage.");

    /* Fill in g storage */
    MOSEK_MUST_SUCCEED(MSK_putafegslice(task, 1, k + 1, h), "Failed to put g storage.");
  }

  /* Define a conic quadratic domain */
  MOSEK_MUST_SUCCEED(MSK_appendquadraticconedomain(task, k + 1, &quadDom), "Failed to append quadratic cone domain.");

  {
    /* Create the ACC */
    MSKint64t afeidx[] = {0, 1, 2};

    MOSEK_MUST_SUCCEED(MSK_appendacc(task, quadDom, /* Domain index */ 
                        k + 1,         /* Dimension */
                        afeidx,        /* Indices of AFE rows [0,...,k] */
                        NULL),         /* Ignored */
                        "Failed to append ACC.");
  }

  /* Begin optimization and fetching the solution */
  MSKrescodee trmcode;

  /* Run optimizer */
  MOSEK_MUST_SUCCEED(MSK_optimizetrm(task, &trmcode), "Failed to optimize.");

  /* Print a summary containing information
      about the solution for debugging purposes*/
  MOSEK_MUST_SUCCEED(MSK_solutionsummary(task, MSK_STREAM_MSG), "Failed to print solution summary.");

  MSKsolstae solsta;
  MSK_getsolsta(task, MSK_SOL_ITR, &solsta);

  switch (solsta) {
    case MSK_SOL_STA_OPTIMAL:
      break;
    case MSK_SOL_STA_DUAL_INFEAS_CER:
    case MSK_SOL_STA_PRIM_INFEAS_CER:
      std::cout << "Primal or dual infeasibility certificate found.\n";
      break;
    case MSK_SOL_STA_UNKNOWN:
      std::cout << "The status of the solution could not be "
                    "determined. Termination code: "
                << trmcode << "\n";
      break;
    default:
      std::cout << "Other solution status.";
      break;
  }

  /* Delete the task and the associated data. */
  MSK_deletetask(&task);

  /* Delete the environment and the associated data. */
  MSK_deleteenv(&env);
}

}  // namespace test
}  // namespace promotion
