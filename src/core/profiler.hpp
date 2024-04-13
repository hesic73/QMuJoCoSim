#ifndef QMUJOCOSIM_PROFILER_HPP
#define QMUJOCOSIM_PROFILER_HPP

/**
 * Adapted from https://github.com/google-deepmind/mujoco/blob/main/simulate/simulate.cc
 */

#include <cstring>
#include <memory>

#include <mujoco/mujoco.h>

class Profiler {
public:
    explicit Profiler() = default;

    // init profiler figures
    void initialize() {
        // set figures to default
        mjv_defaultFigure(&figconstraint);
        mjv_defaultFigure(&figcost);
        mjv_defaultFigure(&figtimer);
        mjv_defaultFigure(&figsize);

        // titles
        std::strcpy(figconstraint.title, "Counts");
        std::strcpy(figcost.title, "Convergence (log 10)");
        std::strcpy(figsize.title, "Dimensions");
        std::strcpy(figtimer.title, "CPU time (msec)");

        // x-labels
        std::strcpy(figconstraint.xlabel, "Solver iteration");
        std::strcpy(figcost.xlabel, "Solver iteration");
        std::strcpy(figsize.xlabel, "Video frame");
        std::strcpy(figtimer.xlabel, "Video frame");

        // y-tick number formats
        std::strcpy(figconstraint.yformat, "%.0f");
        std::strcpy(figcost.yformat, "%.1f");
        std::strcpy(figsize.yformat, "%.0f");
        std::strcpy(figtimer.yformat, "%.2f");

        // colors
        figconstraint.figurergba[0] = 0.1f;
        figcost.figurergba[2] = 0.2f;
        figsize.figurergba[0] = 0.1f;
        figtimer.figurergba[2] = 0.2f;
        figconstraint.figurergba[3] = 0.5f;
        figcost.figurergba[3] = 0.5f;
        figsize.figurergba[3] = 0.5f;
        figtimer.figurergba[3] = 0.5f;

        // repeat line colors for constraint and cost figures
        mjvFigure *fig = &figcost;
        for (int i = kCostNum; i < mjMAXLINE; i++) {
            fig->linergb[i][0] = fig->linergb[i - kCostNum][0];
            fig->linergb[i][1] = fig->linergb[i - kCostNum][1];
            fig->linergb[i][2] = fig->linergb[i - kCostNum][2];
        }
        fig = &figconstraint;
        for (int i = kConstraintNum; i < mjMAXLINE; i++) {
            fig->linergb[i][0] = fig->linergb[i - kConstraintNum][0];
            fig->linergb[i][1] = fig->linergb[i - kConstraintNum][1];
            fig->linergb[i][2] = fig->linergb[i - kConstraintNum][2];
        }

        // legends
        std::strcpy(figconstraint.linename[0], "total");
        std::strcpy(figconstraint.linename[1], "active");
        std::strcpy(figconstraint.linename[2], "changed");
        std::strcpy(figconstraint.linename[3], "evals");
        std::strcpy(figconstraint.linename[4], "updates");
        std::strcpy(figcost.linename[0], "improvement");
        std::strcpy(figcost.linename[1], "gradient");
        std::strcpy(figcost.linename[2], "lineslope");
        std::strcpy(figsize.linename[0], "dof");
        std::strcpy(figsize.linename[1], "body");
        std::strcpy(figsize.linename[2], "constraint");
        std::strcpy(figsize.linename[3], "sqrt(nnz)");
        std::strcpy(figsize.linename[4], "contact");
        std::strcpy(figsize.linename[5], "iteration");
        std::strcpy(figtimer.linename[0], "total");
        std::strcpy(figtimer.linename[1], "collision");
        std::strcpy(figtimer.linename[2], "prepare");
        std::strcpy(figtimer.linename[3], "solve");
        std::strcpy(figtimer.linename[4], "other");

        // grid sizes
        figconstraint.gridsize[0] = 5;
        figconstraint.gridsize[1] = 5;
        figcost.gridsize[0] = 5;
        figcost.gridsize[1] = 5;
        figsize.gridsize[0] = 3;
        figsize.gridsize[1] = 5;
        figtimer.gridsize[0] = 3;
        figtimer.gridsize[1] = 5;

        // minimum ranges
        figconstraint.range[0][0] = 0;
        figconstraint.range[0][1] = 20;
        figconstraint.range[1][0] = 0;
        figconstraint.range[1][1] = 80;
        figcost.range[0][0] = 0;
        figcost.range[0][1] = 20;
        figcost.range[1][0] = -15;
        figcost.range[1][1] = 5;
        figsize.range[0][0] = -200;
        figsize.range[0][1] = 0;
        figsize.range[1][0] = 0;
        figsize.range[1][1] = 100;
        figtimer.range[0][0] = -200;
        figtimer.range[0][1] = 0;
        figtimer.range[1][0] = 0;
        figtimer.range[1][1] = 0.4f;

        // init x axis on history figures (do not show yet)
        for (int n = 0; n < 6; n++) {
            for (int i = 0; i < mjMAXLINEPNT; i++) {
                figtimer.linedata[n][2 * i] = -i;
                figsize.linedata[n][2 * i] = -i;
            }
        }
    }

    // update profiler figures
    void update(const mjModel *m, const mjData *d) {
        // reset lines in Constraint and Cost figures
        std::memset(figconstraint.linepnt, 0, mjMAXLINE * sizeof(int));
        std::memset(figcost.linepnt, 0, mjMAXLINE * sizeof(int));

        // number of islands that have diagnostics
        int nisland = mjMIN(d->solver_nisland, mjNISLAND);

        // iterate over islands
        for (int k = 0; k < nisland; k++) {
            // ==== update Constraint ("Counts") figure

            // number of points to plot, starting line
            int npoints = mjMIN(mjMIN(d->solver_niter[k], mjNSOLVER), mjMAXLINEPNT);
            int start = kConstraintNum * k;

            figconstraint.linepnt[start + 0] = npoints;
            for (int i = 1; i < kConstraintNum; i++) {
                figconstraint.linepnt[start + i] = npoints;
            }
            if (m->opt.solver == mjSOL_PGS) {
                figconstraint.linepnt[start + 3] = 0;
                figconstraint.linepnt[start + 4] = 0;
            }
            if (m->opt.solver == mjSOL_CG) {
                figconstraint.linepnt[start + 4] = 0;
            }
            for (int i = 0; i < npoints; i++) {
                // x
                figconstraint.linedata[start + 0][2 * i] = i;
                figconstraint.linedata[start + 1][2 * i] = i;
                figconstraint.linedata[start + 2][2 * i] = i;
                figconstraint.linedata[start + 3][2 * i] = i;
                figconstraint.linedata[start + 4][2 * i] = i;

                // y
                int nefc = nisland == 1 ? d->nefc : d->island_efcnum[k];
                figconstraint.linedata[start + 0][2 * i + 1] = nefc;
                const mjSolverStat *stat = d->solver + k * mjNSOLVER + i;
                figconstraint.linedata[start + 1][2 * i + 1] = stat->nactive;
                figconstraint.linedata[start + 2][2 * i + 1] = stat->nchange;
                figconstraint.linedata[start + 3][2 * i + 1] = stat->neval;
                figconstraint.linedata[start + 4][2 * i + 1] = stat->nupdate;
            }

            // update cost figure
            start = kCostNum * k;
            figcost.linepnt[start + 0] = npoints;
            for (int i = 1; i < kCostNum; i++) {
                figcost.linepnt[start + i] = npoints;
            }
            if (m->opt.solver == mjSOL_PGS) {
                figcost.linepnt[start + 1] = 0;
                figcost.linepnt[start + 2] = 0;
            }

            for (int i = 0; i < figcost.linepnt[0]; i++) {
                // x
                figcost.linedata[start + 0][2 * i] = i;
                figcost.linedata[start + 1][2 * i] = i;
                figcost.linedata[start + 2][2 * i] = i;

                // y
                const mjSolverStat *stat = d->solver + k * mjNSOLVER + i;
                figcost.linedata[start + 0][2 * i + 1] =
                        mju_log10(mju_max(mjMINVAL, stat->improvement));
                figcost.linedata[start + 1][2 * i + 1] =
                        mju_log10(mju_max(mjMINVAL, stat->gradient));
                figcost.linedata[start + 2][2 * i + 1] =
                        mju_log10(mju_max(mjMINVAL, stat->lineslope));
            }
        }

        // get timers: total, collision, prepare, solve, other
        mjtNum total = d->timer[mjTIMER_STEP].duration;
        int number = d->timer[mjTIMER_STEP].number;
        if (!number) {
            total = d->timer[mjTIMER_FORWARD].duration;
            number = d->timer[mjTIMER_FORWARD].number;
        }
        if (number) {  // skip update if no measurements
            float tdata[5] = {
                    static_cast<float>(total / number),
                    static_cast<float>(d->timer[mjTIMER_POS_COLLISION].duration / number),
                    static_cast<float>(d->timer[mjTIMER_POS_MAKE].duration / number) +
                    static_cast<float>(d->timer[mjTIMER_POS_PROJECT].duration / number),
                    static_cast<float>(d->timer[mjTIMER_CONSTRAINT].duration / number),
                    0
            };
            tdata[4] = tdata[0] - tdata[1] - tdata[2] - tdata[3];
            // update figtimer
            int pnt = mjMIN(201, figtimer.linepnt[0] + 1);
            for (int n = 0; n < 5; n++) {
                // shift data
                for (int i = pnt - 1; i > 0; i--) {
                    figtimer.linedata[n][2 * i + 1] = figtimer.linedata[n][2 * i - 1];
                }

                // assign new
                figtimer.linepnt[n] = pnt;
                figtimer.linedata[n][1] = tdata[n];
            }
        }

        // get total number of iterations and nonzeros
        mjtNum sqrt_nnz = 0;
        int solver_niter = 0;
        for (int island = 0; island < nisland; island++) {
            sqrt_nnz += mju_sqrt(d->solver_nnz[island]);
            solver_niter += d->solver_niter[island];
        }

        // get sizes: nv, nbody, nefc, sqrt(nnz), ncont, iter
        float sdata[6] = {
                static_cast<float>(m->nv),
                static_cast<float>(m->nbody),
                static_cast<float>(d->nefc),
                static_cast<float>(sqrt_nnz),
                static_cast<float>(d->ncon),
                static_cast<float>(solver_niter)
        };

        // update figsize
        int pnt = mjMIN(201, figsize.linepnt[0] + 1);
        for (int n = 0; n < 6; n++) {
            // shift data
            for (int i = pnt - 1; i > 0; i--) {
                figsize.linedata[n][2 * i + 1] = figsize.linedata[n][2 * i - 1];
            }

            // assign new
            figsize.linepnt[n] = pnt;
            figsize.linedata[n][1] = sdata[n];
        }
    }

    // show profiler figures
    void show(mjrContext *con, mjrRect rect) {
        mjrRect viewport = {
                rect.left + rect.width - rect.width / 4,
                rect.bottom,
                rect.width / 4,
                rect.height / 4
        };
        mjr_figure(viewport, &figtimer, con);
        viewport.bottom += rect.height / 4;
        mjr_figure(viewport, &figsize, con);
        viewport.bottom += rect.height / 4;
        mjr_figure(viewport, &figcost, con);
        viewport.bottom += rect.height / 4;
        mjr_figure(viewport, &figconstraint, con);
    }

private:
    static constexpr int kConstraintNum = 5;
    static constexpr int kCostNum = 3;
    mjvFigure figconstraint = {};
    mjvFigure figcost = {};
    mjvFigure figtimer = {};
    mjvFigure figsize = {};
};


#endif //QMUJOCOSIM_PROFILER_HPP
