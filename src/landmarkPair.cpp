#include "landmarkPair.h"

#include "miueTriUI\GeoBall.h"
#include "miueTriTreeViewer.h"
#include "miueUItool.h"
#include "jimGL\SceneTree.h"
#include "miueBase\myconst.h"
#include "Jimlib\MedToolsJM.h"
#include "jimlib\ObjGenJM.h"

landmarkPair::landmarkPair(TriTreeViewer *f, SceneTree *container):myTriUI(f,container)
{
	m_rightJustDown=0;
	m_type=MIUEtriUI_type_geoBall;

	f->m_img->tranMakeWorld2Image(container->m_tran);
 
	m_pairs=container->newAson("pairs");
	
	m_ball=container->newAson("ball", SCENE_OBJ_TRIANGLE);
	m_ball->m_materials->setDiffuse(0,255,0,255);
 
	SceneTriangle *stri=(SceneTriangle *)m_ball->m_object;
	ObjGen::makeUnitSphere(6,stri->m_vex,stri->m_tri);
	ObjGen::calculateNormal(stri->m_vex,stri->m_tri,stri->m_norm);
 

	m_pickedType=this->typeNothing; 
   

	container->tranLetSonAsMe();

	m_curPair=0;

	m_minPickLen=3; //mm

	this->m_triFather->seeItInTree(container); 
}

landmarkPair *landmarkPair::newObject(TriTreeViewer *f, SceneTree *container)
{
	if(container!=0)
		if(container->m_object!=0)
			if(container->m_type==SCENE_OBJ_myTriUI)
			{
				if( ((myTriUI*) container->m_object)->m_type==MIUEtriUI_type_geoBall)
					return (landmarkPair *)container->m_object;
			} 

			landmarkPair *ret=new landmarkPair(f,container);
			ret->m_container->m_object=ret;
			return ret;
}

landmarkPair::~landmarkPair()
{

}

int landmarkPair::delete_me()
{
	if(m_container->m_object==this)
		delete (this);
	return 0;
}

int landmarkPair::funPickTest(float x, float y, float z)
{
	Point3f p;
	double dist, mind;
	int idx;
	p.set(x,y,z);
	
	//Array1DPoint3f *pts;
	//pick the from
	SceneTree *st=m_pairs->m_sons;
	if(st==0)
		return -1;
	 
	SceneTree *curPair,*nd;

	m_pickedType=this->typeNothing;
	st=m_pairs->m_sons;
		
	double R;
    Point3f P0;
	mind=-1;
	while(st!=0)
	{
 	    P0= ( (ScenePoints*) ((st->findByPathName("To"))->m_object) )->m_points->m_data[0];
		dist=(P0-p).Len2(); 
		if( (mind<-0.5) || (mind>dist) )
		{
			mind=dist;
			curPair=st;
			m_pickedType=this->typePTto;
		}
		
		P0= ( (ScenePoints*) ((st->findByPathName("From"))->m_object) )->m_points->m_data[0];
		dist=(P0-p).Len2();  
		if(mind>dist)
		{
			mind=dist;
			curPair=st;
			m_pickedType=this->typePTfrom;
		}
	
	
		if( (m_curPair==st) || (st->findByPathName("ball")!=0)  )
		{  

			st->findByPathName("R")->getVal(R);

			dist=sqrt(dist)-R; 
			dist=dist*dist;

			if(mind>dist)
			{
				mind=dist;
				curPair=st;
				m_pickedType=this->typeBall;
			}
	   }
	
	  st=st->m_next;
	}

	
	if(mind<m_minPickLen)
	{
		if(m_curPair==curPair)
			return 0;
		m_curPair=curPair; 
		return 1;
	}

	//no landmark pair is picked!
	m_curPair=0;
	m_pickedType=this->typeNothing;

	return 0;
}

int landmarkPair::OnLButtonDown(unsigned int nFlags, float x, float y, float z, int dir, float norm[3])
{
	m_rightJustDown=0;
	m_leftJustDown=1;
	m_leftMouseDown=1;
	m_leftMouseDownX=x;
	m_leftMouseDownY=y;
	m_leftMouseDownZ=z;
	if(dir>=3)
		return 0;
	TriTreeViewer *f=this->m_triFather;
 	if( funPickTest(x,y, z)==1) //has one picked
	{
		if(m_ball->m_select==0)
			{m_ball->m_select=1;
		f->seeItInTree(m_ball);
		}
		this->funUpdateBallTran(this->m_curPair);

	}
	else
	{
		if(this->m_curPair==0)
		{
			if(m_ball->m_select==1)
				{
					m_ball->m_select=0;
			f->seeItInTree(m_ball);
		 
			f->showCurrentFocus();
			}
		}
		//if( m_pickedType=this->typeNothing

	}
	return 0;
}

int landmarkPair::OnMouseMove(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3])
{
	float ix,iy,iz,k; 
	ix=x; iy=y; iz=z;
	m_rightJustDown=0;
	m_leftJustDown=0;

	if( (this->m_pickedType==this->typeNothing) || (m_curPair==0) )
		return 0;

	if(dir>=3)
		return 0;
	
	TriTreeViewer *f=this->m_triFather;
	Array1DPoint3f *pts;
	if(m_leftMouseDown==1) 
	{
		if(m_pickedType==typePTfrom) 
		{
			( (ScenePoints*)m_curPair->findByPathName("From")->m_object )->m_points->m_data[0].set(x,y,z);
			( (SceneLines*)m_curPair->findByPathName("Line")->m_object )->m_points->m_data[0].set(x,y,z);
			this->funUpdateBallTran(m_curPair);
		}
	
		if(m_pickedType==typePTto)
		{
			( (ScenePoints*)m_curPair->findByPathName("To")->m_object )->m_points->m_data[0].set(x,y,z);
			( (SceneLines*)m_curPair->findByPathName("Line")->m_object )->m_points->m_data[1].set(x,y,z);

		}

		if(m_pickedType==typeBall)
		{
			Point3f p0= ( (ScenePoints*) (m_curPair->findByPathName("From")->m_object) )->m_points->m_data[0];
			double R;
			R=sqrt( p0.dist2(x,y,z) );

			m_curPair->findByPathName("R")->setVal(R);//->getVal(R);
			this->funUpdateBallTran(m_curPair);
			
			//->m_object) )->m_points->m_data[0];

			 
		
		}
		f->showCurrentFocus();
	}
	 
	return 0;
}

int landmarkPair::OnLButtonUp(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3])
{
	m_rightJustDown=0;
	m_leftMouseDown=0;
	if(dir>=3)
		return 0;
	return 0;
}
int landmarkPair::OnLButtonDblClk(unsigned int nFlags, float x, float y, float z, int dir, float norm[3])
{
	return 0;
}

int landmarkPair::funDeletePair()
{
  float x,y,z;
  TriTreeViewer *f=this->m_triFather;
  f->getFocusWorldXYZ(x,y,z);
  if(this->funPickTest(x,y,z)==-1)
	  return 0;
  if(this->m_curPair!=0)
	  m_curPair->delMe();
  /*
  m_line->del(m_pickedIdx*2);
  m_line->del(this->m_pickedIdx*2);
  m_ptFrom->del(m_pickedIdx);
  m_ptTo->del(m_pickedIdx);
  f->showCurrentFocus();
  */
  return 0;
}


int landmarkPair::funUpdateBallTran(SceneTree *thePair)
{
	double R;
	Point3f P0;
	SceneTree *nd;
	nd=thePair->findByPathName("R");
	nd->getVal(R);
	nd=thePair->findByPathName("From");
	P0=((ScenePoints*)nd->m_object)->m_points->m_data[0];

	//nd=thePair->findByPathName("Ball");
	TriTreeViewer *f=this->m_triFather;
	 double a[16],b[16];
	 ObjGen::tranMakeTranslate(a,P0.x,P0.y, P0.z);
	 ObjGen::tranMakeScaling(b,R,R,R);
	 ObjGen::tranMulab2c(a,b,a);
	 ObjGen::tranMulab2c(thePair->m_tran,a,m_ball->m_tran);//nd->m_tran);

	 if( (nd=thePair->findByPathName("Ball"))!=0)
		 ObjGen::tranCopy(nd->m_tran,m_ball->m_tran);

	 return 0;
}

SceneTree* landmarkPair::funNewApair()
{
	SceneTree *nd;
	ScenePoints *spt;
	char name[1024];
	sprintf(name,"%d",m_pairs->getSonNum());

	 TriTreeViewer *f=this->m_triFather;

	m_curPair=m_pairs->newAson(name);
    
	double fx,fy,fz;
	f->getFocusWorldXYZ(fx,fy,fz);
	SceneLines *sL;

	nd=m_curPair->ensureByPathName("Line", SCENE_OBJ_LINE);
	nd->m_materials->setDiffuse(0,255,255,255);
	sL= (SceneLines*)nd->m_object;
	sL->m_useDisplyList=0; 
	sL->m_size=6; 
	sL->m_points->appendPoint3f(fx,fy,fz);

	nd=m_curPair->ensureByPathName("From", SCENE_OBJ_POINT);
	nd->m_materials->setDiffuse(255,0,0,255);
	spt= (ScenePoints*)nd->m_object;
	spt->m_points->appendPoint3f(fx, fy, fz);
	spt->m_size=6;
	
	nd=m_curPair->ensureByPathName("To", SCENE_OBJ_POINT);
	nd->m_materials->setDiffuse(0,255,0,255);
	spt= (ScenePoints*)nd->m_object;
	spt->m_size=6;

	if(f->m_dir==0) fx+=10;////z
	if(f->m_dir==1) fx+=10; //y
	if(f->m_dir==2) fy+=10; //x
    spt->m_points->appendPoint3f(fx, fy, fz);
	 
	sL->m_points->appendPoint3f(fx,fy,fz);
	

	double v=20;
	m_curPair->ensureByPathName("R")->setVal(v);//, SCENE_OBJ_TRIANGLE);

	//nd=m_curPair->ensureByPathName("Ball", SCENE_OBJ_TRIANGLE);
	//nd->m_materials->setDiffuse(0,255,0,255);
 
	//SceneTriangle *stri=(SceneTriangle *)nd->m_object;
	//ObjGen::makeUnitSphere(6,stri->m_vex,stri->m_tri);
	//ObjGen::calculateNormal(stri->m_vex,stri->m_tri,stri->m_norm);
	m_ball->m_select=1;
	f->seeItInTree(m_ball);
	funUpdateBallTran(m_curPair);
	f->showCurrentFocus();

    return m_curPair;
}

int landmarkPair::OnRButtonDown(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3])
{
	m_rightJustDown=1;
	m_leftJustDown=0;
	m_rightMouseDownX=x; m_rightMouseDownY=y;  m_rightMouseDownZ=z;

	return 0;
}
 
int landmarkPair::OnRButtonUp(unsigned int nFlags, float x, float y, float z, int dir, float nrom[3])
{
	const char *menuName[2]={"delete Pair","new Pair"};
	if(m_rightJustDown==1)
		this->m_triFather->popupMenu(this,menuName,2);  
	m_rightJustDown=0;
	m_leftJustDown=0;
	return 1;
}

int landmarkPair::OnMenu(unsigned int id)
{
	switch (id)
	{
	case 0:
		this->funDeletePair();
		break;  
	case 1:
		funNewApair();//miueShowMessage("test 2",1);
		break;
	}
	return 0;
}

int landmarkPair::funSetPairs(Array1DPoint3f *from, Array1DPoint3f *to, Array1Ddouble *radiu)
{
	SceneTree *st,*nd;
	Array1DPoint3f *pts;
	m_pairs->delSons();
 	for(int i=0;i<from->m_used;i++)
	{
		st=funNewApair();
		nd=st->findByPathName("From");
		pts=((ScenePoints*)nd->m_object)->m_points;
		pts->m_used=0;
		pts->append(from->m_data[i]);

		nd=st->findByPathName("To");
		pts=((ScenePoints*)nd->m_object)->m_points;
		pts->m_used=0;
		pts->append(to->m_data[i]);

		nd=st->findByPathName("Line");
		pts=((SceneLines*)nd->m_object)->m_points;
		pts->m_used=0;
		pts->append(from->m_data[i]);
		pts->append(to->m_data[i]);

		//nd=st->findByPathName("Ball");
		nd=st->findByPathName("R");
		nd->setVal(radiu->m_data[i] ); //double value

	}

	m_ball->m_select=0;
	m_triFather->seeItInTree(m_ball);
	m_triFather->seeItInTree(m_container);
	this->m_triFather->showCurrentFocus();
	return 1;
}

int landmarkPair::funGetPairs(Array1DPoint3f *from, Array1DPoint3f *to, Array1Ddouble *radiu)
{
	SceneTree *st,*nd;
	Array1DPoint3f *pts;
	st=this->m_container->m_sons;
	from->m_used=0;
	to->m_used=0;
	radiu->m_used=0;

 	st=m_pairs->m_sons;
	while(st!=0)
	{
		nd=st->findByPathName("From");
		pts=((ScenePoints*)nd->m_object)->m_points;
		from->append(pts->m_data[0]);//pts->append(from->m_data[i]);

		nd=st->findByPathName("To");
		pts=((ScenePoints*)nd->m_object)->m_points;
		to->append(pts->m_data[0]);//pts->append(to->m_data[i]);

		nd=st->findByPathName("R");
		double r;
		nd->getVal(r);//->setVal(radiu->m_data[i] ); //double value
		radiu->append(r);
		st=st->m_next;
	}
	return 1;
}
