#ifdef __GNUC__
#pragma implementation
#endif

#include "QvVrmlFile.h"

#include <vcl_cstdio.h>
#include <vcl_string.h>
#include <vcl_vector.h>
#include <vcl_iostream.h>

#include "QvVisitor.h"
#include "QvString.h"
#include "QvInput.h"
#include "QvState.h"
#include "QvNode.h"

#include "QvGroup.h"
#include "QvSeparator.h"
#include "QvSwitch.h"

//#include <vbl/vbl_printf.h>
//#include <vbl/vbl_file.h>

struct QvVrmlFileData {
  vcl_vector<QvNode*> nodes;
  vcl_string filename_;
};

// Default ctor
QvVrmlFile::QvVrmlFile(char const* filename):
  p(new QvVrmlFileData)
{
  load(filename);
}

QvVrmlFile::~QvVrmlFile()
{
  delete p;
}

bool QvVrmlFile::load(char const* filename)
{
  p->filename_ = filename;
  FILE* fp = fopen(filename, "r");
  if (!fp) {
    vcl_cerr << "QvVrmlFile::load: Can't open [" << filename << "]\n";
    return false;
  }
  
  // make QvInput
  QvInput in;
  in.setFilePointer(fp);
  
  //TopologyHierarchyNode::DEFER_SUPERIORS = true;
  
  vcl_cerr << "VRML_IO: ";
  while (1) {
    QvNode* node = 0;
    vcl_cerr << "R";
    if (!QvNode::read(&in, node)) break;
    if (!node) break;
    QvState state;
    vcl_cerr << "B";
    node->build(&state);
    p->nodes.push_back(node);
    vcl_cerr << " ";
  }
  vcl_cerr << vcl_endl;
  vcl_cerr << "VRML_IO: Loaded " << p->nodes.size() << " topology objects\n";
  return true;
}

char const* QvVrmlFile::get_filename()
{
  return p->filename_.c_str();
}

void QvVrmlFile::traverse(QvVisitor* visitor)
{
  for(vcl_vector<QvNode*>::iterator np = p->nodes.begin(); np != p->nodes.end(); ++np)
    visitor->Visit(*np);
}

#include <vcl_vector.txx>
VCL_VECTOR_INSTANTIATE(QvNode*);

#include "QvPointSet.h"
#include "QvIndexedLineSet.h"
#include "QvIndexedFaceSet.h"

struct VrmlCentroidVisitor : public QvVisitor {
  float centroid[3];
  int n;
  int pass;

  double radius;
  
  void visit(QvVrmlFile& f) {
    pass = 0;
    centroid[0] = centroid[1] = centroid[2] = 0;
    n = 0;
    f.traverse(this);
    if (n == 0) {
      radius = 1;
      return;
    }
    
    centroid [0] *= 1.0/n;
    centroid [1] *= 1.0/n;
    centroid [2] *= 1.0/n;
    
    pass = 1;
    radius = 0;
    f.traverse(this);
    radius = sqrt(radius);
  }

  void inc(const point3D& p) {
    if (pass == 0) {
      centroid[0] += p.x;
      centroid[1] += p.y;
      centroid[2] += p.z;
      ++n;
    } else {
      double dx = p.x - centroid[0];
      double dy = p.y - centroid[1];
      double dz = p.z - centroid[2];
      double d = dx*dx+dy*dy+dz*dz;
      if (d > radius)
	radius = d;
    }
  }

  // ----
  
  bool Visit(QvPointSet* ps) {
    int n = (ps->numPoints.value == -1) ? ps->num_ : ps->numPoints.value;
    n += ps->startIndex.value;
    for(int i = ps->startIndex.value; i < n; ++i)
      inc(ps->points_[i]);
    return true; // ??
  }

  bool Visit(QvIndexedLineSet* node) {

    const point3D* vertexlist = node->vertexlist_;   // vertex data
    int numvertinds = node->numvertinds_;            // no. of vertex indices
    const int* vertindices = node->vertindices_;     // vertex index list

    for(int j = 0; j < numvertinds-1; ++j) {
      int i1 = vertindices[j];
      if (i1 != -1) inc(vertexlist[i1]);
    }
    return true; // ??
  }

  bool Visit(QvIndexedFaceSet* node) {
    for(int i = 0; i < node->numvertinds_; ++i) {
      int vert_index = node->vertindices_[i];
      if (vert_index != -1) inc(node->vertexlist_[vert_index]);
    }
    return true; // ??
  }
};

void QvVrmlFile::compute_centroid_radius()
{
  VrmlCentroidVisitor vcv;
  vcv.visit(*this);
  centroid[0] = vcv.centroid[0];
  centroid[1] = vcv.centroid[1];
  centroid[2] = vcv.centroid[2];
  radius = vcv.radius;
  vcl_cerr << "QvVrmlFile::compute_centroid_radius: c = " << 
    centroid[0] << " " << centroid[1] << " " << centroid[2] << 
    ", r = " << radius << vcl_endl;
}
