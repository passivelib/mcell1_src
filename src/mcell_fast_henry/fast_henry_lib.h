#ifndef _FASTHENRYLIB_H_
#define _FASTHENRYLIB_H_

#include <vector>
#include <string>
#include "mcell_technology.h"
#include "mcell_error.h"

// ###############################################################################################
// if abs(a-b)<FastHenryEpsilon => a==b
// ###############################################################################################
#define FastHenryEpsilon 1e-6

// ###############################################################################################
// # Point2 class
// ###############################################################################################

class Point2
{
public:
	double x;
	double y;

	int clear();

	Point2(double xx, double yy) : x(xx), y(yy) {}
	~Point2() { this->clear(); }
	Point2() : x(0), y(0) {}
	// Copy constructor
	Point2(const Point2 &a) : x(a.x), y(a.y) {}
	// assignmet operator
	Point2 &operator=(const Point2 &copy);

	void print();

}; // Point3

// ###############################################################################################
// # Point3 class
// ###############################################################################################

class Point3
{
public:
	double x;
	double y;
	double z;

	int clear();

	Point3(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
	~Point3() { this->clear(); }
	Point3() : x(0), y(0), z(0) {}
	// Copy constructor
	Point3(const Point3 &a) : x(a.x), y(a.y), z(a.z) {}
	// assignmet operator
	Point3 &operator=(const Point3 &copy);

	double distance(const Point3 &a);

	void print();

}; // Point3

// ###############################################################################################
// # Segment class
// ###############################################################################################

class Segment
{

public:
	Point3 start;
	Point3 end;
	// width
	double w;
	// hight
	double h;
	// conductivity S/m
	double cond;

	int clear();

	~Segment() { this->clear(); }
	Segment(const Segment &copy) : start(copy.start), end(copy.end), w(copy.w), h(copy.h), cond(copy.cond) {}
	Segment()
	{
		this->clear();
	}
	Segment(Point3 sstart, Point3 eend, double ww = 0, double hh = 0, double ccond = 0) : start(sstart), end(eend), w(ww), h(hh), cond(ccond) {}
	// assignmet operator
	Segment &operator=(const Segment &copy);

	void print();

}; // Segment

// ###############################################################################################
// # FastHenryLibs class
// ###############################################################################################
// Set of functions to be used in FastHenry interface

class FastHenryLib
{

public:
	Tech t;

	FastHenryLib(const char *fileName) : t(fileName)
	{
	}
	FastHenryLib()
	{
	}
	~FastHenryLib()
	{
		this->clear();
	}
	int clear()
	{
		this->t.clear();
		return 0;
	}

	bool isEqual(double x, double y);

	bool isEqual(Point2 &a, Point2 &b);

	bool isEqual(Point3 &a, Point3 &b);

	int path(std::vector<Segment> &out, std::vector<Point2> &points, double width, std::string metalName);

	int rectangle(std::vector<Segment> &out, Point2 p1, Point2 p2, int direction, std::string metalName);
	//*********************************************************************************************************************
	// direction 0->horizontal ; 1->vertical
	//*********************************************************************************************************************

	int fillVias(std::vector<Segment> &out, Point2 lowerLeft, Point2 upperRight, std::string topMetal, std::string bottomMetal);

	int cropSegment(std::vector<Segment> &out, Segment A, Point2 lowerLeft, Point2 upperRight, std::string metalName);
	//*********************************************************************************************************************
	// Cut segment A with rectangle lowerLeft upperRight.
	// Array out is not clean in the begining so you can use function to add to existing array.
	// If segment can not be cut, it will be added to array out
	//*********************************************************************************************************************
	int crop(std::vector<Segment> &out, std::vector<Segment> &in, Point2 lowerLeft, Point2 upperRight, std::string metalName);
	//*********************************************************************************************************************
	// Cut segment array in with rectangle lowerLeft upperRight.
	// Array out is not clean in the begining so you can use function to add to existing array.
	// If segment can not be cut, it will be added to array out
	//*********************************************************************************************************************

	bool counterClockWise(Point2 A, Point2 B, Point2 C);
	//*********************************************************************************************************************
	// Check if points A-B-C are counterclockwise  oriented
	//*********************************************************************************************************************

	bool cross(Point2 A, Point2 B, Point2 C, Point2 D);
	//*********************************************************************************************************************
	// check if A-B cross C-D
	// returns true if A-B cross C-D and false if A-B doesn't cross C-D
	//*********************************************************************************************************************

	bool ConAB(Point2 A, Point2 B, Point2 C);
	//*********************************************************************************************************************
	// check if C lies on A-B
	//*********************************************************************************************************************

	bool cross(std::vector<Point2> &poligonA, std::vector<Point2> &poligonB);
	//*********************************************************************************************************************
	// check if poligonA and poligonB crosses (poligon is closed with array of points)
	//*********************************************************************************************************************

	int segment2Polygon(std::vector<Point2> &out, const Segment &A);
	//*********************************************************************************************************************
	// convert segment to polygon in xy plane
	//*********************************************************************************************************************

	bool cross(const Segment &A, const Segment &B);
	//*********************************************************************************************************************
	// check if A and B crosses
	//*********************************************************************************************************************

	double k(double d);
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

	int bridge45(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string layerName);
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # this two planes are connected with Bridge45
	// #####################################################################################################
	// # 			p1
	// #   ********
	// #          * --------------------------------------
	// #   ********  										|
	// #		   |p2  									| e
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

	int bridge45x2(std::vector<Segment> &result1, std::vector<Segment> &result2, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string layerName1, string layerName2);
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # this two planes are connected with Bridge45
	// #####################################################################################################
	// # 			p1
	// #   ********				    ******************
	// #      1   * ----------------*--4--------------------
	// #	********  				******************		|
	// #		    |p2  									| e
	// #           |               p3 						|
	// #   ********|               *******************		|
	// #       3  *<-------------->* 	2	|w			----
	// #	********       b       	*******************
	// #                          p4
	// #####################################################################################################
	// # Bridge45x2 will conect segments 1 to 2 with layerName1 and 3 to 4 with layer layerName2
	// # result1 and result2 are not clean in this function
	// #####################################################################################################

	int bridge45Vias(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string bottomM, string topM);
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # this two planes are connected with Bridge45
	// #####################################################################################################
	// # Access planes are in topM layer and bridge is made in topM and bottomM
	// #####################################################################################################
	// # 			p1
	// #   	********				******************
	// #      1    *----------------*via*-4-----------------
	// #	********  				******************		|
	// #		    p2  									| e
	// #           	               p3 						|
	// #    ********                *******************		|
	// #     3 *via*<-------------->* 	2	|w			----
	// #	********       b       	*******************
	// #                          p4
	// #####################################################################################################
	// # bridge45Vias will conect segments 1 to 2 with layerName1 and 3 to 4 with layer layerName2
	// # layerName2 is below layerName1
	// # via between layerName1 and layerName2 is placed on segments 3 and 4
	// # all segments are extended in layerName1 maximal to each other os that metal line in layerName2 is
	// # the schortest possible
	// #####################################################################################################

	int bridge45Vias2(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string bottomM, string topM);
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # this two planes are connected with Bridge45
	// #####################################################################################################
	// # Access planes are in bottomM layer and bridge is made in topM and bottomM
	// #####################################################################################################
	// # 			p1
	// #   	********				******************
	// #      1    *----------------*via*-4-----------------
	// #	********  				******************		|
	// #		    p2  									| e
	// #           	               p3 						|
	// #    ********                *******************		|
	// #     3 *via*<-------------->* 	2	|w			----
	// #	********       b       	*******************
	// #                          p4
	// #####################################################################################################
	// # bridge45Vias will conect segments 1 to 2 with layerName1 and 3 to 4 with layer layerName2
	// # layerName2 is below layerName1
	// # via between layerName1 and layerName2 is placed on segments 3 and 4
	// # all segments are extended in layerName1 maximal to each other os that metal line in layerName2 is
	// # the schortest possible
	// #####################################################################################################

	int octSegment(std::vector<Segment> &result, double d, double w, int quadrant, string layerName);
	// 	# Parameters:
	// 	#            w 	: float
	// 	# 					segment width
	// 	#            d 	: float
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
	//  #			0.5*d 	  *	| b
	//  #		<------------>*	|
	//  #
	//	# 		b=(sqrt(2)-1)*0.5*d

	int rectSegment(std::vector<Segment> &result, double d, double w, int quadrant, string layerName);
	// 	# Parameters:
	// 	#            w 	: float
	// 	# 					segment width
	// 	#            d 	: float
	// 	#             		outer diameter
	// 	# 		quadrant: integer
	//  #            		quadrant to draw segment (0 - 3)
	//  #				*****************
	//  #				*	1	*	0	*
	//  #				*****************
	//  #				*	2	*	3	*
	//  #				*****************
	//  ########################################################

	int drawSegment(std::vector<Segment> &result, double d, double w, int quadrant, int geometry, string layerName);
	// 	# Parameters:
	// 	#            w 	: float
	// 	# 					segment width
	// 	#            d 	: float
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

	int translate(std::vector<Segment> &result, const Point2 &shift);
	// translate result for shift vector
};

#endif