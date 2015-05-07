#include "bbsFussion.h"

#include "miueTriTreeViewer.h"
#include "jimLib\ImageSlices.h"
#include "miueBase\xmldom\Xmldom.h"
#include "miueBase\GlobalClass.h"
#include "miueBase\charconfigure.h"
#include "miueJim\jimGL\SceneTree.h"
#include "miueJim\jimLib\ObjGenJM.h"
#include "miueTriUI\JimModelGroup.h"
#include "miueJim\jimLib\MedToolsJM.h"
#include "miueJim\jimLib\MedToolsJM.h"
#include "miueBase\templateUI.h"
#include "landmarkPair.h"


bbsFusion::bbsFusion(TriTreeViewer *f)
{
	ObjGen::tranMakeUnit(m_tran);
	m_landmarkPair=0;//no need to delete

	if(f!=0)
	{
		m_triFather=f;

		m_imUS=f->ensureVol("US");
		m_imUSseg=f->ensureVol("US_seg");

		m_imMRI=f->ensureVol("MRI");
		m_imMRIseg=f->ensureVol("MRI_seg");

		m_imMRIreg=f->ensureVol("MRI_reg");
		m_imMRIsegReg=f->ensureVol("MRI_seg_reg");

		SceneTree *st;
		st=f->m_sceneTree->ensureByPathName("Prostate_Models"); // modified by wujz
		m_JimModelGroup=JimModelGroup::newObject(f,st);
	}

	m_mdTumors=new Array1Dptr;

	m_conX=new Array1Ddouble();
	m_conY=new Array1Ddouble();
	m_conEndAt=new Array1Dint();
}

bbsFusion::~bbsFusion()
{
	if(m_mdTumors) delete m_mdTumors;
	if(m_conX) delete m_conX;//=new Array1Ddouble();
	if(m_conY) delete m_conY;//=new Array1Ddouble();
	if(m_conEndAt) delete m_conEndAt;//=new Array1Dint();
}

int bbsFusion::Init(TriTreeViewer *f)
{
	ObjGen::tranMakeUnit(m_tran);
	m_landmarkPair=0;//no need to delete

	if(f!=0)
	{
		m_triFather=f;

		m_imUS=f->ensureVol("US");
		m_imUSseg=f->ensureVol("US_seg");

		m_imMRI=f->ensureVol("MRI");
		m_imMRIseg=f->ensureVol("MRI_seg");

		m_imMRIreg=f->ensureVol("MRI_reg");
		m_imMRIsegReg=f->ensureVol("MRI_seg_reg");

		SceneTree *st;
		st=f->m_sceneTree->ensureByPathName("Prostate_Models"); // modified by wujz
		m_JimModelGroup=JimModelGroup::newObject(f,st);
	}

	m_mdTumors=new Array1Dptr;

	m_conX=new Array1Ddouble();
	m_conY=new Array1Ddouble();
	m_conEndAt=new Array1Dint();

	return 1;
}

int bbsFusion::loadImage()
{
	TriTreeViewer *f=this->m_triFather;

	if(1)
	{
		// Create a serial of image volumes
		m_imUS=f->ensureVol("US");
		m_imUSseg=f->ensureVol("US_seg");
		m_imMRI=f->ensureVol("MRI");
		m_imMRIseg=f->ensureVol("MRI_seg");
		m_imMRIreg=f->ensureVol("MRI_reg");
		m_imMRIsegReg=f->ensureVol("MRI_seg_reg");
	}

	// Find root directory for US/MR images
	char path[1024],buf[1024];
	strcpy(path,f->m_modulePath);
	path[strlen(path)-strlen("mdTool_share\\win32\\bin\\Debug")]=0;
	strcat(path,"_jiaze/data/prostate/US_MRI_fusion");

	// Load US images
	sprintf(buf,"%s/US data/case_2011_05_05_Sim Tiow Seng/image_000/image_param.txt", path);
	m_imUS->load(buf, true);
	//ensure (0,0,0) to physical (orgx, orgy, orgz)
	/*m_imUS->m_xorg /= -m_imUS->m_scale1;
	m_imUS->m_yorg /= -m_imUS->m_scale2;
	m_imUS->m_zorg /= -m_imUS->m_scale3;*/
	m_imUS->flipIt(1);

	// Load MR images
	sprintf(buf,"%s/MRI images/1.txt", path);
	m_imMRI->load(buf, true);
	//ensure (0,0,0) to physical (orgx, orgy, orgz)
	/*m_imMRI->m_xorg /= -m_imMRI->m_scale1;
	m_imMRI->m_yorg /= -m_imMRI->m_scale2;
	m_imMRI->m_zorg /= -m_imMRI->m_scale3;*/
	m_imMRI->flipIt(1);

	double a[16];
	m_imUS->tranMakeWorld2Image(a);
	m_imMRI->tranMakeWorld2Image(a);

	f->correlateViews();
	f->showCenterFocus();
	return 0;
}

int bbsFusion::LoadImages(char* fileUS, char* fileMR)
{
	if(m_triFather == 0) return 0;

	// Load US images
	m_imUS->load(fileUS, false);
	//ensure (0,0,0) to physical (orgx, orgy, orgz)
	m_imUS->m_xorg /= -m_imUS->m_scale1;
	m_imUS->m_yorg /= -m_imUS->m_scale2;
	m_imUS->m_zorg /= -m_imUS->m_scale3;

	// Load MR images
	m_imMRI->load(fileMR, false);
	//ensure (0,0,0) to physical (orgx, orgy, orgz)
	m_imMRI->m_xorg /= -m_imMRI->m_scale1;
	m_imMRI->m_yorg /= -m_imMRI->m_scale2;
	m_imMRI->m_zorg /= -m_imMRI->m_scale3;

	m_triFather->correlateViews();
	m_triFather->showCenterFocus(); 
	return 1;
}

int bbsFusion::LoadUSModel(char* modelFile)
{
	if(m_triFather == 0)return 0;
	if(m_imUS->isEmpty())
	{ 
		miueShowMessage("Please load US image first", 1);
		return 0;
	}

	// Load the US geometrical model
	Array1Dptr ar;
	loadModel(modelFile,"US", &ar);
	JimModelDebug **mds=(JimModelDebug**)ar.m_data;

	for(int i=0;i<ar.m_used;i++)
	{
		m_triFather->m_img->tranMakeWorld2Image(mds[i]->m_container->m_tran);
		//m_imUS->tranMakeWorld2Image(mds[i]->m_container->m_tran);
		mds[i]->m_container->tranLetSonAsMe();
	} 

	// Manage the US-derived geometrical prostate
	m_mdUS=mds[0];

	return 1;
}

int bbsFusion::LoadMRModel(char* modelFile)
{
	if(m_triFather == 0)return 0;
	if(m_imMRI->isEmpty())
	{ 
		miueShowMessage("Please set MRI image first",1);
		return 0;
	}

	// Load the MR geometrical model
	Array1Dptr ar;
	loadModel(modelFile,"MR",&ar);
	JimModelDebug **mds=(JimModelDebug**)ar.m_data;

	for(int i=0;i<ar.m_used;i++)
	{
		m_triFather->m_img->tranMakeWorld2Image(mds[i]->m_container->m_tran);
		//m_imUS->tranMakeWorld2Image(mds[i]->m_container->m_tran);
		//m_imMRI->tranMakeWorld2Image(mds[i]->m_container->m_tran);
		mds[i]->m_container->tranLetSonAsMe();
	} 

	// Manage the MR-derived geometrical prostate and its tumors
	m_mdMR=mds[0];
	m_mdTumors->clear();
	for(int i=0;i<ar.m_used;i++)
		m_mdTumors->append(mds[i]);

	return 1;
}

int bbsFusion::loadModel()
{
	TriTreeViewer *f = this->m_triFather;

	SceneTree *st;
	if(1)
	{
	/*	st = f->m_sceneTree->findByPathName("Prostate_Models");
		if(st) f->m_sceneTree->delAson(st);*/
		st = f->m_sceneTree->ensureByPathName("Prostate_Models"); // Prostate_Models wujz ?
		m_JimModelGroup = JimModelGroup::newObject(f,st);
	}

	char path[1024], buf[1024];
	strcpy(path,f->m_modulePath);
	path[strlen(path)-strlen("mdTool_share\\win32\\bin\\Debug")] = 0;
	strcat(path,"_jiaze/data/prostate/US_MRI_fusion");

	//for testingt
	double a[16];
	if(1)
	{
		sprintf(buf,"%s/jiminModel/aa.txt", path);
		m_JimModelGroup->loadModel(buf);
		f->triUIappend(m_JimModelGroup);

		m_imMRI=f->ensureVol("MRI");
		//m_imMRI->tranMakeWorld2Image(a); 
		f->m_img->tranMakeWorld2Image(a); // wujz

		st=f->m_sceneTree->findByPathName("Prostate_Models?mdMR");
		ObjGen::tranCopy(st->m_tran,a);
		st->tranLetSonAsMe();

		st=f->m_sceneTree->findByPathName("Prostate_Models?tumor");
		ObjGen::tranCopy(st->m_tran,a);
		st->tranLetSonAsMe();

		st=f->m_sceneTree->findByPathName("Prostate_Models/mdMR");
		m_mdMR=   ( (JimModelAobject *)st->m_object)->m_pModel;

		st=f->m_sceneTree->findByPathName("Prostate_Models/mdUS");
		m_mdUS=   ( (JimModelAobject *)st->m_object)->m_pModel;

		st=f->m_sceneTree->findByPathName("Prostate_Models/tumor");
		m_mdTumor=   ( (JimModelAobject *)st->m_object)->m_pModel;

		//md->m_optAutoBottomCap=1;
		//md->m_optAutoTopCap=1;
		//f->showCurrentFocus();

		return 1;
	}

	//change orignal point of the image system

	ObjGen::tranMakeUnit(a);
	a[12]=m_imUS->m_xorg;
	a[13]=m_imUS->m_yorg;
	a[14]=m_imUS->m_zorg; //

	m_imUS->m_xorg=0;
	m_imUS->m_yorg=0;
	m_imUS->m_zorg=0;
	f->correlateViews();

	//	xorg -41.768
	//  yorg 5.07374
	//  zorg 37.5

	m_JimModelGroup->funNewObject("fromUS");

	JimModelDebug *md=m_JimModelGroup->m_curObject->m_pModel;

	Xmldom dom;
	//wchar_t 	p[1024];
	char p[1024];
	sprintf(buf,"%s/US data/case_2011_05_05_Sim Tiow Seng/model_000/model.xml", path);

	//GlobalClass::mbs2wcs(buf,p,1024);

	dom.xmlFile2Tree(buf);//p); 


	Tok *tk=dom.m_tree->sons; //model --id-0

	tk=tk->next; //model --id-0 sub-model
	tk=tk->sons ;//--id-0
	tk=tk->next;// first curve 

	Tok *acurve;
	acurve=tk;
	double x,y,z;
	my_char *wd;

	//m_imUS=f->ensureVol("US");

	//	a[0]=1.0/im->m_scale1; a[4]=0;                a[8]=0;                 a[12]=0;
	//	a[1]=0;                a[5]=1.0/im->m_scale2; a[9]=0;                 a[13]=0;
	//	a[2]=0;                a[6]=0;                a[10]=1.0/im->m_scale3; a[14]=0;
	//	a[3]=0;                a[7]=0;                a[11]=0;                a[15]=1;

	int num=0;
	Array1DPoint3f vex;
	while(acurve!=0)
	{
		sprintf(buf,"US_con/%d",num);
		num++;
		tk=acurve->sons; //z=my_atof(tk->src); //z
		tk=tk->next->sons; //points
		while(tk!=0)
		{
			my_strcpy(p,tk->src); //x,y, z

			wd=my_strtok(p,my_text(","));
			x= my_atof(wd);

			wd=my_strtok(NULL,my_text(","));
			y= my_atof(wd);

			wd=my_strtok(NULL,my_text(","));
			z= my_atof(wd);

			tk=tk->next;
			vex.appendPoint3f(x,y,z);
		}

		md->contourAdd2conPart(&vex);
		vex.m_used=0;
		acurve=acurve->next;
	}

	md->m_optAutoBottomCap=1;
	md->m_optAutoTopCap=1;
	md->updateSurf();

	f->seeItInTree(st->m_father);

	return 0;
}


int bbsFusion::loadModel(char *fileName, char *toNode, Array1Dptr *mds)
{
	Xmldom dom;
	//wchar_t p[1024];
	char p[1024];
	//sprintf(buf,"%s/US data/case_2011_05_05_Sim Tiow Seng/model_000/model.xml", path);
	JimModelDebug *md=0;
	//GlobalClass::mbs2wcs(fileName,p,1024);

	//dom.xmlFile2Tree(p); 
	dom.xmlFile2Tree(fileName); 

	mds->clear();

	Tok *aSubModel=dom.m_tree->sons; //model --id-0

	aSubModel=aSubModel->next;//tk=tk->next; //model --id-0 sub-model

	char buf[1024];

	while(aSubModel!=0)
	{
		Tok *tk=aSubModel;

		aSubModel=aSubModel->next;

		tk=tk->sons ;//--id-0

		sprintf(buf,"%s_%d",toNode, my_atoi(tk->src));

		tk=tk->next;// first curve 


		if(tk==0)
			continue;

		Tok *acurve=tk;  //first curve
		double x,y,z;
		my_char *wd;

		//	a[0]=1.0/im->m_scale1; a[4]=0;                a[8]=0;                 a[12]=0;
		//	a[1]=0;                a[5]=1.0/im->m_scale2; a[9]=0;                 a[13]=0;
		//	a[2]=0;                a[6]=0;                a[10]=1.0/im->m_scale3; a[14]=0;
		//	a[3]=0;                a[7]=0;                a[11]=0;                a[15]=1;

		//char buf[1024];
		//int num=0;
		Array1DPoint3f vex;


		m_JimModelGroup->funNewObject(buf);//toNode);//"fromUS");

		md=m_JimModelGroup->m_curObject->m_pModel;

		mds->append(md);

		while(acurve!=0)
		{
			//sprintf(buf,"US_con/%d",num);
			//num++;
			tk=acurve->sons; //z=my_atof(tk->src); //z
			tk=tk->next->sons; //points
			while(tk!=0)
			{
				my_strcpy(p,tk->src); //x,y, z

				wd=my_strtok(p,my_text(","));
				x= my_atof(wd);

				wd=my_strtok(NULL,my_text(","));
				y= my_atof(wd);

				wd=my_strtok(NULL,my_text(","));
				z= my_atof(wd);

				tk=tk->next;
				vex.appendPoint3f(x,y,z);
			}

			md->contourAdd2conPart(&vex);
			vex.m_used=0;
			acurve=acurve->next;
		}

		md->m_optAutoBottomCap=1;
		md->m_optAutoTopCap=1;
		md->updateSurf();
	}

	if(md!=0)
		m_triFather->seeItInTree(md->m_container->m_father);//st->m_father);

	return 0;
}

// Update model's transform (world2image matrix) due to changing main volume
int bbsFusion::UpdateModelTransform()
{
	// US prostate
	m_triFather->m_img->tranMakeWorld2Image(m_mdUS->m_container->m_tran);
	m_mdUS->m_container->tranLetSonAsMe();

	// MR prostate and tumours
	m_triFather->m_img->tranMakeWorld2Image(m_mdMR->m_container->m_tran);
	m_mdMR->m_container->tranLetSonAsMe();

	JimModelDebug *tumor;
	for(int i = 0; i < m_mdTumors->m_used; i++)
	{
		tumor = (JimModelDebug *)((*m_mdTumors)[i]);
		m_triFather->m_img->tranMakeWorld2Image(tumor->m_container->m_tran);
		tumor->m_container->tranLetSonAsMe();
	} 

	return 1;
}

int bbsFusion::model2Volume()
{
	TriTreeViewer *f=this->m_triFather;

	ImageSlices *im, *atlas;
	ImageSlices mask;
	int x1,y1,z1;
	SceneTree *st;
	Array1Dptr mds;

	// Create atlas based 'US' volume and 'mdUS' model
	im=m_imUS;//f->ensureVol("US");
	atlas=m_imUSseg;//f->ensureVol("US_seg",1);
	if(atlas->isSameImageSizeAs(im)==0)
	{
		atlas->newBuf(im->m_sz,im->m_sy,im->m_sx,1);
		atlas->m_scale1=im->m_scale1; atlas->m_scale2=im->m_scale2; atlas->m_scale3=im->m_scale3;
		atlas->m_xorg=im->m_xorg; atlas->m_yorg=im->m_yorg; atlas->m_zorg=im->m_zorg;
		atlas->genColorTable();

		atlas->m_color2grayTable->changeColor(0,255,255,255,0);  
		atlas->m_color2grayTable->changeColor(1,255,0,0,255);  
	}
	atlas->setAll(0);
	st=f->m_sceneTree->findByPathName("Prostate_Models/mdUS/surface");
	MedTools::surfJmesh2mask(f->m_sceneTree->m_tran,st  ,&mask,x1,y1,z1);
	atlas->putObject2Me(&mask,1,x1,y1,z1);

	// Create atlas based 'MRI' volume and 'mdMR' and 'mdTumor' models
	im=m_imMRI;//f->ensureVol("MRI");
	atlas=m_imMRIseg;//f->ensureVol("MRI_seg",1);
	if(atlas->isSameImageSizeAs(im)==0)
	{
		atlas->newBuf(im->m_sz,im->m_sy,im->m_sx,1);
		atlas->m_scale1=im->m_scale1; atlas->m_scale2=im->m_scale2; atlas->m_scale3=im->m_scale3;
		atlas->m_xorg=im->m_xorg; atlas->m_yorg=im->m_yorg; atlas->m_zorg=im->m_zorg;
		atlas->genColorTable();

		atlas->m_color2grayTable->changeColor(0,255,255,255,0);  
		atlas->m_color2grayTable->changeColor(1,255,0,0,255);  
		atlas->m_color2grayTable->changeColor(2,255,255,0,255);  
	}
	atlas->setAll(0);
	st=f->m_sceneTree->findByPathName("Prostate_Models/mdMR/surface");
	MedTools::surfJmesh2mask(f->m_sceneTree->m_tran,st  ,&mask,x1,y1,z1);
	atlas->putObject2Me(&mask,1,x1,y1,z1);

	st=f->m_sceneTree->findByPathName("Prostate_Models/tumor/surface");
	MedTools::surfJmesh2mask(f->m_sceneTree->m_tran,st  ,&mask,x1,y1,z1);
	mask.changeColor(2,1,1,0,0,0,-1,-1,-1);
	atlas->putObject2Me(&mask,2,x1,y1,z1);

	f->correlateViews();
	f->showCurrentFocus(1); 
	return 1;
}

// Register MR-US models based on their bounding box
int bbsFusion::RegisterByModelBoundBox()
{
	TriTreeViewer *f = m_triFather;

	double x1,y1,z1,x2,y2,z2;
	double u1,v1,w1,u2,v2,w2;
	//deform imMRI to imMRIreg
	double a[16],b[16];

	ObjGen::tranMakeUnit(a);
	f->m_img->tranMakeImage2World(a); 
	//m_imMRI->tranMakeImage2World(a); 
	m_mdMR->m_container->getBoundBox(a,x1,y1,z1,x2,y2,z2);
	//m_imMRIseg->getBoundingBox(1,1,x1,y1,z1,x2,y2,z2);

	//m_imUS->tranMakeImage2World(a);
	m_mdUS->m_container->getBoundBox(a,u1,v1,w1,u2,v2,w2);
	//m_imUSseg->getBoundingBox(1,1,u1,v1,w1,u2,v2,w2);

	// Transform from MR to US
	ObjGen::tranMakeTranslate(a,-x1,-y1,-z1);
	ObjGen::tranMakeScaling(b, (double)(u2-u1)/(double)(x2-x1), (double)(v2-v1)/(double)(y2-y1),  (double)(w2-w1)/(double)(z2-z1) );
	ObjGen::tranMulab2c(b,a,a); 
	ObjGen::tranMakeTranslate(b,u1,v1,w1);
	ObjGen::tranMulab2c(b,a,a); 

	// Save the transform of MR2US
	//ObjGen::tranMulab2c(a, m_tran,m_tran);
	ObjGen::tranCopy(m_tran,a); // m_tran <- a ?

	SceneTree *st;
	int num = m_mdTumors->m_used;
	JimModelDebug **mds = (JimModelDebug**)( m_mdTumors->m_data );
	f->m_img->tranMakeWorld2Image(b); 
	//m_imUS->tranMakeWorld2Image(b);

	// Registrater the MR tumors to US model according to the transform
	for(int i=0;i<num;i++)
	{
		st= mds[i]->m_container;//m_mdTumors->m_data[0];
		st->tranHardMul(a); // ??? wujz transform the real geometry ?
		//ObjGen::tranCopy(st->m_tran, b);
		//st->tranLetSonAsMe();
	}

	f->showCurrentFocus();
	return 1;
}

int bbsFusion::RegisterByVolumeBoundBox()
{
	TriTreeViewer *f=this->m_triFather;

	int x1,y1,z1,x2,y2,z2;
	int u1,v1,w1,u2,v2,w2;
	double a[16],b[16];

	//deform imMRIseg to imMRIsegReg  -->imUSseg
	//deform imMRI to imMRIreg
	m_imMRIseg->getBoundingBox(1,1,x1,y1,z1,x2,y2,z2);
	m_imUSseg->getBoundingBox(1,1,u1,v1,w1,u2,v2,w2);

	ObjGen::tranMakeTranslate(a,-x1,-y1,-z1);
	ObjGen::tranMakeScaling(b, (double)(u2-u1)/(double)(x2-x1), (double)(v2-v1)/(double)(y2-y1),  (double)(w2-w1)/(double)(z2-z1) );
	ObjGen::tranMulab2c(b,a,a); 
	ObjGen::tranMakeTranslate(b,u1,v1,w1);
	ObjGen::tranMulab2c(b,a,a); 
	//ObjGen::tranMulab2c(a, m_tran,m_tran);
	ObjGen::tranCopy(m_tran,a);

	SceneTree *st;
	st=f->m_sceneTree->findByPathName("Prostate_Models?mdMR");
	ObjGen::tranMulab2c(a, st->m_tran,st->m_tran);
	st->tranLetSonAsMe();

	st=f->m_sceneTree->findByPathName("Prostate_Models?tumor");
	ObjGen::tranMulab2c(a, st->m_tran,st->m_tran);
	st->tranLetSonAsMe();

	mdHardmul();

	ObjGen::tranReverse(a);
	m_imMRIreg->newBufBy(m_imUS);
	m_imMRIreg->trans2Me(m_imMRI,a,0,1);
	m_imMRIsegReg->newBufBy(m_imUSseg);
	m_imMRIsegReg->trans2Me(m_imMRIseg,a,0,0);

	f->showCurrentFocus();
	return 1;
}

#include "miueJim\jimLib\MyBET.h"

// Register MR-US models based on ICP algorithm
int bbsFusion::RegisterByICP()
{
	Array1DPoint3f from, to;
	TriMesh * m;

	// Subdivision for US model
	m_mdUS->m_subLevel=4;
	m_mdUS->m_minEdgeLen=2;
	m_mdUS->updateSurf();
	this->m_triFather->showCurrentFocus();

	// Obtain all the vertex of the US model
	m= ( (SceneTriMesh*) (m_mdUS->m_surface->m_object) )->m_mesh;// ((SceneTriMesh*)m_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&to,1);

	// Obtain all the vertex of the MR model
	m= ( (SceneTriMesh*) (m_mdMR->m_frameBase->m_object) )->m_mesh;// ((SceneTriMesh*)m_surface->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&from);

	// Iterative closest point
	double icp[16];
	pointICP1(&from,&to,10,icp);

	ObjGen::tranMulab2c(icp,this->m_tran,this->m_tran); // m_tran = icp * m_tran

	int num=this->m_mdTumors->m_used;
	JimModelDebug **mds=(JimModelDebug**) ( this->m_mdTumors->m_data );

	// Registrater the MR tumors to US model according to the ICP transform
	for(int i=0;i<num;i++)
	{
		SceneTree *st= mds[i]->m_container;//m_mdTumors->m_data[0];
		st->tranHardMul(icp);
		//ObjGen::tranCopy(st->m_tran,b);
		st->tranLetSonAsMe();
		st->clearAllDisplayList();
	}

	this->m_triFather->showCurrentFocus();
	return 0;
}

int bbsFusion::icpRegistration()
{
	Array1DPoint3f from, to;
	TriMesh * m;

	// Obtain all the vertex of the MR model
	m= ( (SceneTriMesh*) (m_mdMR->m_frameBase->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&from);//,1);

	// Obtain all the vertex of the US model
	m= ( (SceneTriMesh*) (m_mdUS->m_surface->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&to,1);

	/*
	//m_imUSseg->tranMakeImage2World(aIm2wd);
	//m_imUSseg->tranMakeWorld2Image(aWd2im);

	//MedTools::imObjectBoundayExtract(m_imMRIsegReg,1,2,6, &edge);
	//MedTools::castPoint3iTo3f(&edge,&vv1);
	//ObjGen::tranMapPoints(aIm2wd,&vv1);

	if(0)
	{
	m_imUSseg->fillRegion(&edge,3);
	m_imUSseg->m_color2grayTable->changeColor(3,0,255,0,255);
	}

	MedTools::imObjectBoundayExtract(m_imUSseg,1,2,6, &edge);
	MedTools::castPoint3iTo3f(&edge,&vv2);
	ObjGen::tranMapPoints(aIm2wd,&vv2);

	//debug
	if(0)
	{
	m_imMRIsegReg->fillRegion(&edge,3);
	m_imMRIsegReg->m_color2grayTable->changeColor(3,0,255,0,255);
	return 1;
	}
	*/

	if(1) //debug
	{
		double x1,y1,z1,x2,y2,z2;
		double u1,v1,w1, u2,v2,w2;
		from.getBoundBox(x1,y1,z1,x2,y2,z2);
		to.getBoundBox(u1,v1,w1, u2,v2,w2);
		x1=x1;
		SceneTree *nd;
		TriTreeViewer *f=this->m_triFather;
		nd=m_mdUS->m_container->ensureByPathName("from",SCENE_OBJ_POINT);
		ScenePoints *sp=(ScenePoints*) (nd->m_object);
		sp->m_size=6;
		sp->m_points->copyFrom(&from);

		nd=m_mdUS->m_container->ensureByPathName("to",SCENE_OBJ_POINT);
		sp=(ScenePoints*) (nd->m_object);
		sp->m_size=6;
		sp->m_points->copyFrom(&to);
		f->showCurrentFocus();

	}

	//  return 0;

	double icp[16];
	// myBET::pointAfineRegstrate(&from,&to, icp); 
	pointAfineRegstrate(&from,&to, icp); 

	this->m_triFather->showCurrentFocus();

	/*
	//Array1DPoint3f vv1, vv2;
	Array1DPoint3i edge;

	double a[16], icp[16], aIm2wd[16], aWd2im[16];
	return 0;
	//image MR to image US
	ObjGen::tranCopy(a,aIm2wd);
	ObjGen::tranMulab2c(icp,a,a);
	ObjGen::tranMulab2c(aWd2im,a,a);

	TriTreeViewer *f=this->m_triFather;
	SceneTree *st;
	st=f->m_sceneTree->findByPathName("Prostate_Models?a");
	ObjGen::tranMulab2c(a, st->m_tran,st->m_tran);
	st->tranLetSonAsMe();

	st=f->m_sceneTree->findByPathName("Prostate_Models?tumor");
	ObjGen::tranMulab2c(a, st->m_tran,st->m_tran);
	st->tranLetSonAsMe();

	return 1;

	ObjGen::tranMulab2c(a,m_tran,m_tran);
	ObjGen::tranCopy(a,m_tran);
	ObjGen::tranReverse(a);
	m_imMRIsegReg->trans2Me(m_imMRIseg,a,0,0);
	m_imMRIreg->trans2Me(m_imMRI,a,0,1);
	*/
	return 0;
}



int bbsFusion::icpRegistration1()
{
	Array1DPoint3f from, to;
	TriMesh * m;

	// Subdivision for US model
	m_mdUS->m_subLevel=4;
	m_mdUS->m_minEdgeLen=2;
	m_mdUS->updateSurf();
	this->m_triFather->showCurrentFocus();

	// Obtain all the vertex of the US model
	m= ( (SceneTriMesh*) (m_mdUS->m_surface->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&to,1);

	// Obtain all the vertex of the MR model
	m= ( (SceneTriMesh*) (m_mdMR->m_frameBase->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&from);

	// Iterative closest point
	double icp[16];
	pointICP1(&from,&to,10,icp);

	m_mdMR->m_container->tranHardMul(icp);
	m_mdTumor->m_container->tranHardMul(icp);

	/*	ObjGen::tranMulab2c(a,m_tran,m_tran);
	ObjGen::tranCopy(a,m_tran);
	ObjGen::tranReverse(a);
	m_imMRIsegReg->trans2Me(m_imMRIseg,a,0,0);
	m_imMRIreg->trans2Me(m_imMRI,a,0,1);
	*/
	m_mdMR->m_container->clearAllDisplayList();
	m_mdTumor->m_container->clearAllDisplayList();

	this->m_triFather->showCurrentFocus();

	return 0;
}

double bbsFusion::pointICP(Array1DPoint3f *vv1,Array1DPoint3f *vv2, double maxd,double a16[16]) //find transform to register v2 to v1
{
	int i,num;
	Point3f *p;
	double x1,y1,z1,x2,y2,z2;
	double tran[16];

	Array1DPoint3f v1;
	Array1DPoint3f v2;

	v1.copyFrom(vv1);
	v2.copyFrom(vv2); 

	//fist find the boundbox 
	v1.getBoundBox(x1,y1,z1,x2,y2,z2);
	p=v1.m_data;
	num=v1.m_used;

	x1=x2=p[0].x;y1=y2=p[0].y; z1=z2=p[0].z;
	for(i=0;i<num;i++)
	{
		if(x1>p[i].x) x1=p[i].x;
		if(x2<p[i].x) x2=p[i].x;
		if(y1>p[i].y) y1=p[i].y;
		if(y2<p[i].y) y2=p[i].y;
		if(z1>p[i].z) z1=p[i].z;
		if(z2<p[i].z) z2=p[i].z;
	}
	Point3f *pp=v2.m_data;
	num=v2.m_used;
	for(i=0;i<num;i++)
	{
		if(x1>pp[i].x) x1=pp[i].x;
		if(x2<pp[i].x) x2=pp[i].x;
		if(y1>pp[i].y) y1=pp[i].y;
		if(y2<pp[i].y) y2=pp[i].y;
		if(z1>pp[i].z) z1=pp[i].z;
		if(z2<pp[i].z) z2=pp[i].z;
	}

	x1 -=  maxd;
	x2 +=  maxd;
	y1 -=  maxd;
	y2 +=  maxd;
	z1 -=  maxd;
	z2 +=  maxd;

	//	tran*v1<---->tran*v2
	ObjGen::tranMakeTranslate(tran,(float)-x1,(float)-y1,(float)-z1); 
	//ICPpointTrans(&v1,tran);
	ObjGen::tranMapPoints(tran,&v1);
	ObjGen::tranMapPoints(tran,&v2);
	ImageSlices im;
	im.newBuf((int)(z2-z1+1.5),(int)(y2-y1+1.5),(int)(x2-x1+1.5),1); 
	im.setAll(0); 

	// draw_surface(&im,1,1,&v1);   this !!!!!!!!!!!
	unsigned char ***d=im.m_data;
	int x,y,z;
	for(int i=0;i<v1.m_used;i++)
	{
		x=(int)(v1.m_data[i].x+0.5);
		y=(int)(v1.m_data[i].y+0.5);
		z=(int)(v1.m_data[i].z+0.5);
		d[z][y][x]=1;
	}

	Array3Dint32 map;
	Array1DPoint3i bound;
	myBET::ICPgenerateDistMap(&im,&bound,maxd,&map);

	double R16[16];
	Array1DPoint3f v3,v4;
	double b16[16];
	double minError,currentError;
	int iternum;

	// tran*v1<---->R16*tran*v2
	iternum=0;
	minError=1000*v2.m_used;
	ObjGen::tranMakeUnit(R16); 
	while(iternum<100)
	{

		myBET::ICPfindNearestPoint(&bound,&map, &v2,&v3,&v4);

		memcpy(b16,R16,sizeof(float)*16);//tmpery save the R16
		myBET::ICPlmsFIT(&v3,&v4,R16);
		ObjGen::tranMapPoints(R16,&v2); 

		ObjGen::tranMul(R16,b16); //acumulat the transform

		currentError=myBET::ICPgetError(&bound,&map,maxd,&v2);
		if(currentError>minError)
			break;
		minError=currentError;
		iternum++;
	}

	// finally the a16 is   tran~R16*tran;
	ObjGen::tranMakeUnit(a16); //a16=I
	ObjGen::tranReverse(tran); // 
	ObjGen::tranMul(a16,tran); //a16=I*tran~; 
	ObjGen::tranMul(a16,R16); //a16=tran~*R16;
	ObjGen::tranReverse(tran) ;
	ObjGen::tranMul(a16,tran); //a16=tran~*R16*tran;
	return minError;

}

// Debugg the point pairs in an illustrative way
void bbsFusion::debugDisplayPairs(Array1DPoint3f *from, Array1DPoint3f *to)
{
	TriTreeViewer *f=this->m_triFather;
	SceneTree *st=m_mdUS->m_container->ensureByPathName("fromP",SCENE_OBJ_POINT);
	((ScenePoints*)st->m_object)->m_size=6;
	((ScenePoints*)st->m_object)->m_points->copyFrom(from);

	st=m_mdUS->m_container->ensureByPathName("toP",SCENE_OBJ_POINT);
	((ScenePoints*)st->m_object)->m_size=6;
	((ScenePoints*)st->m_object)->m_points->copyFrom(to);

	st=m_mdUS->m_container->ensureByPathName("lines",SCENE_OBJ_LINE);
	((SceneLines*)st->m_object)->m_size=6;
	((SceneLines*)st->m_object)->m_points->m_used=0;//->copyFrom(&to);
	for(int i=0;i<from->m_used;i++)
	{
		((SceneLines*)(st->m_object))->m_points->append(from->m_data[i]);
		((SceneLines*) (st->m_object))->m_points->append(to->m_data[i]);
	}
}

// Find transform to register v2 to v1 on mBase
double bbsFusion::pointICP1(Array1DPoint3f *fromP, Array1DPoint3f *toP, double maxd,double a16[16]) 
{
	int iterNum = 0;

	Array1DPoint3f from,to;

	Point3f *fromV, *toV;
	int i, num;
	double len;
	double R16[16];
	double currentError,minError;
	int vid;

	minError=-1;
	maxd= maxd*maxd; //square...
	toV=toP->m_data;
	ObjGen::tranMakeUnit(a16);

	while(iterNum < 100)
	{
		fromV=fromP->m_data;
		num=fromP->m_used;

		from.m_used=0;
		to.m_used=0;
		currentError=0;

		// Find the nearest 'to' point for each 'from' point
		// and calculate the squared distance error
		for(i=0; i < num; i++)
		{
			vid = toP->findNearestPoint(fromV[i], &len);
			if(len > maxd)
				continue;

			from.append(fromV[i]);
			to.append(toV[vid]);
			currentError += (len * len);
		}

		if(1)
		{
			// Point set 'from'
			TriTreeViewer *f = this->m_triFather; // Transferred outside
			SceneTree *st = m_mdUS->m_container->ensureByPathName("fromP",SCENE_OBJ_POINT);
			((ScenePoints*)st->m_object)->m_size=6;
			((ScenePoints*)st->m_object)->m_points->copyFrom(&from);

			// Point set 'to'
			st=m_mdUS->m_container->ensureByPathName("toP",SCENE_OBJ_POINT);
			((ScenePoints*)st->m_object)->m_size=6;
			((ScenePoints*)st->m_object)->m_points->copyFrom(&to);

			// Line set
			st=m_mdUS->m_container->ensureByPathName("lines",SCENE_OBJ_LINE);
			((SceneLines*)st->m_object)->m_size=6;
			((SceneLines*)st->m_object)->m_points->m_used=0;//->copyFrom(&to);
			for(int i=0;i<from.m_used;i++)
			{
				((SceneLines*)(st->m_object))->m_points->append(from.m_data[i]);
				((SceneLines*) (st->m_object))->m_points->append(to.m_data[i]);
			}

			f->showCurrentFocus();
		}

		if ( (minError >= 0) && (minError < currentError) )
			break;
		minError = currentError;

		// Calculate the transform between 'from' and 'to'
		myBET::ICPlmsFIT(&from,&to,R16);

		// Shift point set "from" according to the transform 
		ObjGen::tranMapPoints(R16,fromP); 
		ObjGen::tranMulab2c(R16,a16,a16);// ObjGen::tranMul(R16,b16); //acumulat the transform

		iterNum++;
	}

	return 0;
}

double bbsFusion::pointAfineRegstrate(Array1DPoint3f *vv1,Array1DPoint3f *vv2, double a16[16]) //find affin transformation to registre v2 to v1
{
	TriTreeViewer *f=this->m_triFather;

	double tran1[16],tran2[16],icp[16],R1[16],R2[16];
	Array1DPoint3f v1;
	Array1DPoint3f v2;

	Point3f center1,center2;
	Array1DPoint3f axis1,axis2;

	v1.copyFrom(vv1);
	v2.copyFrom(vv2);

	myBET::pointList_mainAxis(&v1,&axis1,&center1);
	for(int i=0;i<3;i++) axis1.m_data[i]=axis1.m_data[i]*30;
	SceneTree *st=f->debugDisplayAxis(&axis1, center1, "Name1");
	m_imUS->tranMakeWorld2Image(st->m_tran);
	st->tranLetSonAsMe();


	myBET::pointList_mainAxis(&v2,&axis2,&center2);
	for(int i=0;i<3;i++) axis2.m_data[i]=axis2.m_data[i]*30;
	st=f->debugDisplayAxis(&axis2, center2, "Name2");
	m_imUS->tranMakeWorld2Image(st->m_tran);
	st->tranLetSonAsMe();

	return 0;

	myBET::axis_correlate(axis1.m_data,axis2.m_data);

	//translate v1 and v2 so that their center is shift to (0,0,0)
	//   trans1*v1<---->trans2*v2
	ObjGen::tranMakeTranslate(tran1,(float)-center1.x,(float)-center1.y,(float)-center1.z);
	ObjGen::tranMapPoints(tran1,&v1);//ICPpointTrans(&v1,tran1);

	ObjGen::tranMakeTranslate(tran2,(float)-center2.x,(float)-center2.y,(float)-center2.z);
	ObjGen::tranMapPoints(tran2, &v2);

	//rotate v1 and v2 so that there main axis is align with (x,y,z)
	//  R1*tran1*v1<--->R2*tran2*v2
	ObjGen::tranMakeUnit(R1);  
	for(int i=0;i<3;i++)
	{ 
		R1[i*4]=(float)axis1.m_data[i].x; 
		R1[i*4+1]=(float)axis1.m_data[i].y;
		R1[i*4+2]=(float)axis1.m_data[i].z; 
	}

	ObjGen::tranReverse(R1);

	//ICPpointTrans(&v1,R1);
	ObjGen::tranMapPoints(R1,&v1);

	ObjGen::tranMakeUnit(R2);  
	for(int i=0;i<3;i++)
	{ 
		R2[i*4]=(float)axis2.m_data[i].x; 
		R2[i*4+1]=(float)axis2.m_data[i].y;
		R2[i*4+2]=(float)axis2.m_data[i].z; 
	}
	ObjGen::tranReverse(R2);
	ObjGen::tranMapPoints(R2,&v2);

	//then use ICP to find a transform T to registrer V2 to V1
	//    R1*tran1*v1<--->ICP*R2*tran2*v2
	double minError=myBET::pointICP(&v1,&v2,10,icp); 

	//finally, the a16 is:             tran1~R1~(ICP*R2*tran2)
	ObjGen::tranMakeUnit(a16); //a16=I
	ObjGen::tranReverse(tran1);  
	ObjGen::tranMul(a16,tran1);//a16=I*tran1~
	ObjGen::tranReverse(R1); 
	ObjGen::tranMul(a16,R1); //a16=tran1~*R1~
	ObjGen::tranMul(a16,icp);//a16=tran1~*R1~ICP
	ObjGen::tranMul(a16,R2);  //a16=tran1~*R1~*ICP*R2
	ObjGen::tranMul(a16,tran2);//a16=tran1~R1~(ICP*R2*tran2)
	return minError;

	return 1;
}

// Register MR-US models based on RBF technique
int bbsFusion::RegisterByRBF()
{
	TriTreeViewer *f=this->m_triFather;

	// Build RBF-based mapping model
	Array1Ddouble radius;
	Array1DPoint3f from, to, wt;
	m_landmarkPair->funGetPairs(&from, &to, &radius);
	ObjGen::tranCRBFmake(&from, &to, &radius, &wt, 1, 1, 1);

	// Map tumor models using RBF-based mapping model
	Array1DPoint3f fromP, toP;
	JimModelDebug **mds=(JimModelDebug **)this->m_mdTumors->m_data;
	int num=this->m_mdTumors->m_used;
	for(int i=0;i<num;i++)
	{
		mds[i]->getAllVetex(&fromP);
		toP.copyFrom(&fromP);
		for(int j=0;j<fromP.m_used;j++)
		{
			ObjGen::tranCRBFmap(&from, &radius, &wt, &fromP.m_data[j], &toP.m_data[j]);
		}

		mds[i]->deformByVertex(&toP);
	}

	f->showCurrentFocus();
	return 0;
}

int bbsFusion::rbfRegistration()
{
	TriTreeViewer *f=this->m_triFather;

	Array1Ddouble radius;
	Array1DPoint3f from, to, wt;

	// Obtain the boundary of segmented MRI volume
	Array1DPoint3i pt;
	MedTools::imGetBoundayGridZX(m_imMRIsegReg, 1, 2, 3, 3, 4,&pt);
	MedTools::castPoint3iTo3f(&pt, &from);

	// Obtain the boundary of segmented US volume
	Array1DPoint3f edge;
	m_imUSseg->obejctBoundaryExtract(&pt, 1);
	m_imUSseg->fillRegion(&pt, 2);
	MedTools::castPoint3iTo3f(&pt, &edge);

	// Transforms between world to image
	double aIm2wd[16], aWd2im[16];
	m_imUSseg->tranMakeImage2World(aIm2wd);
	m_imUSseg->tranMakeWorld2Image(aWd2im);

	// Map from image to workd
	ObjGen::tranMapPoints(aIm2wd,&from);
	ObjGen::tranMapPoints(aIm2wd,&edge);
	for(int i=0;i<from.m_used;i++)
	{
		int at=edge.findNearestPoint(from.m_data[i]);
		to.append(edge.m_data[at]);
	}

	// Map from world to image
	ObjGen::tranMapPoints(aWd2im,&from);
	ObjGen::tranMapPoints(aWd2im,&to);

	//for debug
	if(1)
	{
		SceneTree *st=f->m_sceneTree->ensureByPathName("test1",SCENE_OBJ_LINE);
		SceneLines *sL=(SceneLines*) st->m_object;

		st=f->m_sceneTree->ensureByPathName("testFrom",SCENE_OBJ_POINT);
		ScenePoints *spFrom=(ScenePoints*)st->m_object;
		spFrom->m_size=4;

		st=f->m_sceneTree->ensureByPathName("testTo",SCENE_OBJ_POINT);
		ScenePoints *spTo=(ScenePoints*)st->m_object;
		spTo->m_size=4;

		sL->m_points->m_used=0;
		for(int i=0;i<from.m_used;i++)
		{
			sL->m_points->append(from.m_data[i]);
			sL->m_points->append(to.m_data[i]);

			spFrom->m_points->append(from.m_data[i]);
			spTo->m_points->append(to.m_data[i]);
		}
		f->seeItInTree(st);
	}

	for(int i=0;i<from.m_used;i++)
		radius.append(10);

	// Image-to-image registration
	ImageSlices im;
	m_imMRIsegReg->dup(&im);
	ObjGen::tranCRBFimageRegistraion(&im,m_imMRIsegReg, &from, &to, &radius, 0);

	return 1;
}

int bbsFusion::testRBF()
{
	TriTreeViewer *f=this->m_triFather;

	Array1Ddouble radius;
	Array1DPoint3f wt;
	ImageSlices *im, *imTo;

	if( (im=f->getSelectedVolume())==0)
		return 0;

	Array1DPoint3f ptFrom, ptTo;
	this->m_landmarkPair->funGetPairs(&ptFrom,&ptTo, &radius);

	imTo=f->ensureVol("im_test");
	imTo->newBufBy(im);
	imTo->setAll(0);

	double aWd2im[16];
	im->tranMakeWorld2Image(aWd2im);
	ObjGen::tranMapPoints(aWd2im,&ptFrom);
	ObjGen::tranMapPoints(aWd2im,&ptTo);
	ObjGen::tranCRBFimageRegistraion(im,imTo, &ptFrom, &ptTo, &radius, 0);

	f->correlateViews();
	f->showCurrentFocus();
	return 1;
}

int bbsFusion::surfaceRBF()
{
	TriTreeViewer *f=this->m_triFather;

	// Build RBF-based mapping model
	Array1Ddouble radius;
	Array1DPoint3f from, to, wt;
	m_landmarkPair->funGetPairs(&from, &to, &radius);
	ObjGen::tranCRBFmake(&from, &to, &radius, &wt, 1, 1, 1);

	// Map MR model using RBF-based mapping model
	// ??? why
	Array1DPoint3f fromP, toP;
	m_mdMR->getAllVetex(&fromP);
	toP.copyFrom(&fromP);
	for(int i=0;i<fromP.m_used;i++)
	{
		ObjGen::tranCRBFmap(&from, &radius, &wt, &fromP.m_data[i], &toP.m_data[i]);
	}
	m_mdMR->deformByVertex(&toP);

	// Map tumor model using RBF-based mapping model
	m_mdTumor->getAllVetex(&fromP);
	toP.copyFrom(&fromP);
	for(int i=0;i<fromP.m_used;i++)
	{
		ObjGen::tranCRBFmap(&from, &radius, &wt, &fromP.m_data[i], &toP.m_data[i]);
	}
	m_mdTumor->m_optAutoBottomCap=1; // Automatically add cap on the bottom and top
	m_mdTumor->m_optAutoTopCap=1;
	m_landmarkPair->funSetPairs(&fromP, &toP, &radius);  // ??? place othewhere
	m_mdTumor->deformByVertex(&toP);

	f->showCurrentFocus();

	return 0;
}

int bbsFusion::CheckLandmarkEditor(int checked)
{
	TriTreeViewer *f=this->m_triFather;
	SceneTree *st=f->m_sceneTree->ensureByPathName("landmarks");
	m_landmarkPair=landmarkPair::newObject(f,st);
	if(checked==1)
		f->triUIappend(m_landmarkPair);
	else
		f->triUIdel(m_landmarkPair);
	return 0;
}

int bbsFusion::mdHardmul()
{
	TriTreeViewer *f=this->m_triFather;
	SceneTree *st;
	double aIm2wd[16], aWd2im[16], a[16];
	m_imUS->tranMakeImage2World(aIm2wd);
	m_imUS->tranMakeWorld2Image(aWd2im);

	for(int i=0;i<3;i++)
	{
		if(i==0)  st=m_mdUS->m_container;
		if(i==1)  st=m_mdMR->m_container;
		if(i==2)  st=m_mdTumor->m_container;
		ObjGen::tranMulab2c(aIm2wd,st->m_tran,a);
		ObjGen::tranMakeUnit(st->m_tran);
		st->tranLetSonAsMe();
		st->tranHardMul(a);
		ObjGen::tranCopy(st->m_tran,aWd2im);
		st->tranLetSonAsMe();
		st->clearAllDisplayList();
	}
	return 0;
}

// Generate landmarks from the model surface
int bbsFusion::GenLandmarkByModelSurface()
{
	/*TriTreeViewer *f=this->m_triFather;
	Array1DPoint3f from, to;

	mdHardmul();
	m_mdMR->getAllVetex(&from);

	//get to
	double aIm2wd[16];
	m_imUS->tranMakeImage2World(aIm2wd);
	Array1DPoint3i edge;
	Array1DPoint3f pt;
	MedTools::imObjectBoundayExtract(m_imUSseg,1,2,6,&edge);
	MedTools::castPoint3iTo3f(&edge,&pt);
	ObjGen::tranMapPoints(aIm2wd,&pt);
	*/

	Array1DPoint3f from, to, base;
	TriMesh * m;

	// Subdivision for US
	m_mdUS->m_subLevel=4;
	m_mdUS->m_minEdgeLen=2;
	m_mdUS->updateSurf();
	this->m_triFather->showCurrentFocus();

	// Obtain the vertex of US surface model
	m= ( (SceneTriMesh*) (m_mdUS->m_surface->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&base,1);

	// Obtain the vertex of MR base model
	m= ( (SceneTriMesh*) (m_mdMR->m_frameBase->m_object) )->m_mesh;// ((SceneTriMesh*)mm_frameBase->m_object)->m_mesh;
	m->vexToArray1DPoint3f(&from);

	// Collect point pairs
	// Find a nearest 'to' point for each 'from' point
	Array1Ddouble radius;
	for(int i=0;i<from.m_used;i++)
	{
		radius.append(30);
		int idx=base.findNearestPoint( from.m_data[i]);
		to.append(base.m_data[idx]);
	}

	this->debugDisplayPairs(&from,&to);

	// Save point pairs
	m_landmarkPair->funSetPairs(&from, &to, &radius);

	return 0;
}



void bbsFusion::SubdivideMRModel()
{
	TriTreeViewer *f=this->m_triFather;

	//TriMesh * m;
	m_mdMR->updateSurf2Base(); 
	m_mdMR->updateSurf();

	f->showCurrentFocus();

}

// Save the registered MR model into a file
int bbsFusion::SaveRegisteredModelFile(char *fileName, int numOfCut)
{
	TriTreeViewer *f=m_triFather;
	FILE *fp=fopen(fileName,"w");
	SceneTree *st;
	float z;
	JimModelDebug **mds=(JimModelDebug**)m_mdTumors->m_data;
	int num=m_mdTumors->m_used;
	SceneTree *cons;//=m_mdMR->m_container->ensureByPathName("cutContour");//on
	char name[1024];

	double x1,y1,z1,x2,y2,z2;

	double a[16];
	int cnum=0;
	int pNum;
	Point3f *pt;

	cons=f->m_sceneTree->ensureByPathName("bbsCut");
	cons->delSons();
	f->m_img->tranMakeWorld2Image(cons->m_tran);
	//cons=f->m_sceneTree->ensureByPathName("bbsCut");

	fprintf(fp,"<model>\n");
	fprintf(fp,"\t<id>0</id>\n"); 
	for(int i=0;i<num;i++)
	{
		fprintf(fp,"\t<sub-model>\n",i);
		fprintf(fp,"\t\t<id>%d</id>\n",i);

		( (SceneTriMesh *)mds[i]->m_surface->m_object)->m_mesh->getBoundBox(x1,y1,z1,x2,y2,z2);
		for(int j=0;j<numOfCut;j++)
		{
			fprintf(fp,"\t\t\t<curve>\n");

			z=z1+ (z2-z1)*j/(numOfCut-1);

			fprintf(fp,"\t\t\t<position>%f</position>\n",z);

			fprintf(fp,"\t\t\t\t<points>\n");

			sprintf(name,"bbsCut/i=%d/j=%d",i,j);
			cons=f->m_sceneTree->ensureByPathName(name);

			z=f->m_img->m_zorg + z/f->m_img->m_scale3;

			//the cuting plan is in image coordinate..., the result is also in image co-ordinate system...cons->son's tran is the same as f->m_sceneTree->m_tran

			mds[i]->m_surface->cutContour(f->m_sceneTree->m_tran,cons,cnum,0,0,1.0, -z,1+2+4+8);//4 + 8*f->m_optShowCutContourIn3D);

			st=cons->gotoTailSons();

			//change to world co-ordinate
			ObjGen::tranCopy(st->m_tran,cons->m_tran);
			st->tranLetSonAsMe();
			m_imUS->tranMakeImage2World(a);
			st->tranHardMul(a);

			pNum = ((SceneLines*)st->m_object)->m_points->m_used;
			if(pNum>0)
			{
				pt= ((SceneLines*)st->m_object)->m_points->m_data;

				//save the contour
				for( int j=0;j<pNum;j++)
				{
					//m_conX->append(pt[j].x);
					//m_conY->append(pt[j].y);
					fprintf(fp,"\t\t\t\t%f,%f\n", pt[j].x, pt[j].y);
				}
			}
			fprintf(fp,"\t\t\t\t</points>\n");

			fprintf(fp,"\t\t\t</curve>\n");

			sprintf(name,"j=%d,p=%d",j,pNum);
			st->rename(name);// st->m_name cons
		}

		//cons->clearAllDisplayList(); 
		fprintf(fp,"\t</sub-model>\n",i);
	} 
	fprintf(fp,"</model>\n");

	f->seeItInTree(cons);

	f->showCurrentFocus(); 
	fclose(fp);
	return 0;
}

// Global variables
TriTreeViewer *g_triTreeViewer = 0;
bbsFusion *m_bbsFusion0;

// Start the MIUE systema
int startBBSfusion()
{
	if(g_triTreeViewer ==0 ) g_triTreeViewer = new TriTreeViewer();
	g_triTreeViewer->DoModule(1,1);
	m_bbsFusion0=new bbsFusion(g_triTreeViewer);

	return 0;
}

// Show the MIUE system
void showMIUE(int flag)  //AppBBSfusion_API void showMIUE(int flag=1); //flag==1, show, flag==0 hide
{
	if(g_triTreeViewer !=0 ) g_triTreeViewer->ShowWindow(flag);
}

// Close the MIUE system
int exitBBSfusion()
{
	if(g_triTreeViewer==0)
		return 0;
	g_triTreeViewer->exit(true);
	delete g_triTreeViewer;
	g_triTreeViewer = 0;

	if(m_bbsFusion0) delete m_bbsFusion0;
	m_bbsFusion0 = 0;

	return 0;
}


#include "miueJim\jimLib\ImageSlices.h"
#include "miueJim\jimGL\SceneTree.h"

bbsFusion* getBBSFusionInstance()
{
	return m_bbsFusion0;
}

//int LoadUSMRImage(char* fileUS, char* fileMR)
//{
//	if(g_triTreeViewer==0)return 0;
//	
//	// Load US images
//	m_bbsFusion0->m_imUS->load(fileUS, false);
//	//ensure (0,0,0) to physical (orgx, orgy, orgz)
//	m_bbsFusion0->m_imUS->m_xorg /= -m_bbsFusion0->m_imUS->m_scale1;
//	m_bbsFusion0->m_imUS->m_yorg /= -m_bbsFusion0->m_imUS->m_scale2;
//	m_bbsFusion0->m_imUS->m_zorg /= -m_bbsFusion0->m_imUS->m_scale3;
//
//	// Load MR images
//	m_bbsFusion0->m_imMRI->load(fileMR, false);
//	//ensure (0,0,0) to physical (orgx, orgy, orgz)
//	m_bbsFusion0->m_imMRI->m_xorg /= -m_bbsFusion0->m_imMRI->m_scale1;
//	m_bbsFusion0->m_imMRI->m_yorg /= -m_bbsFusion0->m_imMRI->m_scale2;
//	m_bbsFusion0->m_imMRI->m_zorg /= -m_bbsFusion0->m_imMRI->m_scale3;
//
//	m_bbsFusion0->m_triFather->correlateViews();
//	m_bbsFusion0->m_triFather->showCenterFocus(); 
//	return 1;
//}

//int setUSimage(unsigned char ***d, int sx, int sy, int sz, double scalex, double scaley, double scalez, int dataLen, double orgx, double orgy, double orgz)
//{
//	if(g_triTreeViewer==0)return 0;
//
//	m_bbsFusion0->m_imUS->setData(d,sx,sy,sz,scalex,scaley,scalez,dataLen);
//	//ensure (0,0,0) to physical (orgx, orgy, orgz)
//	m_bbsFusion0->m_imUS->m_xorg= - orgx/m_bbsFusion0->m_imUS->m_scale1; // ??? wujz
//	m_bbsFusion0->m_imUS->m_yorg= - orgy/m_bbsFusion0->m_imUS->m_scale2;
//	m_bbsFusion0->m_imUS->m_zorg= - orgz/m_bbsFusion0->m_imUS->m_scale3;
//
//	m_bbsFusion0->m_triFather->correlateViews();
//	m_bbsFusion0->m_triFather->showCenterFocus(); 
//	return 1;
//}
//
//int setMRimage(unsigned char ***d, int sx, int sy, int sz, double scalex, double scaley, double scalez, int dataLen,  double orgx, double orgy, double orgz)
//{
//	if(g_triTreeViewer==0)return 0;
//
//	m_bbsFusion0->m_imMRI->setData(d,sx,sy,sz,scalex,scaley,scalez,dataLen);
//	
//	//ensure (0,0,0) to physical (orgx, orgy, orgz)
//	m_bbsFusion0->m_imMRI->m_xorg= - orgx/m_bbsFusion0->m_imMRI->m_scale1;
//	m_bbsFusion0->m_imMRI->m_yorg= - orgy/m_bbsFusion0->m_imMRI->m_scale2;
//	m_bbsFusion0->m_imMRI->m_zorg= - orgz/m_bbsFusion0->m_imMRI->m_scale3;
//
//	// modified by wujz
//	//m_bbsFusion0->m_imMRI->m_zorg = m_bbsFusion0->m_imUS->m_zorg;
//	//m_bbsFusion0->m_imMRI->m_scale1 = m_bbsFusion0->m_imUS->m_scale1;
//	//m_bbsFusion0->m_imMRI->m_scale2 = m_bbsFusion0->m_imUS->m_scale2;
//	//m_bbsFusion0->m_imMRI->m_scale3 = m_bbsFusion0->m_imUS->m_scale3;
//	
//	m_bbsFusion0->m_triFather->correlateViews();
//	m_bbsFusion0->m_triFather->showCenterFocus();
//	return 1;
//}

//int loadURcoutours(char *fileName)
//{
//	if(g_triTreeViewer==0)return 0;
//	if(m_bbsFusion0->m_imUS->isEmpty())
//	{ 
//		miueShowMessage("please load US image first",1);
//		return 0;
//	}
//
//	// Load the US geometrical model
//	Array1Dptr ar;
//	m_bbsFusion0->loadModel(fileName,"US", &ar);
//	JimModelDebug **mds=(JimModelDebug**)ar.m_data;
//
//	for(int i=0;i<ar.m_used;i++)
//	{
//		g_triTreeViewer->m_img->tranMakeWorld2Image(mds[i]->m_container->m_tran);
//		//m_bbsFusion0->m_imUS->tranMakeWorld2Image(mds[i]->m_container->m_tran);
//		mds[i]->m_container->tranLetSonAsMe();
//	} 
//
//	// Manage the US-derived geometrical prostate
//	m_bbsFusion0->m_mdUS=mds[0];
//
//	return 1;
//}

//int loadMRcoutours(char *fileName)
//{   
//	if(g_triTreeViewer==0)return 0;
//	if(m_bbsFusion0->m_imMRI->isEmpty())
//	{ 
//		miueShowMessage("please set MRI image first",1);
//		return 0;
//	}
//
//	// Load the MR geometrical model
//	Array1Dptr ar;
//	m_bbsFusion0->loadModel(fileName,"MR",&ar);
//	JimModelDebug **mds=(JimModelDebug**)ar.m_data;
//
//	for(int i=0;i<ar.m_used;i++)
//	{
//		g_triTreeViewer->m_img->tranMakeWorld2Image(mds[i]->m_container->m_tran);
//		//m_bbsFusion0->m_imUS->tranMakeWorld2Image(mds[i]->m_container->m_tran);
//		//m_bbsFusion0->m_imMRI->tranMakeWorld2Image(mds[i]->m_container->m_tran);
//		mds[i]->m_container->tranLetSonAsMe();
//	} 
//
//	// Manage the MR-derived geometrical prostate and its tumors
//	m_bbsFusion0->m_mdMR=mds[0];
//	m_bbsFusion0->m_mdTumors->clear();
//	for(int i=0;i<ar.m_used;i++)
//		m_bbsFusion0->m_mdTumors->append(mds[i]);
//
//	return 1;
//}

//// Bound box based rigid registration
//int RegisterByVolumeBoundBox()
//{
//	TriTreeViewer *f = m_bbsFusion0->m_triFather;
//
//	double x1,y1,z1,x2,y2,z2;
//	double u1,v1,w1,u2,v2,w2;
//	//deform imMRI to imMRIreg
//	double a[16],b[16];
//
//	ObjGen::tranMakeUnit(a);
//	m_bbsFusion0->m_imMRI->tranMakeImage2World(a); 
//	m_bbsFusion0->m_mdMR->m_container->getBoundBox(a,x1,y1,z1,x2,y2,z2);
//	//m_imMRIseg->getBoundingBox(1,1,x1,y1,z1,x2,y2,z2);
//
//	m_bbsFusion0->m_imUS->tranMakeImage2World(a);
//	m_bbsFusion0->m_mdUS->m_container->getBoundBox(a,u1,v1,w1,u2,v2,w2);
//	//m_imUSseg->getBoundingBox(1,1,u1,v1,w1,u2,v2,w2);
//
//	// Transform from MR to US
//	ObjGen::tranMakeTranslate(a,-x1,-y1,-z1);
//	ObjGen::tranMakeScaling(b, (double)(u2-u1)/(double)(x2-x1), (double)(v2-v1)/(double)(y2-y1),  (double)(w2-w1)/(double)(z2-z1) );
//	ObjGen::tranMulab2c(b,a,a); 
//	ObjGen::tranMakeTranslate(b,u1,v1,w1);
//	ObjGen::tranMulab2c(b,a,a); 
//
//	// Save the transform of MR2US
//	//ObjGen::tranMulab2c(a, m_tran,m_tran);
//	ObjGen::tranCopy(m_bbsFusion0->m_tran,a); // m_tran <- a ?
//
//	SceneTree *st;
//	int num = m_bbsFusion0->m_mdTumors->m_used;
//	JimModelDebug **mds = (JimModelDebug**)( m_bbsFusion0->m_mdTumors->m_data );
//	m_bbsFusion0->m_imUS->tranMakeWorld2Image(b);
//
//	// Registrater the MR tumors to US model according to the transform
//	for(int i=0;i<num;i++)
//	{
//		st= mds[i]->m_container;//m_bbsFusion0->m_mdTumors->m_data[0];
//		st->tranHardMul(a); // ??? wujz transform the real geometry ?
//		ObjGen::tranCopy(st->m_tran, b);
//		st->tranLetSonAsMe();
//	}
//
//	f->showCurrentFocus();
//	return 1;
//}
//
//// ICP-based rigid registration
//int  icpRegistration()
//{
//	return m_bbsFusion0->RegisterByICP();
//}
//
//// Generate landmarks from the model surface
//int genLandmarkByModelSurface(bool subdiv)//=0)
//{
//	if(subdiv) m_bbsFusion0->SubdivideMRModel();
//	m_bbsFusion0->CheckLandmarkEditor( 1 );
//	m_bbsFusion0->GenLandmarkByModelSurface(); 
//	return 1;
//}

//// RBF-based registration
//int  rbfRegistration()
//{
//	m_bbsFusion0->RegisterByRBF();//testRBF();
//	return 0;
//}

int  getContoursAtZ(int z, double * &x, double * &y, int *&endIdx)
{
	TriTreeViewer *f=m_bbsFusion0->m_triFather;

	JimModelDebug **mds=(JimModelDebug**)m_bbsFusion0->m_mdTumors->m_data;
	int num=m_bbsFusion0->m_mdTumors->m_used;
	SceneTree *cons=m_bbsFusion0->m_mdMR->m_container->ensureByPathName("cutContour");//on
	cons->delSons();

	double a[16];
	int cnum=0;
	int pNum;
	Point3f *pt;
	SceneTree *st;

	m_bbsFusion0->m_conX->clear();//=Array1Ddouble();
	m_bbsFusion0->m_conY->clear();//=Array1Ddouble();
	m_bbsFusion0->m_conEndAt->clear();//=new Array1Dint();
	int mynum=0;
	for(int i=0;i<num;i++)
	{
		//the cuting plan is in image coordinate..., the result is also in image co-ordinate system...cons->son's tran is the same as f->m_sceneTree->m_tran
		mds[i]->m_surface->cutContour(f->m_sceneTree->m_tran,cons,cnum,0,0,1.0, -z,4 + 8*f->m_optShowCutContourIn3D);

		st=cons->gotoTailSons();

		//change to world co-ordinate
		ObjGen::tranCopy(st->m_tran,cons->m_tran);
		st->tranLetSonAsMe();
		m_bbsFusion0->m_imUS->tranMakeImage2World(a);
		st->tranHardMul(a);

		pNum = ((SceneLines*)st->m_object)->m_points->m_used;
		if(pNum==0)
			continue;
		mynum++;

		pt= ((SceneLines*)st->m_object)->m_points->m_data;
		for( int j=0;j<pNum;j++)
		{
			m_bbsFusion0->m_conX->append(pt[j].x);
			m_bbsFusion0->m_conY->append(pt[j].y);
		}

		m_bbsFusion0->m_conEndAt->append(m_bbsFusion0->m_conX->m_used);
	}

	m_bbsFusion0->m_triFather->showFocus(f->m_img->m_sx/2,f->m_img->m_sy/2,z);//f->m_img->m_sx/2);

	return mynum;
}

//// Save the registered MR model into a file
//int  saveRegisteredModelFile(char *fileName, int numOfCut)
//{
//	TriTreeViewer *f=m_bbsFusion0->m_triFather;
//	FILE *fp=fopen(fileName,"w");
//	SceneTree *st;
//	float z;
//	JimModelDebug **mds=(JimModelDebug**)m_bbsFusion0->m_mdTumors->m_data;
//	int num=m_bbsFusion0->m_mdTumors->m_used;
//	SceneTree *cons;//=m_bbsFusion0->m_mdMR->m_container->ensureByPathName("cutContour");//on
//	char name[1024];
//
//	double x1,y1,z1,x2,y2,z2;
//
//	double a[16];
//	int cnum=0;
//	int pNum;
//	Point3f *pt;
//
//	cons=f->m_sceneTree->ensureByPathName("bbsCut");
//	cons->delSons();
//	f->m_img->tranMakeWorld2Image(cons->m_tran);
//	//cons=f->m_sceneTree->ensureByPathName("bbsCut");
//
//	fprintf(fp,"<model>\n");
//	fprintf(fp,"\t<id>0</id>\n"); 
//	for(int i=0;i<num;i++)
//	{
//		fprintf(fp,"\t<sub-model>\n",i);
//		fprintf(fp,"\t\t<id>%d</id>\n",i);
//
//		( (SceneTriMesh *)mds[i]->m_surface->m_object)->m_mesh->getBoundBox(x1,y1,z1,x2,y2,z2);
//		for(int j=0;j<numOfCut;j++)
//		{
//			fprintf(fp,"\t\t\t<curve>\n");
//
//			z=z1+ (z2-z1)*j/(numOfCut-1);
//
//			fprintf(fp,"\t\t\t<position>%f</position>\n",z);
//
//			fprintf(fp,"\t\t\t\t<points>\n");
//
//			sprintf(name,"bbsCut/i=%d/j=%d",i,j);
//			cons=f->m_sceneTree->ensureByPathName(name);
//
//			z=f->m_img->m_zorg + z/f->m_img->m_scale3;
//
//			//the cuting plan is in image coordinate..., the result is also in image co-ordinate system...cons->son's tran is the same as f->m_sceneTree->m_tran
//
//			mds[i]->m_surface->cutContour(f->m_sceneTree->m_tran,cons,cnum,0,0,1.0, -z,1+2+4+8);//4 + 8*f->m_optShowCutContourIn3D);
//
//			st=cons->gotoTailSons();
//
//			//change to world co-ordinate
//			ObjGen::tranCopy(st->m_tran,cons->m_tran);
//			st->tranLetSonAsMe();
//			m_bbsFusion0->m_imUS->tranMakeImage2World(a);
//			st->tranHardMul(a);
//
//			pNum = ((SceneLines*)st->m_object)->m_points->m_used;
//			if(pNum>0)
//			{
//				pt= ((SceneLines*)st->m_object)->m_points->m_data;
//
//				//save the contour
//				for( int j=0;j<pNum;j++)
//				{
//					//m_bbsFusion0->m_conX->append(pt[j].x);
//					//m_bbsFusion0->m_conY->append(pt[j].y);
//					fprintf(fp,"\t\t\t\t%f,%f\n", pt[j].x, pt[j].y);
//				}
//			}
//			fprintf(fp,"\t\t\t\t</points>\n");
//
//			fprintf(fp,"\t\t\t</curve>\n");
//
//			sprintf(name,"j=%d,p=%d",j,pNum);
//			st->rename(name);// st->m_name cons
//		}
//
//		//cons->clearAllDisplayList(); 
//		fprintf(fp,"\t</sub-model>\n",i);
//	} 
//	fprintf(fp,"</model>\n");
//
//	f->seeItInTree(cons);
//
//	f->showCurrentFocus(); 
//	fclose(fp);
//	return 0;
//}

