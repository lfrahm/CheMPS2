
#include "TimeTaylor.h"
#include <assert.h>
#include <iomanip>
#include <iostream>

#include "CDensityMatrix.h"
#include "CHeffNS.h"
#include "CHeffNS_1S.h"
#include "CSobject.h"
#include "CTwoDMBuilder.h"
#include "HamiltonianOperator.h"
#include "Lapack.h"
#include "Special.h"
#include "TwoDMBuilder.h"

CheMPS2::TimeTaylor::TimeTaylor( Problem * probIn, ConvergenceScheme * schemeIn, Logger * loggerIn )
    : prob( probIn ), scheme( schemeIn ), logger( loggerIn ), L( probIn->gL() ) {
   assert( probIn->checkConsistency() );

   prob->construct_mxelem();

   logger->TextWithDate( "Starting to run a time evolution calculation", time( NULL ) );
   ( *logger ) << hashline;

   Ltensors    = new CTensorL **[ L - 1 ];
   LtensorsT   = new CTensorLT **[ L - 1 ];
   F0tensors   = new CTensorF0 ***[ L - 1 ];
   F0tensorsT  = new CTensorF0T ***[ L - 1 ];
   F1tensors   = new CTensorF1 ***[ L - 1 ];
   F1tensorsT  = new CTensorF1T ***[ L - 1 ];
   S0tensors   = new CTensorS0 ***[ L - 1 ];
   S0tensorsT  = new CTensorS0T ***[ L - 1 ];
   S1tensors   = new CTensorS1 ***[ L - 1 ];
   S1tensorsT  = new CTensorS1T ***[ L - 1 ];
   Atensors    = new CTensorOperator ***[ L - 1 ];
   AtensorsT   = new CTensorOperator ***[ L - 1 ];
   Btensors    = new CTensorOperator ***[ L - 1 ];
   BtensorsT   = new CTensorOperator ***[ L - 1 ];
   Ctensors    = new CTensorOperator ***[ L - 1 ];
   CtensorsT   = new CTensorOperator ***[ L - 1 ];
   Dtensors    = new CTensorOperator ***[ L - 1 ];
   DtensorsT   = new CTensorOperator ***[ L - 1 ];
   Qtensors    = new CTensorQ **[ L - 1 ];
   QtensorsT   = new CTensorQT **[ L - 1 ];
   Xtensors    = new CTensorX *[ L - 1 ];
   Otensors    = new CTensorO *[ L - 1 ];
   isAllocated = new int[ L - 1 ];

   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      isAllocated[ cnt ] = 0;
   }
}

CheMPS2::TimeTaylor::~TimeTaylor() {

   deleteAllBoundaryOperators();

   delete[] Ltensors;
   delete[] LtensorsT;
   delete[] F0tensors;
   delete[] F0tensorsT;
   delete[] F1tensors;
   delete[] F1tensorsT;
   delete[] S0tensors;
   delete[] S0tensorsT;
   delete[] S1tensors;
   delete[] S1tensorsT;
   delete[] Atensors;
   delete[] AtensorsT;
   delete[] Btensors;
   delete[] BtensorsT;
   delete[] Ctensors;
   delete[] CtensorsT;
   delete[] Dtensors;
   delete[] DtensorsT;
   delete[] Qtensors;
   delete[] QtensorsT;
   delete[] Xtensors;
   delete[] Otensors;
   delete[] isAllocated;

   logger->TextWithDate( "Finished to run a time evolution calculation", time( NULL ) );
   ( *logger ) << hashline;
}

void CheMPS2::TimeTaylor::updateMovingLeftSafe( const int cnt, CTensorT ** mpsUp, SyBookkeeper * bkUp, CTensorT ** mpsDown, SyBookkeeper * bkDown ) {
   if ( isAllocated[ cnt ] == 1 ) {
      deleteTensors( cnt, true );
      isAllocated[ cnt ] = 0;
   }
   if ( isAllocated[ cnt ] == 0 ) {
      allocateTensors( cnt, false, bkUp, bkDown );
      isAllocated[ cnt ] = 2;
   }
   updateMovingLeft( cnt, mpsUp, bkUp, mpsDown, bkDown );
}

void CheMPS2::TimeTaylor::updateMovingRightSafe( const int cnt, CTensorT ** mpsUp, SyBookkeeper * bkUp, CTensorT ** mpsDown, SyBookkeeper * bkDown ) {
   if ( isAllocated[ cnt ] == 2 ) {
      deleteTensors( cnt, false );
      isAllocated[ cnt ] = 0;
   }
   if ( isAllocated[ cnt ] == 0 ) {
      allocateTensors( cnt, true, bkUp, bkDown );
      isAllocated[ cnt ] = 1;
   }
   updateMovingRight( cnt, mpsUp, bkUp, mpsDown, bkDown );
}

void CheMPS2::TimeTaylor::deleteAllBoundaryOperators() {
   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      if ( isAllocated[ cnt ] == 1 ) {
         deleteTensors( cnt, true );
      }
      if ( isAllocated[ cnt ] == 2 ) {
         deleteTensors( cnt, false );
      }
      isAllocated[ cnt ] = 0;
   }
}

void CheMPS2::TimeTaylor::updateMovingLeft( const int index, CTensorT ** mpsUp, SyBookkeeper * bkUp, CTensorT ** mpsDown, SyBookkeeper * bkDown ) {

   const int dimL = std::max( bkUp->gMaxDimAtBound( index + 1 ), bkDown->gMaxDimAtBound( index + 1 ) );
   const int dimR = std::max( bkUp->gMaxDimAtBound( index + 2 ), bkDown->gMaxDimAtBound( index + 2 ) );

#pragma omp parallel
   {
      dcomplex * workmem = new dcomplex[ dimL * dimR ];
// Ltensors_MPSDT_MPS : all processes own all Ltensors_MPSDT_MPS
#pragma omp for schedule( static ) nowait
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         if ( cnt2 == 0 ) {
            if ( index == L - 2 ) {
               Ltensors[ index ][ cnt2 ]->create( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
               LtensorsT[ index ][ cnt2 ]->create( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
            } else {
               Ltensors[ index ][ cnt2 ]->create( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
               LtensorsT[ index ][ cnt2 ]->create( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
            }
         } else {
            Ltensors[ index ][ cnt2 ]->update( Ltensors[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            LtensorsT[ index ][ cnt2 ]->update( LtensorsT[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
         }
      }

      // Two-operator tensors : certain processes own certain two-operator tensors
      const int k1          = L - 1 - index;
      const int upperbound1 = ( k1 * ( k1 + 1 ) ) / 2;
      int result[ 2 ];
// After this parallel region, WAIT because F0,F1,S0,S1[ index ][ cnt2 ][ cnt3
// == 0 ] is required for the complementary operators
#pragma omp for schedule( static )
      for ( int global = 0; global < upperbound1; global++ ) {
         Special::invert_triangle_two( global, result );
         const int cnt2 = k1 - 1 - result[ 1 ];
         const int cnt3 = result[ 0 ];
         if ( cnt3 == 0 ) {
            if ( cnt2 == 0 ) {
               if ( index == L - 2 ) {
                  F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
                  F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
                  F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
                  F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
                  S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
                  S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
               } else {
                  F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
                  F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
                  F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
                  F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
                  S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
                  S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
               }
            } else {
               F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               S1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index + 1 ][ cnt2 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            }
         } else {
            F0tensors[ index ][ cnt2 ][ cnt3 ]->update( F0tensors[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            F0tensorsT[ index ][ cnt2 ][ cnt3 ]->update( F0tensorsT[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            F1tensors[ index ][ cnt2 ][ cnt3 ]->update( F1tensors[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            F1tensorsT[ index ][ cnt2 ][ cnt3 ]->update( F1tensorsT[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            S0tensors[ index ][ cnt2 ][ cnt3 ]->update( S0tensors[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            S0tensorsT[ index ][ cnt2 ][ cnt3 ]->update( S0tensorsT[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            if ( cnt2 > 0 ) {
               S1tensors[ index ][ cnt2 ][ cnt3 ]->update( S1tensors[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ]->update( S1tensorsT[ index + 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            }
         }
      }

      // Complementary two-operator tensors : certain processes own certain
      // complementary two-operator tensors
      const int k2          = index + 1;
      const int upperbound2 = ( k2 * ( k2 + 1 ) ) / 2;
#pragma omp for schedule( static ) nowait
      for ( int global = 0; global < upperbound2; global++ ) {
         Special::invert_triangle_two( global, result );
         const int cnt2       = k2 - 1 - result[ 1 ];
         const int cnt3       = result[ 0 ];
         const int siteindex1 = index - cnt3 - cnt2;
         const int siteindex2 = index - cnt3;
         const int irrep_prod = Irreps::directProd( bkUp->gIrrep( siteindex1 ), bkUp->gIrrep( siteindex2 ) );
         if ( index == L - 2 ) {
            Atensors[ index ][ cnt2 ][ cnt3 ]->clear();
            AtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]->clear();
               BtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]->clear();
            CtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            Dtensors[ index ][ cnt2 ][ cnt3 ]->clear();
            DtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
         } else {
            Atensors[ index ][ cnt2 ][ cnt3 ]->update( Atensors[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            AtensorsT[ index ][ cnt2 ][ cnt3 ]->update( AtensorsT[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]->update( Btensors[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
               BtensorsT[ index ][ cnt2 ][ cnt3 ]->update( BtensorsT[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]->update( Ctensors[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            CtensorsT[ index ][ cnt2 ][ cnt3 ]->update( CtensorsT[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            Dtensors[ index ][ cnt2 ][ cnt3 ]->update( Dtensors[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            DtensorsT[ index ][ cnt2 ][ cnt3 ]->update( DtensorsT[ index + 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
         }
         for ( int num = 0; num < L - index - 1; num++ ) {
            if ( irrep_prod ==
                 S0tensorsT[ index ][ num ][ 0 ]->get_irrep() ) { // Then the matrix elements are not 0 due to symm.
               double alpha = prob->gMxElement( siteindex1, siteindex2, index + 1, index + 1 + num );
               if ( ( cnt2 == 0 ) && ( num == 0 ) )
                  alpha *= 0.5;
               if ( ( cnt2 > 0 ) && ( num > 0 ) )
                  alpha += prob->gMxElement( siteindex1, siteindex2, index + 1 + num, index + 1 );
               Atensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S0tensors[ index ][ num ][ 0 ] );
               AtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S0tensorsT[ index ][ num ][ 0 ] );

               if ( ( num > 0 ) && ( cnt2 > 0 ) ) {
                  alpha = prob->gMxElement( siteindex1, siteindex2, index + 1, index + 1 + num ) -
                          prob->gMxElement( siteindex1, siteindex2, index + 1 + num, index + 1 );
                  Btensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S1tensors[ index ][ num ][ 0 ] );
                  BtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S1tensorsT[ index ][ num ][ 0 ] );
               }
               alpha = 2 * prob->gMxElement( siteindex1, index + 1, siteindex2, index + 1 + num ) -
                       prob->gMxElement( siteindex1, index + 1, index + 1 + num, siteindex2 );
               Ctensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F0tensors[ index ][ num ][ 0 ] );
               CtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F0tensorsT[ index ][ num ][ 0 ] );

               alpha = -prob->gMxElement( siteindex1, index + 1, index + 1 + num, siteindex2 ); // Second line for CtensorsT
               Dtensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F1tensors[ index ][ num ][ 0 ] );
               DtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F1tensorsT[ index ][ num ][ 0 ] );

               if ( num > 0 ) {
                  alpha = 2 * prob->gMxElement( siteindex1, index + 1 + num, siteindex2, index + 1 ) -
                          prob->gMxElement( siteindex1, index + 1 + num, index + 1, siteindex2 );
                  Ctensors[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCD( alpha, F0tensorsT[ index ][ num ][ 0 ] );
                  CtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCTDT( alpha, F0tensors[ index ][ num ][ 0 ] );

                  alpha = -prob->gMxElement( siteindex1, index + 1 + num, index + 1, siteindex2 ); // Second line for Ctensors_MPS_mpsUp
                  Dtensors[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCD( alpha, F1tensorsT[ index ][ num ][ 0 ] );
                  DtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCTDT( alpha, F1tensors[ index ][ num ][ 0 ] );
               }
            }
         }
      }
// QQtensors  : certain processes own certain QQtensors  --- You don't want to
// locally parallellize when sending and receiving buffers!
#pragma omp for schedule( static ) nowait
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         if ( index == L - 2 ) {
            Qtensors[ index ][ cnt2 ]->clear();
            QtensorsT[ index ][ cnt2 ]->clear();
            Qtensors[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
            QtensorsT[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index + 1 ], mpsDown[ index + 1 ], NULL, NULL );
         } else {
            dcomplex * workmemBIS = new dcomplex[ dimR * dimR ];
            Qtensors[ index ][ cnt2 ]->update( Qtensors[ index + 1 ][ cnt2 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            QtensorsT[ index ][ cnt2 ]->update( QtensorsT[ index + 1 ][ cnt2 + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmem );
            Qtensors[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ], workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsL( Ltensors[ index + 1 ], LtensorsT[ index + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsL( Ltensors[ index + 1 ], LtensorsT[ index + 1 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsAB( Atensors[ index + 1 ][ cnt2 + 1 ][ 0 ], Btensors[ index + 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsAB( AtensorsT[ index + 1 ][ cnt2 + 1 ][ 0 ], BtensorsT[ index + 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsCD( Ctensors[ index + 1 ][ cnt2 + 1 ][ 0 ], Dtensors[ index + 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsCD( CtensorsT[ index + 1 ][ cnt2 + 1 ][ 0 ], DtensorsT[ index + 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index + 1 ], mpsDown[ index + 1 ], workmemBIS, workmem );
            delete[] workmemBIS;
         }
      }

      delete[] workmem;
   }
   // Xtensors
   if ( index == L - 2 ) {
      Xtensors[ index ]->update( mpsUp[ index + 1 ], mpsDown[ index + 1 ] );
   } else {
      Xtensors[ index ]->update( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ],
                                 Ltensors[ index + 1 ], LtensorsT[ index + 1 ], Xtensors[ index + 1 ],
                                 Qtensors[ index + 1 ][ 0 ], QtensorsT[ index + 1 ][ 0 ],
                                 Atensors[ index + 1 ][ 0 ][ 0 ], AtensorsT[ index + 1 ][ 0 ][ 0 ],
                                 CtensorsT[ index + 1 ][ 0 ][ 0 ], DtensorsT[ index + 1 ][ 0 ][ 0 ] );
   }

   // Otensors
   if ( index == L - 2 ) {
      Otensors[ index ]->create( mpsUp[ index + 1 ], mpsDown[ index + 1 ] );
   } else {
      Otensors[ index ]->update_ownmem( mpsUp[ index + 1 ], mpsDown[ index + 1 ], Otensors[ index + 1 ] );
   }
}

void CheMPS2::TimeTaylor::updateMovingRight( const int index, CTensorT ** mpsUp, SyBookkeeper * bkUp, CTensorT ** mpsDown, SyBookkeeper * bkDown ) {

   const int dimL = std::max( bkUp->gMaxDimAtBound( index ), bkDown->gMaxDimAtBound( index ) );
   const int dimR = std::max( bkUp->gMaxDimAtBound( index + 1 ), bkDown->gMaxDimAtBound( index + 1 ) );

#pragma omp parallel
   {
      dcomplex * workmem = new dcomplex[ dimL * dimR ];

// Ltensors
#pragma omp for schedule( static ) nowait
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         if ( cnt2 == 0 ) {
            if ( index == 0 ) {
               Ltensors[ index ][ cnt2 ]->create( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
               LtensorsT[ index ][ cnt2 ]->create( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
            } else {
               Ltensors[ index ][ cnt2 ]->create( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
               LtensorsT[ index ][ cnt2 ]->create( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
            }
         } else {
            Ltensors[ index ][ cnt2 ]->update( Ltensors[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            LtensorsT[ index ][ cnt2 ]->update( LtensorsT[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
         }
      }

      // Two-operator tensors : certain processes own certain two-operator tensors
      const int k1          = index + 1;
      const int upperbound1 = ( k1 * ( k1 + 1 ) ) / 2;
      int result[ 2 ];
// After this parallel region, WAIT because F0,F1,S0,S1[ index ][ cnt2 ][ cnt3
// == 0 ] is required for the complementary operators
#pragma omp for schedule( static )
      for ( int global = 0; global < upperbound1; global++ ) {
         Special::invert_triangle_two( global, result );
         const int cnt2 = index - result[ 1 ];
         const int cnt3 = result[ 0 ];
         if ( cnt3 == 0 ) { // Every MPI process owns the Operator[ index ][ cnt2 ][
            // cnt3 == 0 ]
            if ( cnt2 == 0 ) {
               if ( index == 0 ) {
                  F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
                  F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
                  S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
                  F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
                  F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
                  S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
               } else {
                  F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
                  F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
                  S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
                  F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
                  F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
                  S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
               }
               // // S1[ index ][ 0 ][ cnt3 ] doesn't exist
            } else {
               F0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               F1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               S0tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               S1tensors[ index ][ cnt2 ][ cnt3 ]->makenew( Ltensors[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               F0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               F1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               S0tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ]->makenew( LtensorsT[ index - 1 ][ cnt2 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            }
         } else {
            F0tensors[ index ][ cnt2 ][ cnt3 ]->update( F0tensors[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            F1tensors[ index ][ cnt2 ][ cnt3 ]->update( F1tensors[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            S0tensors[ index ][ cnt2 ][ cnt3 ]->update( S0tensors[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            F0tensorsT[ index ][ cnt2 ][ cnt3 ]->update( F0tensorsT[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            F1tensorsT[ index ][ cnt2 ][ cnt3 ]->update( F1tensorsT[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            S0tensorsT[ index ][ cnt2 ][ cnt3 ]->update( S0tensorsT[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            if ( cnt2 > 0 ) {
               S1tensors[ index ][ cnt2 ][ cnt3 ]->update( S1tensors[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ]->update( S1tensorsT[ index - 1 ][ cnt2 ][ cnt3 - 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            }
         }
      }

      // Complementary two-operator tensors : certain processes own certain
      // complementary two-operator tensors
      const int k2          = L - 1 - index;
      const int upperbound2 = ( k2 * ( k2 + 1 ) ) / 2;
#pragma omp for schedule( static ) nowait
      for ( int global = 0; global < upperbound2; global++ ) {
         Special::invert_triangle_two( global, result );
         const int cnt2       = k2 - 1 - result[ 1 ];
         const int cnt3       = result[ 0 ];
         const int siteindex1 = index + 1 + cnt3;
         const int siteindex2 = index + 1 + cnt2 + cnt3;
         const int irrep_prod = CheMPS2::Irreps::directProd( bkUp->gIrrep( siteindex1 ), bkUp->gIrrep( siteindex2 ) );
         if ( index == 0 ) {
            Atensors[ index ][ cnt2 ][ cnt3 ]->clear();
            AtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]->clear();
               BtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]->clear();
            CtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
            Dtensors[ index ][ cnt2 ][ cnt3 ]->clear();
            DtensorsT[ index ][ cnt2 ][ cnt3 ]->clear();
         } else {
            Atensors[ index ][ cnt2 ][ cnt3 ]->update( Atensors[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            AtensorsT[ index ][ cnt2 ][ cnt3 ]->update( AtensorsT[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]->update( Btensors[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
               BtensorsT[ index ][ cnt2 ][ cnt3 ]->update( BtensorsT[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]->update( Ctensors[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            CtensorsT[ index ][ cnt2 ][ cnt3 ]->update( CtensorsT[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            Dtensors[ index ][ cnt2 ][ cnt3 ]->update( Dtensors[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            DtensorsT[ index ][ cnt2 ][ cnt3 ]->update( DtensorsT[ index - 1 ][ cnt2 ][ cnt3 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
         }

         for ( int num = 0; num < index + 1; num++ ) {
            if ( irrep_prod == S0tensorsT[ index ][ num ][ 0 ]->get_irrep() ) { // Then the matrix elements are not 0 due to symm.
               double alpha = prob->gMxElement( index - num, index, siteindex1, siteindex2 );
               if ( ( cnt2 == 0 ) && ( num == 0 ) ) {
                  alpha *= 0.5;
               }
               if ( ( cnt2 > 0 ) && ( num > 0 ) ) {
                  alpha += prob->gMxElement( index - num, index, siteindex2, siteindex1 );
               }
               Atensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S0tensors[ index ][ num ][ 0 ] );
               AtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S0tensorsT[ index ][ num ][ 0 ] );

               if ( ( num > 0 ) && ( cnt2 > 0 ) ) {
                  alpha =
                      prob->gMxElement( index - num, index, siteindex1, siteindex2 ) -
                      prob->gMxElement( index - num, index, siteindex2, siteindex1 );
                  Btensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S1tensors[ index ][ num ][ 0 ] );
                  BtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, S1tensorsT[ index ][ num ][ 0 ] );
               }

               alpha = 2 * prob->gMxElement( index - num, siteindex1, index, siteindex2 ) -
                       prob->gMxElement( index - num, siteindex1, siteindex2, index );
               Ctensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F0tensors[ index ][ num ][ 0 ] );
               CtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F0tensorsT[ index ][ num ][ 0 ] );

               alpha = -prob->gMxElement( index - num, siteindex1, siteindex2, index ); // Second line for CtensorsT
               Dtensors[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F1tensors[ index ][ num ][ 0 ] );
               DtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy( alpha, F1tensorsT[ index ][ num ][ 0 ] );
               if ( num > 0 ) {
                  alpha = 2 * prob->gMxElement( index - num, siteindex2, index, siteindex1 ) -
                          prob->gMxElement( index - num, siteindex2, siteindex1, index );

                  Ctensors[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCD( alpha, F0tensorsT[ index ][ num ][ 0 ] );
                  CtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCTDT( alpha, F0tensors[ index ][ num ][ 0 ] );

                  alpha = -prob->gMxElement( index - num, siteindex2, siteindex1, index ); // Second line for CtensorsT
                  Dtensors[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCD( alpha, F1tensorsT[ index ][ num ][ 0 ] );
                  DtensorsT[ index ][ cnt2 ][ cnt3 ]->zaxpy_tensorCTDT( alpha, F1tensors[ index ][ num ][ 0 ] );
               }
            }
         }
      }

// QQtensors_mpsUp_MPS : certain processes own certain QQtensors_mpsUp_MPS ---
// You don't want to locally parallellize when sending and receiving buffers!
#pragma omp for schedule( static ) nowait
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         if ( index == 0 ) {
            Qtensors[ index ][ cnt2 ]->clear();
            QtensorsT[ index ][ cnt2 ]->clear();
            Qtensors[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
            QtensorsT[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index ], mpsDown[ index ], NULL, NULL );
         } else {
            dcomplex * workmemBIS = new dcomplex[ dimL * dimL ];
            Qtensors[ index ][ cnt2 ]->update( Qtensors[ index - 1 ][ cnt2 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            QtensorsT[ index ][ cnt2 ]->update( QtensorsT[ index - 1 ][ cnt2 + 1 ], mpsUp[ index ], mpsDown[ index ], workmem );
            Qtensors[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermSimple( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsL( Ltensors[ index - 1 ], LtensorsT[ index - 1 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsL( Ltensors[ index - 1 ], LtensorsT[ index - 1 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsAB( Atensors[ index - 1 ][ cnt2 + 1 ][ 0 ], Btensors[ index - 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsAB( AtensorsT[ index - 1 ][ cnt2 + 1 ][ 0 ], BtensorsT[ index - 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            Qtensors[ index ][ cnt2 ]->AddTermsCD( Ctensors[ index - 1 ][ cnt2 + 1 ][ 0 ], Dtensors[ index - 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            QtensorsT[ index ][ cnt2 ]->AddTermsCD( CtensorsT[ index - 1 ][ cnt2 + 1 ][ 0 ], DtensorsT[ index - 1 ][ cnt2 + 1 ][ 0 ], mpsUp[ index ], mpsDown[ index ], workmemBIS, workmem );
            delete[] workmemBIS;
         }
      }

      delete[] workmem;
   }

   // Xtensors
   if ( index == 0 ) {
      Xtensors[ index ]->update( mpsUp[ index ], mpsDown[ index ] );
   } else {
      Xtensors[ index ]->update( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ], Ltensors[ index - 1 ],
                                 LtensorsT[ index - 1 ], Xtensors[ index - 1 ], Qtensors[ index - 1 ][ 0 ],
                                 QtensorsT[ index - 1 ][ 0 ], Atensors[ index - 1 ][ 0 ][ 0 ],
                                 AtensorsT[ index - 1 ][ 0 ][ 0 ], CtensorsT[ index - 1 ][ 0 ][ 0 ],
                                 DtensorsT[ index - 1 ][ 0 ][ 0 ] );
   }

   // Otensors
   if ( index == 0 ) {
      Otensors[ index ]->create( mpsUp[ index ], mpsDown[ index ] );
   } else {
      Otensors[ index ]->update_ownmem( mpsUp[ index ], mpsDown[ index ], Otensors[ index - 1 ] );
   }
}

void CheMPS2::TimeTaylor::allocateTensors( const int index, const bool movingRight, SyBookkeeper * bkUp, SyBookkeeper * bkDown ) {

   if ( movingRight ) {
      // Ltensors
      Ltensors[ index ]  = new CTensorL *[ index + 1 ];
      LtensorsT[ index ] = new CTensorLT *[ index + 1 ];
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         Ltensors[ index ][ cnt2 ]  = new CTensorL( index + 1, bkUp->gIrrep( index - cnt2 ), movingRight, bkUp, bkDown );
         LtensorsT[ index ][ cnt2 ] = new CTensorLT( index + 1, bkUp->gIrrep( index - cnt2 ), movingRight, bkUp, bkDown );
      }

      // Two-operator tensors : certain processes own certain two-operator tensors
      // To right: F0tens[ cnt][ cnt2 ][ cnt3 ] = operators on sites cnt-cnt3-cnt2
      // and cnt-cnt3; at boundary cnt+1
      F0tensors[ index ]  = new CTensorF0 **[ index + 1 ];
      F0tensorsT[ index ] = new CTensorF0T **[ index + 1 ];
      F1tensors[ index ]  = new CTensorF1 **[ index + 1 ];
      F1tensorsT[ index ] = new CTensorF1T **[ index + 1 ];
      S0tensors[ index ]  = new CTensorS0 **[ index + 1 ];
      S0tensorsT[ index ] = new CTensorS0T **[ index + 1 ];
      S1tensors[ index ]  = new CTensorS1 **[ index + 1 ];
      S1tensorsT[ index ] = new CTensorS1T **[ index + 1 ];
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         F0tensors[ index ][ cnt2 ]  = new CTensorF0 *[ index - cnt2 + 1 ];
         F0tensorsT[ index ][ cnt2 ] = new CTensorF0T *[ index - cnt2 + 1 ];
         F1tensors[ index ][ cnt2 ]  = new CTensorF1 *[ index - cnt2 + 1 ];
         F1tensorsT[ index ][ cnt2 ] = new CTensorF1T *[ index - cnt2 + 1 ];
         S0tensors[ index ][ cnt2 ]  = new CTensorS0 *[ index - cnt2 + 1 ];
         S0tensorsT[ index ][ cnt2 ] = new CTensorS0T *[ index - cnt2 + 1 ];
         if ( cnt2 > 0 ) {
            S1tensors[ index ][ cnt2 ] = new CTensorS1 *[ index - cnt2 + 1 ];
         }
         if ( cnt2 > 0 ) {
            S1tensorsT[ index ][ cnt2 ] = new CTensorS1T *[ index - cnt2 + 1 ];
         }
         for ( int cnt3 = 0; cnt3 < index - cnt2 + 1; cnt3++ ) {
            const int Iprod = CheMPS2::Irreps::directProd( bkUp->gIrrep( index - cnt2 - cnt3 ), bkUp->gIrrep( index - cnt3 ) );

            F0tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorF0( index + 1, Iprod, movingRight, bkUp, bkDown );
            F0tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorF0T( index + 1, Iprod, movingRight, bkUp, bkDown );
            F1tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorF1( index + 1, Iprod, movingRight, bkUp, bkDown );
            F1tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorF1T( index + 1, Iprod, movingRight, bkUp, bkDown );
            S0tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorS0( index + 1, Iprod, movingRight, bkUp, bkDown );
            S0tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorS0T( index + 1, Iprod, movingRight, bkUp, bkDown );
            if ( cnt2 > 0 ) {
               S1tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorS1( index + 1, Iprod, movingRight, bkUp, bkDown );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorS1T( index + 1, Iprod, movingRight, bkUp, bkDown );
            }
         }
      }

      // Complementary two-operator tensors : certain processes own certain
      // complementary two-operator tensors
      // To right: Atens[ cnt][ cnt2 ][ cnt3 ] = operators on sites cnt+1+cnt3 and
      // cnt+1+cnt2+cnt3; at boundary cnt+1
      Atensors[ index ]  = new CTensorOperator **[ L - 1 - index ];
      AtensorsT[ index ] = new CTensorOperator **[ L - 1 - index ];
      Btensors[ index ]  = new CTensorOperator **[ L - 1 - index ];
      BtensorsT[ index ] = new CTensorOperator **[ L - 1 - index ];
      Ctensors[ index ]  = new CTensorOperator **[ L - 1 - index ];
      CtensorsT[ index ] = new CTensorOperator **[ L - 1 - index ];
      Dtensors[ index ]  = new CTensorOperator **[ L - 1 - index ];
      DtensorsT[ index ] = new CTensorOperator **[ L - 1 - index ];
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         Atensors[ index ][ cnt2 ]  = new CTensorOperator *[ L - 1 - index - cnt2 ];
         AtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ L - 1 - index - cnt2 ];
         if ( cnt2 > 0 ) {
            Btensors[ index ][ cnt2 ]  = new CTensorOperator *[ L - 1 - index - cnt2 ];
            BtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ L - 1 - index - cnt2 ];
         }
         Ctensors[ index ][ cnt2 ]  = new CTensorOperator *[ L - 1 - index - cnt2 ];
         CtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ L - 1 - index - cnt2 ];
         Dtensors[ index ][ cnt2 ]  = new CTensorOperator *[ L - 1 - index - cnt2 ];
         DtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ L - 1 - index - cnt2 ];
         for ( int cnt3 = 0; cnt3 < L - 1 - index - cnt2; cnt3++ ) {
            const int Idiff = CheMPS2::Irreps::directProd( bkUp->gIrrep( index + 1 + cnt2 + cnt3 ), bkUp->gIrrep( index + 1 + cnt3 ) );

            Atensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 0, 2, Idiff, movingRight, true, false, bkUp, bkDown );
            AtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 0, -2, Idiff, movingRight, false, false, bkUp, bkDown );
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 2, 2, Idiff, movingRight, true, false, bkUp, bkDown );
               BtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 2, -2, Idiff, movingRight, false, false, bkUp, bkDown );
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 0, 0, Idiff, movingRight, true, false, bkUp, bkDown );
            CtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 0, 0, Idiff, movingRight, false, false, bkUp, bkDown );
            Dtensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 2, 0, Idiff, movingRight, movingRight, false, bkUp, bkDown );
            DtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 2, 0, Idiff, movingRight, !movingRight, false, bkUp, bkDown );
         }
      }

      // QQtensors_MPSDT_MPS
      // To right: Qtens[ cnt][ cnt2 ] = operator on site cnt+1+cnt2; at boundary cnt+1
      Qtensors[ index ]  = new CTensorQ *[ L - 1 - index ];
      QtensorsT[ index ] = new CTensorQT *[ L - 1 - index ];
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         Qtensors[ index ][ cnt2 ]  = new CTensorQ( index + 1, bkUp->gIrrep( index + 1 + cnt2 ), movingRight, bkUp, bkDown, prob, index + 1 + cnt2 );
         QtensorsT[ index ][ cnt2 ] = new CTensorQT( index + 1, bkUp->gIrrep( index + 1 + cnt2 ), movingRight, bkUp, bkDown, prob, index + 1 + cnt2 );
      }

      // Xtensors : a certain process owns the Xtensors
      Xtensors[ index ] = new CTensorX( index + 1, movingRight, bkUp, bkDown, prob );

      // Otensors :
      Otensors[ index ] = new CTensorO( index + 1, movingRight, bkUp, bkDown );
   } else {
      Ltensors[ index ]  = new CTensorL *[ L - 1 - index ];
      LtensorsT[ index ] = new CTensorLT *[ L - 1 - index ];
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         Ltensors[ index ][ cnt2 ]  = new CTensorL( index + 1, bkUp->gIrrep( index + 1 + cnt2 ), movingRight, bkUp, bkDown );
         LtensorsT[ index ][ cnt2 ] = new CTensorLT( index + 1, bkUp->gIrrep( index + 1 + cnt2 ), movingRight, bkUp, bkDown );
      }

      // Two-operator tensors : certain processes own certain two-operator tensors
      // To left: F0tens[ cnt][ cnt2 ][ cnt3 ] = operators on sites cnt+1+cnt3 and
      // cnt+1+cnt3+cnt2; at boundary cnt+1
      F0tensors[ index ]  = new CTensorF0 **[ L - 1 - index ];
      F0tensorsT[ index ] = new CTensorF0T **[ L - 1 - index ];
      F1tensors[ index ]  = new CTensorF1 **[ L - 1 - index ];
      F1tensorsT[ index ] = new CTensorF1T **[ L - 1 - index ];
      S0tensors[ index ]  = new CTensorS0 **[ L - 1 - index ];
      S0tensorsT[ index ] = new CTensorS0T **[ L - 1 - index ];
      S1tensors[ index ]  = new CTensorS1 **[ L - 1 - index ];
      S1tensorsT[ index ] = new CTensorS1T **[ L - 1 - index ];
      for ( int cnt2 = 0; cnt2 < L - 1 - index; cnt2++ ) {
         F0tensors[ index ][ cnt2 ]  = new CTensorF0 *[ L - 1 - index - cnt2 ];
         F0tensorsT[ index ][ cnt2 ] = new CTensorF0T *[ L - 1 - index - cnt2 ];
         F1tensors[ index ][ cnt2 ]  = new CTensorF1 *[ L - 1 - index - cnt2 ];
         F1tensorsT[ index ][ cnt2 ] = new CTensorF1T *[ L - 1 - index - cnt2 ];
         S0tensors[ index ][ cnt2 ]  = new CTensorS0 *[ L - 1 - index - cnt2 ];
         S0tensorsT[ index ][ cnt2 ] = new CTensorS0T *[ L - 1 - index - cnt2 ];
         if ( cnt2 > 0 ) {
            S1tensors[ index ][ cnt2 ]  = new CTensorS1 *[ L - 1 - index - cnt2 ];
            S1tensorsT[ index ][ cnt2 ] = new CTensorS1T *[ L - 1 - index - cnt2 ];
         }
         for ( int cnt3 = 0; cnt3 < L - 1 - index - cnt2; cnt3++ ) {
            const int Iprod = Irreps::directProd( bkUp->gIrrep( index + 1 + cnt3 ), bkUp->gIrrep( index + 1 + cnt2 + cnt3 ) );

            F0tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorF0( index + 1, Iprod, movingRight, bkUp, bkDown );
            F0tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorF0T( index + 1, Iprod, movingRight, bkUp, bkDown );
            F1tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorF1( index + 1, Iprod, movingRight, bkUp, bkDown );
            F1tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorF1T( index + 1, Iprod, movingRight, bkUp, bkDown );
            S0tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorS0( index + 1, Iprod, movingRight, bkUp, bkDown );
            S0tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorS0T( index + 1, Iprod, movingRight, bkUp, bkDown );
            if ( cnt2 > 0 ) {
               S1tensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorS1( index + 1, Iprod, movingRight, bkUp, bkDown );
               S1tensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorS1T( index + 1, Iprod, movingRight, bkUp, bkDown );
            }
         }
      }

      // Complementary two-operator tensors : certain processes own certain
      // complementary two-operator tensors
      // To left: Atens[ cnt][ cnt2 ][ cnt3 ] = operators on sites cnt-cnt2-cnt3
      // and cnt-cnt3; at boundary cnt+1
      Atensors[ index ]  = new CTensorOperator **[ index + 1 ];
      AtensorsT[ index ] = new CTensorOperator **[ index + 1 ];
      Btensors[ index ]  = new CTensorOperator **[ index + 1 ];
      BtensorsT[ index ] = new CTensorOperator **[ index + 1 ];
      Ctensors[ index ]  = new CTensorOperator **[ index + 1 ];
      CtensorsT[ index ] = new CTensorOperator **[ index + 1 ];
      Dtensors[ index ]  = new CTensorOperator **[ index + 1 ];
      DtensorsT[ index ] = new CTensorOperator **[ index + 1 ];
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         Atensors[ index ][ cnt2 ]  = new CTensorOperator *[ index + 1 - cnt2 ];
         AtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ index + 1 - cnt2 ];
         if ( cnt2 > 0 ) {
            Btensors[ index ][ cnt2 ]  = new CTensorOperator *[ index + 1 - cnt2 ];
            BtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ index + 1 - cnt2 ];
         }
         Ctensors[ index ][ cnt2 ]  = new CTensorOperator *[ index + 1 - cnt2 ];
         CtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ index + 1 - cnt2 ];
         Dtensors[ index ][ cnt2 ]  = new CTensorOperator *[ index + 1 - cnt2 ];
         DtensorsT[ index ][ cnt2 ] = new CTensorOperator *[ index + 1 - cnt2 ];
         for ( int cnt3 = 0; cnt3 < index + 1 - cnt2; cnt3++ ) {
            const int Idiff = CheMPS2::Irreps::directProd( bkUp->gIrrep( index - cnt2 - cnt3 ), bkUp->gIrrep( index - cnt3 ) );

            Atensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 0, 2, Idiff, movingRight, true, false, bkUp, bkDown );
            AtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 0, -2, Idiff, movingRight, false, false, bkUp, bkDown );
            if ( cnt2 > 0 ) {
               Btensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 2, 2, Idiff, movingRight, true, false, bkUp, bkDown );
               BtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 2, -2, Idiff, movingRight, false, false, bkUp, bkDown );
            }
            Ctensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 0, 0, Idiff, movingRight, true, false, bkUp, bkDown );
            CtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 0, 0, Idiff, movingRight, false, false, bkUp, bkDown );
            Dtensors[ index ][ cnt2 ][ cnt3 ]  = new CTensorOperator( index + 1, 2, 0, Idiff, movingRight, movingRight, false, bkUp, bkDown );
            DtensorsT[ index ][ cnt2 ][ cnt3 ] = new CTensorOperator( index + 1, 2, 0, Idiff, movingRight, !movingRight, false, bkUp, bkDown );
         }
      }

      // QQtensors  : certain processes own certain QQtensors
      // To left: Qtens[ cnt][ cnt2 ] = operator on site cnt-cnt2; at boundary
      // cnt+1
      Qtensors[ index ]  = new CTensorQ *[ index + 1 ];
      QtensorsT[ index ] = new CTensorQT *[ index + 1 ];
      for ( int cnt2 = 0; cnt2 < index + 1; cnt2++ ) {
         Qtensors[ index ][ cnt2 ]  = new CTensorQ( index + 1, bkUp->gIrrep( index - cnt2 ), movingRight, bkUp, bkDown, prob, index - cnt2 );
         QtensorsT[ index ][ cnt2 ] = new CTensorQT( index + 1, bkUp->gIrrep( index - cnt2 ), movingRight, bkUp, bkDown, prob, index - cnt2 );
      }

      // Xtensors : a certain process owns the Xtensors
      Xtensors[ index ] = new CTensorX( index + 1, movingRight, bkUp, bkDown, prob );

      // Otensors :
      Otensors[ index ] = new CTensorO( index + 1, movingRight, bkUp, bkDown );
   }
}

void CheMPS2::TimeTaylor::deleteTensors( const int index, const bool movingRight ) {
   const int Nbound = movingRight ? index + 1 : L - 1 - index;
   const int Cbound = movingRight ? L - 1 - index : index + 1;

   // Ltensors  : all processes own all Ltensors_MPSDT_MPS
   for ( int cnt2 = 0; cnt2 < Nbound; cnt2++ ) {
      delete Ltensors[ index ][ cnt2 ];
      delete LtensorsT[ index ][ cnt2 ];
   }
   delete[] Ltensors[ index ];
   delete[] LtensorsT[ index ];

   // Two-operator tensors : certain processes own certain two-operator tensors
   for ( int cnt2 = 0; cnt2 < Nbound; cnt2++ ) {
      for ( int cnt3 = 0; cnt3 < Nbound - cnt2; cnt3++ ) {
         delete F0tensors[ index ][ cnt2 ][ cnt3 ];
         delete F0tensorsT[ index ][ cnt2 ][ cnt3 ];
         delete F1tensors[ index ][ cnt2 ][ cnt3 ];
         delete F1tensorsT[ index ][ cnt2 ][ cnt3 ];
         delete S0tensors[ index ][ cnt2 ][ cnt3 ];
         delete S0tensorsT[ index ][ cnt2 ][ cnt3 ];
         if ( cnt2 > 0 ) {
            delete S1tensors[ index ][ cnt2 ][ cnt3 ];
            delete S1tensorsT[ index ][ cnt2 ][ cnt3 ];
         }
      }
      delete[] F0tensors[ index ][ cnt2 ];
      delete[] F0tensorsT[ index ][ cnt2 ];
      delete[] F1tensors[ index ][ cnt2 ];
      delete[] F1tensorsT[ index ][ cnt2 ];
      delete[] S0tensors[ index ][ cnt2 ];
      delete[] S0tensorsT[ index ][ cnt2 ];
      if ( cnt2 > 0 ) {
         delete[] S1tensors[ index ][ cnt2 ];
         delete[] S1tensorsT[ index ][ cnt2 ];
      }
   }
   delete[] F0tensors[ index ];
   delete[] F0tensorsT[ index ];
   delete[] F1tensors[ index ];
   delete[] F1tensorsT[ index ];
   delete[] S0tensors[ index ];
   delete[] S0tensorsT[ index ];
   delete[] S1tensors[ index ];
   delete[] S1tensorsT[ index ];

   // Complementary two-operator tensors : certain processes own certain complementary two-operator tensors
   for ( int cnt2 = 0; cnt2 < Cbound; cnt2++ ) {
      for ( int cnt3 = 0; cnt3 < Cbound - cnt2; cnt3++ ) {
         delete Atensors[ index ][ cnt2 ][ cnt3 ];
         delete AtensorsT[ index ][ cnt2 ][ cnt3 ];
         if ( cnt2 > 0 ) {
            delete Btensors[ index ][ cnt2 ][ cnt3 ];
            delete BtensorsT[ index ][ cnt2 ][ cnt3 ];
         }
         delete Ctensors[ index ][ cnt2 ][ cnt3 ];
         delete CtensorsT[ index ][ cnt2 ][ cnt3 ];
         delete Dtensors[ index ][ cnt2 ][ cnt3 ];
         delete DtensorsT[ index ][ cnt2 ][ cnt3 ];
      }
      delete[] Atensors[ index ][ cnt2 ];
      delete[] AtensorsT[ index ][ cnt2 ];
      if ( cnt2 > 0 ) {
         delete[] Btensors[ index ][ cnt2 ];
         delete[] BtensorsT[ index ][ cnt2 ];
      }
      delete[] Ctensors[ index ][ cnt2 ];
      delete[] CtensorsT[ index ][ cnt2 ];
      delete[] Dtensors[ index ][ cnt2 ];
      delete[] DtensorsT[ index ][ cnt2 ];
   }
   delete[] Atensors[ index ];
   delete[] AtensorsT[ index ];
   delete[] Btensors[ index ];
   delete[] BtensorsT[ index ];
   delete[] Ctensors[ index ];
   delete[] CtensorsT[ index ];
   delete[] Dtensors[ index ];
   delete[] DtensorsT[ index ];

   // QQtensors_MPSDT_MPS : certain processes own certain QQtensors_MPSDT_MPS
   for ( int cnt2 = 0; cnt2 < Cbound; cnt2++ ) {
      delete Qtensors[ index ][ cnt2 ];
      delete QtensorsT[ index ][ cnt2 ];
   }
   delete[] Qtensors[ index ];
   delete[] QtensorsT[ index ];

   // Xtensors
   delete Xtensors[ index ];

   // Otensors
   delete Otensors[ index ];
}

void CheMPS2::TimeTaylor::fitApplyH_1site( CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut, const int nSweeps ) {

   for ( int index = 0; index < L - 1; index++ ) {
      left_normalize( mpsOut[ index ], mpsOut[ index + 1 ] );
   }
   left_normalize( mpsOut[ L - 1 ], NULL );

   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      updateMovingRightSafe( cnt, mpsOut, bkOut, mpsIn, bkIn );
   }

   for ( int i = 0; i < nSweeps; ++i ) {
      for ( int site = L - 1; site > 0; site-- ) {

         CHeffNS_1S * heff = new CHeffNS_1S( bkOut, bkIn, prob );
         heff->Apply( mpsIn[ site ], mpsOut[ site ],
                      Ltensors, LtensorsT,
                      Atensors, AtensorsT,
                      Btensors, BtensorsT,
                      Ctensors, CtensorsT,
                      Dtensors, DtensorsT,
                      S0tensors, S0tensorsT,
                      S1tensors, S1tensorsT,
                      F0tensors, F0tensorsT,
                      F1tensors, F1tensorsT,
                      Qtensors, QtensorsT,
                      Xtensors, Otensors, false );
         delete heff;
         right_normalize( mpsOut[ site - 1 ], mpsOut[ site ] );
         updateMovingLeftSafe( site - 1, mpsOut, bkOut, mpsIn, bkIn );
      }

      for ( int site = 0; site < L - 1; site++ ) {

         CHeffNS_1S * heff = new CHeffNS_1S( bkOut, bkIn, prob );
         heff->Apply( mpsIn[ site ], mpsOut[ site ],
                      Ltensors, LtensorsT,
                      Atensors, AtensorsT,
                      Btensors, BtensorsT,
                      Ctensors, CtensorsT,
                      Dtensors, DtensorsT,
                      S0tensors, S0tensorsT,
                      S1tensors, S1tensorsT,
                      F0tensors, F0tensorsT,
                      F1tensors, F1tensorsT,
                      Qtensors, QtensorsT,
                      Xtensors, Otensors, true );
         delete heff;

         left_normalize( mpsOut[ site ], mpsOut[ site + 1 ] );
         updateMovingRightSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }
   }
}

void CheMPS2::TimeTaylor::fitApplyH( dcomplex factor, const double offset, CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut, const int nSweeps, const int D, const double cut_off ) {

   for ( int index = 0; index < L - 1; index++ ) {
      left_normalize( mpsOut[ index ], mpsOut[ index + 1 ] );
   }
   left_normalize( mpsOut[ L - 1 ], NULL );

   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      updateMovingRightSafe( cnt, mpsOut, bkOut, mpsIn, bkIn );
   }
   for ( int i = 0; i < nSweeps; ++i ) {
      for ( int site = L - 2; site > 0; site-- ) {

         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? Otensors[ site + 1 ] : NULL;

         CSobject * denSB = new CSobject( site, bkIn );
         denSB->Join( mpsIn[ site ], mpsIn[ site + 1 ] );

         CSobject * denPB = new CSobject( site, bkOut );

         CHeffNS * heff = new CHeffNS( bkOut, bkIn, prob, offset );
         heff->Apply( denSB, denPB, Ltensors, LtensorsT, Atensors, AtensorsT,
                      Btensors, BtensorsT, Ctensors, CtensorsT, Dtensors, DtensorsT,
                      S0tensors, S0tensorsT, S1tensors, S1tensorsT, F0tensors,
                      F0tensorsT, F1tensors, F1tensorsT, Qtensors, QtensorsT,
                      Xtensors, leftOverlapA, rightOverlapA );

         double disc = denPB->Split( mpsOut[ site ], mpsOut[ site + 1 ], D, cut_off, false, false );
         delete heff;
         delete denPB;
         delete denSB;

         updateMovingLeftSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }

      for ( int site = 0; site < L - 2; site++ ) {
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? Otensors[ site + 1 ] : NULL;

         CSobject * denSB = new CSobject( site, bkIn );
         denSB->Join( mpsIn[ site ], mpsIn[ site + 1 ] );

         CSobject * denPB = new CSobject( site, bkOut );

         CHeffNS * heff = new CHeffNS( bkOut, bkIn, prob, offset );
         heff->Apply( denSB, denPB, Ltensors, LtensorsT, Atensors, AtensorsT,
                      Btensors, BtensorsT, Ctensors, CtensorsT, Dtensors, DtensorsT,
                      S0tensors, S0tensorsT, S1tensors, S1tensorsT, F0tensors,
                      F0tensorsT, F1tensors, F1tensorsT, Qtensors, QtensorsT,
                      Xtensors, leftOverlapA, rightOverlapA );

         double disc = denPB->Split( mpsOut[ site ], mpsOut[ site + 1 ], D, cut_off, true, false );
         delete heff;
         delete denPB;
         delete denSB;

         updateMovingRightSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }
   }
}

void CheMPS2::TimeTaylor::fitAddMPS( dcomplex factor,
                                     CTensorT ** mpsA, SyBookkeeper * bkA,
                                     CTensorT ** mpsB, SyBookkeeper * bkB,
                                     CTensorT ** mpsOut, SyBookkeeper * bkOut,
                                     const int nSweeps, const int D, const double cut_off ) {

   for ( int index = 0; index < L - 1; index++ ) {
      left_normalize( mpsOut[ index ], mpsOut[ index + 1 ] );
   }
   left_normalize( mpsOut[ L - 1 ], NULL );

   CTensorO ** OtensorsA = new CTensorO *[ L - 1 ];
   CTensorO ** OtensorsB = new CTensorO *[ L - 1 ];

   for ( int index = 0; index < L - 1; index++ ) {
      OtensorsA[ index ] = new CTensorO( index + 1, true, bkOut, bkA );
      OtensorsB[ index ] = new CTensorO( index + 1, true, bkOut, bkB );

      // Otensors
      if ( index == 0 ) {
         OtensorsA[ index ]->create( mpsOut[ index ], mpsA[ index ] );
         OtensorsB[ index ]->create( mpsOut[ index ], mpsB[ index ] );
      } else {
         OtensorsA[ index ]->update_ownmem( mpsOut[ index ], mpsA[ index ], OtensorsA[ index - 1 ] );
         OtensorsB[ index ]->update_ownmem( mpsOut[ index ], mpsB[ index ], OtensorsB[ index - 1 ] );
      }
   }

   for ( int i = 0; i < nSweeps; ++i ) {
      for ( int site = L - 2; site > 0; site-- ) {

         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? OtensorsA[ site - 1 ] : NULL;
         CTensorO * leftOverlapB  = ( site - 1 ) >= 0 ? OtensorsB[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? OtensorsA[ site + 1 ] : NULL;
         CTensorO * rightOverlapB = ( site + 2 ) < L ? OtensorsB[ site + 1 ] : NULL;

         CSobject * denSA = new CSobject( site, bkOut );
         denSA->Join( leftOverlapA, mpsA[ site ], mpsA[ site + 1 ], rightOverlapA );

         CSobject * denSB = new CSobject( site, bkOut );
         denSA->Join( leftOverlapB, mpsB[ site ], mpsB[ site + 1 ], rightOverlapB );

         denSA->Add( factor, denSB );

         double disc = denSA->Split( mpsOut[ site ], mpsOut[ site + 1 ], D, cut_off, false, true );

         delete denSA;
         delete denSB;

         delete OtensorsA[ site ];
         delete OtensorsB[ site ];
         OtensorsA[ site ] = new CTensorO( site + 1, false, bkOut, bkA );
         OtensorsB[ site ] = new CTensorO( site + 1, false, bkOut, bkB );

         // Otensors
         if ( site == L - 2 ) {
            OtensorsA[ site ]->create( mpsOut[ site + 1 ], mpsA[ site + 1 ] );
            OtensorsB[ site ]->create( mpsOut[ site + 1 ], mpsB[ site + 1 ] );
         } else {
            OtensorsA[ site ]->update_ownmem( mpsOut[ site + 1 ], mpsA[ site + 1 ], OtensorsA[ site + 1 ] );
            OtensorsB[ site ]->update_ownmem( mpsOut[ site + 1 ], mpsB[ site + 1 ], OtensorsB[ site + 1 ] );
         }
      }

      for ( int site = 0; site < L - 2; site++ ) {
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? OtensorsA[ site - 1 ] : NULL;
         CTensorO * leftOverlapB  = ( site - 1 ) >= 0 ? OtensorsB[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? OtensorsA[ site + 1 ] : NULL;
         CTensorO * rightOverlapB = ( site + 2 ) < L ? OtensorsB[ site + 1 ] : NULL;

         CSobject * denSA = new CSobject( site, bkOut );
         denSA->Join( leftOverlapA, mpsA[ site ], mpsA[ site + 1 ], rightOverlapA );

         CSobject * denSB = new CSobject( site, bkOut );
         denSB->Join( leftOverlapB, mpsB[ site ], mpsB[ site + 1 ], rightOverlapB );

         denSA->Add( factor, denSB );
         double disc = denSA->Split( mpsOut[ site ], mpsOut[ site + 1 ], D, cut_off, true, true );

         delete denSA;
         delete denSB;

         delete OtensorsA[ site ];
         delete OtensorsB[ site ];
         OtensorsA[ site ] = new CTensorO( site + 1, true, bkOut, bkA );
         OtensorsB[ site ] = new CTensorO( site + 1, true, bkOut, bkB );

         // Otensors
         if ( site == 0 ) {
            OtensorsA[ site ]->create( mpsOut[ site ], mpsA[ site ] );
            OtensorsB[ site ]->create( mpsOut[ site ], mpsB[ site ] );
         } else {
            OtensorsA[ site ]->update_ownmem( mpsOut[ site ], mpsA[ site ], OtensorsA[ site - 1 ] );
            OtensorsB[ site ]->update_ownmem( mpsOut[ site ], mpsB[ site ], OtensorsB[ site - 1 ] );
         }
      }
   }

   for ( int index = 0; index < L - 1; index++ ) {
      delete OtensorsA[ index ];
      delete OtensorsB[ index ];
   }
   delete[] OtensorsA;
   delete[] OtensorsB;
}

void CheMPS2::TimeTaylor::doStep_rk_4( const int currentInstruction, const bool doImaginary, const double offset ) {

   abort();
}

void CheMPS2::TimeTaylor::doStep_krylov( const int currentInstruction, const bool doImaginary, const double offset, CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut ) {

   dcomplex step = doImaginary ? -scheme->get_time_step( currentInstruction ) : dcomplex( 0.0, -1.0 * -scheme->get_time_step( currentInstruction ) );

   HamiltonianOperator * op = new HamiltonianOperator( prob );

   std::vector< CTensorT ** > krylovBasisVectors;
   std::vector< SyBookkeeper * > krylovBasisVectorBookkeepers;
   std::vector< dcomplex > krylovHamiltonianDiagonal;
   std::vector< dcomplex > krylovHamiltonianOffDiagonal;

   // Step 1
   {
      krylovBasisVectors.push_back( mpsIn );
      krylovBasisVectorBookkeepers.push_back( bkIn );
      krylovHamiltonianDiagonal.push_back( op->ExpectationValue( krylovBasisVectors.back(), krylovBasisVectorBookkeepers.back() ) );
   }
   while ( true ) {

      SyBookkeeper * bkTemp = new SyBookkeeper( *bkIn );
      CTensorT ** mpsTemp   = new CTensorT *[ L ];
      for ( int index = 0; index < L; index++ ) {
         mpsTemp[ index ] = new CTensorT( mpsIn[ index ] );
         mpsTemp[ index ]->random();
      }

      if ( krylovBasisVectors.size() == 1 ) {
         dcomplex coef[]              = {-krylovHamiltonianDiagonal.back()};
         CTensorT ** states[]         = {krylovBasisVectors.back()};
         SyBookkeeper * bookkeepers[] = {krylovBasisVectorBookkeepers.back()};

         op->ApplyAndAdd( krylovBasisVectors.back(), krylovBasisVectorBookkeepers.back(),
                          1, coef, states, bookkeepers,
                          mpsTemp, bkTemp );
      } else {
         dcomplex coef[]              = {-krylovHamiltonianOffDiagonal.back(), -krylovHamiltonianDiagonal.back()};
         CTensorT ** states[]         = {krylovBasisVectors.end()[ -2 ], krylovBasisVectors.back()};
         SyBookkeeper * bookkeepers[] = {krylovBasisVectorBookkeepers.end()[ -2 ], krylovBasisVectorBookkeepers.back()};

         op->ApplyAndAdd( krylovBasisVectors.back(), krylovBasisVectorBookkeepers.back(),
                          2, coef, states, bookkeepers,
                          mpsTemp, bkTemp );
      }

      dcomplex beta = norm( mpsTemp );
      if ( abs( beta ) < 1e-10 || krylovBasisVectors.size() > 10 ) {
         break;
      }
      krylovHamiltonianOffDiagonal.push_back( beta );
      mpsTemp[ 0 ]->number_operator( 0.0, 1.0 / beta );
      krylovBasisVectors.push_back( mpsTemp );
      krylovBasisVectorBookkeepers.push_back( bkTemp );
      krylovHamiltonianDiagonal.push_back( op->ExpectationValue( krylovBasisVectors.back(), krylovBasisVectorBookkeepers.back() ) );
   }

   int krylovSpaceDimension = krylovBasisVectors.size();

   dcomplex * krylovHamiltonian = new dcomplex[ krylovSpaceDimension * krylovSpaceDimension ];
   for ( int i = 0; i < krylovSpaceDimension; i++ ) {
      for ( int j = 0; j < krylovSpaceDimension; j++ ) {
         if ( i == j ) {
            krylovHamiltonian[ i + krylovSpaceDimension * j ] = krylovHamiltonianDiagonal[ i ];
         } else if ( i == j - 1 ) {
            krylovHamiltonian[ i + krylovSpaceDimension * j ] = krylovHamiltonianOffDiagonal[ i ];
         } else if ( i == j + 1 ) {
            krylovHamiltonian[ i + krylovSpaceDimension * j ] = krylovHamiltonianOffDiagonal[ j ];
         } else {
            krylovHamiltonian[ i + krylovSpaceDimension * j ] = 0.0;
         }
      }
   }

   char jobz       = 'V';
   char uplo       = 'U';
   double * evals  = new double[ krylovSpaceDimension ];
   int lwork       = 2 * krylovSpaceDimension - 1;
   dcomplex * work = new dcomplex[ lwork ];
   double * rwork  = new double[ 3 * krylovSpaceDimension - 2 ];
   int info;

   zheev_( &jobz, &uplo, &krylovSpaceDimension, krylovHamiltonian, &krylovSpaceDimension, evals, work, &lwork, rwork, &info );

   dcomplex * firstVec = new dcomplex[ krylovSpaceDimension ];
   for ( int i = 0; i < krylovSpaceDimension; i++ ) {
      firstVec[ i ] = exp( evals[ i ] * step ) * krylovHamiltonian[ krylovSpaceDimension * i ];
   }

   char notrans      = 'N';
   int onedim        = 1;
   dcomplex one      = 1.0;
   dcomplex zero     = 0.0;
   dcomplex * result = new dcomplex[ krylovSpaceDimension ];
   zgemm_( &notrans, &notrans, &krylovSpaceDimension, &onedim, &krylovSpaceDimension,
           &one, krylovHamiltonian, &krylovSpaceDimension,
           firstVec, &krylovSpaceDimension, &zero, result, &krylovSpaceDimension );

   op->Sum( krylovSpaceDimension, result, &krylovBasisVectors[ 0 ], &krylovBasisVectorBookkeepers[ 0 ], mpsOut, bkOut );

   delete op;
}

void CheMPS2::TimeTaylor::doStep_euler_g( const int currentInstruction, const bool doImaginary, const double offset, CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut ) {

   CheMPS2::SyBookkeeper * denHPsi = new CheMPS2::SyBookkeeper( prob, scheme->get_D( currentInstruction ) );
   CheMPS2::CTensorT ** HPsi       = new CheMPS2::CTensorT *[ prob->gL() ];
   for ( int index = 0; index < prob->gL(); index++ ) {
      HPsi[ index ] = new CheMPS2::CTensorT( index, denHPsi );
      HPsi[ index ]->random();
   }
   dcomplex step = doImaginary ? -scheme->get_time_step( currentInstruction ) : dcomplex( 0.0, -1.0 * -scheme->get_time_step( currentInstruction ) );

   // fitApplyH( step, offset, mpsIn, bkIn, HPsi, denHPsi );
   fitApplyH_1site( mpsIn, bkIn, HPsi, denHPsi, 5 );
   // fitAddMPS( step, mpsIn, bkIn, HPsi, denHPsi, mpsOut, bkOut );

   for ( int idx = 0; idx < prob->gL(); idx++ ) {
      delete HPsi[ idx ];
   }
   delete[] HPsi;
   delete denHPsi;
}

void CheMPS2::TimeTaylor::doStep_taylor_1site( const int currentInstruction, const bool doImaginary, const double offset, CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut ) {

   dcomplex step = doImaginary ? -scheme->get_time_step( currentInstruction ) : dcomplex( 0.0, -1.0 * -scheme->get_time_step( currentInstruction ) );

   for ( int index = 0; index < L - 1; index++ ) {
      left_normalize( mpsOut[ index ], mpsOut[ index + 1 ] );
   }
   left_normalize( mpsOut[ L - 1 ], NULL );

   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      updateMovingRightSafe( cnt, mpsOut, bkOut, mpsIn, bkIn );
   }

   for ( int i = 0; i < scheme->get_max_sweeps( currentInstruction ); ++i ) {
      for ( int site = L - 1; site > 0; site-- ) {

         CTensorT * linear        = new CTensorT( site, bkOut );
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 1 ) < L ? Otensors[ site ] : NULL;
         linear->Join( leftOverlapA, mpsIn[ site ], rightOverlapA );

         CTensorT * perturb = new CTensorT( mpsOut[ site ] );
         CHeffNS_1S * heff  = new CHeffNS_1S( bkOut, bkIn, prob );
         heff->Apply( mpsIn[ site ], perturb,
                      Ltensors, LtensorsT,
                      Atensors, AtensorsT,
                      Btensors, BtensorsT,
                      Ctensors, CtensorsT,
                      Dtensors, DtensorsT,
                      S0tensors, S0tensorsT,
                      S1tensors, S1tensorsT,
                      F0tensors, F0tensorsT,
                      F1tensors, F1tensorsT,
                      Qtensors, QtensorsT,
                      Xtensors, Otensors, false );
         delete heff;

         perturb->zaxpy( step, linear );
         linear->zcopy( mpsOut[ site ] );
         delete perturb;
         delete linear;

         right_normalize( mpsOut[ site - 1 ], mpsOut[ site ] );
         updateMovingLeftSafe( site - 1, mpsOut, bkOut, mpsIn, bkIn );
      }

      for ( int site = 0; site < L - 1; site++ ) {
         CTensorT * linear        = new CTensorT( site, bkOut );
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 1 ) < L ? Otensors[ site ] : NULL;
         linear->Join( leftOverlapA, mpsIn[ site ], rightOverlapA );

         CTensorT * perturb = new CTensorT( mpsOut[ site ] );
         CHeffNS_1S * heff  = new CHeffNS_1S( bkOut, bkIn, prob );
         heff->Apply( mpsIn[ site ], perturb,
                      Ltensors, LtensorsT,
                      Atensors, AtensorsT,
                      Btensors, BtensorsT,
                      Ctensors, CtensorsT,
                      Dtensors, DtensorsT,
                      S0tensors, S0tensorsT,
                      S1tensors, S1tensorsT,
                      F0tensors, F0tensorsT,
                      F1tensors, F1tensorsT,
                      Qtensors, QtensorsT,
                      Xtensors, Otensors, true );
         delete heff;

         perturb->zaxpy( step, linear );
         linear->zcopy( mpsOut[ site ] );
         delete perturb;
         delete linear;

         left_normalize( mpsOut[ site ], mpsOut[ site + 1 ] );
         updateMovingRightSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }
   }
}
void CheMPS2::TimeTaylor::doStep_taylor_1( const int currentInstruction, const bool doImaginary, const double offset, CTensorT ** mpsIn, SyBookkeeper * bkIn, CTensorT ** mpsOut, SyBookkeeper * bkOut ) {

   for ( int index = 0; index < L - 1; index++ ) {
      left_normalize( mpsOut[ index ], mpsOut[ index + 1 ] );
   }
   left_normalize( mpsOut[ L - 1 ], NULL );

   const dcomplex step = doImaginary ? -scheme->get_time_step( currentInstruction ) : dcomplex( 0.0, -1.0 * -scheme->get_time_step( currentInstruction ) );
   for ( int cnt = 0; cnt < L - 1; cnt++ ) {
      updateMovingRightSafe( cnt, mpsOut, bkOut, mpsIn, bkIn );
   }

   for ( int i = 0; i < scheme->get_max_sweeps( currentInstruction ); ++i ) {
      for ( int site = L - 2; site > 0; site-- ) {
         CSobject * denSB = new CSobject( site, bkIn );
         denSB->Join( mpsIn[ site ], mpsIn[ site + 1 ] );

         CSobject * denPA         = new CSobject( site, bkOut );
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? Otensors[ site + 1 ] : NULL;
         denPA->Join( leftOverlapA, denSB, rightOverlapA );

         CSobject * denPB = new CSobject( site, bkOut );

         CHeffNS * heff = new CHeffNS( bkOut, bkIn, prob, offset );
         heff->Apply( denSB, denPB, Ltensors, LtensorsT, Atensors, AtensorsT,
                      Btensors, BtensorsT, Ctensors, CtensorsT, Dtensors, DtensorsT,
                      S0tensors, S0tensorsT, S1tensors, S1tensorsT, F0tensors,
                      F0tensorsT, F1tensors, F1tensorsT, Qtensors, QtensorsT,
                      Xtensors, leftOverlapA, rightOverlapA );

         denPA->Add( step, denPB );
         double disc = denPA->Split( mpsOut[ site ], mpsOut[ site + 1 ], scheme->get_D( currentInstruction ), scheme->get_cut_off( currentInstruction ), false, true );

         delete heff;
         delete denPA;
         delete denPB;
         delete denSB;

         updateMovingLeftSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }

      for ( int site = 0; site < L - 2; site++ ) {
         CSobject * denSB = new CSobject( site, bkIn );
         denSB->Join( mpsIn[ site ], mpsIn[ site + 1 ] );

         CSobject * denPA         = new CSobject( site, bkOut );
         CTensorO * leftOverlapA  = ( site - 1 ) >= 0 ? Otensors[ site - 1 ] : NULL;
         CTensorO * rightOverlapA = ( site + 2 ) < L ? Otensors[ site + 1 ] : NULL;
         denPA->Join( leftOverlapA, denSB, rightOverlapA );

         CSobject * denPB = new CSobject( site, bkOut );

         CHeffNS * heff = new CHeffNS( bkOut, bkIn, prob, offset );
         heff->Apply( denSB, denPB, Ltensors, LtensorsT, Atensors, AtensorsT,
                      Btensors, BtensorsT, Ctensors, CtensorsT, Dtensors, DtensorsT,
                      S0tensors, S0tensorsT, S1tensors, S1tensorsT, F0tensors,
                      F0tensorsT, F1tensors, F1tensorsT, Qtensors, QtensorsT,
                      Xtensors, leftOverlapA, rightOverlapA );

         denPA->Add( step, denPB );
         double disc = denPA->Split( mpsOut[ site ], mpsOut[ site + 1 ], scheme->get_D( currentInstruction ), scheme->get_cut_off( currentInstruction ), true, true );

         delete heff;
         delete denPA;
         delete denPB;
         delete denSB;

         updateMovingRightSafe( site, mpsOut, bkOut, mpsIn, bkIn );
      }
   }
}

void CheMPS2::TimeTaylor::Propagate( SyBookkeeper * initBK, CTensorT ** initMPS, const bool doImaginary ) {

   SyBookkeeper * MPSBK = new SyBookkeeper( *initBK );
   CTensorT ** MPS      = new CTensorT *[ L ];
   for ( int index = 0; index < L; index++ ) {
      MPS[ index ] = new CTensorT( initMPS[ index ] );
   }

   double t           = 0.0;
   double firstEnergy = 0;

   ( *logger ) << "\n";
   ( *logger ) << "   Starting to propagate MPS\n";
   ( *logger ) << "\n";

   ( *logger ) << "\n";
   ( *logger ) << "   L = " << L << "\n";
   ( *logger ) << "   N = " << prob->gN() << "\n";
   ( *logger ) << "   TwoS = " << prob->gTwoS() << "\n";
   ( *logger ) << "   I = " << prob->gIrrep() << "\n";
   ( *logger ) << "\n";

   ( *logger ) << "   full ci matrix product state dimensions:\n";
   ( *logger ) << "   ";
   for ( int i = 0; i < L + 1; i++ ) {
      ( *logger ) << std::setw( 5 ) << i;
   }
   ( *logger ) << "\n";
   ( *logger ) << "   ";
   for ( int i = 0; i < L + 1; i++ ) {
      ( *logger ) << std::setw( 5 ) << MPSBK->gFCIDimAtBound( i );
   }
   ( *logger ) << "\n";
   ( *logger ) << "\n";
   ( *logger ) << hashline;

   for ( int inst = 0; inst < scheme->get_number(); inst++ ) {

      for ( ; t < scheme->get_max_time( inst ); t += scheme->get_time_step( inst ) ) {
         ( *logger ) << hashline;
         ( *logger ) << "\n";
         ( *logger ) << "   t = " << t << "\n";
         ( *logger ) << "   Tmax = " << scheme->get_max_time( inst ) << "\n";
         ( *logger ) << "   dt = " << scheme->get_time_step( inst ) << "\n";
         ( *logger ) << "\n";

         ( *logger ) << "   matrix product state dimensions:\n";
         ( *logger ) << "   ";
         for ( int i = 0; i < L + 1; i++ ) {
            ( *logger ) << std::setw( 5 ) << i;
         }
         ( *logger ) << "\n";
         ( *logger ) << "   ";
         for ( int i = 0; i < L + 1; i++ ) {
            ( *logger ) << std::setw( 5 ) << MPSBK->gTotDimAtBound( i );
         }
         ( *logger ) << "\n";
         ( *logger ) << "\n";
         ( *logger ) << "   MaxM = " << scheme->get_D( inst ) << "\n";
         ( *logger ) << "   CutO = " << scheme->get_cut_off( inst ) << "\n";
         ( *logger ) << "   NSwe = " << scheme->get_max_sweeps( inst ) << "\n";
         ( *logger ) << "\n";

         CTwoDM * the2DM            = new CTwoDM( MPSBK, prob );
         CTwoDMBuilder * tdmbuilder = new CTwoDMBuilder( logger, prob, MPS, MPSBK );
         tdmbuilder->Build2RDM( the2DM );

         ( *logger ) << "   Norm      = " << norm( MPS ) << "\n";
         ( *logger ) << "   Energy    = " << std::real( the2DM->energy() ) << "\n";

         if ( t == 0.0 ) {
            firstEnergy = std::real( the2DM->energy() );
         }
         ( *logger ) << "   tr(TwoDM) = " << std::real( the2DM->trace() ) << "\n";
         ( *logger ) << "   N(N-1)    = " << prob->gN() * ( prob->gN() - 1.0 ) << "\n";
         ( *logger ) << "\n";
         ( *logger ) << "  occupation numbers of molecular orbitals:\n";
         ( *logger ) << "   ";
         for ( int i = 0; i < L; i++ ) {
            ( *logger ) << std::setw( 20 ) << std::fixed << std::setprecision( 15 ) << std::real( the2DM->get1RDM_DMRG( i, i ) );
         }
         // ( *logger ) << "\n";
         // ( *logger ) << "   ";
         // ( *logger ) << "\n";
         // ( *logger ) << "   real part one body reduced density matrix:\n";
         // ( *logger ) << "\n";
         // for ( int irow = 0; irow < L; irow++ ) {
         //    for ( int icol = 0; icol < L; icol++ ) {
         //       ( *logger ) << std::setw( 20 ) << std::fixed << std::setprecision( 15 ) << std::real( the2DM->get1RDM_DMRG( irow, icol ) );
         //    }
         //    ( *logger ) << "\n";
         // }
         // ( *logger ) << "\n";

         // ( *logger ) << "   imaginary part one body reduced density matrix:\n";
         // ( *logger ) << "\n";
         // for ( int irow = 0; irow < L; irow++ ) {
         //    for ( int icol = 0; icol < L; icol++ ) {
         //       ( *logger ) << std::setw( 20 ) << std::fixed << std::setprecision( 15 ) << std::imag( the2DM->get1RDM_DMRG( irow, icol ) );
         //    }
         //    ( *logger ) << "\n";
         // }

         ( *logger ) << "\n";

         delete tdmbuilder;
         delete the2DM;

         deleteAllBoundaryOperators();

         SyBookkeeper * MPSBKDT = new SyBookkeeper( prob, scheme->get_D( inst ) );
         CTensorT ** MPSDT      = new CTensorT *[ L ];
         for ( int index = 0; index < L; index++ ) {
            MPSDT[ index ] = new CTensorT( index, MPSBKDT );
            MPSDT[ index ]->random();
         }

         //    doStep_taylor_1( inst, doImaginary, firstEnergy, MPS, MPSBK, MPSDT, MPSBKDT );
         // doStep_taylor_1site( inst, doImaginary, firstEnergy, MPS, MPSBK, MPSDT, MPSBKDT );
         doStep_krylov( inst, doImaginary, firstEnergy, MPS, MPSBK, MPSDT, MPSBKDT );
         //    doStep_euler_g( inst, doImaginary, firstEnergy, MPS, MPSBK, MPSDT, MPSBKDT );

         for ( int site = 0; site < L; site++ ) {
            delete MPS[ site ];
         }
         delete[] MPS;
         delete MPSBK;

         MPS   = MPSDT;
         MPSBK = MPSBKDT;

         if ( doImaginary ) {
            double normDT = norm( MPS );
            MPS[ 0 ]->number_operator( 0.0, 1.0 / normDT );
         }

         ( *logger ) << hashline;
      }
   }

   for ( int site = 0; site < L; site++ ) {
      delete MPS[ site ];
   }
   delete[] MPS;
   delete MPSBK;
}