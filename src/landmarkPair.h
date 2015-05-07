#ifndef landarkPair_header_20120403
#define landarkPair_header_20120403

class TriTreeViewer;
class JimModelGroup;
class ImageSlices;

#include "jimGL\SceneTree.h"
#include "miueBase\Array1D.h"
#include "miueBase\templateUI.h"


#include "miueTriUI\miueTriUIcore.h"


class landmarkPair: public myTriUI
{
public:
	landmarkPair(TriTreeViewer *f, SceneTree *container);
	~landmarkPair();

	//int m_reposition;
  
	int m_rightJustDown;
	int m_leftJustDown;

	//Array1DPoint3f *m_ptFrom;
	//Array1DPoint3f *m_ptTo;
	//Array1DPoint3f *m_line;

	SceneTree *m_ball;
	SceneTree *m_pairs;

	static const int typeNothing=0;
	static const int typePTfrom=1;
	static const int typePTto=2;
	static const int typeBall=3;

	double m_minPickLen;
	int m_pickedType; 
	
	SceneTree *m_curPair;
	
	//double m_aIm2wd[16];
	//double m_aWd2im[16];//ImageSlices *m_im;

	static landmarkPair *newObject(TriTreeViewer *f, SceneTree *container);
 
	int OnLButtonDown(unsigned int nFlags, float x, float y, float z, int dir, float norm[3]);
	int OnMouseMove(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3]);
	int OnLButtonUp(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3]);
	int OnLButtonDblClk(unsigned int nFlags, float x, float y, float z, int dir, float norm[3]);

	int OnRButtonDown(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3]);
	int OnRButtonUp(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3]);
	int OnMenu(unsigned int id);
	int delete_me();

	int funDeletePair();
	int funPickTest(float x, float y, float z);
	int funSetPairs(Array1DPoint3f *from, Array1DPoint3f *to, Array1Ddouble *radiu);
	int funGetPairs(Array1DPoint3f *from, Array1DPoint3f *to, Array1Ddouble *radiu);
	SceneTree* funNewApair();
	int funUpdateBallTran(SceneTree *thePair);

};
#endif

/*
    int testHitBoard(float x,float y, float x1,float y1, float x2,float y2, float sx=1.0f,float sy=1.0f);
  int testHitBoard(int dir,float x,float y, float z);

  float m_x1,m_y1,m_z1;
   float m_r;
   int m_hitBoard;
   int m_inDraw;
   
   float m_x0,m_y0,m_z0;
   int m_gap;



 // int attach(SceneTree *st, float ix1=-1, float iy1=-1, float iz1=-1, float r=-1);

  // int attach(SceneTree *st);
 //  int detach();

   

  //void updateBall();
 
  

  int funFillImage();
  void funStayHere();

int landmarkPair::getBall(int &x1,int &y1, int &z1, int &r)
{
	if(m_container==0)
		return 0;
	SceneTree *st;
	if ((st=m_container->findByName("ball"))==0)
		return 0;
	if(st->isSelected()==0)
		return 0;



	float fx1,fy1,fz1,fx2,fy2,fz2;
	fx1=m_x1; fy1=m_y1; fz1=m_z1;
	//fx2=m_x2; fy2=m_y2; fz2=m_z2;
	this->m_triFather->m_img->tranWorld2Image(fx1,fy1,fz1);   
	this->m_triFather->m_img->tranWorld2Image(fx2,fy2,fz2);   
	x1=(int)(fx1+0.5);
	y1=(int)(fy1+0.5);
	z1=(int)(fz1+0.5);

	//x2=(int)(fx2+0.5);
	//y2=(int)(fy2+0.5);
	//z2=(int)(fz2+0.5);


	//if( (x2<=x1) && (y2<=y1) && (z2<=z1) ) 
	//	return 0;



	if(x1<0) x1=0;
	if(y1<0) y1=0;
	if(z1<0) z1=0;
	//if(x2>=m_triFather->m_img->m_sx)  x2=m_triFather->m_img->m_sx-1;
	//if(y2>=m_triFather->m_img->m_sy)  y2=m_triFather->m_img->m_sy-1;
	//if(z2>=m_triFather->m_img->m_sz)  z2=m_triFather->m_img->m_sz-1;
	return 1;
}


int landmarkPair::funFillImage()
{
	int g,x1,y1,z1,x2,y2,z2;
	int r;
	if(getIntInput(g,"the intensity to fill",0)==0)
		return 0;
	getBall(x1,y1,z1,r);
	this->m_triFather->m_img->fillVol(g,x1,y1,z1,x2,y2,z2);   
	this->m_triFather->showCurrentFocus(1);
	return 1;
}
void landmarkPair::funStayHere()
{


}

  //int getBall(int &x1,int &y1, int &z1, int &r);
 //  ImageSlices *getMask(char *volName,int &x1, int &y1, int &z1);


int landmarkPair::attach(SceneTree *st, float ix1, float iy1, float iz1, float r)
{

	if(st==0)
	{
		st=this->m_triFather->m_sceneTree->ensureByPathName("generalSeg?VolEditor?ballROI");
	}
	m_container=st;

	m_x1=ix1; m_y1=iy1;  m_z1=iz1;
	//	m_x2=ix2; m_y2=iy2;  m_z2=iz2;

	SceneTree *ball,*frame,*center;

	//frame=st->findByPathName("frame");
	//if( (frame==0) ||(frame->m_type!=SCENE_OBJ_LINE) )
	//{

	//	frame=st->ensureByPathName("frame",SCENE_OBJ_LINE); 
	//	frame->m_materials->setDiffuse(0,255,255,255);
	//	((SceneLines*)frame->m_object)->m_size=4; 
	//	m_triFather->seeItInTree(st); 
	//}

	//center=st->findByPathName("center");
	//if( (center==0) ||(center->m_type!=SCENE_OBJ_POINT) )
	//{

	//	center=st->ensureByPathName("center",SCENE_OBJ_POINT); 
	//	center->m_materials->setDiffuse(0,255,255,255);
	//	center->m_pickMyPoint=1; 
	//	((SceneLines*)center->m_object)->m_size=8; 
	//	m_triFather->seeItInTree(st); 
	//}

	ball=st->findByPathName("ball");
	if((ball==0) || (ball->m_type!=SCENE_OBJ_TRIANGLE) )
	{
		ball=st->ensureByPathName("ball",SCENE_OBJ_TRIANGLE); 
		ball->m_show2Where=1+2+4+8; //x->1, y->2, z->4, 3D->8 bin flag:  3D z  y x-->0 not display, 1-->display, default value is 1111->1+2+4+8=15 
		ball->m_materials->setDiffuse(255,128,255,128);
		m_triFather->seeItInTree(st); 
	}



	updateBall();

	m_triFather->showCurrentFocus(1);  
	return 1;
}


void landmarkPair::updateBall()
{
	SceneTriangle *stri;
	SceneTree *st;
	Point3f p1,p2;

	p1.x=m_x1; p1.y=m_y1; p1.z=m_z1;
	//    p2.x=m_x2; p2.y=m_y2; p2.z=m_z2;

	if(m_container==0)
		return ;

	if((st=m_container->findByPathName("ball"))==0)
		return ;

	stri=(SceneTriangle*)st->m_object;

	Array1DPoint3f vex;
	Array1DPoint3i tri;
	ObjGen::makeBall(&p1,m_r,&vex,&tri);
	stri->m_vex->copyFrom(&vex);
	stri->m_tri->copyFrom(&tri);
	ObjGen::calculateNormal(stri->m_vex,stri->m_tri,stri->m_norm);

	//st=m_container->findByPathName("frame");// ->m_sons->m_next;
	//if(st==0)
	//	return ;

	//SceneTriangle *strii=(SceneTriangle*)st->m_object;
	//double a[16],b[16],c[16];
	//double r=10,cx=10,cy=10,cz=10;
	//cx = m_x1; cy = m_y1; cz = m_z1;
	//r=20;

	//ObjGen::tranMakeScaling(a,r,r,r); 
	//ObjGen::tranMakeTranslate(b,cx,cy,cz); 
	//ObjGen::tranMulab2c(b,a,c); 
	//ObjGen::makeUnitSphere(6,strii->m_vex,strii->m_tri);
	//ObjGen::tranMapPoints(c,strii->m_vex);  
	//ObjGen::calculateNormal(strii->m_vex,strii->m_tri,strii->m_norm); 
	////ObjGen::makeCubeFrame(m_x1,m_y1,m_z1,m_x2,m_y2,m_z2,sL->m_points); 


	//st=m_container->findByPathName("center");// ->m_sons->m_next;
	//if(st==0)
	//	return;
	//ScenePoints *sp=(ScenePoints*)st->m_object;
	//sp->m_points->m_used=0;
	////sp->m_points->appendPoint3f((m_x1+m_x2)/2, (m_y1+m_y2)/2, (m_z1+m_z2)/2 ); 
	//sp->m_points->appendPoint3f(m_x1, m_y1, m_z1); 

	m_container->clearAllDisplayList();
}

int landmarkPair::testHitBoard(float x,float y, float x1,float y1, float x2,float y2, float sx,float sy)
{
	float bdsize=8;

	if(( y>y1)&&(y<y2) )
	{
		if(MyABS(x-x1)*sx<=bdsize)
			return 1;
		if(MyABS(x-x2)*sx<=bdsize)
			return 2;
	}

	if(( x>x1)&&(x<x2) )
	{
		if(MyABS(y-y1)*sy<=bdsize)
			return 3;
		if(MyABS(y-y2)*sy<=bdsize)
			return 4;
	}


	if(((x-x1)*sx>bdsize)&&( (x-x2)*sx<-bdsize)&&( (y-y1)*sx>bdsize)&&((y-y2)*sy<-bdsize))
		return 5;


	return 0;

}


int landmarkPair::testHitBoard(int dir,float x,float y, float z)
{
	//if(dir>=3)
	//	return 0;
	//float sx,sy;
	//ImageSlices *im=this->m_triFather->m_img;  
	//m_triFather->m_viewerSlices[dir]->getImage2ScreenScale(sx,sy);    
	//if(dir==0)
	//	return testHitBoard(x,y,m_x1,m_y1,m_x2,m_y2,sx/im->m_scale1,sy/im->m_scale2);
	//else if(dir==1)
	//	return testHitBoard(x,z,m_x1, m_z1, m_x2, m_z2,sx/im->m_scale1,sy/im->m_scale3);
	//else if(dir==2)
	//	return testHitBoard(y,z,m_y1,m_z1,m_y2,m_z2,sx/im->m_scale2,sy/im->m_scale3);

	return 0;
}

int landmarkPair::detach()
{
	m_hitBoard=0; 
	m_rightJustDown=0;
	m_inDraw=0;
	m_x1=m_y1=m_z1=m_r=-1;
	updateBall();
	//m_triFather->seeItInTree(m_container); 
	m_triFather->showCurrentFocus(1); //jsut to delete 
	return 1;
}

	if(0) //old code
	{

	// if(m_container->m_sons==0)//||(this->m_reposition==1) )
	{
		if(1)
		{
		m_container->m_useWorld=1; 
		double a[16];
		this->m_triFather->m_img->tranMakeWorld2Image(a);
		ObjGen::tranMulab2c(this->m_triFather->m_sceneTree->m_tran,a,m_container->m_tran);    
		}
		SceneTree * st;
		st=m_triFather->m_sceneTree->ensureByPathName("cutContour?axial");
		st->m_select=1; 
		st=m_triFather->m_sceneTree->ensureByPathName("cutContour?coronal");
		st->m_select=1; 
		st=m_triFather->m_sceneTree->ensureByPathName("cutContour?sagittal");
		st->m_select=1; 
		m_triFather->mdTree_refresh(st->m_father,st->m_father->m_handle);   
	}

	m_inDraw=0;
	m_container->m_select=1;

	attach(m_container, x,y,z,m_r);
	m_x0=x; m_y0=y; m_z0=z;
	m_inDraw=1;

	m_container->tranLetSonAsMe();
	}
	return 0;


	//mouseUp
	
	if(0) //old code in mouse Move
	{
	//m_triFather->m_img->tranWorld2Image(ix,iy,iz);

	if( (this->m_inDraw==1) && (this->m_leftMouseDown==1) )
	{

		m_x1 = x; m_y1 = y; m_z1 = z;
		updateBall();
		this->m_triFather->showCurrentFocus(1);  
		return 0;
	}

	//double a[16],tran[16];
	//ObjGen::tranMakeUnit(tran); 

	//if(dir==0)
	//{
	//	ObjGen::tranMakeTranslate(tran, (ix-m_leftMouseDownX),(iy-m_leftMouseDownY),0);

	//} 
	//else if(dir==1)
	//{
	//	ObjGen::tranMakeTranslate(tran, (ix-m_leftMouseDownX),0,(iz-m_leftMouseDownZ));

	//} 
	//else if(dir==2)
	//{
	//	ObjGen::tranMakeTranslate(tran, 0,(iy-m_leftMouseDownY),(iz-m_leftMouseDownZ));
	//} 

	//m_leftMouseDownX=ix;
	//m_leftMouseDownY=iy;
	//m_leftMouseDownZ=iz;
	//ObjGen::tranMap(tran,m_x1,m_y1,m_z1);	

	//m_container->tranHardMul(tran); 


	//m_triFather->showCurrentFocus(1); 
	}

	
	if(m_inDraw==1)
	{
		if(m_leftJustDown==1)
		{
			if(this->m_container!=0)
				m_container->m_select=0;
			m_leftJustDown=0;
			return 0;
		}

		m_leftJustDown=0; 

		//float gap=m_gap;

		//if(m_x2-m_x1<1) 
		//{ 
		//	if(m_y2-m_y1>m_z2-m_z1)
		//		gap=m_z2-m_z1;
		//	else
		//		gap=m_y2-m_y1;
		//	gap/=2.0f;
		//	m_x1 -= gap;  m_x2=m_x1+2*gap;

		//} //	  m_x2=m_x1+m_gap; 
		//if(m_y2-m_y1<1) 
		//{  

		//	if(m_x2-m_x1>m_z2-m_z1)
		//		gap=m_z2-m_z1;
		//	else
		//		gap=m_x2-m_x1;

		//	gap/=2.0f;
		//	m_y1-= gap; m_y2=m_y1+2*gap;

		//} //  m_y2=m_y1+m_gap;//10;
		//if(m_z2-m_z1<1) 
		//{

		//	if(m_y2-m_y1>m_x2-m_x1)
		//		gap=m_x2-m_x1;
		//	else
		//		gap=m_y2-m_y1;
		//	gap/=2.0f;
		//	m_z1 -= gap; m_z2=m_z1+2*gap;

		//}//  m_z2=m_z1+m_gap;//10;

		this->updateBall();
		this->m_triFather->showCurrentFocus(1);  
	}

	m_leftJustDown=0; 

	TriTreeViewer *f=this->m_triFather;
	f->m_roiBall=m_container; 
	//for(int i=0;i<f->m_triUI->m_used;i++)
	//	f->m_triUI->m_data[i]->OnCubeROImouseLeftUp(nFlags,x,y,z,dir,nrom); 


*/