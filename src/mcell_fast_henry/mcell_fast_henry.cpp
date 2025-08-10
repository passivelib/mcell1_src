#include "mcell_fast_henry.h"

#include <complex>
#include <iostream>
#include <fstream>

int FastHenry::clear()
{
	this->seg.clear();
	this->ports.clear();
	this->eqNodes.clear();

	return 0;
}

int FastHenry::addPort(Port p)
{
	p.n1.num = 0;
	p.n2.num = 0;

	int flag1 = 0;
	int flag2 = 0;

	for (unsigned int i = 0; i < this->seg.size(); i++)
	{
		if (this->l.isEqual(p.n1.p, seg[i].start))
		{
			p.n1.num = 2 * i;
			flag1 = 1;
		}
		if (this->l.isEqual(p.n1.p, seg[i].end))
		{
			p.n1.num = 2 * i + 1;
			flag1 = 1;
		}
		if (this->l.isEqual(p.n2.p, seg[i].start))
		{
			p.n2.num = 2 * i;
			flag2 = 1;
		}
		if (this->l.isEqual(p.n2.p, seg[i].end))
		{
			p.n2.num = 2 * i + 1;
			flag2 = 1;
		}
	}

	if (flag1 > 0 and flag2 > 0)
	{
		this->ports.push_back(p);
	}
	else
	{
		throw HardError("Error: <FastHenry::addPort> : Invalid port! ");
	}

	return 0;
}

int FastHenry::addPort(Point2 p1, Point2 p2, std::string p1MetalName, std::string p2MetalName)
{
	LayerMet metalId1 = this->l.t.getMet(p1MetalName);
	LayerMet metalId2 = this->l.t.getMet(p2MetalName);

	double z1 = metalId1.dSub + 0.5 * metalId1.metT;
	double z2 = metalId2.dSub + 0.5 * metalId2.metT;

	Port tmp;

	Point3 pp1;
	Point3 pp2;

	pp1.x = p1.x;
	pp1.y = p1.y;
	pp1.z = z1;

	pp2.x = p2.x;
	pp2.y = p2.y;
	pp2.z = z2;

	tmp.n1.p = pp1;
	tmp.n2.p = pp2;

	this->addPort(tmp);

	return 0;
}

int FastHenry::createEqualNodes()
{
	for (unsigned long int i = 0; i < this->seg.size() - 1; i++)
	{
		for (unsigned long int j = i + 1; j < this->seg.size(); j++)
		{
			if (this->l.cross(this->seg[i], this->seg[j]))
			{

				double startStart = this->seg[i].start.distance(this->seg[j].start);
				double startEnd = this->seg[i].start.distance(this->seg[j].end);
				double endStart = this->seg[i].end.distance(this->seg[j].start);
				double endEnd = this->seg[i].end.distance(this->seg[j].end);

				EqualNodes tmp;

				if (startStart <= startEnd and startStart <= endStart and startStart <= endEnd)
				{
					tmp.num1 = 2 * i;
					tmp.num2 = 2 * j;
					this->eqNodes.push_back(tmp);
				}
				else if (startEnd <= startStart and startEnd <= endStart and startEnd <= endEnd)
				{
					tmp.num1 = 2 * i;
					tmp.num2 = 2 * j + 1;
					this->eqNodes.push_back(tmp);
				}
				else if (endStart <= startStart and endStart <= startEnd and endStart <= endEnd)
				{
					tmp.num1 = 2 * i + 1;
					tmp.num2 = 2 * j;
					this->eqNodes.push_back(tmp);
				}
				else
				{
					tmp.num1 = 2 * i + 1;
					tmp.num2 = 2 * j + 1;
					this->eqNodes.push_back(tmp);
				}
			}
		}
	}

	return 0;
}

int FastHenry::meshElements(unsigned int &n, double meshWidth, double edgeWidth)
{ // number of mesh elements per edge width
	// n			:number of mesh elements per edge width
	// meshWidth	:mesh width n=round(edgeWidth/meshWidth)
	// edgeWidth	:width of segment or hight of segment

	if (meshWidth <= 0)
	{
		n = 1;
	}
	else
	{
		n = std::ceil(edgeWidth / meshWidth);

		if (n % 2 == 0)
		{
			n++;
		}
	}

	return 0;
}

int FastHenry::writeToFile(const char *fileName, Range freq, double meshWidth)
{

	std::ofstream outFile;
	outFile.open(fileName);
	// outFile << "**-------------PassiveLib-------------**" << std::endl;
	outFile << "**----------------MCELL---------------**" << std::endl
			<< std::endl;

	outFile << "* Default values" << std::endl;
	outFile << ".Units um" << std::endl;
	// outFile<<".Default nhinc=1 nwinc=1 sigma=5.6e7"<<std::endl<<std::endl;

	outFile << "* Nodes definition" << std::endl;
	for (unsigned long int i = 0; i < this->seg.size(); i++)
	{
		outFile << "N" << 2 * i << " " << "x=" << this->seg[i].start.x << " " << "y=" << this->seg[i].start.y << " " << "z=" << this->seg[i].start.z << std::endl;
		outFile << "N" << 2 * i + 1 << " " << "x=" << this->seg[i].end.x << " " << "y=" << this->seg[i].end.y << " " << "z=" << this->seg[i].end.z << std::endl;
	}

	outFile << std::endl
			<< "* Segments definition" << std::endl;
	for (unsigned long int i = 0; i < this->seg.size(); i++)
	{
		unsigned int nhinc = 1;
		unsigned int nwinc = 1;

		this->meshElements(nhinc, meshWidth, this->seg[i].h);
		this->meshElements(nwinc, meshWidth, this->seg[i].w);

		outFile << "E" << i << " N" << 2 * i << " N" << 2 * i + 1 << " w=" << this->seg[i].w << " h=" << this->seg[i].h << " nwinc=" << nwinc << " nhinc=" << nhinc << " sigma=" << seg[i].cond << std::endl;
	}

	outFile << std::endl
			<< "* Equivalent nodes" << std::endl;
	for (unsigned long int i = 0; i < this->eqNodes.size(); i++)
	{
		outFile << ".equiv N" << this->eqNodes[i].num1 << " N" << this->eqNodes[i].num2 << std::endl;
	}

	outFile << std::endl
			<< "* Ports definition" << std::endl;
	for (unsigned long int i = 0; i < this->ports.size(); i++)
	{
		outFile << ".external N" << this->ports[i].n1.num << " N" << this->ports[i].n2.num << std::endl;
	}

	outFile << std::endl
			<< "* Frequency sweep" << std::endl;
	// outFile<<".freq fmin=1e0 fmax=1e0 ndec=1"<<std::endl;
	if (this->l.isEqual(freq.min, 0) and this->l.isEqual(freq.max, 0))
	{
		freq.min = 1;
		freq.max = 1;
		freq.step = 1;
	}
	outFile << ".freq fmin=" << freq.min << " fmax=" << freq.max << " ndec=" << freq.step << std::endl;

	outFile << std::endl
			<< "* End of file" << std::endl;
	outFile << ".end";

	outFile.close();
	return 0;
}

int FastHenry::addSegment(std::vector<Segment> &s)
{
	this->seg.reserve(this->seg.size() + s.size());

	for (unsigned long int i = 0; i < s.size(); i++)
	{
		this->seg.push_back(s[i]);
	}

	s.clear();

	return 0;
}