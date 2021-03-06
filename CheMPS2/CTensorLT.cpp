
#include <assert.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "CTensorLT.h"
#include "Lapack.h"
#include "Special.h"

CheMPS2::CTensorLT::CTensorLT( const int boundary_index, const int Idiff,
                               const bool moving_right,
                               const CheMPS2::SyBookkeeper * book_up,
                               const CheMPS2::SyBookkeeper * book_down )
    : CTensorOperator( boundary_index, // index
                       1,              // two_j
                       -1,             // n_elec
                       Idiff,          // irep
                       moving_right,   // direction
                       false,          // prime_last
                       true,           // jw_phase (one 2nd quantized operator)
                       book_up,        // upper bookkeeper
                       book_down       // lower bookkeeper
                       ) {}

CheMPS2::CTensorLT::~CTensorLT() {}

void CheMPS2::CTensorLT::create( CTensorT * mps_tensor_up, CTensorT * mps_tensor_down, CTensorO * previous, dcomplex * workmem ) {
   clear();

   if ( moving_right ) {
      for ( int ikappa = 0; ikappa < nKappa; ikappa++ ) {
         create_right( ikappa, mps_tensor_up, mps_tensor_down, previous, workmem );
      }
   } else {
      for ( int ikappa = 0; ikappa < nKappa; ikappa++ ) {
         create_left( ikappa, mps_tensor_up, mps_tensor_down, previous, workmem );
      }
   }
}

void CheMPS2::CTensorLT::create_right( const int ikappa, CTensorT * mps_tensor_up, CTensorT * mps_tensor_down, CTensorO * previous, dcomplex * workmem ) {
   const int NRU    = sector_nelec_up[ ikappa ];
   const int TwoSRU = sector_spin_up[ ikappa ];
   const int IRU    = sector_irrep_up[ ikappa ];

   const int NRD    = NRU - 1;
   const int IRD    = Irreps::directProd( IRU, n_irrep );
   const int TwoSRD = sector_spin_down[ ikappa ];

   int dimRU = bk_up->gCurrentDim( index, NRU, TwoSRU, IRU );
   int dimRD = bk_down->gCurrentDim( index, NRD, TwoSRD, IRD );

   char cotrans = 'C';
   char notrans = 'N';

   if ( dimRU > 0 && dimRD > 0 ) {

      for ( int geval = 0; geval < 2; geval++ ) {
         int NL, TwoSL, IL;
         switch ( geval ) {
            case 0:
               NL    = NRD;
               TwoSL = TwoSRD;
               IL    = IRD;
               break;
            case 1:
               NL    = NRU - 2;
               TwoSL = TwoSRU;
               IL    = IRU;
               break;
         }

         int dimLU = bk_up->gCurrentDim( index - 1, NL, TwoSL, IL );
         int dimLD = bk_down->gCurrentDim( index - 1, NL, TwoSL, IL );

         if ( previous == NULL ) {
            assert( dimLU == dimLD );
            if ( dimLU > 0 ) {
               dcomplex * Tup   = mps_tensor_up->gStorage( NL, TwoSL, IL, NRU, TwoSRU, IRU );
               dcomplex * Tdown = mps_tensor_down->gStorage( NL, TwoSL, IL, NRD, TwoSRD, IRD );

               dcomplex alpha = 1.0;
               if ( geval == 1 ) {
                  alpha = Special::phase( TwoSRU - TwoSRD + 1 ) * sqrt( ( TwoSRD + 1.0 ) / ( TwoSRU + 1 ) );
               }
               dcomplex add = 1.0;
               zgemm_( &cotrans, &notrans, &dimRU, &dimRD, &dimLU, &alpha, Tup, &dimLU, Tdown, &dimLU, &add, storage + kappa2index[ ikappa ], &dimRU );
            }
         } else {
            if ( ( dimLU > 0 ) && ( dimLD > 0 ) ) {
               dcomplex * Tup   = mps_tensor_up->gStorage( NL, TwoSL, IL, NRU, TwoSRU, IRU );
               dcomplex * Tdown = mps_tensor_down->gStorage( NL, TwoSL, IL, NRD, TwoSRD, IRD );
               dcomplex * Opart = previous->gStorage( NL, TwoSL, IL, NL, TwoSL, IL );

               dcomplex alpha = 1.0;
               if ( geval == 1 ) {
                  alpha = Special::phase( TwoSRU - TwoSRD + 1 ) * sqrt( ( TwoSRD + 1.0 ) / ( TwoSRU + 1 ) );
               }
               dcomplex set = 0.0;
               zgemm_( &cotrans, &notrans, &dimRU, &dimLD, &dimLU, &alpha, Tup, &dimLU, Opart, &dimLU, &set, workmem, &dimRU );

               dcomplex one = 1.0;
               zgemm_( &notrans, &notrans, &dimRU, &dimRD, &dimLD, &one, workmem, &dimRU, Tdown, &dimLD, &one, storage + kappa2index[ ikappa ], &dimRU );
            }
         }
      }
   }
}

void CheMPS2::CTensorLT::create_left( const int ikappa, CTensorT * mps_tensor_up, CTensorT * mps_tensor_down, CTensorO * previous, dcomplex * workmem ) {
   const int NLU    = sector_nelec_up[ ikappa ];
   const int TwoSLU = sector_spin_up[ ikappa ];
   const int ILU    = sector_irrep_up[ ikappa ];

   const int NLD    = NLU - 1;
   const int ILD    = Irreps::directProd( ILU, n_irrep );
   const int TwoSLD = sector_spin_down[ ikappa ];

   int dimLU = bk_up->gCurrentDim( index, NLU, TwoSLU, ILU );
   int dimLD = bk_down->gCurrentDim( index, NLD, TwoSLD, ILD );

   char cotrans = 'C';
   char notrans = 'N';

   if ( dimLU > 0 && dimLD > 0 ) {
      for ( int geval = 0; geval < 2; geval++ ) {
         int NR, TwoSR, IR;
         switch ( geval ) {
            case 0:
               NR    = NLU;
               TwoSR = TwoSLU;
               IR    = ILU;
               break;
            case 1:
               NR    = NLU + 1;
               TwoSR = TwoSLD;
               IR    = ILD;
               break;
         }

         int dimRU = bk_up->gCurrentDim( index + 1, NR, TwoSR, IR );
         int dimRD = bk_down->gCurrentDim( index + 1, NR, TwoSR, IR );

         if ( previous == NULL ) {
            assert( dimRU == dimRD );
            if ( dimRU > 0 ) {
               dcomplex * Tup   = mps_tensor_up->gStorage( NLU, TwoSLU, ILU, NR, TwoSR, IR );
               dcomplex * Tdown = mps_tensor_down->gStorage( NLD, TwoSLD, ILD, NR, TwoSR, IR );

               dcomplex alpha = 1.0;
               if ( geval == 1 ) {
                  alpha = Special::phase( TwoSLD - TwoSLU + 1 ) * sqrt( ( TwoSLD + 1.0 ) / ( TwoSLU + 1 ) );
               }
               dcomplex add = 1.0;
               zgemm_( &notrans, &cotrans, &dimLU, &dimLD, &dimRU, &alpha, Tup, &dimLU, Tdown, &dimLD, &add, storage + kappa2index[ ikappa ], &dimLU );
            }
         } else {
            if ( ( dimRU > 0 ) && ( dimRD > 0 ) ) {
               dcomplex * Tup   = mps_tensor_up->gStorage( NLU, TwoSLU, ILU, NR, TwoSR, IR );
               dcomplex * Tdown = mps_tensor_down->gStorage( NLD, TwoSLD, ILD, NR, TwoSR, IR );
               dcomplex * Opart = previous->gStorage( NR, TwoSR, IR, NR, TwoSR, IR );

               dcomplex alpha = 1.0;
               if ( geval == 1 ) {
                  alpha = Special::phase( TwoSLD - TwoSLU + 1 ) * sqrt( ( TwoSLD + 1.0 ) / ( TwoSLU + 1 ) );
               }
               dcomplex set = 0.0;
               zgemm_( &notrans, &notrans, &dimLU, &dimRD, &dimRU, &alpha, Tup, &dimLU, Opart, &dimRU, &set, workmem, &dimLU );

               dcomplex one = 1.0;
               zgemm_( &notrans, &cotrans, &dimLU, &dimLD, &dimRD, &one, workmem, &dimLU, Tdown, &dimLD, &one, storage + kappa2index[ ikappa ], &dimLU );
            }
         }
      }
   }
}
