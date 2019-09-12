#include <ib_functions.H>
#include <ib_functions_F.H>
#include <immbdy_namespace.H>
#include <immbdy_namespace_declarations.H>


using namespace immbdy;
using namespace ib_flagellum;


void InitializeImmbdyNamespace() {

    n_immbdy = 0;
    contains_flagellum = false;

    int cf = 0, cfourier = 0;
    initialize_immbdy_namespace(& n_immbdy, & cf, & cfourier);
    if (cf == 1) contains_flagellum = true;
    if (cfourier == 1) contains_fourier = true;

}


void InitializeIBFlagellumNamespace() {

    ib_flagellum::n_marker.resize(n_immbdy);
    ib_flagellum::offset_0.resize(n_immbdy);
    ib_flagellum::amplitude.resize(n_immbdy);
    ib_flagellum::frequency.resize(n_immbdy);
    ib_flagellum::length.resize(n_immbdy);
    ib_flagellum::wavelength.resize(n_immbdy);
    ib_flagellum::k_spring.resize(n_immbdy);
    ib_flagellum::k_driving.resize(n_immbdy);

    int len_buffer = 0;
    chlamy_flagellum_datafile_len(& len_buffer);
    char chlamy_flagellum_datafile[len_buffer+1];

    initialize_ib_flagellum_namespace(n_immbdy,
                                      ib_flagellum::n_marker.dataPtr(),
                                      ib_flagellum::offset_0.dataPtr(),
                                      ib_flagellum::amplitude.dataPtr(),
                                      ib_flagellum::frequency.dataPtr(),
                                      ib_flagellum::length.dataPtr(),
                                      ib_flagellum::wavelength.dataPtr(),
                                      ib_flagellum::k_spring.dataPtr(),
                                      ib_flagellum::k_driving.dataPtr(),
                                      & ib_flagellum::fourier_coef_len,
                                      chlamy_flagellum_datafile);

    ib_flagellum::chlamy_flagellum_datafile = chlamy_flagellum_datafile;
}
