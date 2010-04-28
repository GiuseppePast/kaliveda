/***************************************************************************
*                        GTGanilData.cpp  -  Main Header to ROOTGAnilTape
*                             -------------------
*    begin                : Thu Jun 14 2001
*    copyright            : (C) 2001 by Garp
*    email                : patois@ganil.fr
***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#ifdef CCIN2P3_XRD
#include "TSystem.h"
#endif
// ganil_tape lib headers
#include <ERR_GAN.H>
#include <gan_tape_erreur.h>
#include <gan_tape_general.h>
#include <gan_tape_param.h>
#include <gan_tape_test.h>
#include <gan_tape_alloc.h>
#include <gan_tape_mount.h>
#include <gan_tape_file.h>
#include <acq_mt_fct_ganil.h>
#include <acq_ebyedat_get_next_event.h>
#include <gan_tape_get_parametres.h>
extern "C" {
   typedef struct SCALE {
      UNSINT32 Length; /* Nb bytes utils suivant ce mot */
      UNSINT32 Nb_channel;
      UNSINT32 Acq_status;
      UNSINT32 Reserve[2];
      union JBUS_SCALE {
         scale_struct UnScale[NB_MAX_CHANNEL];
         UNSINT16 Info_jbus  [NB_MAX_JBUS];
      } jbus_scale;
   } scale;
}

#include "GTGanilData.h"

bool AutoswBuf, Swbufon;
Int_t BufSize;

ClassImp ( GTGanilData )

//////////////////////////////////////////////////////////////////////////
//
// Part of ROOTGanilTape
//
// GTGanilData
//
// Read GANIL formated tapes or files.
// Usage can be as simple as:
// GTGanilData myData("filename.dat");
//
// It allow connection of data parameters values by Connect() routines
// that can operate with the index or the parameter name.
//
// Values of parameters are stored in an array of UShort_t (0 - 65535)
// Before reading a new event, every parameter is set to 65535
// Casting back this value to a Short_t (-32768 to 32768) gives "-1" which is the signal that the parameter's coder
// did not fire in the event that was read.
//////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
GTGanilData::GTGanilData()
{
  // Default constructor

  InitDefault();
}

//_____________________________________________________________________________
GTGanilData::GTGanilData(const TString filename)
{
  // Create the reading class over the given file with all defaults values

  InitDefault();
  SetFileName(filename);
}

//_____________________________________________________________________________
void GTGanilData::SetFileName(const TString filename)
{
   //Set the name of the file to read (use if default ctor is used to create object).
	//We check if 'filename' begins with "rfio:"; if so, we remove it

  fFileName=filename;
  if(filename.BeginsWith("rfio:")) fFileName.Remove(0,5);
  strncpy(fDevice->DevName,fFileName,MAX_CARACTERES);
}
  
//______________________________________________________________________________
GTGanilData::~GTGanilData             (void)
{
  // Not sure if I have to close file the delete tree or the other way round.
  if (fScalerFile) {fScalerFile->Write();fScalerFile->Close();}
  //  delete   fScalerTree; // Is it really closed by closing the File ?
  if(fScaler)delete   fScaler;
  if(fDataParameters)delete   fDataParameters;
  if(fDevice)delete   fDevice;
  if(fBuffer)delete   fBuffer;
  if(fStructEvent)delete[] fStructEvent;
  if(fDataArray) delete[] fDataArray;
  if(fEventBrut) delete[] fEventBrut;
  if(fEventCtrl) delete[] fEventCtrl;
}


//______________________________________________________________________________
void GTGanilData::InitDefault(const Int_t argc, char **argv)
{
  // PRIVATE
  // Called by every constructors.
  // Init parameters to default values
  // Data are taken first from command line,
  // Second from environment variables
  // if none works, takes defaults values.
#ifdef CCIN2P3_XRD
   //if we are using version of GanTape compiled with Xrootd support
   //we need to load the ROOT XrdPosix library now
   gSystem->Load("libXrdPosix");
#endif
  fStatus         = ACQ_OK;
  fDataParameters = new GTDataParameters();
  fEventNumber    = -1;
  fEventCount     = 0;
  fIsCtrl         = false;
  fIsScalerBuffer = false;
  fDataArray      = 0;

  fDevice         = new gan_tape_desc;
  fBuffer         = new in2p3_buffer_struct;
  fStructEvent    = new char[STRUCTEVENTSIZE];

  fScalerTree     = NULL; // By default, no scaler tree
  fScalerFile     = NULL; // By default, no scaler file
  fScaler         = new GTScalers; // Scaler class

  val_ret DeRetour;   // Union to communicate with ganil_tape lib routines
  // Getting Logical Unit ID number
  fStatus = acq_get_param_env (lun_id,&DeRetour,argc,argv );
  if ( DeRetour.Val_INT >= 0 ) fLun = DeRetour.Val_INT;
  else fLun=10; // Random
  fDevice->Lun=fLun;
  // Getting tape/file name
  fStatus = acq_get_param_env ( device_id, &DeRetour, argc, argv );
  strncpy(fDevice->DevName, DeRetour.Val_CAR,MAX_CARACTERES);
  fFileName=DeRetour.Val_CAR;
  
  // GLOBAL VARIABLES ASSIGNED HERE
  fStatus = acq_get_param_env ( autoswbuf_id, &DeRetour, argc, argv );
  AutoswBuf = DeRetour.Val_BOL;
            
  fStatus = acq_get_param_env ( swbufon_id, &DeRetour, argc, argv );
  Swbufon = DeRetour.Val_BOL;

  // Variable/fix length events 
  fStatus = acq_get_param_env ( ctrlform_id, &DeRetour, argc, argv );
  fCtrlForm = DeRetour.Val_INT;
  
  fStatus = acq_get_param_env ( density_id, &DeRetour, argc, argv );
  fDensity = DeRetour.Val_INT;

  fStatus = acq_get_param_env ( bufsize_id, &DeRetour, argc, argv );
  fBufSize = DeRetour.Val_INT;
  // GLOBAL VARIABLES ASSIGNED HERE
  BufSize=fBufSize;

  fStatus = acq_get_param_env ( evbsize_id, &DeRetour, argc, argv );
  fEvbsize = DeRetour.Val_INT;

  fStatus = acq_get_param_env ( evcsize_id, &DeRetour, argc, argv );
  fEvcsize = DeRetour.Val_INT;

  fEventBrut = new UShort_t[fEvbsize];
  fEventCtrl = new UShort_t[fEvcsize];
}

//______________________________________________________________________________
void GTGanilData::PrintRunParameters(void) const
{
  // Print every class parameters, for now a simple dump.
  DumpParameterName();
}

//______________________________________________________________________________
void GTGanilData::PrintDataParameters(void) const
{
  // Not implemented.
  // ...
}

//______________________________________________________________________________
void GTGanilData::Open(void)
{
  // Open the data file (could be on a tape) with a few checks.
   // Use IsOpen() to check whether the file is opened successfully.
  // After successfully opening the file, we call ReadParameters()
   // to fill the parameter list from the first data buffer
   // which *has to* be a parameter buffer.

  Int_t structEvent_size=STRUCTEVENTSIZE; // bidon
  
  fStatus=acq_dev_is_alloc_c(*fDevice); // Allocation test
  if (fStatus==ACQ_OK) fAllocated=true;
  else {
    gan_tape_erreur(fStatus,"test d'allocation");
    return;
  }

  fStatus=acq_mt_alloc_c(*fDevice);     // Allocation
  if (fStatus!=ACQ_OK) {
    if (fStatus==ACQ_NOTALLOC) 
      cout << "This tape drive is already in use !"<<endl;
    gan_tape_erreur(fStatus,"allocation");
    return;
  }

  fStatus=acq_mt_mount_c(*fDevice,fDensity,fBufSize);
  if (fStatus!=ACQ_OK) {  // There might be some imprecisions here
    if (fStatus == ACQ_ALREADYMOUNT)
      cout << "The tape you want to use is already mounted"<<endl;
    gan_tape_erreur(fStatus,"mount");
    return;
  }
  
  fStatus = acq_mt_open_c(fDevice, o_read, &fLun);
  if (fStatus!=ACQ_OK) {  // There might be some imprecisions here
    gan_tape_erreur(fStatus,"open");
    return;
  }
  fRunNumber=0;
  fStatus = acq_mt_ini_run_c ( fLun, fBuffer, fBufSize, &fRunNumber,
			      fStructEvent,structEvent_size );

  if ( fStatus != ACQ_OK ) {
    gan_tape_erreur (fStatus , "Init Run");
    return;
  }
  cout << "Run "<<fRunNumber<<" opened"<<endl;
  ReadParameters();
  return;  
}


//______________________________________________________________________________
void GTGanilData::ReadParameters(void)
{
  // PRIVATE
  // Read the data parameters from the current buffer, put it in the 
  // parameter list.

  do {
    ReadBuffer();
    if (strcmp (fHeader, PARAM_Id) == 0 )
      fDataArraySize=fDataParameters->Fill(fBuffer->les_donnees.cas.Buf_param);
  } while (strcmp (fHeader, PARAM_Id) == 0);
  
  fDataArray = new UShort_t[fDataArraySize+1]; // Data buffer is allocated
  for(Int_t i=1; i<=fDataArraySize; i++){
	  fDataArray[i] = (Short_t)-1;
  }
}

//______________________________________________________________________________
void GTGanilData::Connect(const Int_t index,UShort_t **p) const
{
  // Connect a pointer to a data to the defined index in the Data Array
  
  if ((index<1)||(index>fDataArraySize)) {
    cout <<"Invalid connexion:"<<index<<". Valid only in 1<=index<="
	 <<fDataArraySize<<endl; 
    return;
  }
  *p=&(fDataArray[index]);
}

//______________________________________________________________________________
Bool_t GTGanilData::Connect(const TString parName,UShort_t **p) const
{
  // Connect a pointer to a data to a given parameter name in the Data Array

  Int_t index=fDataParameters->GetIndex(parName);
  if(index<0){
	  //parameter not found
	  return kFALSE;
  }
  Connect(index,p);
  return kTRUE;
}

//______________________________________________________________________________
bool GTGanilData::Next(void)
{
  // Read an event on tape/file and put in into the event array
  // return true until read fails 
  if (fEventNumber==-1) ReadBuffer();
  if (fStatus) return(false); // Maybe more to say here
  TString _heads(fHeader); _heads.ToUpper();
  if ( _heads.Contains("ENDRUN") )
  {
	  //we have reached the end of the file
	  return(false);
  }
  if (strcmp (fHeader , SCALER_Id ) == 0 ) 
  {
    cout << "Scaler buffer"<<endl;
    switch(fWhatScaler)
    {
    case kSkipScaler:     return(Next()); // Skip
    case kReportScaler:   fIsScalerBuffer=true; return(true); // Got scaler evnt
    case kAutoWriteScaler: {
      //      cout <<"kAutoWriteScaler: not implemented!"<<endl;
      fScaler->Fill((scale*)&(fBuffer->les_donnees.cas.scale));
      fScaler->DumpScalers();
      fScalerTree->Fill();
      return(Next());}
      break;
    default:
      cout <<"GTGanilData::Next: unexpected value for fWhatScaler"<<endl;
    }
  }
  if ( strcmp (fHeader , EVENTDB_Id ) == 0 )  {
    fIsScalerBuffer=false;
    fIsCtrl = false;
    fEventCount++;
    return(ReadNextEvent());
  }
  else if ( strcmp (fHeader , EVENTCT_Id ) == 0 ) {
    fIsScalerBuffer=false;
    fIsCtrl = true;
    fEventCount++;
    return(ReadNextEvent());   
  }
  else if ( strcmp (fHeader , EBYEDAT_Id ) == 0 ) {
    fIsScalerBuffer=false;
    fIsCtrl = false;
    fEventCount++;
    return(ReadNextEvent_EBYEDAT());   
  }
  cout<<"Unknown header in GTGanilData::Next:"<< fHeader <<endl;
  return(true);
}

//______________________________________________________________________________
void GTGanilData::MakeTree(const TString filename, UInt_t nEvents)
{
  // Automatically create and fill a tree created from ganil data.
  // Can be use to convert a ganil tape or file to a ROOT file.
  // The number of actual converted events is set with the nEvents parameter.

  TString theFilename;
  if (filename=="")
    theFilename=fFileName+".root";
  else theFilename=filename;
  TFile theTreeFile(theFilename,"recreate");
  TTree theTree("AutoTree","Automatic filled Tree");
  for (Int_t i=1;i<=fDataArraySize;i++)
  {
    TString parName=fDataParameters->GetParName(i);
    TString parType=parName+"/s";
    theTree.Branch(parName,&(fDataArray[i]),parType);
  }

  while (Next())
  {
    theTree.Fill();
    nEvents--;
    if (nEvents<=0) break;
  }
  theTree.Write();
}


//______________________________________________________________________________
bool GTGanilData::ReadNextEvent(void)
{
  // PRIVATE
  // Utility routine to read events from buffers.

  if (!fIsCtrl)
  {
    fStatus = get_next_event(fBuffer,fBufSize,
			     (short*)fEventBrut,fEvbsize,&fEventNumber );
    if ( fStatus == ACQ_OK ) {
      fStatus = s_evctrl ((short*)fEventBrut,(short*)fEventCtrl,
			  fStructEvent,&fCtrlForm );
      if ( fStatus == GR_OK )
	return(EventUnravelling((CTRL_EVENT*)fEventCtrl));
      else {
	puts ("\n>>> Erreur de reconstruction");
	return(false);
      }
    }
    else if (fStatus == ACQ_ENDOFBUFFER ) {
      fEventNumber=-1;
      return(Next());
    }
    else {
      gan_tape_erreur (fStatus,"obtention d'evenement");
      return(false);
    }
  }
  else
  {
    fStatus = get_next_event(fBuffer,fBufSize,(short*)fEventCtrl,
			    fEvcsize,&fEventNumber );
    if ( fStatus == ACQ_OK ) {
      return(EventUnravelling((CTRL_EVENT*)fEventCtrl));
    }
    else if ( fStatus == ACQ_ENDOFBUFFER ) {
      fEventNumber=-1;
      return(Next());
    }
    else {
      gan_tape_erreur (fStatus,"obtention d'evenement");
      return(false);
    }
  }
}
//______________________________________________________________________________

bool GTGanilData::ReadNextEvent_EBYEDAT(void)
{
  // PRIVATE
  // Utility routine to read EBYEDAT events from buffers.
   //Shamelessly copied from Luc Legeard's GEvent
  if (!fIsCtrl)
  {
     UNSINT16* ebyeEventCtrl;
    fStatus = acq_ebyedat_get_next_event((UNSINT16*)fBuffer, &ebyeEventCtrl, &fEventNumber, EVCT_VAR);
      if ( fStatus == ACQ_OK ) 	return(EventUnravelling((CTRL_EVENT*)ebyeEventCtrl));
  else 
	if ( fStatus == ACQ_ENDOFBUFFER ) 
	  {
	    fEventNumber=-1;
	    return(Next());
	  }
	else 
	  {
	    cout<< " in ReadNextEvent__EBEYEDAT error status: " << fStatus <<"  n";
	    return(false);
	  }
    }
 else
   {
     fStatus = ACQ_BADEVENTFORM;
   }
 return(false);
}
//______________________________________________________________________________
void GTGanilData::ReadBuffer(void)
{
  // PRIVATE
  // Read a single buffer from file

  fStatus = acq_mt_read_c ( fLun, fBuffer->Buffer, &fBufSize );
  //if ( fStatus != ACQ_OK )
    //cout << "Not OK"<<endl; // Maybe stuff to do to handle the error

  strncpy(fHeader,fBuffer->les_donnees.Ident, 8);
  fHeader[8]='\0';
}

//______________________________________________________________________________
bool GTGanilData::EventUnravelling(CTRL_EVENT* pCtrlEvent)
{
  // PRIVATE
  // If mode is variable length event, we have to reconstruct the Data buffer
  // from the given event.
  // WARNING: temporary the default: we dont check that it's really the case
  // Before reading event, all parameters have their value set to -1 (65535 - fDataArray is UShort_t)
	// Parameters which are not fired in the event will have value -1 (65535 - cast back to Short_t for real value)

  Short_t *brutData     = &(pCtrlEvent->ct_par);

  Int_t eventLength = pCtrlEvent->ct_len;
  //Info("EventUnravelling","eventLength=%d",eventLength);

  for(Int_t i=1; i<=fDataArraySize; i++){
	  fDataArray[i] = (Short_t)-1;
  }
  
  for(Int_t i=0;i<eventLength;i+=2)
  {
	 // cout << "Param index=" << brutData[i] << " Value=" << brutData[i+1]<<endl;
    if (brutData[i]<=fDataArraySize && brutData[i]>=1)
	 {
      fDataArray[brutData[i]]=brutData[i+1];
		}
    else // More on error handling would be cool
    {
/*      cout << "Index overflow : Parameter index "<<i<<" is "<<brutData[i]<<
		" but fDataArraySize is " << fDataArraySize<<endl;
      return(false);
*/
	}
  }
  return(true);
}




//______________________________________________________________________________
void GTGanilData::DumpEvent(void) const
{
  // Dump parameter index, name and value for the current event.

  cout <<"--------- DUMPING EVENT ----------------------------"<<endl;
  cout <<"------- number:"<<fEventCount<<endl;
  for (Int_t i=1;i<=fDataArraySize;i++)
  {
    if((Short_t)fDataArray[i]>-1) cout << "index :"<<i<<" "<<fDataParameters->GetParName(i)<<" : "<<fDataArray[i]<<endl;
  }
  cout <<"--------- END           ----------------------------"<<endl;
}

//______________________________________________________________________________
void GTGanilData::DumpParameterName(void) const
{
  // Dump parameter index and name

  cout <<"--------- DUMPING PARAMETERS  ----------------------------"<<endl;
  cout <<"------- number:"<<fEventCount<<endl;
  for (Int_t i=1;i<=fDataArraySize;i++)
  { // WARNING : index start at one, the first value is boggus
    cout << "index :"<<i<<" "<<fDataParameters->GetParName(i)<<endl;
  }
  cout <<"--------- END           ----------------------------"<<endl;
}

//______________________________________________________________________________
void GTGanilData::SetScalerBuffersManagement(const ScalerWhat_t sc )
{
  // Scaler buffers management can be:
  //  kSkipScaler,     : Skip it.
  //  kReportScaler,   : Report the existance of a scaler buffer. WARNING! user
  //                     have to take care of it.
  //  kAutoWriteScaler : Automatic scaler buffer management, not implemented.    

  // Cases where one change this setup during a run should be added.
  // Until then, DONT DO IT !
  
  fWhatScaler=sc;

  if (fWhatScaler==kAutoWriteScaler) {
    fScalerFile = new TFile("scaler.root","recreate"); // Temporary hard coded name
    fScalerTree = new TTree("AutoScalers","Automatic filled scalers");
    fScalerTree->Branch("scalers",fScaler,8000,99);
  }
}

//______________________________________________________________________________
const UShort_t* GTGanilData::GetBrutScalerBuffer    (void            ) const
{
  // This very primitiv function should be replaced by a far better one 
  // returning a full Scaler Class.
  return((UShort_t*)(&(fBuffer->les_donnees.cas.scale))); 
}

//______________________________________________________________________________

Int_t GTGanilData::GetRunNumber(void) const
{
	//Returns current run number
   return fRunNumber;
}

//______________________________________________________________________________
  
Bool_t GTGanilData::IsOpen(void) const
{
   return (fStatus==ACQ_OK && fLun);
}

//______________________________________________________________________________

void GTGanilData::SetUserTree(TTree*)
{
   //Not used
}