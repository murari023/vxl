/*
  fsm@robots.ox.ac.uk
*/
#include <vcl_cassert.h>
#include <vcl_iostream.h>
#include <vbl/vbl_arg.h>
#include <vil/vil_load.h>
#include <vil/vil_byte.h>
#include <vil/vil_memory_image_of.h>

#include <osl/osl_easy_canny.h>
#include <osl/osl_save_topology.h>

// runs canny on the given input image and outputs
// the segmentation to the given output file.

int main(int argc, char **argv) {
  //cerr << "this is " __FILE__ << endl;
  vbl_arg<int>        canny("-canny", "which canny? (0:oxford, 1:rothwell1, 2:rothwell2)", 0);
  vbl_arg<vcl_string> in   ("-in", "input image", "");
  vbl_arg<vcl_string> out  ("-out", "output file (default is stdout)", "");
  vbl_arg_parse(argc, argv);

  vcl_string* in_file = new vcl_string(in());
  if (*in_file == "") {
    vcl_cout << "input image file: ";
    char tmp[1024];
    vcl_cin >> tmp;
    delete in_file;
    in_file = new vcl_string(tmp);
  }
  assert(*in_file != "");

  vil_image image = vil_load(in_file->c_str());
  if (!image)
    return 1;
  vcl_cerr << in_file << " : " << image << vcl_endl;

  vcl_list<osl_Edge*> edges;
  osl_easy_canny(canny(), image, &edges);
  
  if (out() == "")
    osl_save_topology(vcl_cout, edges, vcl_list<osl_Vertex*>());
  else
    osl_save_topology(out().c_str(), edges, vcl_list<osl_Vertex*>());
  //   for (vcl_list<osl_Edge*>::iterator i=edges.begin(); i!=edges.end(); ++i) {
  //     cerr << "# edge " << (void*) (*i) << endl;
  //     osl_EdgelChain const &c = **i;
  //     cout << c.size() << endl;
  //     for (unsigned j=0; j<c.size(); ++j)
  //       cout << c.GetX(j) << ' ' << c.GetY(j) << endl;
  //     //cout << "-1 -1" << endl;
  //     cout << endl;
  //     (*i)->unref();
  //   }
  
  return 0;
}
