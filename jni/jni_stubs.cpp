//
// Quick-n-dirty JNI interface for using libmodplug in
// Android (P.A. Casey  www.peculiar-games.com)
//
// go ahead and use it however you like, although
// attribution is always appreciated!
//

// JNI and your interface class headers
#include <jni.h>
#include <android/log.h>

#include "modplug.h"
#include "stdafx.h"
#include "sndfile.h"


struct _ModPlugFile
{
	CSoundFile mSoundFile;
};

// for using usleep() when debugging crashes (to give time for log text to show up??)
//#include <fcntl.h>

#include "andmodplug_PlayerThread.h"


	const ModPlug_Settings gSettings8000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING    |
		MODPLUG_ENABLE_NOISE_REDUCTION |
		MODPLUG_ENABLE_REVERB,

		2,
		16,
		8000,
		MODPLUG_RESAMPLE_LINEAR,
		128,
		32,
		50,
		100,
		0,
		0,
		0
	};

	const ModPlug_Settings gSettings16000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING    |
		MODPLUG_ENABLE_NOISE_REDUCTION |
		MODPLUG_ENABLE_REVERB,

		2,
		16,
		16000,
		MODPLUG_RESAMPLE_LINEAR,
		128,
		32,
		50,
		100,
		0,
		0,
		0
	};

	const ModPlug_Settings gSettings22000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING    |
		MODPLUG_ENABLE_NOISE_REDUCTION |
		MODPLUG_ENABLE_REVERB,

		2,
		16,
		22000,
		MODPLUG_RESAMPLE_LINEAR,
		128,
		32,
		50,
		100,
		0,
		0,
		0
	};

	const ModPlug_Settings gSettings32000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING    |
		MODPLUG_ENABLE_NOISE_REDUCTION |
		MODPLUG_ENABLE_REVERB,

		2,
		16,
		32000,
		MODPLUG_RESAMPLE_LINEAR,
		128,
		32,
		50,
		100,
		0,
		0,
		0
	};

//
// ADD FOLLOWING JNI INTERFACE FUNCTIONS after the header files
//
// ************************************************************ 
// Start of JNI stub code
// ************************************************************ 
ModPlugFile *currmodFile;

#define SAMPLEBUFFERSIZE 40000

unsigned char samplebuffer[SAMPLEBUFFERSIZE];

int currsample;
void *Cbuffer;

// DIAB hack to change tempo!!
int DIABtempochange;
int DIABtempooverride;
int DIABholdpattern;
int DIABnextpattern;
int DIABpatternchanged;
int DIABoverridejump;
int DIABforcedpatternchange;

// more generalized pattern jumping...
int ANDMODPLUGpatternfrom;
int ANDMODPLUGpatternto;
int ANDMODPLUGpendingfrom;
int ANDMODPLUGpendingto;
int ANDMODPLUGpatternrangeset;

int ANDMODPLUGnextpattern;
int ANDMODPLUGnextpatternmode;

int ANDMODPLUGjumpeffect;

int ANDMODPLUGlogoutput;

/*
 * Class:     com_peculiar_1games_andmodplug_PlayerThread
 * Method:    ModPlug_Init
 * Signature: (I)Z
 */

JNIEXPORT jboolean JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1Init
  (JNIEnv *env, jclass cls, jint rate)

{
  // if trying to make this truly re-entrant, separate buffers could be allocated here

	__android_log_print(ANDROID_LOG_INFO, "JNI_STUBS", "Initializing modplug with rate %d", rate);

	//usleep(16*1000);

	switch (rate) {
	case 8000:
		ModPlug_SetSettings(&gSettings8000);
		break;
	case 16000:
		ModPlug_SetSettings(&gSettings16000);
		break;
	case 22000:
		ModPlug_SetSettings(&gSettings22000);
		break;
	case 32000:
		ModPlug_SetSettings(&gSettings32000);
		break;
	case 44100:
		// this is the default, so settings needn't be changed
		break;
	}

	// DIAB pitch mod
	//DIABpitchchange = 0;
	DIABtempochange = 0;
	DIABtempooverride = 0;

	DIABpatternchanged = 0;


	ANDMODPLUGpatternrangeset = 0;

	ANDMODPLUGnextpattern = -1;
	ANDMODPLUGnextpatternmode = 0;

	ANDMODPLUGjumpeffect = -1;

	ANDMODPLUGlogoutput = 0;

  return 1;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_JLoad
 * Signature: ([BI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1JLoad
  (JNIEnv *env, jobject obj, jbyteArray buffer, jint size)
{
  int csize = (int) size;

  // set the current sample as already beyond end of buffer (so a reload happens immediately)
  currsample = SAMPLEBUFFERSIZE+1;

  // convert from Java buffer into a C buffer
  Cbuffer = (void *) env->GetByteArrayElements(buffer, 0);
  currmodFile = ModPlug_Load(Cbuffer, csize);

  // test out looping
  //currmodFile->mSoundFile.LoopPattern(0,0);
  //DIABholdpattern = 1;
  //DIABnextpattern = 1;

  DIABpatternchanged = 0;

  ANDMODPLUGpatternfrom = 0;
  ANDMODPLUGpatternto = 0;
  ANDMODPLUGpendingfrom = 0;
  ANDMODPLUGpendingto = 0;
  ANDMODPLUGpatternrangeset = 0;

  ANDMODPLUGnextpattern = -1;
  ANDMODPLUGnextpatternmode = 0;

  ANDMODPLUGjumpeffect = -1;


  if (currmodFile) {
    return 1;
  }
  else {
    return 0;
  }
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_JGetName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1JGetName
  (JNIEnv *env, jobject obj)
{
  return env->NewStringUTF(ModPlug_GetName(currmodFile));
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_JNumChannels
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1JNumChannels
  (JNIEnv *env, jobject obj)
{
  jint numchannels;

  numchannels = ModPlug_NumChannels(currmodFile);
  return numchannels;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetCurrentPos
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetCurrentPos
  (JNIEnv *, jobject)
{
	  jint curr;
	  //curr = ModPlug_GetCurrentOrder(currmodFile);
	  curr = currmodFile->mSoundFile.GetCurrentPos();
	  return curr;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetMaxPos
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetMaxPos
  (JNIEnv *, jobject)
{
	  jint maxpos;
	  maxpos = currmodFile->mSoundFile.GetMaxPosition();
	  return maxpos;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetCurrentOrder
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetCurrentOrder
  (JNIEnv *env, jobject obj)
{
	  jint curr;

	  curr = ModPlug_GetCurrentOrder(currmodFile);
	  return curr;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetCurrentPattern
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetCurrentPattern
  (JNIEnv *env, jobject obj)
{
	  jint curr;

	  curr = ModPlug_GetCurrentPattern(currmodFile);
	  return curr;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetCurrentRow
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetCurrentRow
  (JNIEnv *env, jobject obj)
{
	  jint curr;

	  curr = ModPlug_GetCurrentRow(currmodFile);
	  return curr;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_JGetSoundData
 * Signature: ([SI)I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1JGetSoundData
  (JNIEnv *env, jobject obj, jshortArray jbuffer, jint size)
{
  jint smpsize = 0;

   if (currmodFile == 0)
     return 0;

#ifndef SMALLER_READS
  if (currsample >= SAMPLEBUFFERSIZE) {

    // need to read another buffer full of sample data
    smpsize = ModPlug_Read(currmodFile, samplebuffer, SAMPLEBUFFERSIZE);
    if (smpsize) {
      currsample = 0;
    }
  }
#else // SMALLER_READS
  // IN THIS MODE, WE READ IN EXACTLY HOW MUCH JAVA requested to improve frame rate
  smpsize = ModPlug_Read(currmodFile, samplebuffer, size*sizeof(jshort));
  currsample = 0;
#endif // SMALLER_READS

  // now convert the C sample buffer data to a java short array
  if (size && samplebuffer && (smpsize || currsample < SAMPLEBUFFERSIZE)) {
    env->SetShortArrayRegion(jbuffer, 0 ,size, (jshort *) (((char *) samplebuffer)+currsample));
    currsample += size*sizeof(jshort);

    return size;
  }
  else {
    return 0;
  }
}


/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_JUnload
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1JUnload
  (JNIEnv *env, jclass cls)
{
  if (currmodFile) {
    ModPlug_Unload(currmodFile);
    currmodFile = 0;
  }

  return 1;
}


/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_CloseDown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1CloseDown
  (JNIEnv *env, jclass cls)
{
  // maybe for a proper re-entrant library, need to handle shutdown stuff, deallocting buffers
  // etc.  but for my crappy, hacky single-entry version, do nothing much...

  return 1;
}


/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_GetNativeTempo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1GetNativeTempo
  (JNIEnv *env, jclass cls)
{
	return currmodFile->mSoundFile.m_nMusicTempo;
}

/*
 * Class:     com_peculiargames_modplayer_PlayerThread
 * Method:    ModPlug_ChangeTempo
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1ChangeTempo
  (JNIEnv *env, jclass cls, jint tc)
{
	// hack the tempo
	DIABtempochange = tc;

}

/*
 * Class:     com_peculiargames_modplayer_PlayerThread
 * Method:    ModPlug_SetTempo
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetTempo
  (JNIEnv *env, jclass cls, jint to)
{
	// hack the tempo
	DIABtempooverride = to;

}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_ChangePattern
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1ChangePattern
  (JNIEnv *env, jclass cls, jint newpattern)
{
	// test out looping
	////currmodFile->mSoundFile.LoopPattern(newpattern,0);
	//currmodFile->mSoundFile.SetCurrentOrder(newpattern);
	//currmodFile->mSoundFile.SetCurrentPos(newpattern);



	//currmodFile->mSoundFile.m_nNextPattern = newpattern;
	//currmodFile->mSoundFile.m_nRestartPos = newpattern;
	DIABnextpattern = newpattern-1;
	DIABpatternchanged = 0;

}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_RepeatPattern
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1RepeatPattern
  (JNIEnv *env, jclass cls, jint pattern)
{
	  // test out looping
	  DIABholdpattern = 1;
	  DIABnextpattern = pattern-1;

	  DIABpatternchanged = 0;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_CheckPatternChange
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1CheckPatternChange
  (JNIEnv *env, jclass cls)
{
	if (DIABpatternchanged) {
		DIABpatternchanged = 0;
		return JNI_TRUE;
	}
	else
		return JNI_FALSE;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_SetPatternLoopMode
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetPatternLoopMode
  (JNIEnv *env, jclass cls, jboolean flag)
{
	if (flag == JNI_TRUE)
		DIABholdpattern = 1;
	else
		DIABholdpattern = 0;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_SetPatternLoopRange
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetPatternLoopRange
  (JNIEnv *env, jclass cls, jint from, jint to, jint when)
{
	if (ANDMODPLUGpatternrangeset == 0) {
	//if (ANDMODPLUGpatternfrom == 0 && ANDMODPLUGpatternto == 0) {
		ANDMODPLUGpatternrangeset = 1;

		ANDMODPLUGpatternfrom = from;
		ANDMODPLUGpatternto = to;
		__android_log_print(ANDROID_LOG_INFO, "JNI_STUBS", "ANDMODPLUGpatternfrom=%d to=%d",
				ANDMODPLUGpatternfrom, ANDMODPLUGpatternto);

	}
	else {
		ANDMODPLUGpendingfrom = from;
		ANDMODPLUGpendingto = to;
		__android_log_print(ANDROID_LOG_INFO, "JNI_STUBS", "ANDMODPLUGpendingfrom=%d to=%d",
				ANDMODPLUGpendingfrom, ANDMODPLUGpendingto);
	}

	switch(when) {
	case com_peculiargames_andmodplug_PlayerThread_PATTERN_CHANGE_IMMEDIATE:
		// normal SetCurrentPos() libmodplug call handles this!
		//currmodFile->mSoundFile.SetCurrentPos(from); not really
		ANDMODPLUGpatternfrom = ANDMODPLUGpendingfrom;
		ANDMODPLUGpatternto = ANDMODPLUGpendingto;
		break;
	case com_peculiargames_andmodplug_PlayerThread_PATTERN_CHANGE_AFTER_CURRENT:
		// tweak current/next pattern to force change at end
		////currmodFile->mSoundFile.m_nNextPattern = to+1;
		////currmodFile->mSoundFile.m_nCurrentPattern = to+1;
		//ANDMODPLUGpatternto = ANDMODPLUGpatternfrom;
		ANDMODPLUGpatternto = currmodFile->mSoundFile.GetCurrentPattern();
		break;
	case com_peculiargames_andmodplug_PlayerThread_PATTERN_CHANGE_AFTER_GROUP:
	default:
		break;
	}

}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_SetCurrentPattern
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetCurrentPattern
  (JNIEnv *env, jobject obj, jint pattern)
{
	ANDMODPLUGnextpattern = pattern;
	ANDMODPLUGnextpatternmode = 1;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_SetNextPattern
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetNextPattern
  (JNIEnv *env, jobject obj, jint pattern)
{
	ANDMODPLUGnextpattern = pattern;
	ANDMODPLUGnextpatternmode = 0;
}


/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_LogOutput
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1LogOutput
  (JNIEnv *env, jclass cls, jboolean flag)
{
	if (flag == JNI_TRUE)
		ANDMODPLUGlogoutput = 1;
	else
		ANDMODPLUGlogoutput = 0;
}

/*
 * Class:     com_peculiargames_andmodplug_PlayerThread
 * Method:    ModPlug_SetLoopCount
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_peculiargames_andmodplug_PlayerThread_ModPlug_1SetLoopCount
  (JNIEnv *env, jclass cls, jint loopcount)
{
	ModPlug_Settings settings;

	ModPlug_GetSettings( &settings );
	settings.mLoopCount = loopcount;
	ModPlug_SetSettings( &settings );
}

// ************************************************************ 
// End of JNI stub code for libmodplug
// ************************************************************ 
