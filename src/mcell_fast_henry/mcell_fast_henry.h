#ifndef _FASTHENRY_H_
#define _FASTHENRY_H_

#include <vector>
#include <string>
#include "fast_henry_lib.h"
#include "mcell_command_line.h"

class Node
{
public:
    Point3 p;
    unsigned long int num;
};

class Port
{
public:
    Node n1;
    Node n2;
};

class EqualNodes
{
public:
    unsigned long int num1;
    unsigned long int num2;
};

class FastHenry
{
public:
    // node number for segments is 2*i and 2*i+1, i is element number in array
    std::vector<Segment> seg;
    std::vector<Port> ports;
    std::vector<EqualNodes> eqNodes;

    FastHenryLib l;

    FastHenry(const char *fileName) : l(fileName) { this->clear(); }
    FastHenry() { this->clear(); }
    FastHenry(FastHenryLib ll) : l(ll) { this->clear(); }
    int clear();
    ~FastHenry() { this->clear(); }

    int addPort(Port p);
    int addPort(Point2 p1, Point2 p2, std::string p1MetalName, std::string p2MetalName);
    int createEqualNodes();
    int writeToFile(const char *fileName, Range freq, double meshWidth);
    int meshElements(unsigned int &n, double meshWidth, double edgeWidth);
    // number of mesh elements per edge width
    //  n			:number of mesh elements per edge width
    //  meshWidth	:mesh width n=round(edgeWidth/meshWidth)
    //  edgeWidth	:width of segment or hight of segment

    int addSegment(std::vector<Segment> &s);
};

#endif