
#include "mcell_error.h"
#include "mcell_library.h"

#include "gdstk.h"

#include <vector>
#include <string>
#include <math.h>
#include <iostream>

using namespace std;
using namespace gdstk;

//  ############################
//  #
//  #			k
//  #		-------
//  #		*******
//  #				*	2k
//  #				  *
//  #					*
//  #					  * |
//  #					  *	|
//  #			0.5*d 	  *	| k
//  #		<------------>*	|
//  #
//	# 		k=(sqrt(2)-1)*0.5*d
//  ############################

double Gdslib::k(double d)
{
	double const sqrt2 = 1.41421356237;

	return (sqrt2 - 1) * 0.5 * d;
}

bool Gdslib::isNotOnGreed(Vec2 &a)
// if on greed false else true
{
	double grid = t.getGrid();

	if ((round(a.x / grid) * grid == a.x) and (round(a.y / grid) * grid == a.y))
	{
		return false;
	}
	else
	{
		return true;
	}
}

Vec2 Gdslib::round2grid(Vec2 a)
{
	double grid = t.getGrid();

	return Vec2{round(a.x / grid) * grid, round(a.y / grid) * grid};
}

double Gdslib::round2grid(double a)
{
	double grid = t.getGrid();

	return round(a / grid) * grid;
}

bool Gdslib::isEqual(double x, double y)
{
	// double grid=t.getGrid();
	// return abs(x-y)<0.01*grid ? true : false;
	return t.isEqual(x, y);
}

bool Gdslib::isEqual(Vec2 &a, Vec2 &b)
{
	return (this->isEqual(a.x, b.x) and this->isEqual(a.y, b.y));
}

bool Gdslib::is45deg(Vec2 &a, Vec2 &b)
{
	return this->isEqual(a.x, b.x) or this->isEqual(a.y, b.y) or this->isEqual(abs(a.x - b.x), abs(a.y - b.y));
}

int Gdslib::round2grid(Vec2 &result, Vec2 &prev)
{ // # round to grid so that previous point prev is 45 or 90 deg
	// # result=(x,y)	:point to be rounded
	// # prev=(x,y)		:previous point
	// # return 0 if rounded successfully

	//  ####################################################################################
	//  #
	//  #
	//  #
	//  #
	//  #
	//  #	(x1',y2')	 (x2',y2')			        			p4			p3
	//  #		*			*									*			*
	//  #							 	round()
	//  #			*(x,y)				=========>
	//  #
	//  #
	//  #		*			*  									*			*
	//  #	(x1',y1')   (x2',y1') 			      				p1			p2
	//  #
	//  #
	//  #
	//	#
	//	#
	//	#
	//  ####################################################################################
	//  ####################################################################################
	//  ####################################################################################
	//  ***************should not round x nor y if already on grid*********************
	//  interface between two poligons must be on grid garanteed by higher design
	//  ####################################################################################
	//  ####################################################################################

	double grid = t.getGrid();

	Vec2 pr = this->round2grid(result);

	double x1 = (result.x > pr.x) or this->isEqual(result.x, pr.x) ? pr.x : pr.x - grid;
	double y1 = (result.y > pr.y) or this->isEqual(result.y, pr.y) ? pr.y : pr.y - grid;

	Vec2 p1 = Vec2{x1, y1};
	Vec2 p2 = Vec2{x1 + grid, y1};
	Vec2 p3 = Vec2{x1 + grid, y1 + grid};
	Vec2 p4 = Vec2{x1, y1 + grid};

	// cout<<"*****************************************************************"<<endl;
	// cout<<"("<<result.x<<","<<result.y<<")=>("<<pr.x<<","<<pr.y<<")  Pre: "<<"("<<prev.x<<","<<prev.y<<")"<<endl;
	// cout<<"pr="<<"("<<pr.x<<","<<pr.y<<")"<<endl;
	// cout<<"p1="<<"("<<p1.x<<","<<p1.y<<")"<<endl;
	// cout<<"p2="<<"("<<p2.x<<","<<p2.y<<")"<<endl;
	// cout<<"p3="<<"("<<p3.x<<","<<p3.y<<")"<<endl;
	// cout<<"p4="<<"("<<p4.x<<","<<p4.y<<")"<<endl;

	// check if prev on squer
	if (this->isEqual(prev, pr) or this->isEqual(prev, p1) or this->isEqual(prev, p2) or this->isEqual(prev, p3))
	{
		// round is not possible
		// cout<<"Remove"<<endl;
		return 1;
	}

	if (this->isEqual(pr, result))
	{
		// check if round to pr possible
		if (this->is45deg(pr, prev))
		{
			// cout<<"rounded to pr!"<<endl;
			result.x = pr.x;
			result.y = pr.y;

			return 0;
		}
	}

	else if (this->isEqual(pr.x, result.x))
	{ // x is on grid and should not be changed
	  // pr is p1 or p4

		// check if round to pr possible
		if (this->is45deg(pr, prev))
		{
			result.x = pr.x;
			result.y = pr.y;

			return 0;
		}

		// check if round to p1 possible
		if (this->is45deg(p1, prev))
		{
			result.x = p1.x;
			result.y = p1.y;

			return 0;
		}

		// check if round to p4 possible
		if (this->is45deg(p4, prev))
		{
			result.x = p4.x;
			result.y = p4.y;

			return 0;
		}

	} // if

	else if (this->isEqual(pr.y, result.y))
	{ // y is on grid and should not be changed
	  // pr is p1 or p2

		// check if round to pr possible
		if (this->is45deg(pr, prev))
		{
			result.x = pr.x;
			result.y = pr.y;

			return 0;
		}

		// check if round to p1 possible
		if (this->is45deg(p1, prev))
		{
			result.x = p1.x;
			result.y = p1.y;

			return 0;
		}

		// check if round to p2 possible
		if (this->is45deg(p2, prev))
		{
			result.x = p2.x;
			result.y = p2.y;

			return 0;
		}
	}
	else // x is not on grid and y is not on grid
	{

		// check if round to pr possible
		if (this->is45deg(pr, prev))
		{
			result.x = pr.x;
			result.y = pr.y;

			return 0;
		}

		// check if round to p1 possible
		if (this->is45deg(p1, prev))
		{
			result.x = p1.x;
			result.y = p1.y;

			return 0;
		}

		// check if round to p2 possible
		if (this->is45deg(p2, prev))
		{
			result.x = p2.x;
			result.y = p2.y;

			return 0;
		}

		// check if round to p3 possible
		if (this->is45deg(p3, prev))
		{
			result.x = p3.x;
			result.y = p3.y;

			return 0;
		}

		// check if round to p4 possible
		if (this->is45deg(p4, prev))
		{
			result.x = p4.x;
			result.y = p4.y;

			return 0;
		}

	} // if

	return 2;
}

int Gdslib::round2grid(Polygon *&result) // round all points in result to grid
{
	result->point_array[0].x = this->round2grid(result->point_array[0].x);
	result->point_array[0].y = this->round2grid(result->point_array[0].y);
	for (long unsigned int i = 1; i < result->point_array.count - 1; i++)
	{

		int tmp = this->round2grid(result->point_array[i], result->point_array[i - 1]);

		if (tmp == 1 and i < result->point_array.count - 1) // round is not possible i-1 on box, delete i
		{

			result->point_array.remove(i);
			i--;
		}
		else if (tmp > 1)
		{
			// std::cout<<"Error: Round to grid not possible!"<<endl;
			throw HardError("Error: Round to grid not possible!");
		}

		// std::cout<<"Rounded:"<<"("<<result->point_array[i].x<<","<<result->point_array[i].y<<")"<<endl;

	} // for

	// round last point to be on 45 or 90 with first one and previous

	int tmp = this->round2grid45(result->point_array[result->point_array.count - 1], result->point_array[result->point_array.count - 2], result->point_array[0]);

	if (tmp > 1)
	{
		// std::cout<<"Error: Round to grid not possible!"<<endl;
		throw HardError("Error: Round to grid not possible!");
	}

	// std::cout<<"--------------------------finished function---------------------------------------"<<endl;
	return 0;
}

int Gdslib::round2grid45(Vec2 &result, Vec2 &prev)
{ // # used to round for patterned ground shield
	// # round to grid so that previous point prev is 45 or 90 deg
	// # result=(x,y)	:point to be rounded
	// # prev=(x,y)		:previous point
	// # return 0 if rounded successfully

	//  ####################################################################################
	//  #
	//  #											*			*			*			*
	//  #											p14			p13			p12			p11
	//  #
	//  #
	//  #	(x1',y2')	 (x2',y2')			        p15			p4			p3 			p10
	//  #		*			*						*			*			*			*
	//  #							 	round()
	//  #			*(x,y)				=========>
	//  #
	//  #
	//  #		*			*  						*			*			*			*
	//  #	(x1',y1')   (x2',y1') 			      	p16			p1			p2 			p9
	//  #
	//  #
	//  #											*			*			*			*
	//	#											p5			p6			p7			p8
	//	#
	//	#
	//  ####################################################################################
	//  ####################################################################################
	//  ####################################################################################
	//  ***************should not round x nor y if already on grid*********************
	//  interface between two polygons must be on grid garanteed by higher design
	//  ####################################################################################
	//  ####################################################################################

	double grid = t.getGrid();

	Vec2 pr = this->round2grid(result);

	double x1 = (result.x > pr.x) or this->isEqual(result.x, pr.x) ? pr.x : pr.x - grid;
	double y1 = (result.y > pr.y) or this->isEqual(result.y, pr.y) ? pr.y : pr.y - grid;

	Vec2 p1 = Vec2{x1, y1};
	Vec2 p2 = Vec2{x1 + grid, y1};
	Vec2 p3 = Vec2{x1 + grid, y1 + grid};
	Vec2 p4 = Vec2{x1, y1 + grid};
	Vec2 p5 = Vec2{x1 - grid, y1 - grid};
	Vec2 p6 = Vec2{x1, y1 - grid};
	Vec2 p7 = Vec2{x1 + grid, y1 - grid};
	Vec2 p8 = Vec2{x1 + 2 * grid, y1 - grid};
	Vec2 p9 = Vec2{x1 + 2 * grid, y1};
	Vec2 p10 = Vec2{x1 + 2 * grid, y1 + grid};
	Vec2 p11 = Vec2{x1 + 2 * grid, y1 + 2 * grid};
	Vec2 p12 = Vec2{x1 + grid, y1 + 2 * grid};
	Vec2 p13 = Vec2{x1, y1 + 2 * grid};
	Vec2 p14 = Vec2{x1 - grid, y1 + 2 * grid};
	Vec2 p15 = Vec2{x1 - grid, y1 + grid};
	Vec2 p16 = Vec2{x1 - grid, y1};

	// cout<<"*****************************************************************"<<endl;
	// cout<<"("<<result.x<<","<<result.y<<")=>("<<pr.x<<","<<pr.y<<")  Pre: "<<"("<<prev.x<<","<<prev.y<<")"<<endl;
	// cout<<"pr="<<"("<<pr.x<<","<<pr.y<<")"<<endl;
	// cout<<"p1="<<"("<<p1.x<<","<<p1.y<<")"<<endl;
	// cout<<"p2="<<"("<<p2.x<<","<<p2.y<<")"<<endl;
	// cout<<"p3="<<"("<<p3.x<<","<<p3.y<<")"<<endl;
	// cout<<"p4="<<"("<<p4.x<<","<<p4.y<<")"<<endl;
	// cout<<"p5="<<"("<<p5.x<<","<<p5.y<<")"<<endl;
	// cout<<"p6="<<"("<<p6.x<<","<<p6.y<<")"<<endl;
	// cout<<"p7="<<"("<<p7.x<<","<<p7.y<<")"<<endl;
	// cout<<"p8="<<"("<<p8.x<<","<<p8.y<<")"<<endl;
	// cout<<"p9="<<"("<<p9.x<<","<<p9.y<<")"<<endl;
	// cout<<"p10="<<"("<<p10.x<<","<<p10.y<<")"<<endl;
	// cout<<"p11="<<"("<<p11.x<<","<<p11.y<<")"<<endl;
	// cout<<"p12="<<"("<<p12.x<<","<<p12.y<<")"<<endl;
	// cout<<"p13="<<"("<<p13.x<<","<<p13.y<<")"<<endl;
	// cout<<"p14="<<"("<<p14.x<<","<<p14.y<<")"<<endl;
	// cout<<"p15="<<"("<<p15.x<<","<<p15.y<<")"<<endl;
	// cout<<"p16="<<"("<<p16.x<<","<<p16.y<<")"<<endl;

	// check if prev on squer
	if (this->isEqual(prev, pr) or this->isEqual(prev, p1) or this->isEqual(prev, p2) or this->isEqual(prev, p3))
	{
		// round is not possible
		// cout<<"Remove"<<endl;
		return 1;
	}

	// check if round to pr possible
	if (this->is45deg(pr, prev))
	{
		// cout<<"rounded to pr!"<<endl;
		result.x = pr.x;
		result.y = pr.y;

		return 0;
	}

	// check if round to p1 possible
	if (this->is45deg(p1, prev))
	{
		// cout<<"rounded to pr!"<<endl;
		result.x = p1.x;
		result.y = p1.y;

		return 0;
	}

	// check if round to p2 possible
	if (this->is45deg(p2, prev))
	{
		// cout<<"rounded to p2!"<<endl;
		result.x = p2.x;
		result.y = p2.y;

		return 0;
	}

	// check if round to p3 possible
	if (this->is45deg(p3, prev))
	{
		// cout<<"rounded to p3!"<<endl;
		result.x = p3.x;
		result.y = p3.y;

		return 0;
	}

	// check if round to p4 possible
	if (this->is45deg(p4, prev))
	{
		// cout<<"rounded to p4!"<<endl;
		result.x = p4.x;
		result.y = p4.y;

		return 0;
	}

	// check if round to p5 possible
	if (this->is45deg(p5, prev))
	{
		// cout<<"rounded to p5!"<<endl;
		result.x = p5.x;
		result.y = p5.y;

		return 0;
	}

	// check if round to p6 possible
	if (this->is45deg(p6, prev))
	{
		// cout<<"rounded to p6!"<<endl;
		result.x = p6.x;
		result.y = p6.y;

		return 0;
	}

	// check if round to p7 possible
	if (this->is45deg(p7, prev))
	{
		// cout<<"rounded to p7!"<<endl;
		result.x = p7.x;
		result.y = p7.y;

		return 0;
	}

	// check if round to p8 possible
	if (this->is45deg(p8, prev))
	{
		// cout<<"rounded to p8!"<<endl;
		result.x = p8.x;
		result.y = p8.y;

		return 0;
	}

	// check if round to p9 possible
	if (this->is45deg(p9, prev))
	{
		// cout<<"rounded to p9!"<<endl;
		result.x = p9.x;
		result.y = p9.y;

		return 0;
	}

	// check if round to p10 possible
	if (this->is45deg(p10, prev))
	{
		// cout<<"rounded to p10!"<<endl;
		result.x = p10.x;
		result.y = p10.y;

		return 0;
	}

	// check if round to p11 possible
	if (this->is45deg(p11, prev))
	{
		// cout<<"rounded to p11!"<<endl;
		result.x = p11.x;
		result.y = p11.y;

		return 0;
	}

	// check if round to p12 possible
	if (this->is45deg(p12, prev))
	{
		// cout<<"rounded to p12!"<<endl;
		result.x = p12.x;
		result.y = p12.y;

		return 0;
	}

	// check if round to p13 possible
	if (this->is45deg(p13, prev))
	{
		// cout<<"rounded to p13!"<<endl;
		result.x = p13.x;
		result.y = p13.y;

		return 0;
	}

	// check if round to p14 possible
	if (this->is45deg(p14, prev))
	{
		// cout<<"rounded to p14!"<<endl;
		result.x = p14.x;
		result.y = p14.y;

		return 0;
	}

	// check if round to p15 possible
	if (this->is45deg(p15, prev))
	{
		// cout<<"rounded to p15!"<<endl;
		result.x = p15.x;
		result.y = p15.y;

		return 0;
	}

	// check if round to p16 possible
	if (this->is45deg(p16, prev))
	{
		// cout<<"rounded to p16!"<<endl;
		result.x = p16.x;
		result.y = p16.y;

		return 0;
	}

	return 2;
}

int Gdslib::round2grid45(Vec2 &result, Vec2 &prev, Vec2 &next)
{ // # used to round for patterned ground shield
	// # round to grid so that previous point prev is 45 or 90 deg
	// # result=(x,y)	:point to be rounded
	// # prev=(x,y)		:previous point
	// # return 0 if rounded successfully

	//  ####################################################################################
	//  #
	//  #											*			*			*			*
	//  #											p14			p13			p12			p11
	//  #
	//  #
	//  #	(x1',y2')	 (x2',y2')			        p15			p4			p3 			p10
	//  #		*			*						*			*			*			*
	//  #							 	round()
	//  #			*(x,y)				=========>
	//  #
	//  #
	//  #		*			*  						*			*			*			*
	//  #	(x1',y1')   (x2',y1') 			      	p16			p1			p2 			p9
	//  #
	//  #
	//  #											*			*			*			*
	//	#											p5			p6			p7			p8
	//	#
	//	#
	//  ####################################################################################
	//  ####################################################################################
	//  ####################################################################################
	//  ***************should not round x nor y if already on grid*********************
	//  interface between two polygons must be on grid garanteed by higher design
	//  ####################################################################################
	//  ####################################################################################

	double grid = t.getGrid();

	Vec2 pr = this->round2grid(result);

	double x1 = (result.x > pr.x) or this->isEqual(result.x, pr.x) ? pr.x : pr.x - grid;
	double y1 = (result.y > pr.y) or this->isEqual(result.y, pr.y) ? pr.y : pr.y - grid;

	Vec2 p1 = Vec2{x1, y1};
	Vec2 p2 = Vec2{x1 + grid, y1};
	Vec2 p3 = Vec2{x1 + grid, y1 + grid};
	Vec2 p4 = Vec2{x1, y1 + grid};
	Vec2 p5 = Vec2{x1 - grid, y1 - grid};
	Vec2 p6 = Vec2{x1, y1 - grid};
	Vec2 p7 = Vec2{x1 + grid, y1 - grid};
	Vec2 p8 = Vec2{x1 + 2 * grid, y1 - grid};
	Vec2 p9 = Vec2{x1 + 2 * grid, y1};
	Vec2 p10 = Vec2{x1 + 2 * grid, y1 + grid};
	Vec2 p11 = Vec2{x1 + 2 * grid, y1 + 2 * grid};
	Vec2 p12 = Vec2{x1 + grid, y1 + 2 * grid};
	Vec2 p13 = Vec2{x1, y1 + 2 * grid};
	Vec2 p14 = Vec2{x1 - grid, y1 + 2 * grid};
	Vec2 p15 = Vec2{x1 - grid, y1 + grid};
	Vec2 p16 = Vec2{x1 - grid, y1};

	// cout<<"*****************************************************************"<<endl;
	// cout<<"("<<result.x<<","<<result.y<<")=>("<<pr.x<<","<<pr.y<<")  Pre: "<<"("<<prev.x<<","<<prev.y<<")"<<endl;
	// cout<<"pr="<<"("<<pr.x<<","<<pr.y<<")"<<endl;
	// cout<<"p1="<<"("<<p1.x<<","<<p1.y<<")"<<endl;
	// cout<<"p2="<<"("<<p2.x<<","<<p2.y<<")"<<endl;
	// cout<<"p3="<<"("<<p3.x<<","<<p3.y<<")"<<endl;
	// cout<<"p4="<<"("<<p4.x<<","<<p4.y<<")"<<endl;
	// cout<<"p5="<<"("<<p5.x<<","<<p5.y<<")"<<endl;
	// cout<<"p6="<<"("<<p6.x<<","<<p6.y<<")"<<endl;
	// cout<<"p7="<<"("<<p7.x<<","<<p7.y<<")"<<endl;
	// cout<<"p8="<<"("<<p8.x<<","<<p8.y<<")"<<endl;
	// cout<<"p9="<<"("<<p9.x<<","<<p9.y<<")"<<endl;
	// cout<<"p10="<<"("<<p10.x<<","<<p10.y<<")"<<endl;
	// cout<<"p11="<<"("<<p11.x<<","<<p11.y<<")"<<endl;
	// cout<<"p12="<<"("<<p12.x<<","<<p12.y<<")"<<endl;
	// cout<<"p13="<<"("<<p13.x<<","<<p13.y<<")"<<endl;
	// cout<<"p14="<<"("<<p14.x<<","<<p14.y<<")"<<endl;
	// cout<<"p15="<<"("<<p15.x<<","<<p15.y<<")"<<endl;
	// cout<<"p16="<<"("<<p16.x<<","<<p16.y<<")"<<endl;

	// check if prev on squer
	if (this->isEqual(prev, pr) or this->isEqual(prev, p1) or this->isEqual(prev, p2) or this->isEqual(prev, p3))
	{
		// round is not possible
		// cout<<"Remove"<<endl;
		return 1;
	}

	// check if round to pr possible
	if (this->is45deg(pr, prev) and this->is45deg(pr, next))
	{
		// cout<<"rounded to pr!"<<endl;
		result.x = pr.x;
		result.y = pr.y;

		return 0;
	}

	// check if round to p1 possible
	if (this->is45deg(p1, prev) and this->is45deg(p1, next))
	{
		// cout<<"rounded to pr!"<<endl;
		result.x = p1.x;
		result.y = p1.y;

		return 0;
	}

	// check if round to p2 possible
	if (this->is45deg(p2, prev) and this->is45deg(p2, next))
	{
		// cout<<"rounded to p2!"<<endl;
		result.x = p2.x;
		result.y = p2.y;

		return 0;
	}

	// check if round to p3 possible
	if (this->is45deg(p3, prev) and this->is45deg(p3, next))
	{
		// cout<<"rounded to p3!"<<endl;
		result.x = p3.x;
		result.y = p3.y;

		return 0;
	}

	// check if round to p4 possible
	if (this->is45deg(p4, prev) and this->is45deg(p4, next))
	{
		// cout<<"rounded to p4!"<<endl;
		result.x = p4.x;
		result.y = p4.y;

		return 0;
	}

	// check if round to p5 possible
	if (this->is45deg(p5, prev) and this->is45deg(p5, next))
	{
		// cout<<"rounded to p5!"<<endl;
		result.x = p5.x;
		result.y = p5.y;

		return 0;
	}

	// check if round to p6 possible
	if (this->is45deg(p6, prev) and this->is45deg(p6, next))
	{
		// cout<<"rounded to p6!"<<endl;
		result.x = p6.x;
		result.y = p6.y;

		return 0;
	}

	// check if round to p7 possible
	if (this->is45deg(p7, prev) and this->is45deg(p7, next))
	{
		// cout<<"rounded to p7!"<<endl;
		result.x = p7.x;
		result.y = p7.y;

		return 0;
	}

	// check if round to p8 possible
	if (this->is45deg(p8, prev) and this->is45deg(p8, next))
	{
		// cout<<"rounded to p8!"<<endl;
		result.x = p8.x;
		result.y = p8.y;

		return 0;
	}

	// check if round to p9 possible
	if (this->is45deg(p9, prev) and this->is45deg(p9, next))
	{
		// cout<<"rounded to p9!"<<endl;
		result.x = p9.x;
		result.y = p9.y;

		return 0;
	}

	// check if round to p10 possible
	if (this->is45deg(p10, prev) and this->is45deg(p10, next))
	{
		// cout<<"rounded to p10!"<<endl;
		result.x = p10.x;
		result.y = p10.y;

		return 0;
	}

	// check if round to p11 possible
	if (this->is45deg(p11, prev) and this->is45deg(p11, next))
	{
		// cout<<"rounded to p11!"<<endl;
		result.x = p11.x;
		result.y = p11.y;

		return 0;
	}

	// check if round to p12 possible
	if (this->is45deg(p12, prev) and this->is45deg(p12, next))
	{
		// cout<<"rounded to p12!"<<endl;
		result.x = p12.x;
		result.y = p12.y;

		return 0;
	}

	// check if round to p13 possible
	if (this->is45deg(p13, prev) and this->is45deg(p13, next))
	{
		// cout<<"rounded to p13!"<<endl;
		result.x = p13.x;
		result.y = p13.y;

		return 0;
	}

	// check if round to p14 possible
	if (this->is45deg(p14, prev) and this->is45deg(p14, next))
	{
		// cout<<"rounded to p14!"<<endl;
		result.x = p14.x;
		result.y = p14.y;

		return 0;
	}

	// check if round to p15 possible
	if (this->is45deg(p15, prev) and this->is45deg(p15, next))
	{
		// cout<<"rounded to p15!"<<endl;
		result.x = p15.x;
		result.y = p15.y;

		return 0;
	}

	// check if round to p16 possible
	if (this->is45deg(p16, prev) and this->is45deg(p16, next))
	{
		// cout<<"rounded to p16!"<<endl;
		result.x = p16.x;
		result.y = p16.y;

		return 0;
	}

	return 2;
}

int Gdslib::round2grid45(Polygon *&result) // round all points in result to grid
{
	result->point_array[0].x = this->round2grid(result->point_array[0].x);
	result->point_array[0].y = this->round2grid(result->point_array[0].y);
	for (long unsigned int i = 1; i < result->point_array.count - 1; i++)
	{

		int tmp = this->round2grid45(result->point_array[i], result->point_array[i - 1]);

		if (tmp == 1) // round is not possible i-1 on box, delete i
		{
			// std::cout<<"Error: Remove!"<<endl;
			result->point_array.remove(i);
			i--;
		}
		else if (tmp > 1)
		{
			// std::cout<<"Error: Round to grid not possible!"<<endl;
			throw HardError("Error: Round to grid not possible!");
		}

		// std::cout<<"Rounded:"<<"("<<result->point_array[i].x<<","<<result->point_array[i].y<<")"<<endl;

	} // for

	int tmp = this->round2grid45(result->point_array[result->point_array.count - 1], result->point_array[result->point_array.count - 2], result->point_array[0]);

	if (tmp > 1)
	{
		// std::cout<<"Error: Round to grid not possible!"<<endl;
		throw HardError("Error: Round to grid not possible!");
	}

	// std::cout<<"--------------------------finished function---------------------------------------"<<endl;
	return 0;
}

int Gdslib::appendArrToCell(Cell &cell, Array<Polygon *> &arry)
{
	for (long unsigned int i = 0; i < arry.count; i++)
	{
		cell.polygon_array.append(arry[i]);
	}

	arry.clear();
	arry = {0};

	return 0;
}

LayerVia Gdslib::getVia(string ttopMet, string bbotMet)
{
	return this->t.getVia(ttopMet, bbotMet);
}

LayerMet Gdslib::getMet(string nname)
{
	return this->t.getMet(nname);
}

double Gdslib::getGrid()
{
	return this->t.getGrid();
}

vector<char *> Gdslib::splitStr(char *str, char *delimiter)
{
	return this->t.splitStr(str, delimiter);
}

int Gdslib::rectangle(Polygon *&result, Vec2 p1, Vec2 p2, string layerName)
{
	// # p1=(x,y)		:upper right corner
	// # p2=(x,y)		:lower left corner
	// # layerName		:metal name from technology

	p1 = round2grid(p1);
	p2 = round2grid(p2);

	LayerMet met = t.getMet(layerName);

	result = (Polygon *)allocate_clear(sizeof(Polygon));

	*result = {0};

	result->copy_from(gdstk::rectangle(p1, p2, met.gdsNum, met.gdsType));

	return 0;
}

int Gdslib::via(Polygon *&result, Vec2 p1, Vec2 p2, string topM, string bottomM)
{
	// # result			:output, pointer on Polygon
	// # p1=(x,y)		:upper right corner
	// # p2=(x,y)		:lower left corner
	// # topM			:top metal name
	// # bottomM		:bottom metal name

	p1 = this->round2grid(p1);
	p2 = this->round2grid(p2);

	LayerVia viaId = this->t.getVia(topM, bottomM);

	result = (Polygon *)allocate_clear(sizeof(Polygon));
	*result = {0};

	result->copy_from(gdstk::rectangle(p1, p2, viaId.gdsNum, viaId.gdsType));

	return 0;
}

int Gdslib::path(Polygon *&result, Array<Vec2> &pointList, double w, string layerName)

{
	// create FlexPath

	FlexPath *fp = (FlexPath *)allocate_clear(1 * sizeof(FlexPath));
	fp->simple_path = true;

	// grid
	double grid = t.getGrid();

	// layer
	LayerMet met = t.getMet(layerName);

	// FlexPath inicialization
	fp->init(Vec2{pointList[0].x, pointList[0].y}, 1, w, 0, grid);
	pointList.remove(0);

	// assign pointList to path
	fp->segment(pointList, NULL, NULL, false);

	// assign layer to path
	fp->properties = NULL;
	fp->elements[0].layer = met.gdsNum;
	fp->elements[0].datatype = met.gdsType;

	// covert to polygon
	Array<Polygon *> ap = {0};
	fp->to_polygons(ap);

	// free path from memory
	fp->clear();
	fp = NULL;

	result = ap[0];

	this->round2grid(result);

	// function argument is removed from memory
	pointList.clear();
	pointList = {0};

	result->layer = met.gdsNum;
	result->datatype = met.gdsType;

	return 0;
}

int Gdslib::crop(Array<Polygon *> &result, Polygon *&poly, Vec2 p1, Vec2 p2, string layerName)
{

	Array<Polygon *> array_poly = {0};
	array_poly.append(poly);

	Polygon *rec = NULL;
	this->rectangle(rec, p1, p2, layerName);
	Array<Polygon *> array_rec = {0};
	array_rec.append(rec);

	// double grid=this->getGrid();

	boolean(array_poly, array_rec, Operation::Not, 1 / this->getGrid(), result);

	// free from memory

	rec->clear();
	free(rec);
	rec = NULL;

	poly->clear();
	free(poly);
	poly = NULL;

	array_poly.clear();
	array_rec.clear();

	// layer
	LayerMet met = this->t.getMet(layerName);

	for (long unsigned int i = 0; i < result.count; i++)
	{

		// cout << result[i];
		result[i]->layer = met.gdsNum;

		result[i]->datatype = met.gdsType;
	}

	return 0;
}

int Gdslib::bridge45(Polygon *&result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string layerName)

{
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # this two planes are connected with Bridge45
	// #####################################################################################################
	// # 			p1
	// #   ********
	// #          * --------------------------------------
	// #   ********  										|
	// #		  |p2  										| e
	// #          |                p3 						|
	// #          |                *******************		|
	// #          <--------------->* 		|w			----
	// #			       b       *******************
	// #                          p4
	// #####################################################################################################
	// #			*	*
	// #			*	*
	// #			*	*
	// #		 p1	***** p2 ------------------
	// #			  |						  |
	// #             |    e 				  |
	// #			  <-------->|			  | b
	// #						|			  |
	// #				   p3 ***** p4 --------
	// #					  *	  *
	// #					  *<->*
	// #					  * w *
	// #####################################################################################################

	// layer
	LayerMet met = this->t.getMet(layerName);

	// double grid=2*this->t.getGrid();

	// p1=this->round2grid(p1);
	// p2=this->round2grid(p2);
	// p3=this->round2grid(p3);
	// p4=this->round2grid(p4);

	// 	cout<<"p1="<<"("<<p1.x<<","<<p1.y<<")"<<endl;
	//  cout<<"p2="<<"("<<p2.x<<","<<p2.y<<")"<<endl;
	//  cout<<"p3="<<"("<<p3.x<<","<<p3.y<<")"<<endl;
	//  cout<<"p4="<<"("<<p4.x<<","<<p4.y<<")"<<endl;

	Vec2 pright = {0};
	Vec2 pleft = {0};

	Vec2 pup = {0};
	Vec2 pdown = {0};

	double e = 0;
	double b = 0;
	double w = 0;

	if (this->isEqual(p1.x, p2.x) and this->isEqual(p3.x, p4.x) and this->isEqual(abs(p1.y - p2.y), abs(p3.y - p4.y))) // case 1 from the picture
	//(p1.x==p2.x and p3.x==p4.x) and (abs(p1.y-p2.y) == abs(p3.y-p4.y))
	{

		if (p1.x <= p3.x)
		{
			pleft = Vec2{p1.x, 0.5 * (p1.y + p2.y)};
			pright = Vec2{p3.x, 0.5 * (p3.y + p4.y)};
		}

		else
		{
			pright = Vec2{p1.x, 0.5 * (p1.y + p2.y)};
			pleft = Vec2{p3.x, 0.5 * (p3.y + p4.y)};
		}

		// Conductor dimensions
		w = abs(p1.y - p2.y);
		b = abs(pright.x - pleft.x);
		e = abs(pright.y - pleft.y);

		if (b <= e + w / (1 + sqrt(2)))
		{
			// Error: soft error
			throw SoftError("Warning: <Gdslib::bridge45> : Invalid geometry, case 1! ");
		}

		Vec2 pright_shift = Vec2{pright.x - 0.5 * (b - e), pright.y};
		Vec2 pleft_shift = Vec2{pleft.x + 0.5 * (b - e), pleft.y};

		Array<Vec2> pointList = {0};

		pointList.ensure_slots(4);

		pointList.append(pleft);
		pointList.append(pleft_shift);
		pointList.append(pright_shift);
		pointList.append(pright);

		this->path(result, pointList, w, layerName);

		pointList.clear();
	}

	else if (this->isEqual(p1.y, p2.y) and this->isEqual(p3.y, p4.y) and this->isEqual(abs(p1.x - p2.x), abs(p3.x - p4.x))) // case 2 from the picture
																															//(p1.y==p2.y and p3.y==p4.y) and (abs(p1.x-p2.x) == abs(p3.x-p4.x))
	{

		if (p1.y >= p3.y)
		{
			pup = Vec2{0.5 * (p1.x + p2.x), p1.y};
			pdown = Vec2{0.5 * (p3.x + p4.x), p3.y};
		}
		else
		{
			pdown = Vec2{0.5 * (p1.x + p2.x), p1.y};
			pup = Vec2{0.5 * (p3.x + p4.x), p3.y};
		}

		// Conductor dimensions
		w = abs(p1.x - p2.x);
		b = abs(pup.y - pdown.y);
		e = abs(pup.x - pdown.x);

		if (b <= e + w / (1 + sqrt(2)))
		{
			// Error: soft error
			throw SoftError("Warning: <Gdslib::bridge45> : Invalid geometry, case 2!");
		}

		Vec2 pup_shift = Vec2{pup.x, pup.y - 0.5 * (b - e)};
		Vec2 pdown_shift = Vec2{pdown.x, pdown.y + 0.5 * (b - e)};

		Array<Vec2> pointList = {0};

		pointList.ensure_slots(4);

		pointList.append(pup);
		pointList.append(pup_shift);
		pointList.append(pdown_shift);
		pointList.append(pdown);

		this->path(result, pointList, w, layerName);

		pointList.clear();
	}

	else
	{
		// Error: soft error
		throw SoftError("Warning: <Gdslib::bridge45> : Invalid geometry, case X!");
	}

	return 0;
}

int Gdslib::bridge45x2(Polygon *&result1, Polygon *&result2, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string layerName1, string layerName2)
{

	this->bridge45(result1, p1, p2, p3, p4, layerName1);

	Vec2 pp1, pp2, pp3, pp4;

	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horisontal connection
	{
		pp1.x = p1.x;
		pp1.y = p3.y;

		pp2.x = p1.x;
		pp2.y = p4.y;

		pp3.x = p3.x;
		pp3.y = p1.y;

		pp4.x = p3.x;
		pp4.y = p2.y;
	}
	else if (this->isEqual(p1.y, p2.y)) // vertical connection p1.y == p2.y
	{
		pp1.x = p3.x;
		pp1.y = p1.y;

		pp2.x = p4.x;
		pp2.y = p1.y;

		pp3.x = p1.x;
		pp3.y = p3.y;

		pp4.x = p2.x;
		pp4.y = p3.y;
	}

	else // error
	{
		// Error: soft error
		throw SoftError("Warning: <Gdslib::bridge45x2> : Invalid geometry!");
	}

	this->bridge45(result2, pp1, pp2, pp3, pp4, layerName2);

	return 0;
}

int Gdslib::octSegment(Polygon *&result, double r, double w, int quadrant, string layerName)
{
	// 	# Parameters:
	// 	#            w 	: float
	// 	# 					segment width
	// 	#            r 	: float
	// 	#             		outer diameter
	// 	# 		quadrant: integer
	//  #            		quadrant to draw segment (0 - 3)
	//  #				*****************
	//  #				*	1	*	0	*
	//  #				*****************
	//  #				*	2	*	3	*
	//  #				*****************
	//  ########################################################
	//  #
	//  #			b
	//  #		-------
	//  #		*******
	//  #				*	2b
	//  #				  *
	//  #					*
	//  #					  * |
	//  #					  *	|
	//  #			0.5*r 	  *	| b
	//  #		<------------>*	|
	//  #
	//	# 		b=(sqrt(2)-1)*0.5*r

	double b = (sqrt(2) - 1) * 0.5 * r;
	double a = (sqrt(2) - 1) * (0.5 * r - w);

	double xs = 1;
	double ys = 1;

	if (a <= 0)
	{
		throw SoftError("Warning: <Gdslib::octSegment> : Invalid geometry!");
	}

	if (quadrant != 0 and quadrant != 1 and quadrant != 2 and quadrant != 3)
	{
		throw SoftError("Warning: <Gdslib::octSegment> : Invalid geometry!");
	}

	if (quadrant == 1)
	{
		xs = -1;
		ys = 1;
	}
	else if (quadrant == 2)
	{
		xs = -1;
		ys = -1;
	}
	else if (quadrant == 3)
	{
		xs = 1;
		ys = -1;
	}

	Vec2 p1 = Vec2{0, 0.5 * (r - w) * ys};
	Vec2 p2 = Vec2{0.5 * (a + b) * xs, 0.5 * (r - w) * ys};
	Vec2 p3 = Vec2{0.5 * (r - w) * xs, 0.5 * (a + b) * ys};
	Vec2 p4 = Vec2{0.5 * (r - w) * xs, 0};

	Array<Vec2> pointsList = {0};
	pointsList.ensure_slots(4);

	pointsList.append(p1);
	pointsList.append(p2);
	pointsList.append(p3);
	pointsList.append(p4);

	this->path(result, pointsList, w, layerName);

	pointsList.clear();

	return 0;
}

int Gdslib::rectSegment(Polygon *&result, double r, double w, int quadrant, string layerName)
{

	double b = 0.5 * r;
	double a = 0.5 * r - w;

	double xs = 1;
	double ys = 1;

	if (a <= 0)
	{
		throw SoftError("Warning: <Gdslib::rectSegment> : Invalid geometry!");
	}

	if (quadrant != 0 and quadrant != 1 and quadrant != 2 and quadrant != 3)
	{
		throw SoftError("Warning: <Gdslib::rectSegment> : Invalid geometry!");
	}

	if (quadrant == 1)
	{
		xs = -1;
		ys = 1;
	}
	else if (quadrant == 2)
	{
		xs = -1;
		ys = -1;
	}
	else if (quadrant == 3)
	{
		xs = 1;
		ys = -1;
	}

	Vec2 p1 = Vec2{0, 0.5 * (a + b) * ys};
	Vec2 p2 = Vec2{0.5 * (a + b) * xs, 0.5 * (a + b) * ys};
	Vec2 p3 = Vec2{0.5 * (a + b) * xs, 0};

	Array<Vec2> pointsList = {0};
	pointsList.ensure_slots(3);

	pointsList.append(p1);
	pointsList.append(p2);
	pointsList.append(p3);

	this->path(result, pointsList, w, layerName);

	pointsList.clear();

	return 0;
}

int Gdslib::drawSegment(Polygon *&result, double r, double w, int quadrant, int geometry, string layerName)
{
	// 	# Parameters:
	// 	#            w 	: float
	// 	# 					segment width
	// 	#            r 	: float
	// 	#             		outer radius
	// 	# 		quadrant: integer
	//  #            		quadrant to draw segment (0 - 3)
	//  #				*****************
	//  #				*	1	*	0	*
	//  #				*****************
	//  #				*	2	*	3	*
	//  #				*****************
	//  #		geometry: integer
	//  #					0-> octagonal, else rectangular
	//  # 		layerName: string
	//  #					metal name from tech file
	//  ########################################################

	if (geometry == 0) // octagonal
	{

		octSegment(result, r, w, quadrant, layerName);
	}
	else // rectangular
	{
		rectSegment(result, r, w, quadrant, layerName);
	}

	return 0;
}

int Gdslib::fillVias(Array<Polygon *> &result, Vec2 p1, Vec2 p2, string topM, string bottomM)
{
	// Fills a given rectangle rect with vias.
	// Rectangle is defined as a tuple of xy coordinates - p1=(x1,y1), p2=(x2,y2)
	// Parameters:
	//     	p1 : Vec2
	//         (x1,y1) lower left
	//     	p2 : Vec2
	//         (x2,y2) upper right
	//
	// Returns a list of rectangles.
	//  ########################################################

	// result={0};

	LayerMet topMId = this->getMet(topM);
	LayerMet bottomMId = this->getMet(bottomM);
	LayerVia vId = this->getVia(topM, bottomM);

	double viaL = abs(p1.x - p2.x);
	double viaW = abs(p1.y - p2.y);

	double x0, dx, y0, dy;

	if ((viaL < vId.viaSize + 2 * vId.viaEnc) or (viaW < vId.viaSize + 2 * vId.viaEnc))
	{
		throw SoftError("Warning: <Gdslib::fillVias> : Invalid geometry, no enough space for one via!");
	}

	if (p2.x > p1.x)
	{
		x0 = p1.x;
		dx = p2.x - p1.x;
	}
	else
	{
		x0 = p2.x;
		dx = p1.x - p2.x;
	}

	if (p2.y > p1.y)
	{
		y0 = p1.y;
		dy = p2.y - p1.y;
	}
	else
	{
		y0 = p2.y;
		dy = p1.y - p2.y;
	}

	int nrows = int(floor((dy - 2 * vId.viaEnc + vId.viaSpace + 1e-9) / (vId.viaSize + vId.viaSpace)));
	int ncols = int(floor((dx - 2 * vId.viaEnc + vId.viaSpace + 1e-9) / (vId.viaSize + vId.viaSpace)));

	double r_off = (dy - 2 * vId.viaEnc + vId.viaSpace - nrows * (vId.viaSize + vId.viaSpace)) / 2;
	double c_off = (dx - 2 * vId.viaEnc + vId.viaSpace - ncols * (vId.viaSize + vId.viaSpace)) / 2;

	result.ensure_slots(nrows * ncols);

	Polygon *poly = NULL;

	for (int i = 0; i < nrows; i++)
	{
		for (int j = 0; j < ncols; j++)
		{

			Vec2 pp1 = Vec2{x0 + vId.viaEnc + c_off + j * (vId.viaSize + vId.viaSpace), y0 + vId.viaEnc + r_off + i * (vId.viaSize + vId.viaSpace)};
			Vec2 pp2 = Vec2{pp1.x + vId.viaSize, pp1.y + vId.viaSize};

			this->via(poly, pp1, pp2, topM, bottomM);
			result.append(poly);
		}
	}

	poly = NULL;
	this->rectangle(poly, p1, p2, topM);
	result.append(poly);

	poly = NULL;
	this->rectangle(poly, p1, p2, bottomM);
	result.append(poly);

	return 0;
}

int Gdslib::bridge45Vias(Array<Polygon *> &result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string bottomM, string topM)
// #####################################################################################################
// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
// # this two planes are connected with Bridge45
// #####################################################################################################
// # Access planes are in topM layer and bridge is made in topM and bottomM
// #####################################################################################################
// # 			p1
// #   	********				******************
// #      1*via*----------------*------4-----------------
// #	********  				******************		|
// #		    p2  									| e
// #           	               p3 						|
// #    ********                *******************		|
// #     	3  *<-------------->*via*	2	|w			----
// #	********       b       	*******************
// #                          p4
// #####################################################################################################
// # bridge45Vias will conect segments 1 to 2 with bottomM and 3 to 4 with layer topM
// # bottomM is below topM
// # via between bottomM and topM is placed on segments 1 and 2
// #####################################################################################################
// #####################################################################################################
// # 			p1
// #   ********
// #          * --------------------------------------
// #   ********  										|
// #		  |p2  										| e
// #          |                p3 						|
// #          |                *******************		|
// #          <--------------->* 		|w			----
// #			       b       *******************
// #                          p4
// #####################################################################################################
// #			*	*
// #			*	*
// #			*	*
// #		 p1	***** p2 ------------------
// #			  |						  |
// #              |    e 				  |
// #			  <-------->|			  | b
// #						|			  |
// #				   p3 ***** p4 --------
// #					  *	  *
// #					  *<->*
// #					  * w *
// #####################################################################################################

{

	double w = 0;
	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horisontal connection
	{
		w = abs(p1.y - p2.y);

		if (p1.x < p3.x)
		{
			this->fillVias(result, Vec2{p2.x - w, p2.y}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x + w, p3.y}, p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Vec2{p2.x + w, p2.y}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x - w, p3.y}, p4, topM, bottomM);
		}
	}

	else // vertical connection
	{
		w = abs(p1.x - p2.x);

		if (p1.y > p3.y)
		{
			this->fillVias(result, Vec2{p2.x, p2.y + w}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x, p3.y - w}, p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Vec2{p2.x, p2.y - w}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x, p3.y + w}, p4, topM, bottomM);
		}
	}

	Polygon *poly1 = NULL;
	Polygon *poly2 = NULL;

	this->bridge45x2(poly1, poly2, p1, p2, p3, p4, bottomM, topM);

	result.append(poly1);
	result.append(poly2);

	return 0;
}

int Gdslib::bridge45Vias2(Array<Polygon *> &result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string bottomM, string topM)
// #####################################################################################################
// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
// # this two planes are connected with Bridge45
// #####################################################################################################
// # Access planes are in bottomM layer and bridge is made in topM and bottomM
// #####################################################################################################
// # 			p1
// #   	********				******************
// #      1*via*----------------*------4-----------------
// #	********  				******************		|
// #		    p2  									| e
// #           	               p3 						|
// #    ********                *******************		|
// #     	3  *<-------------->*via*	2	|w			----
// #	********       b       	*******************
// #                          p4
// #####################################################################################################
// # bridge45Vias will conect segments 1 to 2 with bottomM and 3 to 4 with layer topM
// # bottomM is below topM
// # via between bottomM and topM is placed on segments 1 and 2
// #####################################################################################################
// #####################################################################################################
// # 			p1
// #   ********
// #          * --------------------------------------
// #   ********  										|
// #		  |p2  										| e
// #          |                p3 						|
// #          |                *******************		|
// #          <--------------->* 		|w			----
// #			       b       *******************
// #                          p4
// #####################################################################################################
// #			*	*
// #			*	*
// #			*	*
// #		 p1	***** p2 ------------------
// #			  |						  |
// #              |    e 				  |
// #			  <-------->|			  | b
// #						|			  |
// #				   p3 ***** p4 --------
// #					  *	  *
// #					  *<->*
// #					  * w *
// #####################################################################################################

{

	double w = 0;
	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horisontal connection
	{
		w = abs(p1.y - p2.y);

		if (p1.x < p3.x)
		{
			this->fillVias(result, Vec2{p2.x - w, p2.y}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x + w, p3.y}, p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Vec2{p2.x + w, p2.y}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x - w, p3.y}, p4, topM, bottomM);
		}
	}

	else // vertical connection
	{
		w = abs(p1.x - p2.x);

		if (p1.y > p3.y)
		{
			this->fillVias(result, Vec2{p2.x, p2.y + w}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x, p3.y - w}, p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Vec2{p2.x, p2.y - w}, p1, topM, bottomM);
			this->fillVias(result, Vec2{p3.x, p3.y + w}, p4, topM, bottomM);
		}
	}

	Polygon *poly1 = NULL;
	Polygon *poly2 = NULL;

	this->bridge45x2(poly1, poly2, p1, p2, p3, p4, topM, bottomM);

	result.append(poly1);
	result.append(poly2);

	return 0;
}

int Gdslib::patternedGroundShield(Array<Polygon *> &result, double d, double w, double s, int geometry, string layerName)
{
	// # result			:output, pointer on Polygon array
	// # d 				:diameter not radius
	// # w 				:width of metal lines
	// # s 				:width of hols
	// # geometry		:integer
	// #					0-> octagonal, 1 rectangular
	// # layerName		:metal name from technology

	result = {0};

	LayerMet metId = this->getMet(layerName);
	Array<Polygon *> ArrTmp = {0};
	Array<Polygon *> ArrTmp2 = {0};

	Polygon *segment = NULL;

	if (w <= 0)
	{
		w = 1 < 2 * metId.minW ? 2 * metId.minW : 1;
	}

	if (s <= 0)
	{
		s = 0.5 < 1.1 * metId.minS ? 1.1 * metId.minS : 0.5;
	}

	d = 2 * this->round2grid(0.5 * d);
	w = 2 * this->round2grid(0.5 * w);
	s = 2 * this->round2grid(0.5 * s);

	if (geometry == 1) // rect geometry
	{

		this->rectangle(segment, Vec2{-0.5 * d, 0.5 * d}, Vec2{0.5 * d, -0.5 * d}, layerName);

		double z = 0.5 * d - w;
		double x = 0.5 * s;
		double y = -w;
		Polygon *tmpPoly;

		// first iteration for middle hole

		//***************************************************************************
		// vertical hols
		//***************************************************************************

		// lower right
		this->rectangle(tmpPoly, Vec2{x, y}, Vec2{x - s, -d}, layerName);
		ArrTmp.append(tmpPoly);

		// upper right
		this->rectangle(tmpPoly, Vec2{x, -y}, Vec2{x - s, d}, layerName);
		ArrTmp.append(tmpPoly);

		//***************************************************************************
		// horizontal hols
		//***************************************************************************
		this->rectangle(tmpPoly, Vec2{y, x}, Vec2{-d, x - s}, layerName);
		ArrTmp.append(tmpPoly);

		this->rectangle(tmpPoly, Vec2{-y, x}, Vec2{d, x - s}, layerName);
		ArrTmp.append(tmpPoly);

		z = z - w - s;
		x = x + w + s;
		y = y - w - s;

		while (z > w)
		{

			//***************************************************************************
			// vertical hols
			//***************************************************************************

			// lower right
			this->rectangle(tmpPoly, Vec2{x, y}, Vec2{x - s, -d}, layerName);
			ArrTmp.append(tmpPoly);

			// upper right
			this->rectangle(tmpPoly, Vec2{x, -y}, Vec2{x - s, d}, layerName);
			ArrTmp.append(tmpPoly);

			// lower left
			this->rectangle(tmpPoly, Vec2{-x, y}, Vec2{-x + s, -d}, layerName);
			ArrTmp.append(tmpPoly);

			// upper left
			this->rectangle(tmpPoly, Vec2{-x, -y}, Vec2{-x + s, d}, layerName);
			ArrTmp.append(tmpPoly);

			//***************************************************************************
			// horizontal hols
			//***************************************************************************
			this->rectangle(tmpPoly, Vec2{y, x}, Vec2{-d, x - s}, layerName);
			ArrTmp.append(tmpPoly);

			this->rectangle(tmpPoly, Vec2{y, -x}, Vec2{-d, -x + s}, layerName);
			ArrTmp.append(tmpPoly);

			this->rectangle(tmpPoly, Vec2{-y, x}, Vec2{d, x - s}, layerName);
			ArrTmp.append(tmpPoly);

			this->rectangle(tmpPoly, Vec2{-y, -x}, Vec2{d, -x + s}, layerName);
			ArrTmp.append(tmpPoly);

			z = z - w - s;
			x = x + w + s;
			y = y - w - s;

		} // while

		ArrTmp2.append(segment);

		// result = ArrTmp2 - ArrTmp
		boolean(ArrTmp2, ArrTmp, Operation::Not, 1 / this->getGrid(), result);

		// free ArrTmp2 and ArrTmp

		// free memory ArrTmp2 and segment point to the same data
		// it is enough to call free only for segment
		ArrTmp2.clear();
		ArrTmp2 = {0};
		segment->clear();
		free(segment);
		segment = NULL;

		for (long unsigned int i = 0; i < ArrTmp.count; i++)
		{
			ArrTmp[i]->clear();
			free(ArrTmp[i]);
			ArrTmp[i] = NULL;
		}
		ArrTmp.clear();
		ArrTmp = {0};

		// Assign gds number and data type for result
		for (long unsigned int i = 0; i < result.count; i++)
		{
			this->round2grid(result[i]);
			result[i]->datatype = metId.gdsType;
			result[i]->layer = metId.gdsNum;
		}

	} // if rectangular

	else if (geometry == 0) // octagonal
	{
		//  ########################################################
		//  #
		//  #			k
		//  #		-------
		//  #		*******
		//  #				*	2k
		//  #				  *
		//  #					*
		//  #					  * |
		//  #					  *	|
		//  #			0.5*d 	  *	| k
		//  #		<------------>*	|
		//  #
		//	# 		k=(sqrt(2)-1)*0.5*d
		//  #
		//  ########################################################
		//  #
		//  #
		//  #
		//  #						      *								    *
		//  #							* |	*								  *
		//  #						  *	  |h  *									*
		//  #						*  | *** 	*				  *		0.5w  *90 *
		//  #					  *	   | * *	  *					*		* 		*
		//  #  					*	   | * *  ***<->*				  *   *	45/2	  *
		//  # 				  *		  z| * *  * * l	  * 				*<------------->*
		//  #				*		   | * *  * *       *				  *		l		  *
		//  #			  *			   | * *  * *         *  				*
		//  #			*			   | * *- *	*			*				  *
		//	# 		  *					 * *s *	*			  *
		//	#							 <->
		//	#							  w
		//	#															l=0.5w/cos(45/2)

		double b = this->k(d);

		b = 2 * this->round2grid(0.5 * b);

		Vec2 points[] = {{-0.5 * d, b}, {-b, 0.5 * d}, {b, 0.5 * d}, {0.5 * d, b}, {0.5 * d, -b}, {b, -0.5 * d}, {-b, -0.5 * d}, {-0.5 * d, -b}};

		segment = (Polygon *)allocate_clear(sizeof(Polygon));
		segment->point_array.extend({.count = COUNT(points), .items = points});

		this->round2grid(segment);

		double cos_45_over_2 = 0.92387953251; // cos(45/2)
		double tg_45_over_2 = 0.41421356237;  // tg(45/2)

		double l = (0.5 * w) / cos_45_over_2; // leave a bit space (it could be 0.5*w but 0.6*w is used to have more space between pica parcadi)
		l = this->round2grid(l);

		double h = (0.5 * s + l) / tg_45_over_2;
		h = this->round2grid(h);

		double z = 0.5 * d - h;
		double x = -0.5 * s;
		double y = z - 0.5 * d;

		// midle path is done here since it is common for right and left segments

		Polygon *rect = NULL;
		this->rectangle(rect, Vec2{x, -w}, Vec2{x + s, -0.5 * d - w}, layerName);
		ArrTmp.append(rect);

		// array for 45 deg
		Array<Polygon *> ArrTmp45 = {0};
		this->rectangle(rect, Vec2{x, y}, Vec2{x + s, -0.5 * d + w}, layerName);
		ArrTmp45.append(rect);

		z = z - (s + w) / tg_45_over_2;
		z = this->round2grid(z);

		x = x + s + w;
		y = z - 0.5 * d;

		while (z > w)
		{
			//***************************************************************************
			// vertical hols
			//***************************************************************************

			// lover right
			this->rectangle(rect, Vec2{x, y}, Vec2{x + s, -0.5 * d}, layerName);
			ArrTmp.append(rect);

			// lover left
			this->rectangle(rect, Vec2{-x, y}, Vec2{-x - s, -0.5 * d}, layerName);
			ArrTmp.append(rect);

			// 45 deg, must avoid holes shorter than w
			if (z > 2 * w)
			{

				// lover right
				this->rectangle(rect, Vec2{x, y}, Vec2{x + s, -0.5 * d + w}, layerName);
				ArrTmp45.append(rect);

				// lover left
				this->rectangle(rect, Vec2{-x, y}, Vec2{-x - s, -0.5 * d + w}, layerName);
				ArrTmp45.append(rect);
			}

			z = z - (s + w) / tg_45_over_2;
			z = this->round2grid(z);

			x = x + s + w;
			y = z - 0.5 * d;
		}

		// this->round2grid(segment);
		// result.append(segment);

		for (long unsigned int i = 0; i < ArrTmp.count; i++)
		{
			double pi = 3.14159265358979323846264338327950288419;
			// add first

			ArrTmp2.append(ArrTmp[i]);

			// 90 deg rotation
			// right side

			Polygon *tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp[i]);
			tmpSegment->rotate(pi / 2, Vec2{0, 0});

			this->round2grid(tmpSegment);

			ArrTmp2.append(tmpSegment);

			// 180 deg rotation

			tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp[i]);
			tmpSegment->rotate(pi, Vec2{0, 0});

			this->round2grid(tmpSegment);

			ArrTmp2.append(tmpSegment);

			// (6/4)*180 deg rotation
			// left side

			tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp[i]);
			tmpSegment->rotate(6 * pi / 4, Vec2{0, 0});

			this->round2grid(tmpSegment);

			ArrTmp2.append(tmpSegment);

		} // for

		// no free since the same data is in ArrTmp2
		ArrTmp.clear();

		for (long unsigned int i = 0; i < ArrTmp45.count; i++)
		{

			double pi = 3.14159265358979323846264338327950288419;

			// 45 deg rotation

			Polygon *tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp45[i]);
			tmpSegment->rotate(pi / 4, Vec2{0, 0});

			this->round2grid45(tmpSegment);

			ArrTmp2.append(tmpSegment);

			// (3/4)*180 deg rotation

			tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp45[i]);
			tmpSegment->rotate(3 * pi / 4, Vec2{0, 0});

			this->round2grid45(tmpSegment);

			ArrTmp2.append(tmpSegment);

			// (5/4)*180 deg rotation

			tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp45[i]);
			tmpSegment->rotate(5 * pi / 4, Vec2{0, 0});

			this->round2grid45(tmpSegment);

			ArrTmp2.append(tmpSegment);

			// (7/4)*180 deg rotation

			tmpSegment = (Polygon *)allocate_clear(sizeof(Polygon));
			*tmpSegment = {0};

			tmpSegment->copy_from(*ArrTmp45[i]);
			tmpSegment->rotate(7 * pi / 4, Vec2{0, 0});

			this->round2grid45(tmpSegment);

			ArrTmp2.append(tmpSegment);

		} // for

		// no free since the same data is in ArrTmp2
		ArrTmp45.clear();
		ArrTmp.append(segment);

		boolean(ArrTmp, ArrTmp2, Operation::Not, 1 / this->getGrid(), result);

		// this->round2grid(result);

		ArrTmp.clear();

		// free memory
		segment->clear();
		free(segment);

		for (long unsigned int i = 0; i < ArrTmp2.count; i++)
		{
			ArrTmp2[i]->clear();
			free(ArrTmp2[i]);
			ArrTmp2[i] = NULL;
		}
		ArrTmp2.clear();

		for (long unsigned int i = 0; i < result.count; i++)
		{
			// this->round2grid(result[i]);
			result[i]->datatype = metId.gdsType;
			result[i]->layer = metId.gdsNum;
		}

	} // else

	// contact for ground shield

	string upperLayerName = this->t.getMetNamePP(layerName);

	this->fillVias(result, Vec2{-w, -w}, Vec2{w, w}, upperLayerName, layerName);
	this->rectangle(segment, Vec2{-0.5 * d, -w}, Vec2{w, w}, upperLayerName);
	result.append(segment);

	return 0;
}

int Gdslib::surroundingGround(Cell &result, std::vector<string> &metal, double w, double sl, double sr, double r)
{
	// # result			:Cell in gds file
	// # metal 			:array of metal names, metals must start from bottom to top metal layer
	// # w 				:width of metal lines
	// # sl 			:space between ground for left pins
	// # sr 			:space between ground for right pins
	// # r 				:inner diameter of surrounding ground

	Vec2 p1r = Vec2{0, 0.5 * sr};
	Vec2 p2r = Vec2{r, -0.5 * sr};
	Vec2 p1l = Vec2{0, 0.5 * sl};
	Vec2 p2l = Vec2{-r, -0.5 * sl};

	Polygon *path0, *path1, *path2, *path3;

	Array<Polygon *> path1Arr = {0};
	Array<Polygon *> path2Arr = {0};

	string metNamePrev = "";

	for (long unsigned int i = 0; i < metal.size(); i++)
	{
		// ###############################################################################################
		// # 1. segment
		// ###############################################################################################

		//  # Parameters:
		//  #            w  : float
		//  #           segment width
		//  #            r  : float
		//  #                 outer radius
		//  #     quadrant: integer
		//  #               quadrant to draw segment (0 - 3)
		//  #       *****************
		//  #       * 1 * 0 *
		//  #       *****************
		//  #       * 2 * 3 *
		//  #       *****************
		//  #   geometry: integer
		//  #         0-> octagonal, else rectangular
		//  #     layerName: string
		//  #         metal name from tech file
		//  ########################################################

		// ###############################################################################################
		// # right side 0 and 3
		// ###############################################################################################

		if (sr <= 0)
		{

			this->rectSegment(path0, r + 2 * w, w, 0, metal[i]);
			this->rectSegment(path3, r + 2 * w, w, 3, metal[i]);

			result.polygon_array.append(path0);
			result.polygon_array.append(path3);
		}
		else
		{

			this->rectSegment(path0, r + 2 * w, w, 0, metal[i]);
			this->rectSegment(path3, r + 2 * w, w, 3, metal[i]);

			this->crop(path1Arr, path0, p1r, p2r, metal[i]);
			this->crop(path2Arr, path3, p1r, p2r, metal[i]);

			result.polygon_array.append(path1Arr[0]);
			result.polygon_array.append(path2Arr[0]);

			path1Arr.clear();
			path2Arr.clear();

			path1Arr = {0};
			path2Arr = {0};
		}

		// ###############################################################################################
		// # left side 1 and 2
		// ###############################################################################################

		if (sl <= 0)
		{

			this->rectSegment(path1, r + 2 * w, w, 1, metal[i]);
			this->rectSegment(path2, r + 2 * w, w, 2, metal[i]);

			result.polygon_array.append(path1);
			result.polygon_array.append(path2);
		}
		else
		{

			this->rectSegment(path1, r + 2 * w, w, 1, metal[i]);
			this->rectSegment(path2, r + 2 * w, w, 2, metal[i]);

			this->crop(path1Arr, path1, p1l, p2l, metal[i]);
			this->crop(path2Arr, path2, p1l, p2l, metal[i]);

			result.polygon_array.append(path1Arr[0]);
			result.polygon_array.append(path2Arr[0]);

			path1Arr.clear();
			path2Arr.clear();

			path1Arr = {0};
			path2Arr = {0};
		}

		// ###############################################################################################
		// # add vias
		// ###############################################################################################

		if (metNamePrev != "")
		{

			if (sl <= 0)
			{
				// left segment
				this->fillVias(path1Arr, Vec2{-0.5 * r, -0.5 * r - w}, Vec2{-0.5 * r - w, 0.5 * r + w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};

			} // if
			else
			{
				// upper left segment
				this->fillVias(path1Arr, Vec2{-0.5 * r, 0.5 * sl}, Vec2{-0.5 * r - w, 0.5 * r + w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};

				// lover left segment
				this->fillVias(path1Arr, Vec2{-0.5 * r, -0.5 * sl}, Vec2{-0.5 * r - w, -0.5 * r - w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};

			} // else

			if (sr <= 0)
			{
				// right segment
				this->fillVias(path1Arr, Vec2{0.5 * r, -0.5 * r - w}, Vec2{0.5 * r + w, 0.5 * r + w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};
			}
			else
			{
				// upper right segment
				this->fillVias(path1Arr, Vec2{0.5 * r, 0.5 * sr}, Vec2{0.5 * r + w, 0.5 * r + w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};

				// lover right segment
				this->fillVias(path1Arr, Vec2{0.5 * r, -0.5 * sl}, Vec2{0.5 * r + w, -0.5 * r - w}, metal[i], metNamePrev);
				for (long unsigned int j = 0; j < path1Arr.count; j++)
				{
					result.polygon_array.append(path1Arr[j]);
				}

				path1Arr.clear();
				path1Arr = {0};
			}

			// upper segment
			LayerVia viaTmp = this->getVia(metal[i], metNamePrev);

			this->fillVias(path1Arr, Vec2{-0.5 * r + viaTmp.viaSpace, 0.5 * r}, Vec2{0.5 * r - viaTmp.viaSpace, 0.5 * r + w}, metal[i], metNamePrev);
			for (long unsigned int j = 0; j < path1Arr.count; j++)
			{
				result.polygon_array.append(path1Arr[j]);
			}

			path1Arr.clear();
			path1Arr = {0};

			// lower segment
			this->fillVias(path1Arr, Vec2{-0.5 * r + viaTmp.viaSpace, -0.5 * r}, Vec2{0.5 * r - viaTmp.viaSpace, -0.5 * r - w}, metal[i], metNamePrev);
			for (long unsigned int j = 0; j < path1Arr.count; j++)
			{
				result.polygon_array.append(path1Arr[j]);
			}

			path1Arr.clear();
			path1Arr = {0};

		} // if

		metNamePrev = metal[i];

	} // for

	return 0;
}

int Gdslib::label(Label *&result, string labelName, Vec2 point, string layerName)
{

	result = (Label *)allocate_clear(1 * sizeof(Label));
	*result = {0};

	LayerMet layerNameId = this->getMet(layerName);

	Label label = {
		.layer = unsigned(layerNameId.gdsNum),
		.texttype = unsigned(layerNameId.gdsType),
		.text = &labelName[0],
		.origin = point,
		.magnification = 1,
	};

	result->copy_from(label);

	return 0;
}

int Gdslib::merge(Polygon *&result, Polygon *&poly1, Polygon *&poly2, uint32_t layer, uint32_t datatype)
{
	// ##############################################################################################################
	// # merge two polygons poly1 and poly2
	// ##############################################################################################################
	// # if mearge is not possible return 1
	// # if mearge is possible  return 0
	// ##############################################################################################################
	// # result 		:merged polygon, it is not required to allocate memory for this pointer
	// # poly1,2 		:Polygon, Path or Rectangle
	// #				:it is NEVER deleted from memory
	// # layerName		:metal name from tech file
	// ##############################################################################################################
	if (poly1 == NULL or poly2 == NULL)
	{
		return 1;
	}

	Vec2 min1 = {0};
	Vec2 min2 = {0};
	Vec2 max1 = {0};
	Vec2 max2 = {0};

	poly1->bounding_box(min1, max1);
	poly2->bounding_box(min2, max2);

	// ##############################################################################################################
	// they don't overlap
	// ##############################################################################################################

	if (min1.x > max2.x or min1.y > max2.y)
	{
		return 1;
	}

	if (min2.x > max1.x or min2.y > max1.y)
	{
		return 1;
	}

	// ##############################################################################################################
	// they maybe overlap
	// ##############################################################################################################

	Array<Polygon *> array_poly1 = {0};
	Array<Polygon *> array_poly2 = {0};
	Array<Polygon *> array_result = {0};

	array_poly1.append(poly1);
	array_poly2.append(poly2);

	boolean(array_poly1, array_poly2, Operation::Or, 1 / this->getGrid(), array_result);

	if (array_result.count != 1) // they don't overlap
	{

		array_poly1.clear();
		array_poly2.clear();
		array_result.clear();
		return 1;
	}
	else
	{

		result = array_result[0];
		array_poly1.clear();
		array_poly2.clear();
		array_result.clear();
	}

	// layer
	result->layer = layer;
	result->datatype = datatype;

	return 0;
}

int Gdslib::merge(Cell &result)
{

	uint64_t i = 0;
	int flag1 = 0;
	while (i < result.polygon_array.count - 1)
	{

		for (uint64_t j = i + 1; j < result.polygon_array.count; j++)
		{

			// std::cout<<"i="<<i<<std::endl;
			// std::cout<<"j="<<j<<std::endl;
			// std::cout<<"n="<<result.polygon_array.count<<std::endl;

			if ((result.polygon_array[i]->layer == result.polygon_array[j]->layer) and (result.polygon_array[i]->datatype == result.polygon_array[j]->datatype))
			{

				Polygon *poly = NULL;

				int flag2 = this->merge(poly, result.polygon_array[i], result.polygon_array[j], result.polygon_array[i]->layer, result.polygon_array[j]->datatype);

				if (flag2 == 0)
				{ // meging

					free_allocation(result.polygon_array[i]);
					result.polygon_array[i] = NULL;
					free_allocation(result.polygon_array[j]);
					result.polygon_array[j] = NULL;

					// you must remove first j since j>i
					// if i is removed first j=j-1
					result.polygon_array.remove_unordered(j);
					result.polygon_array.remove_unordered(i);

					result.polygon_array.append(poly);
					i = 0;
					flag1 = 1; // merging
					// std::cout<<"removing..."<<std::endl;
					// std::cout<<"i="<<i<<std::endl;
					// std::cout<<"j="<<j<<std::endl;
					break;

				} // if
			} // if
		} // for

		if (flag1 != 0)
		{
			flag1 = 0;
		}
		else
		{
			i++;
		}

	} // while

	return 0;
}

int Gdslib::inside(gdstk::Array<bool> &result, gdstk::Array<gdstk::Vec2> &point_array, Polygon *&polygon)
{

	gdstk::Polygon *points = new gdstk::Polygon;
	points->point_array = point_array;
	gdstk::Array<gdstk::Polygon *> group = {0};
	group.append(points);

	gdstk::Array<gdstk::Polygon *> polygon_array = {0};
	polygon_array.append(polygon);

	gdstk::inside(group, polygon_array, gdstk::ShortCircuit::None, 1 / this->getGrid(), result);

	// free memory
	if (points != NULL)
	{
		delete points;
	}

	group.clear();
	polygon_array.clear();

	return 0;
}