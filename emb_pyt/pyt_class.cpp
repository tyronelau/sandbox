#include "pyt_class.h"

DEFINE_CLASS_METHODS_START(python_class)
DEFINE_METHOD(connect, cpp_connect)
DEFINE_METHOD(send_packet, cpp_send_packet)
DEFINE_CLASS_METHODS_END

