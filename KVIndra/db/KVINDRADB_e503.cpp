/*
$Id: KVINDRADB_e503.cpp,v 1.1 2009/01/22 15:39:26 franklan Exp $
$Revision: 1.1 $
$Date: 2009/01/22 15:39:26 $
*/

//Created by KVClassFactory on Wed Jan 21 13:44:30 2009
//Author: Chbihi 

#include "KVINDRADB_e503.h"

ClassImp(KVINDRADB_e503)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRADB_e503</h2>
<h4>Database for e503 & e494s experiments with VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRADB_e503::KVINDRADB_e503()
{
   // Default constructor
}

KVINDRADB_e503::~KVINDRADB_e503()
{
   // Destructor
}


//________________________________________________________________

void KVINDRADB_e503::Build () 
{
	// Read runlist and systems list, then read VAMOS scalers
	
   KVINDRADB::Build();
   ReadVamosScalers();
}

//________________________________________________________________

 KVINDRADB_e503::KVINDRADB_e503 (const Char_t * name) : KVINDRADB(name)
{
}

//________________________________________________________________

void KVINDRADB_e503::ReadVamosScalers () 
{
      ifstream fin;

    if( !OpenCalibFile("VamosScaler", fin) ){
         Warning("ReadVamosScalers", "VAMOS scalers file not found : %s",
					GetCalibFileName("VamosScaler"));
         return;
   }
   Info("ReadVamosScalers", "Reading in Vamos Scaler file : %s",
			GetCalibFileName("VamosScaler"));

   Float_t  NormScaler = 0;
   Float_t DT=0;
   Float_t NormVamos = 0;
	TString sline; 
   Int_t irun=0;
   Int_t ScalerValue;
   Char_t Crunnumber[40];
   Char_t str[30];
   Bool_t runfound = kFALSE;
 	while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (!fin.eof()) {          //fin du fichier
		   if (sline.Sizeof() > 1 && !sline.BeginsWith("#") ){
            if (sline.BeginsWith("ScalersOfRunNumber")){
               sscanf(sline.Data(), "%30s %d ", Crunnumber, &irun); 
               KVINDRADBRun * idb = GetRun(irun);
			   if (idb){
					runfound = kTRUE;
               for (Int_t i =0;i<5;i++) {       //retrouve dans les 4 prochaines lignes NormVamos,normSca etDT
					   sline.ReadLine(fin);
               
					   if (sline.BeginsWith("NormVamos")){
						   sscanf(sline.Data(), "%20s %f ", str, &NormVamos);
                    idb->Set(str,NormVamos);
					   } 
					   if(sline.BeginsWith("NormScalers")){
						   sscanf(sline.Data(), "%20s %f ", str, &NormScaler);
                     idb->Set(str,NormScaler);
					   } 
					   if(sline.BeginsWith("DT")){
						   sscanf(sline.Data(), "%20s %f ", str, &DT);
                     idb->Set(str,DT);
					   } 
               
               }
					for (Int_t i = 0;i<40;i++){
						sline.ReadLine(fin);
						sscanf(sline.Data(), "%s %d ", str, &ScalerValue);
                  idb->SetScaler(str,ScalerValue);
					}
                  					
			   }
             }
			}   
		} 
	}			
   fin.close();

         
         
         
}
