#ifndef bbsFusion_header_20120330
#define bbsFusion_header_20120330

class TriTreeViewer;
class JimModelGroup;
class ImageSlices;
class landmarkPair;
//it is only a test class 
class JimModelDebug;// *mdMR, *mdTumor;
class Array1DPoint3f;
class Array1Dptr;
class Array1Ddouble;
class Array1Dint;

#ifdef AppBBSfusion_DLL
#define AppBBSfusion_API __declspec(dllexport)
#else
#ifdef USE_STATIC
#define AppBBSfusion_API 
#else
#define AppBBSfusion_API   __declspec(dllimport) 
#endif
#endif


//AppBBSfusion_API

class  AppBBSfusion_API bbsFusion
{
	// Data
public:
	TriTreeViewer *m_triFather;
	JimModelGroup *m_JimModelGroup;  // ??? JimModelGroup ???
	double m_tran[16];	// Store the transform from MR model to US model

	ImageSlices *m_imUSseg;//=f->ensureVol("US_seg");
	ImageSlices *m_imMRIseg;//=f->ensureVol("MRI_seg");

	ImageSlices *m_imUS;//=f->ensureVol("US");
	ImageSlices *m_imMRI;//=f->ensureVol("MRI");
	ImageSlices *m_imMRIreg;//=f->ensureVol("MRI_reg");
	ImageSlices *m_imMRIsegReg;//=f->ensureVol("MRI_seg_reg");

	landmarkPair *m_landmarkPair;

	JimModelDebug *m_mdMR;
	JimModelDebug *m_mdUS;
	JimModelDebug *m_mdTumor;

	Array1Dptr *m_mdTumors;

	// Be used for obtaining a contour from a model
	Array1Ddouble *m_conX;
	Array1Ddouble *m_conY;
	Array1Dint  *m_conEndAt;

	// Methods
public:
	bbsFusion(TriTreeViewer *f=0);
	~bbsFusion();

	// Initialization
	int Init(TriTreeViewer *f=0);

	// Load images
	int loadImage();
	int LoadImages(char* fileUS, char* fileMR);

	// Load models
	int LoadUSModel(char* modelFile);
	int LoadMRModel(char* modelFile);
	int loadModel(); // for test wujz
	int loadModel(char *fileName, char *toNode, Array1Dptr *mds);

	// Update model's transform (world2image matrix) due to changing main volume
	int UpdateModelTransform();

	// Convert geometrical model to volume
	int model2Volume();

	// Register MR-US models based on their bounding box
	int RegisterByModelBoundBox();
	int RegisterByVolumeBoundBox();

	// Register MR-US models based on ICP algorithm
	int RegisterByICP();
	int icpRegistration();
	int icpRegistration1();

	double pointAfineRegstrate(Array1DPoint3f *vv1,Array1DPoint3f *vv2, double a16[16]) ;
	double pointICP(Array1DPoint3f *vv1,Array1DPoint3f *vv2, double maxd,double a16[16]);
	double pointICP1(Array1DPoint3f *fromP, Array1DPoint3f *toP, double maxd,double a16[16]) ;

	// Register MR-US models based on RBF technique
	int RegisterByRBF();
	int rbfRegistration();
	int testRBF();
	int surfaceRBF();
	//int surfaceRBF2();

	int CheckLandmarkEditor(int checked);
	// Generate landmarks from the model surface
	int GenLandmarkByModelSurface();

	int mdHardmul();
	
	// Debugg the point pairs in an illustrative way
	void debugDisplayPairs(Array1DPoint3f *from, Array1DPoint3f *to);
	void SubdivideMRModel();

	// Save the registered MR model into a file
	int SaveRegisteredModelFile(char *fileName, int numOfCut);
};


#define AppBBSfusion_APIC   extern "C" __declspec(dllexport) 

// Start the MIUE system
AppBBSfusion_APIC int startBBSfusion();
// Show the MIUE system
AppBBSfusion_APIC void showMIUE(int flag=1); //flag==1, show, flag==0 hide
// Close the MIUE system
AppBBSfusion_APIC int exitBBSfusion();

AppBBSfusion_APIC bbsFusion* getBBSFusionInstance();

// Load US/MR images
//AppBBSfusion_APIC int LoadUSMRImage(char* fileUS, char* fileMR);
//AppBBSfusion_APIC int setUSimage(unsigned char ***d, int sx, int sy, int sz, double scalex, double scaley, double scalez, int dataLen,  double orgx=0, double orgy=0, double orgz=0);
//AppBBSfusion_APIC int setMRimage(unsigned char ***d, int sx, int sy, int sz, double scalex, double scaley, double scalez, int dataLen,  double orgx=0, double orgy=0, double orgz=0);

// Load US/MR models
//AppBBSfusion_APIC int loadURcoutours(char *fileName);
//AppBBSfusion_APIC int loadMRcoutours(char *fileName);

// Bound box based rigid registration
//AppBBSfusion_APIC int RegisterByVolumeBoundBox();
// ICP-based rigid registration
//AppBBSfusion_APIC int icpRegistration();

// Generate landmarks from the model surface
//AppBBSfusion_APIC int genLandmarkByModelSurface(bool subdiv=false);
// RBF-based registration
//AppBBSfusion_APIC int rbfRegistration();

// Function: getContoursAtZ
// Description: Obtain the contour at z position
// Return: the number of contour 
// it return all points in contours, in format: (x0 x1, x1,...); (y0, y1, y2....)
// it return the end index of each contour in the point array,
//    all points in date are pair of lines
//   example, it there are two contours
//           p0(x0,y0)---p1(x1,y1)   q0(x4,y4)--q1 (x5,y5)
//           |               |        |      /
//           p3(x3,y3)---p2(x2,y2)    q2 (x6,y6)
//
//it will return 2
// in x, y, it will have
//             0  1 2  3  4  5  6  7  8  9  10  11 12 13
//            x0 x1 x1 x2 x2 x3 x3 x0 x4 x5 x5 x6 x6 x4           
//            y0 y1 y1 y2 ............y4.............y4
// in endIdx, it will have two integers,
//                                 7 13
//            which indicate: the contour 1 has eight points from index 0 to index 7
//                             the contour 2 has six points from index 8 to index 13    
// you need not to care the two pointers data and endIdx
AppBBSfusion_APIC int getContoursAtZ(int z, double * &x, double * &y, int * &endIdx);

// Save the registered MR model into a file
//AppBBSfusion_APIC int saveRegisteredModelFile(char *fileName, int numOfCut);


#endif