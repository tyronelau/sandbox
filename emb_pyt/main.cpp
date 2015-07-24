#include <Python.h>

#include <fstream>
#include <string>

using namespace std;

PyObject* init_python_module();
void destroy_python_module();

int main(int argc, char *argv[]) {
  // PyImport_AppendInittab("strategy", &init_python_module);

  init_python_module();

  ifstream fin(argv[1]);
  string content;
  string buf;
  while (getline(fin, buf, '\n')) {
    content += buf;
    content += "\n";
  }

  PyRun_SimpleString(content.c_str());
  
  destroy_python_module();
  return 0;
}

