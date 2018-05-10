#include "gmres_functions.H"
#include "gmres_functions_F.H"
#include "gmres_namespace.H"

#include "common_functions.H"
#include "common_namespace.H"

using namespace amrex;
using namespace gmres;
using namespace common;

// solve "(theta*alpha*I - L) phi = rhs" using multigrid with Jacobi relaxation
// if abs(visc_type) = 1, L = div beta grad
// if abs(visc_type) = 2, L = div [ beta (grad + grad^T) ]
// if abs(visc_type) = 3, L = div [ beta (grad + grad^T) + I (gamma - (2/3)*beta) div ]
// if visc_type > 1 we assume constant coefficients
// if visc_type < 1 we assume variable coefficients
// beta_cc, and gamma_cc are cell-centered
// alpha_fc, phi_fc, and rhs_fc are face-centered
// beta_ed is nodal (2d) or edge-centered (3d)
// phi_fc must come in initialized to some value, preferably a reasonable guess
void StagMGSolver(const std::array< MultiFab, AMREX_SPACEDIM >& alpha_fc,
                  const MultiFab& beta_cc,
                  const std::array< MultiFab, NUM_EDGE >& beta_ed,
                  const MultiFab& gamma_cc,
                  const MultiFab& phi_fc,
                  const std::array< MultiFab, AMREX_SPACEDIM >& rhs_fc,
                  const Real& theta,
                  const Geometry& geom)
{

    // get the problem domain and boxarray at level 0
    Box pd_base = geom.Domain();
    BoxArray ba_base = beta_cc.boxArray();

    // compute the number of multigrid levels assuming stag_mg_minwidth is the length of the
    // smallest dimension of the smallest grid at the coarsest multigrid level
    int nlevs_mg = ComputeNlevsMG(ba_base);

    // allocate multifabs used in multigrid coarsening

    // cell-centered
    Vector<MultiFab>  beta_cc_mg(nlevs_mg);
    Vector<MultiFab> gamma_cc_mg(nlevs_mg);

    // face-centered
    Vector<std::array< MultiFab, AMREX_SPACEDIM > > alpha_fc_mg(nlevs_mg);
    Vector<std::array< MultiFab, AMREX_SPACEDIM > >   rhs_fc_mg(nlevs_mg);
    Vector<std::array< MultiFab, AMREX_SPACEDIM > >   phi_fc_mg(nlevs_mg);
    Vector<std::array< MultiFab, AMREX_SPACEDIM > >  Lphi_fc_mg(nlevs_mg);
    Vector<std::array< MultiFab, AMREX_SPACEDIM > > resid_fc_mg(nlevs_mg);
    Vector<std::array< MultiFab, NUM_EDGE > >  beta_ed_mg(nlevs_mg); // nodal in 2D, edge-based in 3D

    const Real* dx = geom.CellSize();

    Vector<std::array< Real, AMREX_SPACEDIM > > dx_mg;

    DistributionMapping dmap = beta_cc.DistributionMap();

    for (int n=0; n<nlevs_mg; ++n) {
        for (int d=0; d<AMREX_SPACEDIM; ++d) {
            // compute dx at this level of multigrid
            dx_mg[n][d] = dx[d] * pow(2,n);
        }

        // create the problem domain for this multigrid level
        Box pd = pd_base.coarsen(pow(2,n));

        // create the boxarray for this multigrid level
        BoxArray ba(ba_base); 
        ba.coarsen(pow(2,n));
     
        if ( n == 0 && !(ba == ba_base) ) {
            Abort("Finest multigrid level boxarray and coarsest problem boxarrays do not match");
        }

        // build multifabs used in multigrid coarsening
         beta_cc_mg[n].define(ba,dmap,1,1);
        gamma_cc_mg[n].define(ba,dmap,1,1);

        AMREX_D_TERM(alpha_fc_mg[n][0].define(convert(ba,nodal_flag_x), dmap, 1, 0);,
                     alpha_fc_mg[n][1].define(convert(ba,nodal_flag_y), dmap, 1, 0);,
                     alpha_fc_mg[n][2].define(convert(ba,nodal_flag_z), dmap, 1, 0););
        AMREX_D_TERM(  rhs_fc_mg[n][0].define(convert(ba,nodal_flag_x), dmap, 1, 0);,
                       rhs_fc_mg[n][1].define(convert(ba,nodal_flag_y), dmap, 1, 0);,
                       rhs_fc_mg[n][2].define(convert(ba,nodal_flag_z), dmap, 1, 0););
        AMREX_D_TERM(  phi_fc_mg[n][0].define(convert(ba,nodal_flag_x), dmap, 1, 1);,
                       phi_fc_mg[n][1].define(convert(ba,nodal_flag_y), dmap, 1, 1);,
                       phi_fc_mg[n][2].define(convert(ba,nodal_flag_z), dmap, 1, 1););
        AMREX_D_TERM( Lphi_fc_mg[n][0].define(convert(ba,nodal_flag_x), dmap, 1, 1);,
                      Lphi_fc_mg[n][1].define(convert(ba,nodal_flag_y), dmap, 1, 1);,
                      Lphi_fc_mg[n][2].define(convert(ba,nodal_flag_z), dmap, 1, 1););
        AMREX_D_TERM(resid_fc_mg[n][0].define(convert(ba,nodal_flag_x), dmap, 1, 0);,
                     resid_fc_mg[n][1].define(convert(ba,nodal_flag_y), dmap, 1, 0);,
                     resid_fc_mg[n][2].define(convert(ba,nodal_flag_z), dmap, 1, 0););

        // build beta_ed_mg
        if (AMREX_SPACEDIM == 2) {
            beta_ed_mg[n][0].define(convert(ba,nodal_flag), dmap, 1, 0);
        }
        else if (AMREX_SPACEDIM == 3) {
            beta_ed_mg[n][0].define(convert(ba,nodal_flag_xy), dmap, 1, 0);
            beta_ed_mg[n][1].define(convert(ba,nodal_flag_xz), dmap, 1, 0);
            beta_ed_mg[n][2].define(convert(ba,nodal_flag_yz), dmap, 1, 0);
        }
    } // end loop over multigrid levels

    // copy level 1 coefficients into mg array of coefficients
    MultiFab::Copy(beta_cc_mg[0],beta_cc,0,0,1,1);
    MultiFab::Copy(gamma_cc_mg[0],gamma_cc,0,0,1,1);
    for (int d=0; d<AMREX_SPACEDIM; ++d) {
        MultiFab::Copy(alpha_fc_mg[0][d],alpha_fc[d],0,0,1,0);
        // multiply alpha_fc_mg by theta
        alpha_fc_mg[0][d].mult(theta,0,1,0);
    }
    MultiFab::Copy(beta_ed_mg[0][0],beta_ed[0],0,0,1,0);
    if (AMREX_SPACEDIM == 3) {
        MultiFab::Copy(beta_ed_mg[0][1],beta_ed[1],0,0,1,0);
        MultiFab::Copy(beta_ed_mg[0][2],beta_ed[2],0,0,1,0);
    }
    
    // coarsen coefficients
    for (int n=1; n<nlevs_mg; ++n) {
        // need ghost cells set to zero to prevent intermediate NaN states
        // that cause some compilers to fail
        beta_cc_mg[n].setVal(0.);
        gamma_cc_mg[n].setVal(0.);

        // cc_restriction on beta_cc_mg and gamma_cc_mg
        CCRestriction( beta_cc_mg[n], beta_cc_mg[n-1]);
        CCRestriction(gamma_cc_mg[n],gamma_cc_mg[n-1]);

        // stag_restriction on alpha_fc_mg
        StagRestriction(alpha_fc_mg[n],alpha_fc_mg[n-1],1);

#if (AMREX_SPACEDIM == 2)
            // nodal_restriction on beta_ed_mg
            NodalRestriction(beta_ed_mg[n][0],beta_ed_mg[n-1][0]);
#elif (AMREX_SPACEDIM == 3)
            // edge_restriction on beta_ed_mg
            EdgeRestriction(beta_ed_mg[n],beta_ed_mg[n-1]);
#endif
    }

    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Now we wolve the homogeneous problem
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    for (int d=0; d<AMREX_SPACEDIM; ++d) {
        
        // initialize phi_fc_mg = phi_fc as an initial guess

        // fill periodic ghost cells

    }

    // set rhs_fc_mg at level 1 by copying in passed-in rhs_fc


    // compute norm of initial residual
    // first compute Lphi



    for (int d=0; d<AMREX_SPACEDIM; ++d) {

        // compute Lphi - rhs



        // compute L0 norm of Lphi - rhs


    }




}

// compute the number of multigrid levels assuming minwidth is the length of the
// smallest dimension of the smallest grid at the coarsest multigrid level
int ComputeNlevsMG(const BoxArray& ba) {

    int nlevs_mg = -1;

    for (int i=0; i<ba.size(); ++i) {
        Box bx = ba.get(i);
        IntVect iv = bx.bigEnd() - bx.smallEnd() + IntVect(1);

        for (int d=0; d<AMREX_SPACEDIM; ++d) {
            int temp = iv[d];
            int rdir = 1;
            while (temp%2 == 0 && temp/stag_mg_minwidth != 1) {
                temp /= 2;
                ++rdir;
            }

            if (nlevs_mg == -1) {
                nlevs_mg = rdir;
            }
            else {
                nlevs_mg = std::min(rdir,nlevs_mg);
            }
        }
    }

    return nlevs_mg;
}

void CCRestriction(MultiFab& phi_c, const MultiFab& phi_f)
{
    // loop over boxes (make sure mfi takes a cell-centered multifab as an argument)
    for ( MFIter mfi(phi_c); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        const Box& validBox = mfi.validbox();

        cc_restriction(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                       BL_TO_FORTRAN_3D(phi_c[mfi]),
                       BL_TO_FORTRAN_3D(phi_f[mfi]));
    }
}

void StagRestriction(std::array< MultiFab, AMREX_SPACEDIM >& phi_c, 
                     const std::array< MultiFab, AMREX_SPACEDIM >& phi_f,
                     int simple_stencil)
{

    // loop over boxes (note we are not passing in a cell-centered MultiFab)
    for ( MFIter mfi(phi_c[0]); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        // there are no cell-centered MultiFabs so use this to get
        // a cell-centered box
        const Box& validBox = amrex::enclosedCells(mfi.validbox());

        stag_restriction(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                         BL_TO_FORTRAN_3D(phi_c[0][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[0][mfi]),
                         BL_TO_FORTRAN_3D(phi_c[1][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[1][mfi]),
#if (AMREX_SPACEDIM == 3)
                         BL_TO_FORTRAN_3D(phi_c[2][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[2][mfi]),
#endif
                         &simple_stencil);
    }
}

void NodalRestriction(MultiFab& phi_c, const MultiFab& phi_f)
{
    // loop over boxes (note we are not passing in a cell-centered MultiFab)
    for ( MFIter mfi(phi_c); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        // there are no cell-centered MultiFabs so use this to get
        // a cell-centered box
        const Box& validBox = amrex::enclosedCells(mfi.validbox());

        nodal_restriction(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                          BL_TO_FORTRAN_3D(phi_c[mfi]),
                          BL_TO_FORTRAN_3D(phi_f[mfi]));

    }
}

void EdgeRestriction(std::array< MultiFab, NUM_EDGE >& phi_c, 
                     const std::array< MultiFab, NUM_EDGE >& phi_f)
{
    if (AMREX_SPACEDIM != 3) {
        Abort("Edge restriction can only be called for 3D!");
    }

    // loop over boxes (note we are not passing in a cell-centered MultiFab)
    for ( MFIter mfi(phi_c[0]); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        // there are no cell-centered MultiFabs so use this to get
        // a cell-centered box
        const Box& validBox = amrex::enclosedCells(mfi.validbox());

        edge_restriction(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                         BL_TO_FORTRAN_3D(phi_c[0][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[0][mfi]),
                         BL_TO_FORTRAN_3D(phi_c[1][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[1][mfi]),
                         BL_TO_FORTRAN_3D(phi_c[2][mfi]),
                         BL_TO_FORTRAN_3D(phi_f[2][mfi]));
    }
}

void StagProlongation(const std::array< MultiFab, AMREX_SPACEDIM >& phi_c, 
                      std::array< MultiFab, AMREX_SPACEDIM >& phi_f)
{

    // loop over boxes (note we are not passing in a cell-centered MultiFab)
    for ( MFIter mfi(phi_f[0]); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        // there are no cell-centered MultiFabs so use this to get
        // a cell-centered box
        const Box& validBox = amrex::enclosedCells(mfi.validbox());

        stag_prolongation(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                          BL_TO_FORTRAN_3D(phi_c[0][mfi]),
                          BL_TO_FORTRAN_3D(phi_f[0][mfi]),
                          BL_TO_FORTRAN_3D(phi_c[1][mfi]),
                          BL_TO_FORTRAN_3D(phi_f[1][mfi])
#if (AMREX_SPACEDIM == 3)
                        , BL_TO_FORTRAN_3D(phi_c[2][mfi]),
                          BL_TO_FORTRAN_3D(phi_f[2][mfi])
#endif
                          );
    }

}

void StagMGUpdate(std::array< MultiFab, AMREX_SPACEDIM >& phi_fc,
                  const std::array< MultiFab, AMREX_SPACEDIM >& rhs_fc,
                  const std::array< MultiFab, AMREX_SPACEDIM >& Lphi_fc,
                  const std::array< MultiFab, AMREX_SPACEDIM >& alpha_fc,
                  const MultiFab& beta_cc,
                  const std::array< MultiFab, NUM_EDGE >& beta_ed,
                  const MultiFab& gamma_cc,
                  const Real* dx,
                  const int& color)
{

    // loop over boxes (make sure mfi takes a cell-centered multifab as an argument)
    for ( MFIter mfi(beta_cc); mfi.isValid(); ++mfi ) {

        // Get the index space of the valid region
        const Box& validBox = mfi.validbox();


        stag_mg_update(ARLIM_3D(validBox.loVect()), ARLIM_3D(validBox.hiVect()),
                       BL_TO_FORTRAN_3D(phi_fc[0][mfi]),
                       BL_TO_FORTRAN_3D(phi_fc[1][mfi]),
#if (AMREX_SPACEDIM == 3)
                       BL_TO_FORTRAN_3D(phi_fc[2][mfi]),
#endif
                       BL_TO_FORTRAN_3D(rhs_fc[0][mfi]),
                       BL_TO_FORTRAN_3D(rhs_fc[1][mfi]),
#if (AMREX_SPACEDIM == 3)
                       BL_TO_FORTRAN_3D(rhs_fc[2][mfi]),
#endif
                       BL_TO_FORTRAN_3D(Lphi_fc[0][mfi]),
                       BL_TO_FORTRAN_3D(Lphi_fc[1][mfi]),
#if (AMREX_SPACEDIM == 3)
                       BL_TO_FORTRAN_3D(Lphi_fc[2][mfi]),
#endif
                       BL_TO_FORTRAN_3D(alpha_fc[0][mfi]),
                       BL_TO_FORTRAN_3D(alpha_fc[1][mfi]),
#if (AMREX_SPACEDIM == 3)
                       BL_TO_FORTRAN_3D(alpha_fc[2][mfi]),
#endif
                       BL_TO_FORTRAN_3D(beta_cc[mfi]),
#if (AMREX_SPACEDIM == 2)
                       BL_TO_FORTRAN_3D(beta_ed[0][mfi]),
#elif (AMREX_SPACEDIM == 3)
                       BL_TO_FORTRAN_3D(beta_ed[0][mfi]),
                       BL_TO_FORTRAN_3D(beta_ed[1][mfi]),
                       BL_TO_FORTRAN_3D(beta_ed[2][mfi]),
#endif
                       BL_TO_FORTRAN_3D(gamma_cc[mfi]),
                       dx, &color);



    }

}
