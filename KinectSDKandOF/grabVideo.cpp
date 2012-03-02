// For configuring DMO properties
#include <wmcodecdsp.h>
// For discovering microphone array device
#include <MMDeviceApi.h>
#include <devicetopology.h>

#include "stdafx.h"
#include "grabVideo.h"
#include "MSR_NuiApi.h"



#define SAFE_ARRAYDELETE(p) {if (p) delete[] (p); (p) = NULL;}
#define SAFE_RELEASE(p) {if (NULL != p) {(p)->Release(); (p) = NULL;}}

#define CHECK_RET(hr, message) if (FAILED(hr)) { printf("%s: %08X\n", message, hr); goto exit;}
#define CHECKHR(x) hr = x; if (FAILED(hr)) {printf("%d: %08X\n", __LINE__, hr); goto exit;}
#define CHECK_ALLOC(pb, message) if (NULL == pb) { puts(message); goto exit;}
#define CHECK_BOOL(b, message) if (!b) { hr = E_FAIL; puts(message); goto exit;}


// Helper functions used to discover microphone array device
HRESULT GetMicArrayDeviceIndex(int *piDeviceIndex);
HRESULT GetJackSubtypeForEndpoint(IMMDevice* pEndpoint, GUID* pgSubtype);

HRESULT DShowRecord();
HRESULT WriteToFile(HANDLE hFile, void* p, DWORD cb);
HRESULT WriteWaveHeader(HANDLE hFile, WAVEFORMATEX *pWav, DWORD *pcbWritten);
HRESULT FixUpChunkSizes(HANDLE hFile, DWORD cbHeader, DWORD cbAudioData);
// Takes in an x and y depth value, returns corresponding x and y  values of the color image
void KinectGrabber::Kinect_ColorFromDepth(LONG depthX, LONG depthY, LONG *pColorX, LONG *pColorY) {
	NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, NULL, LONG(depthX/2), LONG(depthY/2), m_depthBuffer[depthY*DEPTH_WIDTH + depthX] << 3, pColorX, pColorY); 
}

// initialize properties to default values
void KinectGrabber::Kinect_Zero()
{
    m_hNextDepthPlayerFrameEvent = NULL;
	m_hNextDepthFrameEvent = NULL;
    m_hNextVideoFrameEvent = NULL;
    m_hNextSkeletonFrameEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_pVideoStreamHandle = NULL;
    m_pDepthPlayerStreamHandle = NULL;
    	
	// audio
	mmHandle = NULL;
	pDMO = NULL;  
    pPS = NULL;
    dBeamAngle = 0;
	dAngle = 0;
	dConf = 0;

}

// Initialize reading data from kinect
HRESULT KinectGrabber::Kinect_Init() {
	
	HRESULT hr;

    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );    
	m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );    
	m_hNextSkeletonFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );    
	m_hNextDepthPlayerFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	hr = NuiInitialize( 
        NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
    if( FAILED( hr ) )
    {
		printf("failed to inialize nui");
	}
	hr = NuiSkeletonTrackingEnable( m_hNextSkeletonFrameEvent, 0 );
    if( FAILED( hr ) )
    {
		printf("failed to open skeleton tracking.");//    MessageBoxResource(m_hWnd,IDS_ERROR_SKELETONTRACKING,MB_OK | MB_ICONHAND);
        return hr;
    }
	hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );
    if( FAILED( hr ) )
    {
		printf("failed to open NuiImagesStream");
        return hr;
    }
	hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
    if( FAILED( hr ) )
    {
    	printf("failed to open NuiImagesStream");
        return hr;
    }
	//return hr;

	



	//audio init?


	//init other added parameters
	isSkeletonTracked=false;

}


void KinectGrabber::Kinect_UnInit( )
{
    // Shut down Nui and clean up data stream objects
    NuiShutdown( );

    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextSkeletonFrameEvent && ( m_hNextSkeletonFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonFrameEvent );
        m_hNextSkeletonFrameEvent = NULL;
    }
	if( m_hNextDepthPlayerFrameEvent && ( m_hNextDepthPlayerFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthPlayerFrameEvent );
        m_hNextDepthPlayerFrameEvent = NULL;
    }


	// Shutdown of audio stuff
	SAFE_RELEASE(pDMO);
    SAFE_RELEASE(pPS);

    AvRevertMmThreadCharacteristics(mmHandle);
    CoUninitialize();
}


int KinectGrabber::Kinect_Update()
{

    HANDLE                hEvents[3];
    int                    nEventIdx;

    // Configure events to be listened on
    hEvents[0]=m_hNextVideoFrameEvent;
	hEvents[1]=m_hNextDepthFrameEvent;
	hEvents[2]=m_hNextSkeletonFrameEvent;
    
    // Wait for an event to be signalled
    nEventIdx=WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,INFINITE);
    //printf("index obtained %d out of %d\n",nEventIdx, sizeof(hEvents)/sizeof(hEvents[0]));
	
	// TODO: Fix the structure so that the case statement is actually working. 
	// Right now, the Depth and Skeleton alerts are being called on every event, which is wrong.
	// Not sure why only video events are being registered.
    
	// If the stop event, stop looping and exit
    //if(nEventIdx==0)
        //break;            
	//	return 1;
		
        // Process signal events
        switch(nEventIdx)
        {
            case 0:
                Kinect_GotVideoAlert();
				break;
			
			case 1:
                Kinect_GotDepthAlert();
                break;
					
            case 2:
            //    Kinect_GotSkeletonAlert();
                break;
        }
		Kinect_GotDepthAlert();
		Kinect_GotSkeletonAlert();
		

    return (0);
}

void KinectGrabber::Kinect_GotVideoAlert( )
{
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        &pImageFrame );
    if( FAILED( hr ) )
    {
		printf("Unable to get the frame after recieving alert for video frame \n");
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
        memcpy(m_rgbBuffer, LockedRect.pBits, sizeof(BYTE) * VIDEO_HEIGHT * VIDEO_WIDTH * 4);
		//m_rgbBuffer = (BYTE*) LockedRect.pBits;
		Kinect_FormatRGBForOutput();	
		Kinect_makeRGBFromRGBA();
		//2560 bytes per line = 640 * 4 (4 bytes per pixel)
	} else {
        printf("buffer length of recieved texture is bogus\n");
    }
	NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}


void KinectGrabber::Kinect_FormatRGBForOutput() {
	int totalPixels = VIDEO_HEIGHT*VIDEO_WIDTH*4;
	//printf("%d\n",totalPixels);
	for (int i = 0; i < totalPixels; i= i + 4) {
		//set alpha to 255
		m_rgbBuffer[i+3] = 255;

		//invert the rgb order
		int blue = m_rgbBuffer[i+2];
		int red = m_rgbBuffer[i];
		m_rgbBuffer[i+2] = red;
		m_rgbBuffer[i] = blue;

	}
}


void KinectGrabber::Kinect_makeRGBFromRGBA() {
	int totalPixels = VIDEO_HEIGHT*VIDEO_WIDTH;
	for (int i = 0; i < totalPixels; i++) {
		m_rgb_noalpa_Buffer[3*i] = m_rgbBuffer[4*i];
		m_rgb_noalpa_Buffer[3*i+1] = m_rgbBuffer[4*i+1];
		m_rgb_noalpa_Buffer[3*i+2] = m_rgbBuffer[4*i+2];

	}
}


void KinectGrabber::Kinect_GotDepthAlert( ) {
	const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &pImageFrame );

    if( FAILED( hr ) )
    {
    	printf("Unable to get the frame after recieving alert for depth frame \n");
		return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

        // draw the bits to the bitmap
        RGBQUAD * rgbrun = m_rgbDepth;
		USHORT * depthrun = m_depthBuffer;
		USHORT * playerrun = m_playerBuffer;
        USHORT * pBufferRun = (USHORT*) pBuffer;
        for( int y = 0 ; y < DEPTH_HEIGHT ; y++ )
        {
            for( int x = 0 ; x < DEPTH_WIDTH ; x++ )
            {
				// set the color (just for visualization)
                RGBQUAD quad = Kinect_DepthToRGB( *pBufferRun );
                *rgbrun = quad;
                rgbrun++;
				
				//USHORT RealDepth = (*pBufferRun & 0xfff8) >> 3;
				USHORT RealDepth = (*pBufferRun & 0x0fff);			
				//*depthrun = RealDepth;
				//depthrun++;
				//flip the depth image to match the video image
				m_depthBuffer[((y+1)*DEPTH_WIDTH) - x] = RealDepth;

				USHORT Player = *pBufferRun  & 7;
				*playerrun = Player;
				playerrun++;

				//inc buffer pointer
				pBufferRun++;
            }
        }
		
    }
    else
    {
        printf( "Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );

}

/*
void KinectGrabber::print_bytes( ) {
	//2560 bytes per line = 640 * 4 (4 bytes per pixel)
	//printf("byte data written: r:%d, g:%d, b:%d, other:%d\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]);
}
*/


BYTE* KinectGrabber::Kinect_getAlphaPixels() {
	return m_rgbBuffer;
}

BYTE* KinectGrabber::Kinect_getRGBBuffer() {
	return m_rgb_noalpa_Buffer;
}

USHORT* KinectGrabber::Kinect_getPlayerBuffer() {
	return m_playerBuffer;
}

USHORT* KinectGrabber::Kinect_getDepthBuffer() {
	return m_depthBuffer;
}

//use for visualizing depth
RGBQUAD* KinectGrabber::Kinect_getDepthPixels() {
	return m_rgbDepth;
}
RGBQUAD KinectGrabber::Kinect_DepthToRGB( USHORT s )
{
    //USHORT RealDepth = (s & 0xfff8) >> 3;
	USHORT RealDepth =  (s & 0x0fff);
    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    
    q.rgbRed = l / 2;
    q.rgbBlue = l / 2;
    q.rgbGreen = l / 2;
 
	return q;
}

USHORT		  m_playerJointDepth[NUI_SKELETON_POSITION_COUNT];
POINT         m_Points[NUI_SKELETON_POSITION_COUNT];
void KinectGrabber::Kinect_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame;

    HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );

	isSkeletonTracked=false;
	//POINT         m_Points[NUI_SKELETON_POSITION_COUNT];

    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
        {
			isSkeletonTracked=true;
			
			int scaleX = VIDEO_WIDTH; //scaling up to image coordinates
			int scaleY = VIDEO_HEIGHT;
			USHORT depthValue;
			float fx=0,fy=0;
			//POINT         m_Points[NUI_SKELETON_POSITION_COUNT];
			NUI_SKELETON_DATA * pSkel;
			
			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
			{
				pSkel=& SkeletonFrame.SkeletonData[i];
				NuiTransformSkeletonToDepthImageF( pSkel->SkeletonPositions[j], &fx, &fy, &depthValue);
				m_Points[j].x = (int) ( fx * scaleX + 0.5f );
				m_Points[j].y = (int) ( fy * scaleY + 0.5f );
				// make sure to shift depth by 3 bits
				m_playerJointDepth[j] = depthValue >> 3;
				//m_playerJointDepth[j] = depthValue;	
			}

			// Store the values of the head position of each skeleton
			headXValues[i] = m_Points[3].x;
			headYValues[i] = m_Points[3].y;
			headZValues[i] = m_playerJointDepth[3];

			leftShoulderXValues[i]=m_Points[4].x;
			rightShoulderXValues[i]=m_Points[8].x;

			leftHandXValues[i]=m_Points[7].x;
			leftHandYValues[i]=m_Points[7].y;

			rightHandXValues[i]=m_Points[11].x;
			rightHandYValues[i]=m_Points[11].y;
			/*
			if(m_Points[3].x!=0){
				//printf("headPosition");
				//printf("head Z=%4.2f /r", m_Points[3].x);
				//printf("%d\n",m_Points[3].x);
				//return m_Points[NUI_SKELETON_POSITION_COUNT];
            }else{
				NuiSkeletonGetNextFrame( 0, &SkeletonFrame );
            }
			*/
			/*
            float head_z=SkeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
            if (head_z!=0){
            printf("head Z=%4.2f /r", head_z);
            }else{
            NuiSkeletonGetNextFrame( 0, &SkeletonFrame );
            }
			*/	
        } else {
			// if that skeleton is no longer tracked, set the X values to some default value so that it isn't accidentally chosen while the person is off screen
			headXValues[i] = -1000;
			leftShoulderXValues[i]=-2000;
			rightShoulderXValues[i]=2000;
			leftHandXValues[i]=-2000;
			rightHandXValues[i]=2000;
		}
    }

    // no skeletons!
    if( !isSkeletonTracked )
    {
        return;
    }

    // smooth out the skeleton data
    NuiTransformSmooth(&SkeletonFrame,NULL);
}

void KinectGrabber::getJointsPoints() {
	headJoints_x=m_Points[3].x;
	headJoints_y=m_Points[3].y;
	headJoints_z = m_playerJointDepth[3] >> 3;
	handLeft_x=m_Points[7].x;
	handLeft_y=m_Points[7].y;
	handRight_x=m_Points[11].x;
	handRight_y=m_Points[11].y;
	shoulderLeft_x=m_Points[4].x;
	shoulderLeft_y=m_Points[4].y;
	shoulderRight_x=m_Points[8].x;
	shoulderRight_y=m_Points[8].y;
	//printf("unmodified: %d shifted: %d\n", m_playerJointDepth[3], m_playerJointDepth[3] >> 3 );	
}

void KinectGrabber::recordAudioInit() {
	minDiscrepancyIdx=7;  //when skeleton is detected, the number should be between 0 to 6. Set it to a random number beyond this range is OK
	HRESULT hr = S_OK;
	int  iMicDevIdx = -1; 
	int  iSpkDevIdx = 0;  //Asume default speakers
	pDMO = NULL;  
    pPS = NULL;
    HANDLE mmHandle = NULL;
    DWORD mmTaskIndex = 0;
    szOutputFile = _T("data/sounds/out.wav");


    // Set high priority to avoid getting preempted while capturing sound
    mmHandle = AvSetMmThreadCharacteristics(L"Audio", &mmTaskIndex);
    CHECK_BOOL(mmHandle != NULL, "failed to set thread priority\n");

    // DMO initialization
 	CoCreateInstance(CLSID_CMSRKinectAudio, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&pDMO);
 	pDMO->QueryInterface(IID_IPropertyStore, (void**)&pPS);

    // Set AEC-MicArray DMO system mode.
    // This must be set for the DMO to work properly
    PROPVARIANT pvSysMode;
    PropVariantInit(&pvSysMode);
    pvSysMode.vt = VT_I4;
    //   SINGLE_CHANNEL_AEC = 0
    //   OPTIBEAM_ARRAY_ONLY = 2
    //   OPTIBEAM_ARRAY_AND_AEC = 4
    //   SINGLE_CHANNEL_NSAGC = 5
    pvSysMode.lVal = (LONG)(2);
    CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode));
    PropVariantClear(&pvSysMode);

    // Tell DMO which capture device to use (we're using whichever device is a microphone array).
    // Default rendering device (speaker) will be used.
	hr = GetMicArrayDeviceIndex(&iMicDevIdx);
    CHECK_RET(hr, "Failed to find microphone array device. Make sure microphone array is properly installed.");
    
    PROPVARIANT pvDeviceId;
    PropVariantInit(&pvDeviceId);
    pvDeviceId.vt = VT_I4;
	//Speaker index is the two high order bytes and the mic index the two low order ones
    pvDeviceId.lVal = (unsigned long)(iSpkDevIdx<<16) | (unsigned long)(0x0000ffff & iMicDevIdx);
    CHECKHR(	pPS->SetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, pvDeviceId););
    PropVariantClear(&pvDeviceId);

	
	
	exit:
    // main loop to get mic output from the DMO
    puts("\nMicArray is running \n");


}

void KinectGrabber::recordAudioStart(){

	WAVEFORMATEX wfxOut = {WAVE_FORMAT_PCM, 1, 16000, 32000, 2, 16, 0};

	pSC = NULL;

    cOutputBufLen = 0;
    pbOutputBuffer = NULL;

	//OutputBufferStruct = {0};
	OutputBufferStruct.pBuffer = &outputBuffer;
    DMO_MEDIA_TYPE mt = {0};

    cbProduced = 0;

    // Set DMO output format
    HRESULT hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    CHECK_RET(hr, "MoInitMediaType failed");
    
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;	
    memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));
    
    hr = pDMO->SetOutputType(0, &mt, 0); 
    CHECK_RET(hr, "SetOutputType failed");
    MoFreeMediaType(&mt);

    // Allocate streaming resources. This step is optional. If it is not called here, it
    // will be called when first time ProcessInput() is called. However, if you want to 
    // get the actual frame size being used, it should be called explicitly here.
    hr = pDMO->AllocateStreamingResources();
    CHECK_RET(hr, "AllocateStreamingResources failed");
    
    // Get actually frame size being used in the DMO. (optional, do as you need)
    int iFrameSize;
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, &pvFrameSize));
    iFrameSize = pvFrameSize.lVal;
    PropVariantClear(&pvFrameSize);

    // allocate output buffer
    cOutputBufLen = wfxOut.nSamplesPerSec * wfxOut.nBlockAlign;
    pbOutputBuffer = new BYTE[cOutputBufLen];
    CHECK_ALLOC (pbOutputBuffer, "out of memory.\n");

	hFile = CreateFile(
            szOutputFile,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL
            );
    if(hFile == INVALID_HANDLE_VALUE)
	{		
		_tprintf(_T("Could not open the output file: %s\n"), szOutputFile);
		goto exit;
	}

	writtenWAVHeader = 0;
	WriteWaveHeader(hFile, &wfxOut, &writtenWAVHeader);
	totalBytesWritten = 0;
	
	hr = pDMO->QueryInterface(IID_ISoundSourceLocalizer, (void**)&pSC);
	CHECK_RET (hr, "QueryInterface for IID_ISoundSourceLocalizer failed");

exit:
	return;
}

void KinectGrabber::recordAudioEnd(){

//	#pragma warning(pop)

	FixUpChunkSizes(hFile, writtenWAVHeader, totalBytesWritten);
	CloseHandle(hFile);



    SAFE_ARRAYDELETE(pbOutputBuffer);    
	SAFE_RELEASE(pSC);
	
	DWORD dwRet = GetFullPathName(szOutputFile, (DWORD)ARRAYSIZE(szOutfileFullName), szOutfileFullName,NULL);
	HRESULT hr;
	CHECK_BOOL(dwRet != 0, "\nSound output could not be written");

    _tprintf(_T("\nSound output was written to file: %s\n"),szOutfileFullName);
exit:
	return;
}

///////////////////////////////////////////////////////////////////////////
// DShowRecord
//
// Uses the DMO in source mode to retrieve clean audio samples and record
// them to a .wav file.
//
///////////////////////////////////////////////////////////////////////////
HRESULT KinectGrabber::DShowRecord()
{
HRESULT hr;
//#pragma warning(push)
//#pragma warning(disable: 4127) // conditional expression is constant

    do{
            outputBuffer.Init((byte*)pbOutputBuffer, cOutputBufLen, 0);
            OutputBufferStruct.dwStatus = 0;
            hr = pDMO->ProcessOutput(0, 1, &OutputBufferStruct, &dwStatus);
            CHECK_RET (hr, "ProcessOutput failed. You must be rendering sound through the speakers before you start recording in order to perform echo cancellation.");

            if (hr == S_FALSE) {
                cbProduced = 0;
            } else {
                hr = outputBuffer.GetBufferAndLength(NULL, &cbProduced);
                CHECK_RET (hr, "GetBufferAndLength failed");
            }
			
			WriteToFile(hFile, pbOutputBuffer, cbProduced);
			totalBytesWritten += cbProduced;

            // Obtain beam angle from ISoundSourceLocalizer afforded by microphone array
			hr = pSC->GetBeam(&dBeamAngle);
			double dConf;
			hr = pSC->GetPosition(&dAngle, &dConf);
			if(SUCCEEDED(hr))
			{								
				
				//Use a moving average to smooth this out
				if(dConf>0.9)
				{					
					//_tprintf(_T("Position: %f\t\tConfidence: %f\t\tBeam Angle = %f\r"), dAngle, dConf, dBeamAngle);					
					
				// Map the width sound angle to a pixel
				soundPixel = max( min(640, (dAngle + 0.33) * VIDEO_WIDTH), 0);
				/*printf("------------------------------------------\n");
				printf(" sound \n");
				printf("------------------------------------------\n");
				printf("angle %f \n", dAngle);
				printf("the pixel number corresponding to sound %f \n", soundPixel);*/

				}
			}

    } while (OutputBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);
		

    
	exit:
    return hr;
}

	

///////////////////////////////////////////////////////////////////////////
// WriteWaveHeader
//
// Writes a WAVE file header placeholder that will be updated by
// FixUpChunkSizes after recording is complete.
//
///////////////////////////////////////////////////////////////////////////
HRESULT WriteWaveHeader(
    HANDLE hFile,               // Output file.
    WAVEFORMATEX *pWav,
    DWORD *pcbWritten           // Receives the size of the header.    
    )
{
    HRESULT hr = S_OK;
    UINT32 cbFormat = sizeof(WAVEFORMATEX);
	*pcbWritten = 0;       

    // Write the 'RIFF' header and the start of the 'fmt ' chunk.

    if (SUCCEEDED(hr))
    {
        DWORD header[] = { 
            // RIFF header
            FCC('RIFF'), 
            0, 
            FCC('WAVE'),  
            // Start of 'fmt ' chunk
            FCC('fmt '), 
            cbFormat 
        };

        DWORD dataHeader[] = { FCC('data'), 0 };

        hr = WriteToFile(hFile, header, sizeof(header));

        // Write the WAVEFORMATEX structure.
        if (SUCCEEDED(hr))
        {
            hr = WriteToFile(hFile, pWav, cbFormat);
        }

        // Write the start of the 'data' chunk

        if (SUCCEEDED(hr))
        {
            hr = WriteToFile(hFile, dataHeader, sizeof(dataHeader));
        }

        if (SUCCEEDED(hr))
        {
            *pcbWritten = sizeof(header) + cbFormat + sizeof(dataHeader);
        }
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////
// FixUpChunkSizes
//
// Writes the file-size information into the WAVE file header.
// Note that WAVE files use the RIFF file format. Each RIFF chunk has a
// data size, and the RIFF header has a total file size.
//
///////////////////////////////////////////////////////////////////////////
HRESULT FixUpChunkSizes(
    HANDLE hFile,           // Output file.
    DWORD cbHeader,         // Size of the 'fmt ' chuck.
    DWORD cbAudioData       // Size of the 'data' chunk.
    )
{
    HRESULT hr = S_OK;

    LARGE_INTEGER ll;
    ll.QuadPart = cbHeader - sizeof(DWORD);

    if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Write the data size.

    if (SUCCEEDED(hr))
    {
        hr = WriteToFile(hFile, &cbAudioData, sizeof(cbAudioData));
    }

    if (SUCCEEDED(hr))
    {
        // Write the file size.
        ll.QuadPart = sizeof(FOURCC);

        if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        DWORD cbRiffFileSize = cbHeader + cbAudioData - 8;

        // NOTE: The "size" field in the RIFF header does not include
        // the first 8 bytes of the file. i.e., it is the size of the
        // data that appears _after_ the size field.

        hr = WriteToFile(hFile, &cbRiffFileSize, sizeof(cbRiffFileSize));
    }

    return hr;
}

HRESULT WriteToFile(HANDLE hFile, void* p, DWORD cb)
{
    DWORD cbWritten = 0;
    HRESULT hr = S_OK;

    BOOL bResult = WriteFile(hFile, p, cb, &cbWritten, NULL);
    if (!bResult)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}


///////////////////////////////////////////////////////////////////////////
// GetMicArrayDeviceIndex
//
// Obtain device index corresponding to microphone array device.
//
// Parameters: piDevice: [out] Index of microphone array device.
//
// Return: S_OK if successful
//         Failure code otherwise (e.g.: if microphone array device is not found).
//
///////////////////////////////////////////////////////////////////////////////
HRESULT GetMicArrayDeviceIndex(int *piDevice)
{
    HRESULT hr = S_OK;
    UINT index, dwCount;
    IMMDeviceEnumerator* spEnumerator = NULL;
    IMMDeviceCollection* spEndpoints = NULL;

    *piDevice = -1;

    CHECKHR(CoCreateInstance(__uuidof(MMDeviceEnumerator),  NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&spEnumerator));

    CHECKHR(spEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &spEndpoints));

    CHECKHR(spEndpoints->GetCount(&dwCount));

    // Iterate over all capture devices until finding one that is a microphone array
    for (index = 0; index < dwCount; index++)
    {
        IMMDevice* spDevice;

        CHECKHR(spEndpoints->Item(index, &spDevice));
        
        GUID subType = {0};
        CHECKHR(GetJackSubtypeForEndpoint(spDevice, &subType));
        if (subType == KSNODETYPE_MICROPHONE_ARRAY)
        {
            *piDevice = index;
            break;
        }
    }

    hr = (*piDevice >=0) ? S_OK : E_FAIL;

exit:
	SAFE_RELEASE(spEnumerator);
    SAFE_RELEASE(spEndpoints);    
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// GetJackSubtypeForEndpoint
//
// Gets the subtype of the jack that the specified endpoint device is plugged
// into.  E.g. if the endpoint is for an array mic, then we would expect the
// subtype of the jack to be KSNODETYPE_MICROPHONE_ARRAY
//
///////////////////////////////////////////////////////////////////////////////
HRESULT GetJackSubtypeForEndpoint(IMMDevice* pEndpoint, GUID* pgSubtype)
{
    HRESULT hr = S_OK;
    IDeviceTopology*    spEndpointTopology = NULL;
    IConnector*         spPlug = NULL;
    IConnector*         spJack = NULL;
    IPart*            spJackAsPart = NULL;
    
    if (pEndpoint == NULL)
        return E_POINTER;
   
    // Get the Device Topology interface
    CHECKHR(pEndpoint->Activate(__uuidof(IDeviceTopology), CLSCTX_INPROC_SERVER, 
                            NULL, (void**)&spEndpointTopology));

    CHECKHR(spEndpointTopology->GetConnector(0, &spPlug));

    CHECKHR(spPlug->GetConnectedTo(&spJack));

	CHECKHR(spJack->QueryInterface(__uuidof(IPart), (void**)&spJackAsPart));

    hr = spJackAsPart->GetSubType(pgSubtype);

exit:
   SAFE_RELEASE(spEndpointTopology);
   SAFE_RELEASE(spPlug);    
   SAFE_RELEASE(spJack);    
   SAFE_RELEASE(spJackAsPart);
   return hr;
}//GetJackSubtypeForEndpoint()
