// gdsLib.h

#ifndef _GDSLIB_H_
#define _GDSLIB_H_

#include <string>
#include <vector>
#include "mcell_technology.h"
#include "gdstk.h"
#include <math.h>

using namespace std;
using namespace gdstk;

class Gdslib
{

public:
	Tech t;

	Gdslib(const char *fileName) : t(fileName)
	{
	}
	~Gdslib()
	{
		this->clear();
	}
	int clear()
	{
		this->t.clear();
		return 0;
	}

	bool isEqual(double x, double y);

	bool isEqual(Vec2 &a, Vec2 &b);

	bool is45deg(Vec2 &a, Vec2 &b);
	// check if vectors are on n*45 deg shift
	// if not drc is not clear non45 deg error

	bool isNotOnGreed(Vec2 &a);
	// if on greed false else true

	Vec2 round2grid(Vec2 a);

	int round2grid(Vec2 &result, Vec2 &prev);
	// # round to grid so that previous point prev is 45 or 90 deg
	// # result=(x,y)	:point to be rounded
	// # prev=(x,y)		:previous point
	// # return 0 if rounded successfully

	int round2grid45(Vec2 &result, Vec2 &prev);
	// # used to round for patterned ground shield
	// # round to grid so that previous point prev is 45 or 90 deg
	// # result=(x,y)	:point to be rounded
	// # prev=(x,y)		:previous point
	// # return 0 if rounded successfully

	int round2grid45(Vec2 &result, Vec2 &prev, Vec2 &next);
	// used only for last point
	// last point should be 45deg or 90 deg with prev and first point(next)

	double round2grid(double a);

	int round2grid(Polygon *&result);

	int round2grid45(Polygon *&result);

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

	int appendArrToCell(Cell &cell, Array<Polygon *> &arry);

	LayerVia getVia(string ttopMet, string bbotMet);

	LayerMet getMet(string nname);

	double getGrid();

	vector<char *> splitStr(char *str, char *delimiter);

	int rectangle(Polygon *&result, Vec2 p1, Vec2 p2, string layerName);
	// # result			:output, pointer on Polygon
	// # p1=(x,y)		:uper right corner
	// # p2=(x,y)		:lower left corner
	// # layerName		:metal name from technology

	int via(Polygon *&result, Vec2 p1, Vec2 p2, string topM, string bottomM);
	// # result			:output, pointer on Polygon
	// # p1=(x,y)		:uper right corner
	// # p2=(x,y)		:lower left corner
	// # topM			:top metal name
	// # bottomM		:bottom metal name

	int path(Polygon *&result, Array<Vec2> &pointList, double w, string layerName);

	// # path converts path to poligon ant foce to grid
	// # result		   : output, pointer on Polygon
	// # pointsList    : array of points [(0, 0), (3, 0), (3, 2), (5, 3), (3, 4), (0, 4)]
	//				   : array is delated from memory in this function
	// # w             : width
	// # layerName     :metal name from tech file

	int crop(Array<Polygon *> &result, Polygon *&poly, Vec2 p1, Vec2 p2, string layerName);
	// #######################################################
	// # crop polygon with rectangle (p1,p2)
	// #######################################################
	// # crop vraca listu poligona jer dva dela koji nisu spojeni su dva poligona
	// #######################################################
	// #
	// #		*********			**********************
	// #		* poly1	*			*	poly2			 *
	// #		*********			**********************
	// #
	// ########################################################
	// # ako se krop koristi na delu od 45deg onda treba proci kroz listu poligona
	// # i obezbediti da svaki poligon bude on grid
	// #######################################################
	// # result 		:Output, array of poligons
	// # poly 			:Polygon, Path or Rectangle
	// #				:it is deleted from memory poly=NULL
	// # p1=(x,y)		:uper left corner
	// # p2=(x,y)		:lower right corner
	// # layerName		:metal name from tech file
	// #######################################################

	int bridge45(Polygon *&result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string layerName);
	// #####################################################################################################
	// # with p1 and p2 is defined one acess plane and with p3 and p4 second access plane
	// # thise two planes are connected with Bridge45
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

	int bridge45x2(Polygon *&result1, Polygon *&result2, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string layerName1, string layerName2);
	// #####################################################################################################
	// # with p1 and p2 is defined one acess plane and with p3 and p4 second access plane
	// # thise two planes are connected with Bridge45
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
	// # memory is assigned for result1 and result2 inside of this function
	// #####################################################################################################

	int octSegment(Polygon *&result, double r, double w, int quadrant, string layerName);
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

	int rectSegment(Polygon *&result, double r, double w, int quadrant, string layerName);
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
	//  ########################################################

	int drawSegment(Polygon *&result, double r, double w, int quadrant, int geomerty, string layerName);
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
	//  #		geomerty: integer
	//  #					0-> octogonal, else rectangular
	//  # 		layerName: string
	//  #					metal name from tech file
	//  ########################################################

	int fillVias(Array<Polygon *> &result, Vec2 p1, Vec2 p2, string topM, string bottomM);

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

	int bridge45Vias(Array<Polygon *> &result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string bottomM, string topM);
	// #####################################################################################################
	// # with p1 and p2 is defined one acess plane and with p3 and p4 second access plane
	// # thise two planes are connected with Bridge45
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
	// # the schortest posible
	// #####################################################################################################

	int bridge45Vias2(Array<Polygon *> &result, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, string bottomM, string topM);
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
	// # the schortest posible
	// #####################################################################################################

	int patternedGroundShield(Array<Polygon *> &result, double r, double w, double s, int geomerty, string layerName);
	// # result			:output, pointer on Polygon array
	// # r 				:diameter
	// # w 				:width of metal lines
	// # s 				:width of hols
	// # geomerty		:integer
	// #					0-> octogonal, else rectangular
	// # layerName		:metal name from technology

	int surroundingGround(Cell &result, std::vector<string> &metal, double w, double sl, double sr, double r);
	// # result			:Cell in gds file
	// # metal 			:array of metal names, metals must start from bottom to top metal layer
	// # w 				:width of metal lines
	// # sl 			:space between ground for left pins
	// # sr 			:space between ground for right pins
	// # r 				:inner diameter of surrounding ground

	int label(Label *&result, string labelName, Vec2 point, string layerName);

	int merge(Polygon *&result, Polygon *&poly1, Polygon *&poly2, uint32_t layer, uint32_t datatype);
	// #######################################################
	// # merge two polygons poly1 and poly2
	// #######################################################
	// # if meage is not possible result=NULL and return 0
	// # if meage is possible  return 1
	// #######################################################
	// # result 		:merged polygon
	// # poly1,2 		:Polygon, Path or Rectangle
	// #				:it is NEVER deleted from memory
	// # layerName		:metal name from tech file
	// #######################################################

	int merge(Cell &result);
	// #######################################################
	// # merge all in cell
	// #######################################################

	int inside(gdstk::Array<bool> &result, gdstk::Array<gdstk::Vec2> &point_array, Polygon *&polygon);
	// #######################################################
	// # check if points are inside polygon
	// #######################################################
	// # result			: if point point_array[i] is in polygon
	//					: result[i]=true, else result[i]=false
	// #######################################################
};

#endif
