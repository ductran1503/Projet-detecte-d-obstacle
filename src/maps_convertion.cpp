////////////////////////////////
// RTMaps SDK Component
////////////////////////////////

////////////////////////////////
// Purpose of this module :
////////////////////////////////
#include "DetectionObstacle.h"
#include "opencv\cv.h"
#include "maps_convertion.h"	// Includes the header of this component

// Use the macros to declare the inputs
MAPS_BEGIN_INPUTS_DEFINITION(MAPSconvertion)
	MAPS_INPUT("image_in", MAPS::FilterIplImage, MAPS::FifoReader)
MAPS_END_INPUTS_DEFINITION

// Use the macros to declare the outputs
MAPS_BEGIN_OUTPUTS_DEFINITION(MAPSconvertion)
    MAPS_OUTPUT("image_out",MAPS::IplImage,NULL,NULL,1)
	MAPS_OUTPUT("image_out1", MAPS::IplImage, NULL, NULL, 1)
MAPS_END_OUTPUTS_DEFINITION

// Use the macros to declare the properties
MAPS_BEGIN_PROPERTIES_DEFINITION(MAPSconvertion)
	MAPS_PROPERTY("MIN_INTENSITY", 10, false, true)
	MAPS_PROPERTY("MIN_STATURATION", 10, false, true)
	MAPS_PROPERTY("MIN_PIXELHUE", 30, false, true)
	MAPS_PROPERTY("MIN_PIXELINT", 30, false, true)
	MAPS_PROPERTY("ecartFenetre", 150, false, true)
	MAPS_PROPERTY("hauteurFenetre", 200, false, true)
	MAPS_PROPERTY("angleFenetre", 60, false, true)
MAPS_END_PROPERTIES_DEFINITION

// Use the macros to declare the actions
MAPS_BEGIN_ACTIONS_DEFINITION(MAPSconvertion)
    //MAPS_ACTION("aName",MAPSconvertion::ActionName)
MAPS_END_ACTIONS_DEFINITION

// Use the macros to declare this component (convertion) behaviour
MAPS_COMPONENT_DEFINITION(MAPSconvertion,"convertion","1.0",128,
			  MAPS::Threaded,MAPS::Threaded,
			  -1, // Nb of inputs. Leave -1 to use the number of declared input definitions
			  -1, // Nb of outputs. Leave -1 to use the number of declared output definitions
			  -1, // Nb of properties. Leave -1 to use the number of declared property definitions
			  -1) // Nb of actions. Leave -1 to use the number of declared action definitions



bool m_firstTime;
//Initialization: Birth() will be called once at diagram execution startup.			  
void MAPSconvertion::Birth()
{
	m_firstTime = true;
    // Reports this information to the RTMaps console. You can remove this line if you know when Birth() is called in the component lifecycle.
    //ReportInfo("Passing through Birth() method");
}

//ATTENTION: 
//	Make sure there is ONE and ONLY ONE blocking function inside this Core method.
//	Consider that Core() will be called inside an infinite loop while the diagram is executing.
//	Something similar to: 
//		while (componentIsRunning) {Core();}
//
//	Usually, the one and only blocking function is one of the following:
//		* StartReading(MAPSInput& input); //Data request on a single BLOCKING input. A "blocking input" is an input declared as FifoReader, LastOrNextReader, Wait4NextReader or NeverskippingReader (declaration happens in MAPS_INPUT: see the beginning of this file). A SamplingReader input is non-blocking: StartReading will not block with a SamplingReader input.
//		* StartReading(int nCount, MAPSInput* inputs[], int* inputThatAnswered, int nCountEvents = 0, MAPSEvent* events[] = NULL); //Data request on several BLOCKING inputs.
//		* SynchroStartReading(int nb, MAPSInput** inputs, MAPSIOElt** IOElts, MAPSInt64 synchroTolerance = 0, MAPSEvent* abortEvent = NULL); // Synchronized reading - waiting for samples with same or nearly same timestamps on several BLOCKING inputs.
//		* Wait(MAPSTimestamp t); or Rest(MAPSDelay d); or MAPS::Sleep(MAPSDelay d); //Pauses the current thread for some time. Can be used for instance in conjunction with StartReading on a SamplingReader input (in which case StartReading is not blocking).
//		* Any blocking grabbing function or other data reception function from another API (device driver,etc.). In such case, make sure this function cannot block forever otherwise it could freeze RTMaps when shutting down diagram.
//**************************************************************************/
//	In case of no blocking function inside the Core, your component will consume 100% of a CPU.
//  Remember that the StartReading function used with an input declared as a SamplingReader is not blocking.
//	In case of two or more blocking functions inside the Core, this is likely to induce synchronization issues and data loss. (Ex: don't call two successive StartReading on FifoReader inputs.)
/***************************************************************************/
void MAPSconvertion::Core() 
{
    // Reports this information to the RTMaps console. You can remove this line if you know when Core() is called in the component lifecycle.
    //ReportInfo("Passing through Core() method");
	MAPSIOElt* ioEltIn1 = StartReading(Input("image_in"));
	if (ioEltIn1 == NULL)
		return;
	IplImage iplImg = ioEltIn1->IplImage();
	if (m_firstTime) // First time we pass into Core
	{
		m_firstTime = false; // We won't pass anymore
		Output(1).AllocOutputBufferIplImage(iplImg);
		Output(0).AllocOutputBufferIplImage(iplImg);
		//Output(0).AllocOutputBuffer(ioEltIn1->BufferSize()); // Allocate the buffer
		//Output(1).AllocOutputBuffer(ioEltIn1->BufferSize()); // Allocate the buffer
	}

	Mat img(&iplImg);
	Mat detection = detect(img, GetIntegerProperty("ecartFenetre"), GetIntegerProperty("hauteurFenetre"), 60, GetIntegerProperty("MIN_INTENSITY"), GetIntegerProperty("MIN_STATURATION"), GetIntegerProperty("MIN_PIXELHUE"), GetIntegerProperty("MIN_PIXELINT"));
	IplImage* ipl_detection = cvCloneImage(&(IplImage)detection);

	MAPSIOElt* ioEltOut1 = StartWriting(Output("image_out"));
	MAPSIOElt* ioEltOut2 = StartWriting(Output("image_out1"));
	ioEltOut2->IplImage() = ioEltIn1->IplImage();
	ioEltOut1->IplImage() = *ipl_detection;

	ioEltOut1->Timestamp() = ioEltIn1->Timestamp();
	ioEltOut2->Timestamp() = ioEltIn1->Timestamp();
    // Sleeps during 500 milliseconds (500000 microseconds).
	//This line will most probably have to be removed when you start programming your component.
	// Replace it with another blocking function. (StartReading?)
	//cvReleaseImage(&ipl_detection);
	StopWriting(ioEltOut1);
	StopWriting(ioEltOut2);
}

//De-initialization: Death() will be called once at diagram execution shutdown.
void MAPSconvertion::Death()
{
    // Reports this information to the RTMaps console. You can remove this line if you know when Death() is called in the component lifecycle.
    ReportInfo("Passing through Death() method");
}
