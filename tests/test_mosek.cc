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

namespace mosek_example {
namespace test {

static void MSKAPI printstr(void* handle, const char str[]) {
  printf("%s", str);
} /* printstr */

TEST(TestMosekLisence, basic) {
  MSKrescodee r;
  MSKint32t i, j;

  MSKenv_t env = NULL;
  MSKtask_t task = NULL;
  MSKint64t quadDom;

  /* Input data dimensions */
  const MSKint32t n = 3, k = 2;

  /* Create the mosek environment. */
  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK) {
    /* Create the optimization task. */
    r = MSK_maketask(env, 0, 0, &task);

    if (r == MSK_RES_OK) {
      MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

      /* Create n free variables */
      if (r == MSK_RES_OK)
        r = MSK_appendvars(task, n);
      if (r == MSK_RES_OK)
        r = MSK_putvarboundsliceconst(task, 0, n, MSK_BK_FR, -MSK_INFINITY,
                                      +MSK_INFINITY);

      /* Set up the objective */
      {
        double c[] = {2.0, 3.0, -1.0};

        if (r == MSK_RES_OK)
          r = MSK_putcslice(task, 0, n, c);
        if (r == MSK_RES_OK)
          r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);
      }

      /* One linear constraint sum(x) == 1 */
      if (r == MSK_RES_OK)
        r = MSK_appendcons(task, 1);
      if (r == MSK_RES_OK)
        r = MSK_putconbound(task, 0, MSK_BK_FX, 1.0, 1.0);
      for (i = 0; i < n && r == MSK_RES_OK; i++)
        r = MSK_putaij(task, 0, i, 1.0);

      /* Append empty AFE rows for affine expression storage */
      if (r == MSK_RES_OK)
        r = MSK_appendafes(task, k + 1);

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
        if (r == MSK_RES_OK)
          r = MSK_putafefentrylist(task, numEntries, Fsubi, Fsubj, Fval);

        /* Fill in g storage */
        if (r == MSK_RES_OK)
          r = MSK_putafeg(task, 0, gamma);
        if (r == MSK_RES_OK)
          r = MSK_putafegslice(task, 1, k + 1, h);
      }

      /* Define a conic quadratic domain */
      if (r == MSK_RES_OK)
        r = MSK_appendquadraticconedomain(task, k + 1, &quadDom);

      {
        /* Create the ACC */
        MSKint64t afeidx[] = {0, 1, 2};

        if (r == MSK_RES_OK)
          r = MSK_appendacc(task, quadDom, /* Domain index */
                            k + 1,         /* Dimension */
                            afeidx,        /* Indices of AFE rows [0,...,k] */
                            NULL);         /* Ignored */
      }

      /* Begin optimization and fetching the solution */
      if (r == MSK_RES_OK) {
        MSKrescodee trmcode;

        /* Run optimizer */
        r = MSK_optimizetrm(task, &trmcode);

        /* Print a summary containing information
           about the solution for debugging purposes*/
        MSK_solutionsummary(task, MSK_STREAM_MSG);

        if (r == MSK_RES_OK) {
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
        } else {
          std::cout << "Error while optimizing.\n";
        }
      }

      if (r != MSK_RES_OK) {
        /* In case of an error print error code and description. */
        char symname[MSK_MAX_STR_LEN];
        char desc[MSK_MAX_STR_LEN];

        std::cout << "An error occurred while optimizing.\n";
      }
    }
    /* Delete the task and the associated data. */
    MSK_deletetask(&task);
  }

  /* Delete the environment and the associated data. */
  MSK_deleteenv(&env);
}

}  // namespace test
}  // namespace promotion
