// This is brl/bpro/bprb/bprb_process.h
#ifndef bprb_test_process_h_
#define bprb_test_process_h_
//--------------------------------------------------------------------------------
//:
// \file
// \brief A test process
//
//--------------------------------------------------------------------------------

#include <vcl_vector.h>
#include <vcl_string.h>
#include <vbl/vbl_ref_count.h>
#include <brdb/brdb_database.h>
#include <bprb/bprb_process.h>

class bprb_test_process : public bprb_process
{
 public:
  bprb_test_process();
  virtual ~bprb_test_process(){}
  //: Clone the process
  virtual bprb_process* clone() const {return new bprb_test_process(*this);}
  
  //: The name of the process
  virtual vcl_string name(){ return "Test Process";}

  //: The name and type of each input, <name, value type>
  virtual vcl_vector<vcl_pair<vcl_string, vcl_string> > inputs();

  //: The name and type of each output, <name, value type>
  virtual vcl_vector<vcl_pair<vcl_string, vcl_string> > outputs();


  //: Perform any initialization required by the process
  virtual bool init() {return true;}

  //: Execute the process 
  virtual bool execute();

  //: Perform any clean up or final computation 
  virtual bool finish() {return true;}
  
 protected:
  
  //: Copy Constructor
  //  bprb_test_process(const bprb_test_process& other);
 private:
  //: The parameters of this process
  bprb_parameters_sptr parameters_;
  //: The database of inputs and outputs for the process
  brdb_database_sptr db_;
};

#endif // bprb_test_process_h_
