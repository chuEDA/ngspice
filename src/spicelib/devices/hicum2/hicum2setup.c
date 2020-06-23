/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
Model Author: 1990 Michael Schröter TU Dresden
Spice3 Implementation: 2019 Dietmar Warning
**********/

/*
 * This routine should only be called when circuit topology
 * changes, since its computations do not depend on most
 * device or model parameters, only on topology (as
 * affected by emitter, collector, and base resistances)
 */

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "ngspice/smpdefs.h"
#include "hicum2defs.h"
#include "ngspice/const.h"
#include "ngspice/sperror.h"
#include "ngspice/ifsim.h"
#include "ngspice/suffix.h"

#define MIN_R 0.001

int
HICUMsetup(SMPmatrix *matrix, GENmodel *inModel, CKTcircuit *ckt, int *states)
        /* load the HICUM structure with those pointers needed later
         * for fast matrix loading
         */
{
    HICUMmodel *model = (HICUMmodel*)inModel;
    HICUMinstance *here;
    int error;
    CKTnode *tmp;

    /*  loop through all the transistor models */
    for( ; model != NULL; model = HICUMnextModel(model)) {

        int selfheat = (((model->HICUMflsh == 1) || (model->HICUMflsh == 2)) && (model->HICUMrthGiven) && (model->HICUMrth > 0.0));
        int nqs      = ( (model->HICUMflnqs != 0 || model->HICUMflcomp == 0.0 || model->HICUMflcomp == 2.1) && (model->HICUMalit > 0 || model->HICUMalqf > 0));

//Circuit simulator specific parameters
        if(model->HICUMtype != NPN && model->HICUMtype != PNP)
            model->HICUMtype = NPN;

        if(!model->HICUMtnomGiven)
            model->HICUMtnom = ckt->CKTnomTemp;

        if (!model->HICUMversionGiven)
            model->HICUMversion = copy("2.3.4");

//Transfer current
        if(!model->HICUMc10Given)
            model->HICUMc10 = 2e-30;

        if(!model->HICUMqp0Given)
            model->HICUMqp0 = 2e-14;

        if(!model->HICUMichGiven)
            model->HICUMich = 0.0;

        if(!model->HICUMhf0Given)
            model->HICUMhf0 = 1.0;

        if(!model->HICUMhfeGiven)
            model->HICUMhfe = 1.0;

        if(!model->HICUMhfcGiven)
            model->HICUMhfc = 1.0;

        if(!model->HICUMhjeiGiven)
            model->HICUMhjei = 1.0;

        if(!model->HICUMahjeiGiven)
            model->HICUMahjei = 0.0;

        if(!model->HICUMrhjeiGiven)
            model->HICUMrhjei = 1.0;

        if(!model->HICUMhjciGiven)
            model->HICUMhjci = 1.0;

//Base-Emitter diode;
        if(!model->HICUMibeisGiven)
            model->HICUMibeis = 1e-18;

        if(!model->HICUMmbeiGiven)
            model->HICUMmbei = 1.0;

        if(!model->HICUMireisGiven)
            model->HICUMireis = 0.0;

        if(!model->HICUMmreiGiven)
            model->HICUMmrei = 2.0;

        if(!model->HICUMibepsGiven)
            model->HICUMibeps = 0.0;

        if(!model->HICUMmbepGiven)
            model->HICUMmbep = 1.0;

        if(!model->HICUMirepsGiven)
            model->HICUMireps = 0.0;

        if(!model->HICUMmrepGiven)
            model->HICUMmrep = 2.0;

        if(!model->HICUMmcfGiven)
            model->HICUMmcf = 1.0;

//Transit time for excess recombination current at b-c barrier
        if(!model->HICUMtbhrecGiven)
            model->HICUMtbhrec = 0.0;

//Base-Collector diode currents
        if(!model->HICUMibcisGiven)
            model->HICUMibcis = 1e-16;

        if(!model->HICUMmbciGiven)
            model->HICUMmbci = 1.0;

        if(!model->HICUMibcxsGiven)
            model->HICUMibcxs = 0.0;

        if(!model->HICUMmbcxGiven)
            model->HICUMmbcx = 1.0;

//Base-Emitter tunneling current
        if(!model->HICUMibetsGiven)
            model->HICUMibets = 0.0;

        if(!model->HICUMabetGiven)
            model->HICUMabet = 40.0;

        if(!model->HICUMtunodeGiven)
            model->HICUMtunode = 1;

//Base-Collector avalanche current
        if(!model->HICUMfavlGiven)
            model->HICUMfavl = 0.0;

        if(!model->HICUMqavlGiven)
            model->HICUMqavl = 0.0;

        if(!model->HICUMkavlGiven)
            model->HICUMkavl = 0.0;

        if(!model->HICUMalfavGiven)
            model->HICUMalfav = 0.0;

        if(!model->HICUMalqavGiven)
            model->HICUMalqav = 0.0;

        if(!model->HICUMalkavGiven)
            model->HICUMalkav = 0.0;

//Series resistances
        if(!model->HICUMrbi0Given)
            model->HICUMrbi0 = MIN_R;

        if(!model->HICUMrbxGiven)
            model->HICUMrbx = MIN_R;

        if(!model->HICUMfgeoGiven)
            model->HICUMfgeo = 0.6557;

        if(!model->HICUMfdqr0Given)
            model->HICUMfdqr0 = 0.0;

        if(!model->HICUMfcrbiGiven)
            model->HICUMfcrbi = 0.0;

        if(!model->HICUMfqiGiven)
            model->HICUMfqi = 1.0;

        if(!model->HICUMreGiven)
            model->HICUMre = MIN_R;

        if(!model->HICUMrcxGiven)
            model->HICUMrcx = MIN_R;

//Substrate transistor
        if(!model->HICUMitssGiven)
            model->HICUMitss = 0.0;

        if(!model->HICUMmsfGiven)
            model->HICUMmsf = 1.0;

        if(!model->HICUMiscsGiven)
            model->HICUMiscs = 0.0;

        if(!model->HICUMmscGiven)
            model->HICUMmsc = 1.0;

        if(!model->HICUMtsfGiven)
            model->HICUMtsf = 0.0;

//Intra-device substrate coupling
        if(!model->HICUMrsuGiven)
            model->HICUMrsu = MIN_R;

        if(!model->HICUMcsuGiven)
            model->HICUMcsu = 0.0;

//Depletion Capacitances
        if(!model->HICUMcjei0Given)
            model->HICUMcjei0 = 1.0e-20;

        if(!model->HICUMvdeiGiven)
            model->HICUMvdei = 0.9;

        if(!model->HICUMzeiGiven)
            model->HICUMzei = 0.5;

        if(!model->HICUMajeiGiven)
            model->HICUMajei = 2.5;

        if(!model->HICUMcjep0Given)
            model->HICUMcjep0 = 1.0e-20;

        if(!model->HICUMvdepGiven)
            model->HICUMvdep = 0.9;

        if(!model->HICUMzepGiven)
            model->HICUMzep = 0.5;

        if(!model->HICUMajepGiven)
            model->HICUMajep = 2.5;

        if(!model->HICUMcjci0Given)
            model->HICUMcjci0 = 1.0e-20;

        if(!model->HICUMvdciGiven)
            model->HICUMvdci = 0.7;

        if(!model->HICUMzciGiven)
            model->HICUMzci = 0.4;

        if(!model->HICUMvptciGiven)
            model->HICUMvptci = 100.0;

        if(!model->HICUMcjcx0Given)
            model->HICUMcjcx0 = 1.0e-20;

        if(!model->HICUMvdcxGiven)
            model->HICUMvdcx = 0.7;

        if(!model->HICUMzcxGiven)
            model->HICUMzcx = 0.4;

        if(!model->HICUMvptcxGiven)
            model->HICUMvptcx = 100.0;

        if(!model->HICUMfbcparGiven)
            model->HICUMfbcpar = 0.0;

        if(!model->HICUMfbeparGiven)
            model->HICUMfbepar = 1.0;

        if(!model->HICUMcjs0Given)
            model->HICUMcjs0 = 0.0;

        if(!model->HICUMvdsGiven)
            model->HICUMvds = 0.6;

        if(!model->HICUMzsGiven)
            model->HICUMzs = 0.5;

        if(!model->HICUMvptsGiven)
            model->HICUMvpts = 100.0;

        if(!model->HICUMcscp0Given)
            model->HICUMcscp0 = 0.0;

        if(!model->HICUMvdspGiven)
            model->HICUMvdsp = 0.6;

        if(!model->HICUMzspGiven)
            model->HICUMzsp = 0.5;

        if(!model->HICUMvptspGiven)
            model->HICUMvptsp = 100.0;

//Diffusion Capacitances
        if(!model->HICUMt0Given)
            model->HICUMt0 = 0.0;

        if(!model->HICUMdt0hGiven)
            model->HICUMdt0h = 0.0;

        if(!model->HICUMtbvlGiven)
            model->HICUMtbvl = 0.0;

        if(!model->HICUMtef0Given)
            model->HICUMtef0 = 0.0;

        if(!model->HICUMgtfeGiven)
            model->HICUMgtfe = 1.0;

        if(!model->HICUMthcsGiven)
            model->HICUMthcs = 0.0;

        if(!model->HICUMahcGiven)
            model->HICUMahc = 0.1;

        if(!model->HICUMfthcGiven)
            model->HICUMfthc = 0.0;

        if(!model->HICUMrci0Given)
            model->HICUMrci0 = 150;

        if(!model->HICUMvlimGiven)
            model->HICUMvlim = 0.5;

        if(!model->HICUMvcesGiven)
            model->HICUMvces = 0.1;

        if(!model->HICUMvptGiven)
            model->HICUMvpt = 100.0;

        if(!model->HICUMaickGiven)
            model->HICUMaick = 1.0e-03;

        if(!model->HICUMdelckGiven)
            model->HICUMdelck = 2.0;

        if(!model->HICUMtrGiven)
            model->HICUMtr = 0.0;

        if(!model->HICUMvcbarGiven)
            model->HICUMvcbar = 0.0;

        if(!model->HICUMicbarGiven)
            model->HICUMicbar = 0.0;

        if(!model->HICUMacbarGiven)
            model->HICUMacbar = 0.01;

//Isolation Capacitances
        if(!model->HICUMcbeparGiven)
            model->HICUMcbepar = 0.0;

        if(!model->HICUMcbcparGiven)
            model->HICUMcbcpar = 0.0;

//Non-quasi-static Effect
        if(!model->HICUMalqfGiven)
            model->HICUMalqf = 0.167;

        if(!model->HICUMalitGiven)
            model->HICUMalit = 0.333;

        if(!model->HICUMflnqsGiven)
            model->HICUMflnqs = 0;

//Noise
        if(!model->HICUMkfGiven)
            model->HICUMkf = 0.0;

        if(!model->HICUMafGiven)
            model->HICUMaf = 2.0;

        if(!model->HICUMcfbeGiven)
            model->HICUMcfbe = -1;

        if(!model->HICUMflconoGiven)
            model->HICUMflcono = 0;

        if(!model->HICUMkfreGiven)
            model->HICUMkfre = 0.0;

        if(!model->HICUMafreGiven)
            model->HICUMafre = 2.0;

//Lateral Geometry Scaling (at high current densities)
        if(!model->HICUMlatbGiven)
            model->HICUMlatb = 0.0;

        if(!model->HICUMlatlGiven)
            model->HICUMlatl = 0.0;

//Temperature dependence
        if(!model->HICUMvgbGiven)
            model->HICUMvgb = 1.17;

        if(!model->HICUMalt0Given)
            model->HICUMalt0 = 0.0;

        if(!model->HICUMkt0Given)
            model->HICUMkt0 = 0.0;

        if(!model->HICUMzetaciGiven)
            model->HICUMzetaci = 0.0;

        if(!model->HICUMalvsGiven)
            model->HICUMalvs = 0.0;

        if(!model->HICUMalcesGiven)
            model->HICUMalces = 0.0;

        if(!model->HICUMzetarbiGiven)
            model->HICUMzetarbi = 0.0;

        if(!model->HICUMzetarbxGiven)
            model->HICUMzetarbx = 0.0;

        if(!model->HICUMzetarcxGiven)
            model->HICUMzetarcx = 0.0;

        if(!model->HICUMzetareGiven)
            model->HICUMzetare = 0.0;

        if(!model->HICUMzetacxGiven)
            model->HICUMzetacx = 1.0;

        if(!model->HICUMvgeGiven)
            model->HICUMvge = 1.17;

        if(!model->HICUMvgcGiven)
            model->HICUMvgc = 1.17;

        if(!model->HICUMvgsGiven)
            model->HICUMvgs = 1.17;

        if(!model->HICUMf1vgGiven)
            model->HICUMf1vg = -1.02377e-4;

        if(!model->HICUMf2vgGiven)
            model->HICUMf2vg = 4.3215e-4;

        if(!model->HICUMzetactGiven)
            model->HICUMzetact = 3.0;

        if(!model->HICUMzetabetGiven)
            model->HICUMzetabet = 3.5;

        if(!model->HICUMalbGiven)
            model->HICUMalb = 0.0;

        if(!model->HICUMdvgbeGiven)
            model->HICUMdvgbe = 0.0;

        if(!model->HICUMzetahjeiGiven)
            model->HICUMzetahjei = 1.0;

        if(!model->HICUMzetavgbeGiven)
            model->HICUMzetavgbe = 1.0;

//Self-Heating
        if(!model->HICUMflshGiven)
            model->HICUMflsh = 0;
        if(!model->HICUMrth_deGiven)
            model->HICUMrth_de = 0;

        if(!model->HICUMrthGiven)
            model->HICUMrth = 0.0;

        if(!model->HICUMzetarthGiven)
            model->HICUMzetarth = 0.0;

        if(!model->HICUMalrthGiven)
            model->HICUMalrth = 0.0;

        if(!model->HICUMcthGiven)
            model->HICUMcth = 0.0;

        if((model->HICUMrthGiven) && (model->HICUMcth < 1e-12))
            model->HICUMcth = 1e-12;

//Compatibility with V2.1
        if(!model->HICUMflcompGiven)
            model->HICUMflcomp = 0.0;


        if(!model->HICUMvbeMaxGiven)
            model->HICUMvbeMax = 1e99;

        if(!model->HICUMvbcMaxGiven)
            model->HICUMvbcMax = 1e99;

        if(!model->HICUMvceMaxGiven)
            model->HICUMvceMax = 1e99;

        /* loop through all the instances of the model */
        for (here = HICUMinstances(model); here != NULL ;
                here=HICUMnextInstance(here)) {
            CKTnode *tmpNode;
            IFuid tmpName;

            if(!here->HICUMareaGiven) {
                here->HICUMarea = 1.0;
            }
            if(!here->HICUMmGiven) {
                here->HICUMm = 1.0;
            }
            if(!here->HICUMdtempGiven) {
                here->HICUMdtemp = 0.0;
            }

            here->HICUMstate = *states;
            *states += HICUMnumStates;

            if(model->HICUMrcx == 0) {
                here->HICUMcollCINode = here->HICUMcollNode;
            } else if(here->HICUMcollCINode == 0) {
                error = CKTmkVolt(ckt,&tmp,here->HICUMname,"collCI");
                if(error) return(error);
                here->HICUMcollCINode = tmp->number;
                if (ckt->CKTcopyNodesets) {
                  if (CKTinst2Node(ckt,here,1,&tmpNode,&tmpName)==OK) {
                     if (tmpNode->nsGiven) {
                       tmp->nodeset=tmpNode->nodeset;
                       tmp->nsGiven=tmpNode->nsGiven;
                    }
                  }
                }
            }
            if(model->HICUMrbx == 0) {
                here->HICUMbaseBPNode = here->HICUMbaseNode;
            } else if(here->HICUMbaseBPNode == 0){
                error = CKTmkVolt(ckt,&tmp,here->HICUMname, "baseBP");
                if(error) return(error);
                here->HICUMbaseBPNode = tmp->number;
                if (ckt->CKTcopyNodesets) {
                  if (CKTinst2Node(ckt,here,2,&tmpNode,&tmpName)==OK) {
                     if (tmpNode->nsGiven) {
                       tmp->nodeset=tmpNode->nodeset;
                       tmp->nsGiven=tmpNode->nsGiven;
                     }
                  }
                }
            }
            if(model->HICUMre == 0) {
                here->HICUMemitEINode = here->HICUMemitNode;
            } else if(here->HICUMemitEINode == 0) {
                error = CKTmkVolt(ckt,&tmp,here->HICUMname, "emitEI");
                if(error) return(error);
                here->HICUMemitEINode = tmp->number;
                if (ckt->CKTcopyNodesets) {
                  if (CKTinst2Node(ckt,here,3,&tmpNode,&tmpName)==OK) {
                     if (tmpNode->nsGiven) {
                       tmp->nodeset=tmpNode->nodeset;
                       tmp->nsGiven=tmpNode->nsGiven;
                     }
                  }
                }
            }
            if(model->HICUMrsu == 0) {
                here->HICUMsubsSINode = here->HICUMsubsNode;
            } else if(here->HICUMsubsSINode == 0) {
                error = CKTmkVolt(ckt,&tmp,here->HICUMname, "subsSI");
                if(error) return(error);
                here->HICUMsubsSINode = tmp->number;
                if (ckt->CKTcopyNodesets) {
                  if (CKTinst2Node(ckt,here,4,&tmpNode,&tmpName)==OK) {
                     if (tmpNode->nsGiven) {
                       tmp->nodeset=tmpNode->nodeset;
                       tmp->nsGiven=tmpNode->nsGiven;
                     }
                  }
                }
            }

            if(here->HICUMbaseBINode == 0) {
                error = CKTmkVolt(ckt, &tmp, here->HICUMname, "baseBI");
                if(error) return(error);
                here->HICUMbaseBINode = tmp->number;
            }

            if (selfheat) {
               if (here->HICUMtempNode == 0) {                      // no external node for temperature
                  error = CKTmkVolt(ckt,&tmp,here->HICUMname,"dT"); // create internal node
                  if (error) return(error);
                     here->HICUMtempNode = tmp->number;
               }
            } else {
                if (here->HICUMtempNode > 0) { // external temp node is given, but no she parameter
                    here->HICUMtempNode = 0;
                }
            }

           if (nqs) {
            if(here->HICUMxfNode == 0) {
                error = CKTmkVolt(ckt, &tmp, here->HICUMname, "xf");
                if(error) return(error);
                here->HICUMxfNode = tmp->number;
            }
            if(here->HICUMxf1Node == 0) {
                error = CKTmkVolt(ckt, &tmp, here->HICUMname, "xf1");
                if(error) return(error);
                here->HICUMxf1Node = tmp->number;
            }
            if(here->HICUMxf2Node == 0) {
                error = CKTmkVolt(ckt, &tmp, here->HICUMname, "xf2");
                if(error) return(error);
                here->HICUMxf2Node = tmp->number;
            }
           } else {
             here->HICUMxfNode  = 0;
             here->HICUMxf1Node = 0;
             here->HICUMxf2Node = 0;
           }

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
do { if((here->ptr = SMPmakeElt(matrix, here->first, here->second)) == NULL){\
    return(E_NOMEM);\
} } while(0)
            TSTALLOC(HICUMcollCollPtr,HICUMcollNode,HICUMcollNode);
            TSTALLOC(HICUMbaseBasePtr,HICUMbaseNode,HICUMbaseNode);
            TSTALLOC(HICUMemitEmitPtr,HICUMemitNode,HICUMemitNode);
            TSTALLOC(HICUMsubsSubsPtr,HICUMsubsNode,HICUMsubsNode);

            TSTALLOC(HICUMcollCICollCIPtr,HICUMcollCINode,HICUMcollCINode);
            TSTALLOC(HICUMbaseBIBaseBIPtr,HICUMbaseBINode,HICUMbaseBINode);
            TSTALLOC(HICUMemitEIEmitEIPtr,HICUMemitEINode,HICUMemitEINode);
            TSTALLOC(HICUMbaseBPBaseBPPtr,HICUMbaseBPNode,HICUMbaseBPNode);
            TSTALLOC(HICUMsubsSISubsSIPtr,HICUMsubsSINode,HICUMsubsSINode);

            TSTALLOC(HICUMbaseEmitPtr,HICUMbaseNode,HICUMemitNode); //b-e
            TSTALLOC(HICUMemitBasePtr,HICUMemitNode,HICUMbaseNode); //e-b

            TSTALLOC(HICUMbaseBaseBPPtr,HICUMbaseNode,HICUMbaseBPNode); //b-bp
            TSTALLOC(HICUMbaseBPBasePtr,HICUMbaseBPNode,HICUMbaseNode); //bp-b
            TSTALLOC(HICUMemitEmitEIPtr,HICUMemitNode,HICUMemitEINode); //e-ei
            TSTALLOC(HICUMemitEIEmitPtr,HICUMemitEINode,HICUMemitNode); //ei-e

            TSTALLOC(HICUMsubsSubsSIPtr,HICUMsubsNode,HICUMsubsSINode); //s-si
            TSTALLOC(HICUMsubsSISubsPtr,HICUMsubsSINode,HICUMsubsNode); //si-s

            TSTALLOC(HICUMcollCIBasePtr,HICUMcollCINode,HICUMbaseNode); //b-ci
            TSTALLOC(HICUMbaseCollCIPtr,HICUMbaseNode,HICUMcollCINode); //ci-b

            TSTALLOC(HICUMcollCIEmitEIPtr,HICUMcollCINode,HICUMemitEINode); //ci-ei
            TSTALLOC(HICUMemitEICollCIPtr,HICUMemitEINode,HICUMcollCINode); //ei-ci

            TSTALLOC(HICUMbaseBPBaseBIPtr,HICUMbaseBPNode,HICUMbaseBINode); //bp-bi
            TSTALLOC(HICUMbaseBIBaseBPPtr,HICUMbaseBINode,HICUMbaseBPNode); //bi-bp

            TSTALLOC(HICUMbaseBPEmitEIPtr,HICUMbaseBPNode,HICUMemitEINode); //bp-ei
            TSTALLOC(HICUMemitEIBaseBPPtr,HICUMemitEINode,HICUMbaseBPNode); //ei-bp

            TSTALLOC(HICUMbaseBPEmitPtr,HICUMbaseBPNode,HICUMemitNode); //bp-e
            TSTALLOC(HICUMemitBaseBPPtr,HICUMemitNode,HICUMbaseBPNode); //e-bp

            TSTALLOC(HICUMbaseBPSubsSIPtr,HICUMbaseBPNode,HICUMsubsSINode); //bp-si
            TSTALLOC(HICUMsubsSIBaseBPPtr,HICUMsubsSINode,HICUMbaseBPNode); //si-bp

            TSTALLOC(HICUMbaseBIEmitEIPtr,HICUMbaseBINode,HICUMemitEINode); //ei-bi
            TSTALLOC(HICUMemitEIBaseBIPtr,HICUMemitEINode,HICUMbaseBINode); //bi-ei
            if (nqs) {
                TSTALLOC(HICUMbaseBIXfPtr    ,HICUMbaseBINode,HICUMxfNode); //ei-bi
                TSTALLOC(HICUMemitEIXfPtr    ,HICUMemitEINode,HICUMxfNode); //ei-bi
            }

            TSTALLOC(HICUMbaseBICollCIPtr,HICUMbaseBINode,HICUMcollCINode); //ci-bi
            TSTALLOC(HICUMcollCIBaseBIPtr,HICUMcollCINode,HICUMbaseBINode); //bi-ci

            TSTALLOC(HICUMbaseBPCollCIPtr,HICUMbaseBPNode,HICUMcollCINode); //bp-ci
            TSTALLOC(HICUMcollCIBaseBPPtr,HICUMcollCINode,HICUMbaseBPNode); //ci-bp

            TSTALLOC(HICUMsubsSICollCIPtr,HICUMsubsSINode,HICUMcollCINode); //si-ci
            TSTALLOC(HICUMcollCISubsSIPtr,HICUMcollCINode,HICUMsubsSINode); //ci-si

            TSTALLOC(HICUMcollCICollPtr,HICUMcollCINode,HICUMcollNode); //ci-c
            TSTALLOC(HICUMcollCollCIPtr,HICUMcollNode,HICUMcollCINode); //c-ci

            TSTALLOC(HICUMsubsCollPtr,HICUMsubsNode,HICUMcollNode); //s-c
            TSTALLOC(HICUMcollSubsPtr,HICUMcollNode,HICUMsubsNode); //c-s

            if (nqs) {
                TSTALLOC(HICUMxf1Xf1Ptr   ,HICUMxf1Node   ,HICUMxf1Node);
                TSTALLOC(HICUMxf1BaseBIPtr,HICUMxf1Node   ,HICUMbaseBINode);
                TSTALLOC(HICUMxf1EmitEIPtr,HICUMxf1Node   ,HICUMemitEINode);
                TSTALLOC(HICUMxf1CollCIPtr,HICUMxf1Node   ,HICUMcollCINode);
                TSTALLOC(HICUMxf1Xf2Ptr   ,HICUMxf1Node   ,HICUMxf2Node);

                TSTALLOC(HICUMxf2Xf1Ptr   ,HICUMxf2Node   ,HICUMxf1Node);
                TSTALLOC(HICUMxf2BaseBIPtr,HICUMxf2Node   ,HICUMbaseBINode);
                TSTALLOC(HICUMxf2EmitEIPtr,HICUMxf2Node   ,HICUMemitEINode);
                TSTALLOC(HICUMxf2CollCIPtr,HICUMxf2Node   ,HICUMcollCINode);
                TSTALLOC(HICUMxf2Xf2Ptr   ,HICUMxf2Node   ,HICUMxf2Node);
                TSTALLOC(HICUMemitEIXf2Ptr,HICUMemitEINode,HICUMxf2Node);
                TSTALLOC(HICUMcollCIXf2Ptr,HICUMcollCINode,HICUMxf2Node);

                TSTALLOC(HICUMxfXfPtr     ,HICUMxfNode,HICUMxfNode);
                TSTALLOC(HICUMxfEmitEIPtr ,HICUMxfNode,HICUMemitEINode);
                TSTALLOC(HICUMxfCollCIPtr ,HICUMxfNode,HICUMcollCINode);
                TSTALLOC(HICUMxfBaseBIPtr ,HICUMxfNode,HICUMbaseBINode);


            }

            TSTALLOC(HICUMbaseBPSubsSIPtr ,HICUMbaseBPNode,HICUMsubsSINode);
            TSTALLOC(HICUMsubsSIBaseBPPtr ,HICUMsubsSINode,HICUMbaseBPNode);

            if (selfheat) {
                TSTALLOC(HICUMcollTempPtr, HICUMcollNode, HICUMtempNode);
                TSTALLOC(HICUMbaseTempPtr,HICUMbaseNode,HICUMtempNode);
                TSTALLOC(HICUMemitTempPtr,HICUMemitNode,HICUMtempNode);

                TSTALLOC(HICUMcollCItempPtr,HICUMcollCINode,HICUMtempNode);
                TSTALLOC(HICUMbaseBItempPtr,HICUMbaseBINode,HICUMtempNode);
                TSTALLOC(HICUMbaseBPtempPtr,HICUMbaseBPNode,HICUMtempNode);
                TSTALLOC(HICUMemitEItempPtr,HICUMemitEINode,HICUMtempNode);
                TSTALLOC(HICUMsubsSItempPtr,HICUMsubsSINode,HICUMtempNode);
                TSTALLOC(HICUMsubsTempPtr  ,HICUMsubsNode  ,HICUMtempNode);
                TSTALLOC(HICUMcollTempPtr  ,HICUMcollNode  ,HICUMtempNode);

                TSTALLOC(HICUMtempCollPtr,HICUMtempNode,HICUMcollNode);
                TSTALLOC(HICUMtempBasePtr,HICUMtempNode,HICUMbaseNode);
                TSTALLOC(HICUMtempEmitPtr,HICUMtempNode,HICUMemitNode);

                TSTALLOC(HICUMtempCollCIPtr,HICUMtempNode,HICUMcollCINode);
                TSTALLOC(HICUMtempBaseBIPtr,HICUMtempNode,HICUMbaseBINode);
                TSTALLOC(HICUMtempBaseBPPtr,HICUMtempNode,HICUMbaseBPNode);
                TSTALLOC(HICUMtempEmitEIPtr,HICUMtempNode,HICUMemitEINode);
                TSTALLOC(HICUMtempSubsSIPtr,HICUMtempNode,HICUMsubsSINode);

                TSTALLOC(HICUMtempTempPtr,HICUMtempNode,HICUMtempNode);

                if (nqs) {
                    TSTALLOC(HICUMxfTempPtr   ,HICUMxfNode,HICUMtempNode);
                    TSTALLOC(HICUMxf2TempPtr  ,HICUMxf2Node   ,HICUMtempNode);
                    TSTALLOC(HICUMxf1TempPtr  ,HICUMxf1Node   ,HICUMtempNode);
                }
           } 
        }
    }
    return(OK);
}

int
HICUMunsetup(
    GENmodel *inModel,
    CKTcircuit *ckt)
{
    HICUMmodel *model;
    HICUMinstance *here;

    for (model = (HICUMmodel *)inModel; model != NULL;
        model = HICUMnextModel(model))
    {
        int selfheat = (((model->HICUMflsh == 1) || (model->HICUMflsh == 2)) && (model->HICUMrthGiven) && (model->HICUMrth > 0.0));
        int nqs      = ( (model->HICUMflnqs != 0 || model->HICUMflcomp == 0.0 || model->HICUMflcomp == 2.1) && (model->HICUMalit > 0 || model->HICUMalqf > 0));

        for (here = HICUMinstances(model); here != NULL;
                here=HICUMnextInstance(here))
        {
            if (here->HICUMbaseBINode > 0)
                CKTdltNNum(ckt, here->HICUMbaseBINode);
            here->HICUMbaseBINode = 0;

            if (here->HICUMsubsSINode > 0
                && here->HICUMsubsSINode != here->HICUMsubsNode)
                CKTdltNNum(ckt, here->HICUMsubsSINode);
            here->HICUMsubsSINode = 0;

            if (here->HICUMemitEINode > 0
                && here->HICUMemitEINode != here->HICUMemitNode)
                CKTdltNNum(ckt, here->HICUMemitEINode);
            here->HICUMemitEINode = 0;

            if (here->HICUMbaseBPNode > 0
                && here->HICUMbaseBPNode != here->HICUMbaseNode)
                CKTdltNNum(ckt, here->HICUMbaseBPNode);
            here->HICUMbaseBPNode = 0;

            if (here->HICUMcollCINode > 0
                && here->HICUMcollCINode != here->HICUMcollNode)
                CKTdltNNum(ckt, here->HICUMcollCINode);
            here->HICUMcollCINode = 0;

            if (selfheat) {
                if (here->HICUMtempNode > 5) { // it is an internal node
                     CKTdltNNum(ckt, here->HICUMtempNode);
                     here->HICUMtempNode = 0;
                }
            }

           if (nqs) {

           if(here->HICUMxfNode > 0)
                CKTdltNNum(ckt, here->HICUMxfNode);
            here->HICUMxfNode = 0;

            if(here->HICUMxf1Node > 0)
                CKTdltNNum(ckt, here->HICUMxf1Node);
            here->HICUMxf1Node = 0;

            if(here->HICUMxf2Node > 0)
                CKTdltNNum(ckt, here->HICUMxf2Node);
            here->HICUMxf2Node = 0;
           }

        }
    }
    return OK;
}