#include "fast_henry_lib.h"
#include <complex>
#include <iostream>

int Point2::clear()
{
	this->x = 0;
	this->y = 0;

	return 0;
}

Point2 &Point2::operator=(const Point2 &copy)
{
	this->x = copy.x;
	this->y = copy.y;

	return *this;
}

void Point2::print()
{
	std::cout << "x=" << this->x << " y=" << this->y << std::endl;
}

int Point3::clear()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;

	return 0;
}

Point3 &Point3::operator=(const Point3 &copy)
{
	this->x = copy.x;
	this->y = copy.y;
	this->z = copy.z;

	return *this;
}

double Point3::distance(const Point3 &a)
{
	return std::sqrt((this->x - a.x) * (this->x - a.x) + (this->y - a.y) * (this->y - a.y) + (this->z - a.z) * (this->z - a.z));
}

void Point3::print()
{
	std::cout << "x=" << this->x << " y=" << this->y << " z=" << this->z << std::endl;
}

int Segment::clear()
{
	this->start.clear();
	this->end.clear();
	this->w = 0;
	this->h = 0;
	this->cond = 0;

	return 0;
}

Segment &Segment::operator=(const Segment &copy)
{
	if (this == &copy)
	{
		return *this;
	}

	this->start = copy.start;
	this->end = copy.end;
	this->w = copy.w;
	this->h = copy.h;
	this->cond = copy.cond;

	return *this;
}

void Segment::print()
{
	std::cout << "start: ";
	this->start.print();
	std::cout << "end: ";
	this->end.print();
}

bool FastHenryLib::isEqual(double x, double y)
{
	double grid = t.getGrid();
	return std::abs(x - y) < 0.01 * grid ? true : false;
}

bool FastHenryLib::isEqual(Point2 &a, Point2 &b)
{
	return (this->isEqual(a.x, b.x) and this->isEqual(a.y, b.y));
}

bool FastHenryLib::isEqual(Point3 &a, Point3 &b)
{
	return (this->isEqual(a.x, b.x) and this->isEqual(a.y, b.y) and this->isEqual(a.z, b.z));
}

int FastHenryLib::path(std::vector<Segment> &out, std::vector<Point2> &points, double width, std::string metalName)
{
	LayerMet metalId = this->t.getMet(metalName);
	double z = metalId.dSub + 0.5 * metalId.metT;
	double sigma = metalId.cond;

	out.clear();
	out.reserve(points.size() - 1);

	for (long unsigned int i = 0; i < points.size() - 1; i++)
	{
		Segment tmp;
		tmp.start.x = points[i].x;
		tmp.start.y = points[i].y;
		tmp.start.z = z;

		tmp.end.x = points[i + 1].x;
		tmp.end.y = points[i + 1].y;
		tmp.end.z = z;

		tmp.w = width;
		tmp.h = metalId.metT;

		tmp.cond = sigma;

		out.push_back(tmp);
	}

	return 0;
}

int FastHenryLib::fillVias(std::vector<Segment> &out, Point2 lowerLeft, Point2 upperRight, std::string topMetal, std::string bottomMetal)
{
	LayerMet topMetalId = this->t.getMet(topMetal);
	LayerMet bottomMetalId = this->t.getMet(bottomMetal);
	LayerVia vId = this->t.getVia(topMetal, bottomMetal);

	double x = 0.5 * (lowerLeft.x + upperRight.x);
	double y = 0.5 * (lowerLeft.y + upperRight.y);

	double z1 = topMetalId.dSub + 0.5 * topMetalId.metT;
	double z2 = bottomMetalId.dSub + 0.5 * bottomMetalId.metT;

	Segment seg;
	seg.start.x = x;
	seg.start.y = y;
	seg.start.z = z1;

	seg.end.x = x;
	seg.end.y = y;
	seg.end.z = z2;

	seg.w = std::abs(upperRight.x - lowerLeft.x);
	seg.h = std::abs(upperRight.y - lowerLeft.y);

	// scale conductivity to get correct resistivity at DC

	double viaL = std::abs(lowerLeft.x - upperRight.x);
	double viaW = std::abs(lowerLeft.y - upperRight.y);

	if ((viaL < vId.viaSize + 2 * vId.viaEnc) or (viaW < vId.viaSize + 2 * vId.viaEnc))
	{
		throw SoftError("Warning: <Gdslib::fillVias> : Invalid geometry, no enough space for one via!");
	}

	int nrows = int(floor((viaW - 2 * vId.viaEnc + vId.viaSpace + 1e-9) / (vId.viaSize + vId.viaSpace)));
	int ncols = int(floor((viaL - 2 * vId.viaEnc + vId.viaSpace + 1e-9) / (vId.viaSize + vId.viaSpace)));

	double totalArea = viaL * viaW;
	double viaArea = nrows * ncols * vId.viaSize * vId.viaSize;

	seg.cond = vId.cond * viaArea / totalArea;

	out.push_back(seg);

	return 0;
}

bool FastHenryLib::counterClockWise(Point2 A, Point2 B, Point2 C)
{
	//*********************************************************************************************************************
	// Check if points A-B-C are counterclockwise  oriented
	//*********************************************************************************************************************

	return (this->isEqual((C.y - A.y) * (B.x - A.x), (B.y - A.y) * (C.x - A.x)) or ((C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x)));
}

bool FastHenryLib::cross(Point2 A, Point2 B, Point2 C, Point2 D)
{
	bool c1 = (this->counterClockWise(A, C, D) != this->counterClockWise(B, C, D) and this->counterClockWise(A, B, C) != this->counterClockWise(A, B, D));

	return (c1 or this->ConAB(A, B, C) or this->ConAB(A, B, D) or this->ConAB(C, D, A) or this->ConAB(C, D, B));
}

bool FastHenryLib::ConAB(Point2 A, Point2 B, Point2 C)
{
	//*********************************************************************************************************************
	// check if C lies on A-B
	//*********************************************************************************************************************

	std::complex<double> A1(A.x, A.y);
	std::complex<double> B1(B.x, B.y);
	std::complex<double> C1(C.x, C.y);

	std::complex<double> q;

	// B1-A1!=0
	if (!this->isEqual(A, B))
	{
		q = (C1 - A1) / (B1 - A1);

		if (std::abs(std::imag(q)) < FastHenryEpsilon and std::real(q) >= 0 and std::real(q) <= 1)
		{
			return true;
		}

		else
			return false;
	}
	else
	{ // C1==A1
		if (this->isEqual(C, A))
			return true;
		else
			return false;
	}
}

bool FastHenryLib::cross(std::vector<Point2> &polygonA, std::vector<Point2> &polygonB)
{
	//*********************************************************************************************************************
	// check if polygonA and polygonB crosses (polygon is closed with array of points)
	//*********************************************************************************************************************

	//*********************************************************************************************************************
	// first and last element in polygon should be equal in order for polygon to be closed
	//*********************************************************************************************************************

	if (!this->isEqual(polygonA[0], polygonA[polygonA.size() - 1]))
	{
		polygonA.push_back(polygonA[0]);
	}

	if (!this->isEqual(polygonB[0], polygonB[polygonB.size() - 1]))
	{
		polygonB.push_back(polygonB[0]);
	}

	for (unsigned long int i = 0; i < polygonA.size() - 1; i++)
	{
		for (unsigned long int j = 0; j < polygonB.size() - 1; j++)
		{
			if (cross(polygonA[i], polygonA[i + 1], polygonB[j], polygonB[j + 1]))
				return true;
		}
	}

	return false;
}

int FastHenryLib::segment2Polygon(std::vector<Point2> &out, const Segment &s)
{
	//*********************************************************************************************************************
	// convert segment to polygon in xy plane
	//*********************************************************************************************************************
	out.clear();
	out.reserve(5);

	std::complex<double> begin(s.start.x, s.start.y);
	std::complex<double> end(s.end.x, s.end.y);

	// rotation angle
	double theta = std::arg(end - begin);
	// rotation vector
	std::complex<double> r = std::exp(theta * std::complex<double>(0, 1));
	// length
	double l = std::abs(end - begin);

	// construct points so that begin is in (0,0) and length is horizontal

	std::complex<double> A(0, 0.5 * s.w);
	std::complex<double> B(0, -0.5 * s.w);
	std::complex<double> C = B + l;
	std::complex<double> D = A + l;

	// rotate and move with begin vector
	A = A * r + begin;
	B = B * r + begin;
	C = C * r + begin;
	D = D * r + begin;

	// add to array

	out.push_back(Point2(A.real(), A.imag()));
	out.push_back(Point2(B.real(), B.imag()));
	out.push_back(Point2(C.real(), C.imag()));
	out.push_back(Point2(D.real(), D.imag()));
	out.push_back(Point2(A.real(), A.imag()));

	return 0;
}

bool FastHenryLib::cross(const Segment &A, const Segment &B)
{
	//*********************************************************************************************************************
	// check if A and B cross
	//*********************************************************************************************************************
	// A is metal line A.star.z==A.end.z
	if (this->isEqual(A.start.z, A.end.z))
	{
		// B is metal line B.start.z==B.end.z
		if (this->isEqual(B.start.z, B.end.z))
		{
			// A and B are the same metal layers B.start.z==A.start.z
			if (this->isEqual(A.start.z, B.start.z))
			{
				std::vector<Point2> polyA;
				std::vector<Point2> polyB;

				this->segment2Polygon(polyA, A);
				this->segment2Polygon(polyB, B);

				return (this->cross(polyA, polyB));
			}
			else
			{
				return false;
			} // else
		} // if
		else // B is via
		{
			if (this->isEqual(B.start.z, A.start.z) or this->isEqual(B.end.z, A.start.z)) // B.start.z==A.start.z or B.end.z==A.start.z
			{
				std::vector<Point2> polyA;
				std::vector<Point2> polyB;
				this->segment2Polygon(polyA, A);

				polyB.clear();
				polyB.reserve(5);

				double x = B.start.x;
				double y = B.start.y;
				double w = 0.5 * B.w;
				double h = 0.5 * B.h;

				polyB.push_back(Point2(x - w, y - h));
				polyB.push_back(Point2(x + w, y - h));
				polyB.push_back(Point2(x + w, y + h));
				polyB.push_back(Point2(x - w, y + h));
				polyB.push_back(Point2(x - w, y - h));

				return (this->cross(polyA, polyB));
			}
			else
			{
				return false;
			}
		}

	} // if
	else // A is via
	{
		// B is metal line
		if (this->isEqual(B.start.z, B.end.z)) // B.start.z==B.end.z
		{
			std::vector<Point2> polyA;
			std::vector<Point2> polyB;
			this->segment2Polygon(polyB, B);

			polyA.clear();
			polyA.reserve(5);

			double x = A.start.x;
			double y = A.start.y;
			double w = 0.5 * A.w;
			double h = 0.5 * A.h;

			polyA.push_back(Point2(x - w, y - h));
			polyA.push_back(Point2(x + w, y - h));
			polyA.push_back(Point2(x + w, y + h));
			polyA.push_back(Point2(x - w, y + h));
			polyA.push_back(Point2(x - w, y - h));

			return (this->cross(polyA, polyB));
		}
		else // B is vua
		{
			return false;
		}
	}

} // cross

int FastHenryLib::cropSegment(std::vector<Segment> &out, Segment A, Point2 lowerLeft, Point2 upperRight, std::string metalName)
{
	//*********************************************************************************************************************
	// Cut segment A with rectangle lowerLeft upperRight.
	// Array out is not clean in the begining so you can use function to add to existing array.
	// If segment can not be cut, it will be added to array out
	//*********************************************************************************************************************

	LayerMet metalId = this->t.getMet(metalName);
	double z1 = metalId.dSub + 0.5 * metalId.metT;
	double cond = metalId.cond;

	// check if segment is via or it is not in metalName or segment is not horizontal or vertical
	if (!this->isEqual(A.start.z, A.end.z) or !this->isEqual(A.start.z, z1) or (!this->isEqual(A.start.x, A.end.x) and !this->isEqual(A.start.y, A.end.y))) // A.start.z!=A.end.z or A.start.z!=z1 or (A.start.x!=A.end.x and A.start.y!=A.end.y)
	{
		out.push_back(A);
		return 1;
	}

	// lower left and upper right for cutting rectangle coordinate
	Point2 ll, ur;

	ll.x = lowerLeft.x < upperRight.x ? lowerLeft.x : upperRight.x;
	ll.y = lowerLeft.y < upperRight.y ? lowerLeft.y : upperRight.y;

	ur.x = lowerLeft.x >= upperRight.x ? lowerLeft.x : upperRight.x;
	ur.y = lowerLeft.y >= upperRight.y ? lowerLeft.y : upperRight.y;

	// vertical segment start.x=end.x
	if (this->isEqual(A.start.x, A.end.x)) // A.start.x==A.end.x
	{
		Point2 segLowerL, segUpperR;

		segLowerL.x = A.start.x - 0.5 * A.w;
		segLowerL.y = A.start.y < A.end.y ? A.start.y : A.end.y;

		segUpperR.x = A.start.x + 0.5 * A.w;
		segUpperR.y = A.start.y >= A.end.y ? A.start.y : A.end.y;

		// Segment is fully covered with rectangle
		if ((ll.x <= segLowerL.x and ur.x >= segUpperR.x) and (ll.y <= segLowerL.y and ur.y >= segUpperR.y))
		{
			return 0;
		}
		else if ((ll.x <= segLowerL.x and ur.x >= segUpperR.x) and (ll.y <= segLowerL.y and ur.y < segUpperR.y and ur.y > segLowerL.y)) // lower cut of rectangle
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.x = A.start.x;
			tmp.end.x = A.end.x;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.y = ur.y;
			tmp.end.y = segUpperR.y;

			out.push_back(tmp);

			return 0;
		}
		else if ((ll.x <= segLowerL.x and ur.x >= segUpperR.x) and (ll.y > segLowerL.y and ll.y < segUpperR.y and ur.y >= segUpperR.y)) // upper cut of rectangle
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.x = A.start.x;
			tmp.end.x = A.end.x;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.y = segLowerL.y;
			tmp.end.y = ll.y;

			out.push_back(tmp);

			return 0;
		}
		else if ((ll.x <= segLowerL.x and ur.x >= segUpperR.x) and (ll.y > segLowerL.y and ur.y < segUpperR.y)) // cut in the middle of rectangle
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.x = A.start.x;
			tmp.end.x = A.end.x;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.y = segLowerL.y;
			tmp.end.y = ll.y;

			out.push_back(tmp);

			tmp.start.y = ur.y;
			tmp.end.y = segUpperR.y;

			out.push_back(tmp);

			return 0;
		}
		else // irregular cut, segment is not changed
		{
			out.push_back(A);

			return 1;
		}

	} // if vertical
	else if (this->isEqual(A.start.y, A.end.y)) // horizontal segment A.start.y==A.end.y
	{
		Point2 segLowerL, segUpperR;

		segLowerL.x = A.start.x < A.end.x ? A.start.x : A.end.x;
		segLowerL.y = A.start.y - 0.5 * A.w;

		segUpperR.x = A.start.x >= A.end.x ? A.start.x : A.end.x;
		segUpperR.y = A.start.y + 0.5 * A.w;

		// Segment is fully covered with rectangle
		if ((ll.x <= segLowerL.x and ur.x >= segUpperR.x) and (ll.y <= segLowerL.y and ur.y >= segUpperR.y))
		{
			return 0;
		}
		else if ((ll.x <= segLowerL.x and ur.x < segUpperR.x and ur.x > segLowerL.x) and (ll.y <= segLowerL.y and ur.y >= segUpperR.y))
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.y = A.start.y;
			tmp.end.y = A.end.y;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.x = ur.x;
			tmp.end.x = segUpperR.x;

			out.push_back(tmp);

			return 0;
		}
		else if ((ll.x > segLowerL.x and ll.x < segUpperR.x and ur.x > segUpperR.x) and (ll.y <= segLowerL.y and ur.y >= segUpperR.y))
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.y = A.start.y;
			tmp.end.y = A.end.y;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.x = segLowerL.x;
			tmp.end.x = ll.x;

			out.push_back(tmp);

			return 0;
		}
		else if ((ll.x > segLowerL.x and ur.x < segUpperR.x) and (ll.y <= segLowerL.y and ur.y >= segUpperR.y))
		{
			Segment tmp;

			tmp.cond = cond;

			tmp.start.z = z1;
			tmp.end.z = z1;

			tmp.start.y = A.start.y;
			tmp.end.y = A.end.y;

			tmp.w = A.w;
			tmp.h = A.h;

			tmp.start.x = segLowerL.x;
			tmp.end.x = ll.x;

			out.push_back(tmp);

			tmp.start.x = ur.x;
			tmp.end.x = segUpperR.x;

			out.push_back(tmp);

			return 0;
		}
		else
		{
			out.push_back(A);

			return 1;
		}

	} // if horizontal
	else // irregular cut, segment is not changed
	{
		out.push_back(A);

		return 1;
	}

	return 0;
}

int FastHenryLib::crop(std::vector<Segment> &out, std::vector<Segment> &in, Point2 lowerLeft, Point2 upperRight, std::string metalName)
{
	//*********************************************************************************************************************
	// Cut segment array in with rectangle lowerLeft upperRight.
	// Array out is not clean in the begining so you can use function to add to existing array.
	// If segment can not be cut, it will be added to array out
	//*********************************************************************************************************************

	out.reserve(out.size() + in.size());

	for (long unsigned int i = 0; i < in.size(); i++)
	{
		this->cropSegment(out, in[i], lowerLeft, upperRight, metalName);
	}

	in.clear();

	return 0;
}

double FastHenryLib::k(double d)
{
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
	double const sqrt2 = 1.41421356237;

	return (sqrt2 - 1) * 0.5 * d;
}

int FastHenryLib::bridge45(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string layerName)
{
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

	// layer
	LayerMet met = this->t.getMet(layerName);

	Point2 pright(0, 0);
	Point2 pleft(0, 0);

	Point2 pup(0, 0);
	Point2 pdown(0, 0);

	double e = 0;
	double b = 0;
	double w = 0;

	if (this->isEqual(p1.x, p2.x) and this->isEqual(p3.x, p4.x) and this->isEqual(abs(p1.y - p2.y), abs(p3.y - p4.y))) // case 1 from the picture
	//(p1.x==p2.x and p3.x==p4.x) and (abs(p1.y-p2.y) == abs(p3.y-p4.y))
	{

		if (p1.x <= p3.x)
		{
			pleft = Point2(p1.x, 0.5 * (p1.y + p2.y));
			pright = Point2(p3.x, 0.5 * (p3.y + p4.y));
		}

		else
		{
			pright = Point2(p1.x, 0.5 * (p1.y + p2.y));
			pleft = Point2(p3.x, 0.5 * (p3.y + p4.y));
		}

		// Conductor dimensions
		w = abs(p1.y - p2.y);
		b = abs(pright.x - pleft.x);
		e = abs(pright.y - pleft.y);

		if (b <= e + w / (1 + sqrt(2)))
		{
			// Error: soft error
			throw SoftError("Warning: <FastHenryLib::bridge45> : Invalid geometry, case 1! ");
		}

		Point2 pright_shift = Point2(pright.x - 0.5 * (b - e), pright.y);
		Point2 pleft_shift = Point2(pleft.x + 0.5 * (b - e), pleft.y);

		std::vector<Point2> pointList;
		pointList.clear();

		pointList.reserve(4);

		pointList.push_back(pleft);
		pointList.push_back(pleft_shift);
		pointList.push_back(pright_shift);
		pointList.push_back(pright);

		this->path(result, pointList, w, layerName);

		pointList.clear();
	}

	else if (this->isEqual(p1.y, p2.y) and this->isEqual(p3.y, p4.y) and this->isEqual(abs(p1.x - p2.x), abs(p3.x - p4.x))) // case 2 from the picture
																															//(p1.y==p2.y and p3.y==p4.y) and (abs(p1.x-p2.x) == abs(p3.x-p4.x))
	{

		if (p1.y >= p3.y)
		{
			pup = Point2(0.5 * (p1.x + p2.x), p1.y);
			pdown = Point2(0.5 * (p3.x + p4.x), p3.y);
		}
		else
		{
			pdown = Point2(0.5 * (p1.x + p2.x), p1.y);
			pup = Point2(0.5 * (p3.x + p4.x), p3.y);
		}

		// Conductor dimensions
		w = abs(p1.x - p2.x);
		b = abs(pup.y - pdown.y);
		e = abs(pup.x - pdown.x);

		if (b <= e + w / (1 + sqrt(2)))
		{
			// Error: soft error
			throw SoftError("Warning: <FastHenryLib::bridge45> : Invalid geometry, case 2!");
		}

		Point2 pup_shift(pup.x, pup.y - 0.5 * (b - e));
		Point2 pdown_shift(pdown.x, pdown.y + 0.5 * (b - e));

		std::vector<Point2> pointList;

		pointList.clear();

		pointList.reserve(4);

		pointList.push_back(pup);
		pointList.push_back(pup_shift);
		pointList.push_back(pdown_shift);
		pointList.push_back(pdown);

		this->path(result, pointList, w, layerName);

		pointList.clear();
	}

	else
	{
		// Error: soft error
		throw SoftError("Warning: <FastHenryLib::bridge45> : Invalid geometry, case X!");
	}

	return 0;
}

int FastHenryLib::bridge45x2(std::vector<Segment> &result1, std::vector<Segment> &result2, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string layerName1, string layerName2)
{
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # thise two planes are connected with Bridge45
	// #####################################################################################################
	// # 			p1
	// #    ********				******************
	// #      1   * ----------------*--4--------------------
	// #	********  				******************		|
	// #		   |p2  									| e
	// #           |               p3 						|
	// #   ********|               *******************		|
	// #       3  *<-------------->* 	2	|w			----
	// #   ********       b       *******************
	// #                          p4
	// #####################################################################################################
	// # Bridge45x2 will conect segments 1 to 2 with layerName1 and 3 to 4 with layer layerName2
	// # result1 and result2 are not cleaned in this function
	// #####################################################################################################

	this->bridge45(result1, p1, p2, p3, p4, layerName1);

	Point2 pp1, pp2, pp3, pp4;

	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horizontal connection
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
		throw SoftError("Warning: <FastHenryLib::bridge45x2> : Invalid geometry!");
	}

	this->bridge45(result2, pp1, pp2, pp3, pp4, layerName2);

	return 0;
}

int FastHenryLib::bridge45Vias(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string bottomM, string topM)
{
	// #####################################################################################################
	// # with p1 and p2 is defined one access plane and with p3 and p4 second access plane
	// # those two planes are connected with Bridge45
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

	double w = 0;
	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horizontal connection
	{
		w = abs(p1.y - p2.y);

		if (p1.x < p3.x)
		{
			this->fillVias(result, Point2(p2.x - w, p2.y), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x + w, p3.y), p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Point2(p2.x + w, p2.y), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x - w, p3.y), p4, topM, bottomM);
		}
	}

	else // vertical connection
	{
		w = abs(p1.x - p2.x);

		if (p1.y > p3.y)
		{
			this->fillVias(result, Point2(p2.x, p2.y + w), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x, p3.y - w), p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Point2(p2.x, p2.y - w), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x, p3.y + w), p4, topM, bottomM);
		}
	}

	std::vector<Segment> poly1;
	std::vector<Segment> poly2;

	poly1.clear();
	poly2.clear();

	this->bridge45x2(poly1, poly2, p1, p2, p3, p4, bottomM, topM);

	result.reserve(result.size() + poly1.size() + poly2.size());

	for (unsigned long int i = 0; i < poly1.size(); i++)
	{
		result.push_back(poly1[i]);
	}

	for (unsigned long int i = 0; i < poly2.size(); i++)
	{
		result.push_back(poly2[i]);
	}

	poly1.clear();
	poly2.clear();

	return 0;
}

int FastHenryLib::bridge45Vias2(std::vector<Segment> &result, Point2 p1, Point2 p2, Point2 p3, Point2 p4, string bottomM, string topM)
{
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

	double w = 0;
	// p1.x == p2.x

	if (this->isEqual(p1.x, p2.x)) // horizontal connection
	{
		w = abs(p1.y - p2.y);

		if (p1.x < p3.x)
		{
			this->fillVias(result, Point2(p2.x - w, p2.y), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x + w, p3.y), p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Point2(p2.x + w, p2.y), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x - w, p3.y), p4, topM, bottomM);
		}
	}

	else // vertical connection
	{
		w = abs(p1.x - p2.x);

		if (p1.y > p3.y)
		{
			this->fillVias(result, Point2(p2.x, p2.y + w), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x, p3.y - w), p4, topM, bottomM);
		}
		else
		{
			this->fillVias(result, Point2(p2.x, p2.y - w), p1, topM, bottomM);
			this->fillVias(result, Point2(p3.x, p3.y + w), p4, topM, bottomM);
		}
	}

	std::vector<Segment> poly1;
	std::vector<Segment> poly2;

	poly1.clear();
	poly2.clear();

	this->bridge45x2(poly1, poly2, p1, p2, p3, p4, topM, bottomM);

	result.reserve(result.size() + poly1.size() + poly2.size());

	for (unsigned long int i = 0; i < poly1.size(); i++)
	{
		result.push_back(poly1[i]);
	}

	for (unsigned long int i = 0; i < poly2.size(); i++)
	{
		result.push_back(poly2[i]);
	}

	poly1.clear();
	poly2.clear();

	return 0;
}

int FastHenryLib::octSegment(std::vector<Segment> &result, double d, double w, int quadrant, string layerName)
{
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

	double b = this->k(d);
	double a = this->k(d - 2 * w);

	double xs = 1;
	double ys = 1;

	if (a <= 0)
	{
		throw SoftError("Warning: <FastHenryLib::octSegment> : Invalid geometry!");
	}

	if (quadrant != 0 and quadrant != 1 and quadrant != 2 and quadrant != 3)
	{
		throw SoftError("Warning: <FastHenryLib::octSegment> : Invalid geometry!");
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

	Point2 p1(0, 0.5 * (d - w) * ys);
	Point2 p2(0.5 * (a + b) * xs, 0.5 * (d - w) * ys);
	Point2 p3(0.5 * (d - w) * xs, 0.5 * (a + b) * ys);
	Point2 p4(0.5 * (d - w) * xs, 0);

	std::vector<Point2> pointsList;
	pointsList.clear();
	pointsList.reserve(4);

	pointsList.push_back(p1);
	pointsList.push_back(p2);
	pointsList.push_back(p3);
	pointsList.push_back(p4);

	this->path(result, pointsList, w, layerName);

	pointsList.clear();

	return 0;
}

int FastHenryLib::rectSegment(std::vector<Segment> &result, double d, double w, int quadrant, string layerName)
{
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

	double b = 0.5 * d;
	double a = 0.5 * d - w;

	double xs = 1;
	double ys = 1;

	if (a <= 0)
	{
		throw SoftError("Warning: <FastHenryLib::rectSegment> : Invalid geometry!");
	}

	if (quadrant != 0 and quadrant != 1 and quadrant != 2 and quadrant != 3)
	{
		throw SoftError("Warning: <FastHenryLib::rectSegment> : Invalid geometry!");
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

	Point2 p1(0, 0.5 * (a + b) * ys);
	Point2 p2(0.5 * (a + b) * xs, 0.5 * (a + b) * ys);
	Point2 p3(0.5 * (a + b) * xs, 0);

	std::vector<Point2> pointsList;
	pointsList.clear();
	pointsList.reserve(3);

	pointsList.push_back(p1);
	pointsList.push_back(p2);
	pointsList.push_back(p3);

	this->path(result, pointsList, w, layerName);

	pointsList.clear();

	return 0;
}

int FastHenryLib::drawSegment(std::vector<Segment> &result, double d, double w, int quadrant, int geometry, string layerName)
{
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

	if (geometry == 0) // octagonal
	{

		this->octSegment(result, d, w, quadrant, layerName);
	}
	else // rectangular
	{
		this->rectSegment(result, d, w, quadrant, layerName);
	}

	return 0;
}

int FastHenryLib::rectangle(std::vector<Segment> &out, Point2 p1, Point2 p2, int direction, std::string metalName)
{
	//*********************************************************************************************************************
	// direction 0->horizontal ; 1->vertical
	//*********************************************************************************************************************
	std::vector<Point2> tmp;
	tmp.clear();

	if (direction == 0)
	{
		double y = (p1.y + p2.y) / 2;
		double w = std::abs(p1.y - p2.y);

		p1.y = y;
		p2.y = y;

		tmp.push_back(p1);
		tmp.push_back(p2);

		this->path(out, tmp, w, metalName);
	}
	else
	{
		double x = (p1.x + p2.x) / 2;
		double w = std::abs(p1.x - p2.x);

		p1.x = x;
		p2.x = x;

		tmp.push_back(p1);
		tmp.push_back(p2);

		this->path(out, tmp, w, metalName);
	}

	return 0;
}

int FastHenryLib::translate(std::vector<Segment> &result, const Point2 &shift)
{

	for (unsigned long int i = 0; i < result.size(); i++)
	{
		result[i].start.x = result[i].start.x + shift.x;
		result[i].start.y = result[i].start.y + shift.y;

		result[i].end.x = result[i].end.x + shift.x;
		result[i].end.y = result[i].end.y + shift.y;
	}

	return 0;
}