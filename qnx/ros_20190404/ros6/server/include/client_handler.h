#include <sys/time.h>
#include "global_server_variables.h"
void *control_handler(struct ControlProgram *control_program);
struct ControlProgram *control_init();
struct ControlProgram* find_reg_cp_by_rchan(int rad, int chan);
struct ControlPRM cp_fill_params(struct ControlProgram *ctrl_prog);
