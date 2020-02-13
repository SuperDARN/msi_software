#include "global_server_variables.h"

void *rx_rxfe_settings(void *arg);
void *rx_end_cp(struct ControlProgram *arg);
void *rx_ready_cp(struct ControlProgram *arg);
void *rx_pretrigger(void *arg);
void *rx_posttrigger(void *arg);
void *rx_cp_get_data(struct ControlProgram *arg);
void *rx_assign_freq(struct ControlProgram *arg);
void *rx_clrfreq(struct ControlProgram *arg);

