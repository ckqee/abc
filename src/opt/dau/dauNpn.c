/**CFile****************************************************************

  FileName    [dau.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [DAG-aware unmapping.]

  Synopsis    []

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: dau.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "dauInt.h"
#include "misc/util/utilTruth.h"
#include "misc/extra/extra.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

#define USE4VARS 1

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////
 
/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Dau_TruthEnum()
{
    int fUseTable = 1;
    abctime clk = Abc_Clock();
#ifdef USE4VARS
    int nVars   = 4;
    int nSizeW  = 1 << 14;
    char * pFileName = "tableW14.data";
#else
    int nVars   = 5;
    int nSizeW  = 1 << 30;
    char * pFileName = "tableW30.data";
#endif
    int nPerms  = Extra_Factorial( nVars );
    int nMints  = 1 << nVars;
    int * pPerm = Extra_PermSchedule( nVars );
    int * pComp = Extra_GreyCodeSchedule( nVars );
    word nFuncs = ((word)1 << (((word)1 << nVars)-1));
    word * pPres = ABC_CALLOC( word, 1 << ((1<<nVars)-7) );
    unsigned * pTable = fUseTable ? (unsigned *)ABC_CALLOC(word, nSizeW) : NULL;
    Vec_Int_t * vNpns = Vec_IntAlloc( 1000 );
    word tMask = Abc_Tt6Mask( 1 << nVars );
    word tTemp, tCur;
    int i, k;
    if ( pPres == NULL )
    {
        printf( "Cannot alloc memory for marks.\n" );
        return;
    }
    if ( pTable == NULL )
        printf( "Cannot alloc memory for table.\n" );
    for ( tCur = 0; tCur < nFuncs; tCur++ )
    {
        if ( (tCur & 0x3FFFF) == 0 )
        {
            printf( "Finished %08x.  Classes = %6d.  ", (int)tCur, Vec_IntSize(vNpns) );
            Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
            fflush(stdout);
        }
        if ( Abc_TtGetBit(pPres, (int)tCur) )
            continue;
        //Extra_PrintBinary( stdout, (unsigned *)&tCur, 16 ); printf( " %04x\n", (int)tCur );
        //Dau_DsdPrintFromTruth( &tCur, 4 ); printf( "\n" );
        Vec_IntPush( vNpns, (int)tCur );
        tTemp = tCur;
        for ( i = 0; i < nPerms; i++ )
        {
            for ( k = 0; k < nMints; k++ )
            {
                if ( tCur < nFuncs )
                {
                    if ( pTable ) pTable[(int)tCur] = tTemp;
                    Abc_TtSetBit( pPres, (int)tCur );
                }
                if ( (tMask & ~tCur) < nFuncs )
                {
                    if ( pTable ) pTable[(int)(tMask & ~tCur)] = tTemp;
                    Abc_TtSetBit( pPres, (int)(tMask & ~tCur) );
                }
                tCur = Abc_Tt6Flip( tCur, pComp[k] );
            }
            tCur = Abc_Tt6SwapAdjacent( tCur, pPerm[i] );
        }
        assert( tTemp == tCur );
    }
    printf( "Computed %d NPN classes of %d variables.  ", Vec_IntSize(vNpns), nVars );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    fflush(stdout);
    Vec_IntFree( vNpns );
    ABC_FREE( pPres );
    ABC_FREE( pPerm );
    ABC_FREE( pComp );
    // write into file
    if ( pTable )
    {
        FILE * pFile = fopen( pFileName, "wb" );
        int RetValue = fwrite( pTable, 8, nSizeW, pFile );
        fclose( pFile );
        ABC_FREE( pTable );
    }
}

 
/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
unsigned * Dau_ReadFile( char * pFileName, int nSizeW )
{
    abctime clk = Abc_Clock();
    FILE * pFile = fopen( pFileName, "rb" );
    unsigned * p = (unsigned *)ABC_CALLOC(word, nSizeW);
    int RetValue = fread( p, sizeof(word), nSizeW, pFile );
    fclose( pFile );
    Abc_PrintTime( 1, "File reading", Abc_Clock() - clk );
    return p;
}
int Dau_AddFunction( word tCur, int nVars, unsigned * pTable, Vec_Int_t * vNpns, Vec_Int_t * vNpns_ )
{
    int Digit  = (1 << nVars)-1;
    word tMask = Abc_Tt6Mask( 1 << nVars );
    word tNorm = (tCur >> Digit) & 1 ? ~tCur : tCur;
    unsigned t = (unsigned)(tNorm & tMask);
    unsigned tRep = pTable[t] & 0x7FFFFFFF;
    unsigned tRep2 = pTable[tRep];
    assert( ((tNorm >> Digit) & 1) == 0 );
    if ( (tRep2 >> 31) == 0 ) // first time
    {
        Vec_IntPush( vNpns, tRep2 );
        if ( Abc_TtSupportSize(&tCur, nVars) < nVars )
            Vec_IntPush( vNpns_, tRep2 );
        pTable[tRep] = tRep2 | (1 << 31);
        return tRep2;
    }
    return 0;
}
void Dau_NetworkEnum()
{
    abctime clk = Abc_Clock();
    int Limit = 2;
    int UseTwo = 1;
#ifdef USE4VARS
    int nVars = 4;
    int nSizeW  = 1 << 14;
    char * pFileName = "tableW14.data";
#else
    int nVars = 5;
    int nSizeW  = 1 << 30;
    char * pFileName = "tableW30.data";
#endif
    unsigned * pTable  = Dau_ReadFile( pFileName, nSizeW );
    Vec_Wec_t * vNpns  = Vec_WecStart( 32 );
    Vec_Wec_t * vNpns_ = Vec_WecStart( 32 );
    int i, v, u, g, k, m, n, Res, Entry, iPrev = 0, iLast = 1;
    unsigned Inv = (unsigned)Abc_Tt6Mask(1 << (nVars-1));
    // create constant function and buffer/inverter function
    pTable[0]   |= (1 << 31);
    pTable[Inv] |= (1 << 31);
    Vec_IntPushTwo( Vec_WecEntry(vNpns,  0), 0, Inv );
    Vec_IntPushTwo( Vec_WecEntry(vNpns_, 0), 0, Inv );
    printf("Nodes = %2d.   New = %4d. Total = %6d.   New = %4d. Total = %6d.  ", 
        0, Vec_IntSize(Vec_WecEntry(vNpns,  0)), Vec_WecSizeSize(vNpns), 
           Vec_IntSize(Vec_WecEntry(vNpns_, 0)), Vec_WecSizeSize(vNpns_) );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    // numerate other functions based on how many nodes they have
    for ( n = 1; n < 32; n++ )
    {
        Vec_Int_t * vFuncsN2 = n > 1 ? Vec_WecEntry( vNpns, n-2 ) : NULL;
        Vec_Int_t * vFuncsN1 = Vec_WecEntry( vNpns, n-1 );
        Vec_Int_t * vFuncsN  = Vec_WecEntry( vNpns, n   );
        Vec_Int_t * vFuncsN_ = Vec_WecEntry( vNpns_,n   );
        Vec_IntForEachEntry( vFuncsN1, Entry, i )
        {
            word uTruth = (((word)Entry) << 32) | (word)Entry;
            int nSupp = Abc_TtSupportSize( &uTruth, nVars );
            assert( nSupp == 6 || !Abc_Tt6HasVar(uTruth, nVars-1-nSupp) );
            //printf( "Exploring function %4d with %d vars: ", i, nSupp );
            //printf( " %04x\n", (int)uTruth );
            //Dau_DsdPrintFromTruth( &uTruth, 4 );
            for ( v = 0; v < nSupp; v++ )
            {
                word tGate, tCur;
                word Cof0 = Abc_Tt6Cofactor0( uTruth, nVars-1-v );
                word Cof1 = Abc_Tt6Cofactor1( uTruth, nVars-1-v );
                for ( g = 0; g < Limit; g++ )
                {
                    if ( nSupp < nVars )
                    {
                        if ( g == 0 )
                        {
                            tGate = s_Truths6[nVars-1-v] & s_Truths6[nVars-1-nSupp];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                        else
                        {
                            tGate = s_Truths6[nVars-1-v] ^ s_Truths6[nVars-1-nSupp];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                    }
                }
                for ( g = 0; g < Limit; g++ )
                {
                    // add one cross bar
                    for ( k = 0; k < nSupp; k++ ) if ( k != v )
                    {
                        if ( g == 0 )
                        {
                            tGate = s_Truths6[nVars-1-v] & s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tGate = s_Truths6[nVars-1-v] & ~s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                        else
                        {
                            tGate = s_Truths6[nVars-1-v] ^ s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                    }
                }
                for ( g = 0; g < Limit; g++ )
                {
                    // add two cross bars
                    for ( k = 0;   k < nSupp; k++ ) if ( k != v )
                    for ( m = k+1; m < nSupp; m++ ) if ( m != v )
                    {
                        if ( g == 0 )
                        {
                            tGate = s_Truths6[nVars-1-m] & s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tGate = s_Truths6[nVars-1-m] & ~s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tGate = ~s_Truths6[nVars-1-m] & s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tGate = ~s_Truths6[nVars-1-m] & ~s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );

                            tCur  = (tGate & Cof0) | (~tGate & Cof1);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                        else
                        {
                            tGate = s_Truths6[nVars-1-m] ^ s_Truths6[nVars-1-k];
                            tCur  = (tGate & Cof1) | (~tGate & Cof0);
                            Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                        }
                    }
                }
            }
        }
        if ( UseTwo && vFuncsN2 )
        Vec_IntForEachEntry( vFuncsN2, Entry, i )
        {
            word uTruth = (((word)Entry) << 32) | (word)Entry;
            int nSupp = Abc_TtSupportSize( &uTruth, nVars );
            assert( nSupp == 6 || !Abc_Tt6HasVar(uTruth, nVars-1-nSupp) );
            //printf( "Exploring function %4d with %d vars: ", i, nSupp );
            //printf( " %04x\n", (int)uTruth );
            //Dau_DsdPrintFromTruth( &uTruth, 4 );
            for ( v = 0; v < nSupp; v++ )
//            for ( u = v+1; u < nSupp; u++ ) if ( u != v )
            for ( u = 0; u < nSupp; u++ ) if ( u != v )
            {
                word tGate1, tGate2, tCur;
                word Cof0 = Abc_Tt6Cofactor0( uTruth, nVars-1-v );
                word Cof1 = Abc_Tt6Cofactor1( uTruth, nVars-1-v );

                word Cof00 = Abc_Tt6Cofactor0( Cof0, nVars-1-u );
                word Cof01 = Abc_Tt6Cofactor1( Cof0, nVars-1-u );
                word Cof10 = Abc_Tt6Cofactor0( Cof1, nVars-1-u );
                word Cof11 = Abc_Tt6Cofactor1( Cof1, nVars-1-u );

                tGate1 = s_Truths6[nVars-1-v] & s_Truths6[nVars-1-u];
                tGate2 = s_Truths6[nVars-1-v] ^ s_Truths6[nVars-1-u];

                Cof0  = (tGate2 & Cof00) | (~tGate2 & Cof01);
                Cof1  = (tGate2 & Cof10) | (~tGate2 & Cof11);

                tCur  = (tGate1 & Cof1)  | (~tGate1 & Cof0);
                Res = Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                if ( Res )
                    printf( "Found function %d\n", Res );

                tCur  = (tGate1 & Cof0)  | (~tGate1 & Cof1);
                Res = Dau_AddFunction( tCur, nVars, pTable, vFuncsN, vFuncsN_ );
                if ( Res )
                    printf( "Found function %d\n", Res );
            }
        }
        printf("Nodes = %2d.   New = %4d. Total = %6d.   New = %4d. Total = %6d.  ", 
            n, Vec_IntSize(vFuncsN), Vec_WecSizeSize(vNpns), Vec_IntSize(vFuncsN_), Vec_WecSizeSize(vNpns_) );
        Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
        fflush(stdout);
        if ( Vec_IntSize(vFuncsN) == 0 )
            break;
    }
    printf( "Functions with 7 nodes:\n" );
    Vec_IntForEachEntry( Vec_WecEntry(vNpns_,7), Entry, i )
        printf( "%04x ", Entry );
    printf( "\n" );

    Vec_WecFree( vNpns );
    Vec_WecFree( vNpns_ );
    ABC_FREE( pTable );
    Abc_PrintTime( 1, "Total time", Abc_Clock() - clk );
    fflush(stdout);
}
void Dau_NetworkEnumTest()
{
    //Dau_TruthEnum();
    Dau_NetworkEnum();
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END
